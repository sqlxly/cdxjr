/* 抛弃通道控制模块
1、引入抛弃模块头文件,user_FSmachine.c 在 production_param.h 后加入：#include "discard_channel.h"
2、改动
    if (tickState.tickEncoderAdd > 50000)
    {
        tickState.tickEncoderAdd -= 40000;
        discard_channel_encoder_rebase(-40000);
    }
3、 当前主轨实时位置：已归一化的整杯累计量 + 当前杯内实时脉冲。 
在 packag_channel_run_ctrol() 内、eggs_cup_close_detect() 后、函数结束 } 前加入：
    discard_channel_process(tickState.tickEncoderAdd + tickState.encoderCnt);
*/

/*
 * 抛弃通道控制模块。
 * 杯号由 eInputIO16 的下降沿推进；编码器只用于反射检测点到继电器踢蛋点的距离计算。
 */
#include "discard_channel.h"
#include "mainOrbitCacheQueue.h"  
#include "dev_config.h"
#include "mdbus_tcp_slaver.h"
#include <string.h>

/* 抛弃原因枚举 */
typedef enum
{
    DISCARD_REASON_NONE = 0,         // 正常
    DISCARD_REASON_RULE_MISMATCH,    // 分配失败
    DISCARD_REASON_KICK_FAILURE,     // 踢蛋失败
    DISCARD_REASON_POWER_LOSS        // 信息丢失
} DiscardReason;

#define DISCARD_CHANNEL_INVALID_CUP          0U     // 0 不是有效蛋杯号；有效范围是 1 至 512。
#define DISCARD_CHANNEL_TASK_QUEUE_SIZE      8U     // 单轨最多缓存 8 个
#define DISCARD_CHANNEL_EGG_ACTIVE_LEVEL     1U     // 四路反射开关均为高电平有效。
#define DISCARD_CHANNEL_MAGNET_ACTIVE_LEVEL  1U     // 四路继电器均为高电平有效。
#define DISCARD_DETECT_OFFSET                20U    // 抛弃检测点到主轨道踢蛋点的蛋杯距离（待实际测量后修改）
#define DISCARD_CHANNEL_MAX_CORRECTION 		 5U     // 最大允许误差±5个杯 
#define DISCARD_REPORT_MAX_GROUPS            14U    // 抛弃报告最多缓存 14 个

/* 每条轨的固定机械参数；四条轨独立配置，以后仅在此处修改固定值。 */
typedef struct
{
    uint16_t home_detect_cup;          // 检测位的蛋杯号，固定值 
    int32_t detect_to_magnet_pulse;    // 红外到踢蛋距离脉冲距离
    int32_t magnet_width_pulse;        // 踢蛋保持脉冲数
    uint8_t  orbit_offset;             // 检测点到踢蛋队列剔除点的间隔(末通道位置（距离剔除点）- 末通道与检测点距离)
} DiscardChannelFixedConfig;

//mainOrbitQueue.front + cacheState[ch].mask + offset + n
#define EGG_DETECT_FIFO_SIZE  (23-14)
static const DiscardChannelFixedConfig s_fixed_config[DISCARD_CHANNEL_TRACK_COUNT] =
{
    {10U, 0, 120 , EGG_DETECT_FIFO_SIZE}, 
    {10U, 0, 120 , EGG_DETECT_FIFO_SIZE}, 
    {10U, 0, 120 , EGG_DETECT_FIFO_SIZE}, 
    {10U, 0, 120 , EGG_DETECT_FIFO_SIZE}  
};

/* 一个在途任务只保存其蛋杯标签和继电器应打开的编码器绝对位置。 */
typedef struct
{
    uint16_t cup;            //有蛋的蛋杯号
    int32_t target_encoder;  //踢蛋位置
    uint16_t weigh;          //0 = unknown
    uint8_t  level;         
    uint8_t  dirty;         
    uint8_t  color;         
    uint8_t  toChannel;      //0 = unknown, 100 = 参数不符
    uint8_t  toTag;          
    uint8_t  isAct;         
    DiscardReason  discard_reason; // 抛弃原因
} DiscardChannelTask;

/* 每条轨独立保存任务队列、反射检测历史及继电器状态。 */
typedef struct
{
    DiscardChannelTask task_queue[DISCARD_CHANNEL_TASK_QUEUE_SIZE]; //鸡蛋任务队列
    uint8_t queue_front;              
    uint8_t queue_rear;               // 尾插
    uint8_t queue_count;            
    uint8_t egg_previous;             // 反射开关上次电平
    uint8_t has_last_egg;             // 是否已经记录过上一颗检测到的蛋。
    uint32_t last_egg_step;           // 上一颗检测到的蛋对应的蛋杯步进序号
    uint32_t consecutive_egg_count;   // 当前连续有蛋段的第几杯：1、2、3...；仅奇数杯软件踢
    uint8_t magnet_active;            // 继电器状态       1：踢蛋中
    int32_t magnet_close_encoder;     // 编码器达到此位置后关闭继电器
} DiscardChannelTrackRuntime;

static DiscardChannelStatus s_status; // 对外（HMI）提供的杯号、同步状态和统计
static DiscardChannelTrackRuntime s_runtime[DISCARD_CHANNEL_TRACK_COUNT]; // 四条轨各自的运行状态
static uint32_t s_unopened_count[DISCARD_CHANNEL_TRACK_COUNT][DISCARD_CHANNEL_MAX_CUPS_PER_TRACK]; // 每轨每杯鸡蛋累计检测数
static uint32_t s_kick_failure_count[DISCARD_CHANNEL_TRACK_COUNT][DISCARD_CHANNEL_MAX_CUPS_PER_TRACK];//踢蛋失败数
static uint8_t s_home_previous; // 原点开关是全局公共信号，只需保存一份历史电平
static uint8_t s_cup_step_previous; // eInputIO16 是全局公共信号，只需保存一份历史电平
static uint8_t s_home_seen; // 首次经过原点只作为本圈计数起点；第二次原点起才能确认完整一圈的蛋杯总数
static uint32_t s_steps_since_home; // 当前原点周期已累计蛋杯步进数


/*
 * track = 0..3，group = 0..13。
 * 蛋杯编号地址：
 *   轨 1: 6054..6067，轨 2: 6068..6081，轨 3: 6082..6095，轨 4: 6096..6109。
 * 踢蛋失败次数地址：
 *   轨 1: 6110..6123，轨 2: 6124..6137，轨 3: 6138..6151，轨 4: 6152..6165。
 */
#define DISCARD_REPORT_THRESHOLD_REG        6053U  /*踢蛋失败次数上报下限*/
#define DISCARD_REPORT_CUP_BASE_REG         6054U
#define DISCARD_REPORT_FAIL_BASE_REG        6110U
#define DISCARD_REPORT_GROUPS_PER_TRACK     14U

static void discard_channel_update_report_registers(void)
{
    uint16_t report_cup[DISCARD_CHANNEL_TRACK_COUNT]
                       [DISCARD_REPORT_GROUPS_PER_TRACK] = {0U};
    uint16_t report_failure[DISCARD_CHANNEL_TRACK_COUNT]
                           [DISCARD_REPORT_GROUPS_PER_TRACK] = {0U};
    uint16_t threshold;
    uint16_t cup;
    uint8_t track;

    /*6053：屏幕设定的踢蛋失败次数上报下限*/
    threshold = u_tab_registers[DISCARD_REPORT_THRESHOLD_REG];

    //完整筛选到局部 RAM 快照
    if ((threshold != 0U) &&
        (s_status.confirmed_cup_count != 0U) &&
        (s_status.confirmed_cup_count <= DISCARD_CHANNEL_MAX_CUPS_PER_TRACK))
    {
        for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; ++track)
        {
            uint8_t group = 0U;

            for (cup = 1U;(cup <= s_status.confirmed_cup_count) &&
                 (group < DISCARD_REPORT_GROUPS_PER_TRACK);
                 ++cup)
            {
                uint32_t failure_count = s_kick_failure_count[track][cup - 1U];

                if (failure_count >= (uint32_t)threshold)
                {
                    report_cup[track][group] = cup;
                    report_failure[track][group] =
                        (failure_count > 0xFFFFU) ? 0xFFFFU :
                                                      (uint16_t)failure_count;
                    ++group;
                }
            }
        }
    }
    /*
     * 杯号 6054..6109（56 个寄存器）；
     * 失败次数 6110..6165（56 个寄存器）。
     */
    memcpy(&u_tab_registers[DISCARD_REPORT_CUP_BASE_REG],
           &report_cup[0][0],
           sizeof(report_cup));
    memcpy(&u_tab_registers[DISCARD_REPORT_FAIL_BASE_REG],
           &report_failure[0][0],
           sizeof(report_failure));
} 


static uint16_t discard_channel_wrap_cup(int32_t cup, uint16_t cup_count)
{
    int32_t value;

    if ((cup_count == 0U) || // 未确认总杯数时不允许进行杯号环形计算。
        (cup_count > DISCARD_CHANNEL_MAX_CUPS_PER_TRACK))
    {
        return DISCARD_CHANNEL_INVALID_CUP;
    }

    value = (cup - 1) % (int32_t)cup_count; // 先转换成以 0 为起点的环形下标。
    if (value < 0) // 归一化处理
    {
        value += cup_count;
    }

    return (uint16_t)(value + 1); // 恢复为从 1 开始的蛋杯号。
}

/* 蛋杯沿输送方向编号递增：...->总杯数->1 */
static uint16_t discard_channel_next_cup(uint16_t cup, uint16_t cup_count)
{
    if ((cup == DISCARD_CHANNEL_INVALID_CUP) || // 未同步的无效杯号不能继续推进。
        (cup > cup_count) || 
        (cup_count == 0U) || 
        (cup_count > DISCARD_CHANNEL_MAX_CUPS_PER_TRACK))
    {
        return DISCARD_CHANNEL_INVALID_CUP;
    }

    return (cup == cup_count) ? 1U : (uint16_t)(cup + 1U); 
}

static int32_t discard_channel_cup_correction(uint16_t current_cup,
                                               uint16_t expected_cup,
                                               uint16_t cup_count)
{
    int32_t correction;

    if ((cup_count == 0U) || // 未确认总杯数
        (cup_count > DISCARD_CHANNEL_MAX_CUPS_PER_TRACK))
    {
        return 0;
    }

    correction = (int32_t)expected_cup - (int32_t)current_cup; // 例如当前 108、期望 107，普通差为-1 
 
    return correction; 
}
static uint8_t discard_channel_sync_egg_info(uint8_t track, DiscardChannelTask *task)
{
    const MainOrbitCircularQueue *q = &mainOrbitQueue;
    const DiscardChannelFixedConfig *config = &s_fixed_config[track];
    uint16_t index;
    const EggsClassifyInfo *info;
    
    // 计算队列中的有效条目数
    uint16_t count = (q->rear >= q->front) ? (q->rear - q->front) 
                                            : (MAIN_ORBIT_QUEUE_SIZE - q->front + q->rear);
    
    // 检查是否有足够的数据
    if (count <= config->orbit_offset)
    {
        // 数据不足，可能是断电导致信息丢失
        task->weigh = 0;
        task->level = 0;
        task->dirty = 0;
        task->color = 0;
        task->toChannel = 0;        // 0 表示未知/默认值
        task->toTag = 0;
        task->isAct = 0;
        task->discard_reason = DISCARD_REASON_POWER_LOSS;  // 断电导致信息丢失
        return 0;
    }
    
    // 计算目标索引：front + 本轨偏移量
    index = (q->front + config->orbit_offset) % MAIN_ORBIT_QUEUE_SIZE;
    info = &q->items[index];
    
    // 检查轨道和有效性
    if (info->toChannel[track] == 0)
    {
        // 无效条目，可能是断电或信息丢失
        task->weigh = 0;
        task->level = 0;
        task->dirty = 0;
        task->color = 0;
        task->toChannel = 0;
        task->toTag = 0;
        task->isAct = 0;
        task->discard_reason = DISCARD_REASON_POWER_LOSS;
        return 0;
    }
    
    // 同步可用信息
    task->toChannel = info->toChannel[track];
    task->toTag = info->toTag[track];
    task->isAct = info->isAct[track];
    
    // weigh/level/dirty/color 无法从 EggsClassifyInfo 获取，标记为 unknown
    task->weigh = 0;
    task->level = 0;
    task->dirty = 0;
    task->color = 0;
    
    // 判断抛弃原因
    if (task->toChannel == 100)
    {
        // 通道为 0、100 或超出有效范围，表示规则不匹配
        task->discard_reason = DISCARD_REASON_RULE_MISMATCH;
    }
    else if (info->isAct[track] == 0)
    {
        // 通道分配正常，但仍在抛弃通道检测到，可能是踢蛋失败
        task->discard_reason = DISCARD_REASON_KICK_FAILURE;
        if ((task->cup != DISCARD_CHANNEL_INVALID_CUP) &&
            (task->cup <= s_status.confirmed_cup_count))
        {
            s_kick_failure_count[track][task->cup - 1U]++;
        }
    }
    else
    {
        task->discard_reason = DISCARD_REASON_NONE;
    }
    
    return 1;
}

/* 清空指定轨道的待踢任务并关闭继电器；仅人工复位时使用，原点校准绝不可调用。 */
static void discard_channel_cancel_track(uint8_t track)
{
    DiscardChannelTrackRuntime *runtime = &s_runtime[track]; // 获取当前轨运行状态。

    runtime->queue_front = 0U; // 清除最早待执行任务下标。
    runtime->queue_rear = 0U; // 清除新任务写入下标。
    runtime->queue_count = 0U; // 复位后不允许执行旧任务。
    runtime->magnet_active = 0U; // 复位时立即关闭继电器。
    s_status.track[track].task_pending = 0U; // 同步对外待踢标志。
    s_status.track[track].magnet_active = 0U; // 同步对外继电器状态。
}

//加入队列
static uint8_t discard_channel_enqueue_task(uint8_t track, uint16_t cup, int32_t target_encoder)
{
    DiscardChannelTrackRuntime *runtime = &s_runtime[track]; // 获取当前轨的环形任务队列。

    if (runtime->queue_count >= DISCARD_CHANNEL_TASK_QUEUE_SIZE) // 队列已满时不能覆盖旧任务。
    {
        return 0U; // 由调用者记录溢出诊断。
    }

    runtime->task_queue[runtime->queue_rear].cup = cup; // 保存此任务对应的真实蛋杯号。
    runtime->task_queue[runtime->queue_rear].target_encoder = target_encoder; // 保存该蛋杯应开始踢蛋的绝对脉冲位置。
    runtime->queue_rear = (uint8_t)((runtime->queue_rear + 1U) % DISCARD_CHANNEL_TASK_QUEUE_SIZE); // 环形队列写指针前进。
    runtime->queue_count++; // 待处理任务数加一。
    s_status.track[track].task_pending = 1U; // 通知外部本轨有在途踢蛋任务。
    return 1U; // 入队成功。
}

/* 删除已启动继电器的队头任务；脉冲持续和关闭由后续逻辑单独控制。 */
static void discard_channel_dequeue_task(uint8_t track)
{
    DiscardChannelTrackRuntime *runtime = &s_runtime[track]; // 获取当前轨任务队列。

    if (runtime->queue_count == 0U) // 空队列无任务可取。
    {
        return;
    }

    runtime->queue_front = (uint8_t)((runtime->queue_front + 1U) % DISCARD_CHANNEL_TASK_QUEUE_SIZE); // 读指针移到下一任务。
    runtime->queue_count--; // 当前队头已转为正在执行。
    s_status.track[track].task_pending = (runtime->queue_count != 0U) ? 1U : 0U; // 更新对外待执行状态。
}
/*
 * 主轨累计编码器坐标整体平移时调用。
 * 当前只有 tickEncoderAdd > 50000 后减 40000 的场景，delta 为 -40000。
 *
 * 调用该函数时，target_encoder <= 50000 的待踢任务已错过踢蛋位置：
 * 不允许减去 40000 后在新坐标系补踢；仅保留 target_encoder > 50000 的有效任务。
 */
#define DISCARD_CHANNEL_ENCODER_REBASE_TRIGGER 50000
void discard_channel_encoder_rebase(int32_t delta)
{
    uint8_t track;

    if (delta == 0)
    {
        return;
    }

    for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; ++track)
    {
        DiscardChannelTrackRuntime *runtime = &s_runtime[track];
        uint8_t old_count;
        uint8_t read_index;
        uint8_t write_index;
        uint8_t count;
        uint8_t valid_count = 0U;

        /*
         * 正在踢蛋时，关闭位置应位于当前位置之后。
         * 正常情况下必然大于 50000，直接随坐标系平移。
         * 若关闭位置已过期，立即停止本轨电磁铁，避免负坐标或超长输出。
         */
        if (runtime->magnet_active != 0U)
        {
            if (runtime->magnet_close_encoder >
                DISCARD_CHANNEL_ENCODER_REBASE_TRIGGER)
            {
                runtime->magnet_close_encoder += delta;
            }
            else
            {
                runtime->magnet_active = 0U;
                s_status.track[track].magnet_active = 0U;
            }
        }

        /*
         * 压缩 FIFO：
         * - 目标大于 50000：尚在前方，减 40000 后保留；
         * - 目标小于等于 50000：错过丢弃
         */
        old_count = runtime->queue_count;
        read_index = runtime->queue_front;
        write_index = runtime->queue_front;

        for (count = 0U; count < old_count; count++)
        {
            DiscardChannelTask task = runtime->task_queue[read_index];

            read_index = (uint8_t)((read_index + 1U) %
                                   DISCARD_CHANNEL_TASK_QUEUE_SIZE);

            if (task.target_encoder <=
                DISCARD_CHANNEL_ENCODER_REBASE_TRIGGER)
            {
                continue;
            }

            task.target_encoder += delta;
            runtime->task_queue[write_index] = task;
            write_index = (uint8_t)((write_index + 1U) %
                                    DISCARD_CHANNEL_TASK_QUEUE_SIZE);
            valid_count++;
        }

        runtime->queue_rear = write_index;
        runtime->queue_count = valid_count;
        s_status.track[track].task_pending =
            (valid_count != 0U) ? 1U : 0U;
    }
}

/* 原点校准*/
static void discard_channel_apply_correction(uint8_t track, int32_t correction, uint16_t cup_count)
{
    DiscardChannelTrackRuntime *runtime = &s_runtime[track]; // 获取当前轨队列。
    uint8_t index; 
    uint8_t count; 

    if (correction == 0) // 推算杯号与原点实际杯号一致时不做任何改动。
    {
        return;
    }
	//修正当前蛋杯号
    s_status.track[track].current_detect_cup = 
        discard_channel_wrap_cup((int32_t)s_status.track[track].current_detect_cup + correction, cup_count);
    // 修正最近踢蛋蛋杯号
    s_status.track[track].last_magnet_cup = 
        (s_status.track[track].last_magnet_cup == DISCARD_CHANNEL_INVALID_CUP) ?
        DISCARD_CHANNEL_INVALID_CUP :
        discard_channel_wrap_cup((int32_t)s_status.track[track].last_magnet_cup + correction, cup_count);

    for (count = 0U; count < runtime->queue_count; count++) // 遍历所有任务。
    {
        index = (uint8_t)((runtime->queue_front + count) % DISCARD_CHANNEL_TASK_QUEUE_SIZE); // 得到当前有效任务下标
        //校正任务中的蛋杯号
        runtime->task_queue[index].cup = 
            discard_channel_wrap_cup((int32_t)runtime->task_queue[index].cup + correction, cup_count);
        // target_encoder 脉冲值不修改：该蛋杯已经在实际机械位置上，踢蛋脉冲坐标必须保持不变。
    }
}

static void discard_channel_sync_home(void)
{
    uint8_t track;
    uint32_t measured_step_count = s_steps_since_home; // 上一原点到当前原点之间累计的实际蛋杯步进数
    uint16_t measured_cup_count = (measured_step_count <= DISCARD_CHANNEL_MAX_CUPS_PER_TRACK) ?
                                  (uint16_t)measured_step_count : DISCARD_CHANNEL_INVALID_CUP; // 超过数组容量时截断
    uint8_t cup_count_valid = ((measured_cup_count != DISCARD_CHANNEL_INVALID_CUP) &&
                               (measured_cup_count <= DISCARD_CHANNEL_MAX_CUPS_PER_TRACK)) ? 1U : 0U;

    s_status.last_measured_cup_count = measured_cup_count; // 有效时保存实测总杯数；超量异常时保存 0。
		
    if (s_home_seen == 0U) //首次经过原点
    {
        s_home_seen = 1U;
        s_steps_since_home = 0U; 
			  rt_kprintf("first 0\n");
        return; 
    }

    if (cup_count_valid == 0U) //测量无效
    {
        s_status.home_period_error_count++; 
        s_steps_since_home = 0U; // 下一原点周期重新测量。
        return;
    }

    for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; track++) 
    {
        if ((s_fixed_config[track].home_detect_cup == DISCARD_CHANNEL_INVALID_CUP) ||
            (s_fixed_config[track].home_detect_cup > measured_cup_count))
        {
            s_status.home_period_error_count++; 
            s_steps_since_home = 0U;
            return;
        }
    }
	//有效测量
    s_status.confirmed_cup_count = measured_cup_count; // 原点与完整一圈步进脉冲双重确认本圈实际总杯数
		
		rt_kprintf("cup1=%d,cup2=%d\n", s_status.last_measured_cup_count,s_status.confirmed_cup_count);
		
    for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; track++) // 按各轨独立检测位杯号校验并修正。
    {
        uint16_t expected_cup = s_fixed_config[track].home_detect_cup; // 一号杯在原点时本轨检测位应有的真实杯号
        int32_t correction; // 当前推算杯号相对真实杯号的误差

        if (s_status.track[track].synchronized == 0U) // 首次需建立杯号基准
        {
            s_status.track[track].current_detect_cup = expected_cup;
            s_status.track[track].synchronized = 1U;
        }
		else
		{
			correction = discard_channel_cup_correction(s_status.track[track].current_detect_cup,
														 expected_cup,
														 measured_cup_count);
			/* 大误差：记录异常 */
			if ((correction > DISCARD_CHANNEL_MAX_CORRECTION) ||
				(correction < -(int32_t)DISCARD_CHANNEL_MAX_CORRECTION))
			{
				s_status.home_period_error_count++;
			}
			
			/* 直接应用补偿，保持同步状态有效 */
			discard_channel_apply_correction(track, correction, measured_cup_count);
		}
		
		s_status.track[track].resync_count++; // 记录一次成功的原点校验。

    }

    s_steps_since_home = 0U; // 已使用本圈步进数；开始统计下一圈。
}

static void discard_channel_advance_cup(void)
{
    uint8_t track; 
    uint16_t cup_count = s_status.confirmed_cup_count; // 使用两次原点确认实际总杯数

    if (cup_count == 0U) // 第一整圈尚未确认总杯数时不能产生可信杯号。
    {
        return;
    }

    for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; track++) // 一个主轨步进脉冲同时推进四条轨。
    {
        if (s_status.track[track].synchronized == 0U) 
        {
            continue;
        }

        s_status.track[track].current_detect_cup = 
            discard_channel_next_cup(s_status.track[track].current_detect_cup, cup_count);
        s_status.track[track].cup_step_count++; // 保存单调步进序号，用于判断是否为连续蛋杯。
    }
}


static void discard_channel_handle_egg_detect(uint8_t track, int32_t encoder_position)
{
    const DiscardChannelFixedConfig *config = &s_fixed_config[track]; // 取本轨固定距离参数。
    DiscardChannelTrackRuntime *runtime = &s_runtime[track]; // 取本轨连续蛋判断状态。
    uint16_t cup = s_status.track[track].current_detect_cup; // 此刻正位于反射检测点的蛋杯号。

    if (s_status.track[track].synchronized == 0U) // 未校准时无法确认反射信号属于哪个真实蛋杯。
    {
        s_status.track[track].ignored_before_home_count++; // 仅统计，不建立错误踢蛋任务。
        return;
    }

    s_status.track[track].egg_detect_count++; // 记录一次有效反射检测。
    if ((cup == DISCARD_CHANNEL_INVALID_CUP) || // 防止未确认总杯数或异常杯号访问统计数组。
        (cup > s_status.confirmed_cup_count))
    {
        s_status.track[track].ignored_before_home_count++;
        return;
    }

    s_unopened_count[track][cup - 1U]++; // 记录此蛋杯检测到蛋的累计次数，不论原因

    if ((runtime->has_last_egg != 0U) && // 已经检测过上一颗蛋，
        (s_status.track[track].cup_step_count == runtime->last_egg_step + 1U)) // 且本蛋与上一颗之间恰好只经过一个蛋杯步进。
    {
        runtime->consecutive_egg_count++; 
    }
    else // 上一杯无蛋
    {
        runtime->consecutive_egg_count = 1U; // 新连续段的第一杯必须由软件踢。
    }

    runtime->has_last_egg = 1U; // 本次蛋将成为下一次判断连续性的参考。
    runtime->last_egg_step = s_status.track[track].cup_step_count; // 保存本次蛋所在的步进序号。

    if ((runtime->consecutive_egg_count & 1U) == 0U) // 连续蛋偶数跳过
    {
        return; 
    }
    //建立踢蛋任务
    DiscardChannelTask *task = &runtime->task_queue[runtime->queue_rear];
    task->cup = cup;
    task->target_encoder = encoder_position + config->detect_to_magnet_pulse;

    //从主轨道队列同步鸡蛋信息
    discard_channel_sync_egg_info(track, task);

    // 入队
    if (discard_channel_enqueue_task(track, cup, task->target_encoder) == 0U)
    {
        s_status.track[track].queue_overflow_count++;//满了
        return;
    }

    // 根据抛弃原因统计
    if (task->discard_reason == DISCARD_REASON_RULE_MISMATCH)
    {
        s_status.track[track].discard_rule_mismatch_count++;
    }
    else if (task->discard_reason == DISCARD_REASON_KICK_FAILURE)
    {
        s_status.track[track].discard_kick_failure_count++;
    }
    else if (task->discard_reason == DISCARD_REASON_POWER_LOSS)
    {
        s_status.track[track].discard_power_loss_count++;
    }

    s_status.track[track].last_magnet_cup = cup;

}

/* 编码器达到任务目标后打开继电器；再经过固定编码器脉冲数后关闭继电器。 */
static void discard_channel_update_magnet(uint8_t track, int32_t encoder_position)
{
    const DiscardChannelFixedConfig *config = &s_fixed_config[track]; // 取本轨继电器脉冲宽度。
    DiscardChannelTrackRuntime *runtime = &s_runtime[track]; // 取本轨继电器和队列状态。
    DiscardChannelTask *task; // 指向最早待执行任务。

    if (runtime->magnet_active != 0U) // 当前继电器已经处于踢蛋有效状态。
    {
        if (encoder_position >= runtime->magnet_close_encoder) // 编码器已走过固定踢蛋宽度。
        {
            runtime->magnet_active = 0U; // 关闭继电器。
            s_status.track[track].magnet_active = 0U; // 同步对外继电器状态。
        }
        return; // 同一轨继电器未关闭前禁止开启下一次踢蛋。
    }

    if (runtime->queue_count == 0U) // 当前轨没有在途蛋杯
    {
        return;
    }

    task = &runtime->task_queue[runtime->queue_front];  
    if (encoder_position >= task->target_encoder)  
    {
        discard_channel_dequeue_task(track); // 先出队，避免下一次扫描重复启动同一任务。
        runtime->magnet_active = 1U; // 继电器输出高有效踢蛋脉冲。
        runtime->magnet_close_encoder = encoder_position + config->magnet_width_pulse; // 结束位置以实际开启时读取的编码器位置为基准计算。
        s_status.track[track].magnet_active = 1U; // 更新对外实时状态。
        s_status.track[track].magnet_kick_count++; // 累计一次实际软件踢蛋。
    }
}

void discard_channel_init(void)
{
    memset(&s_status, 0, sizeof(s_status)); // 清除对外状态和累计统计。
    memset(&s_runtime, 0, sizeof(s_runtime)); // 清除四条轨的任务队列、连续蛋状态和继电器状态。
    memset(&s_unopened_count, 0, sizeof(s_unopened_count)); // 清除每轨每杯的累计检测数。
    memset(&s_kick_failure_count, 0, sizeof(s_kick_failure_count)); 
    s_home_previous = 0U; // 初始化原点输入历史电平。
    s_cup_step_previous = 0U; // 初始化 eInputIO16 输入历史电平。
    s_home_seen = 0U; // 上电后等待首次原点作为完整一圈测量的起点。
    s_steps_since_home = 0U; // 清除当前原点周期的步进计数。
}
#define DISCARD_RESET_ALL_REG               6052U
#define DISCARD_SINGLE_RESET_CUP_BASE_REG   6166U
#define DISCARD_SINGLE_RESET_CONFIRM_BASE_REG 6167U
#define DISCARD_SINGLE_RESET_REG_STRIDE     2U

/*
 * 处理屏幕发出的清除命令：
 * - 6052 == 1：清除全部抛弃通道统计；
 * - 6166/6168/6170/6172：分别填写四条轨道待清除的蛋杯号；
 * - 6167/6169/6171/6173 == 1：确认清除对应轨道、对应蛋杯的累计次数。
 */
static void discard_channel_process_reset_command(void)
{
    uint8_t track;

    /* 全部复位优先级最高。执行完成后主动回写 0，形成确认应答。 */
    if (u_tab_registers[DISCARD_RESET_ALL_REG] == 1U)
    {
        discard_channel_init();
        u_tab_registers[DISCARD_RESET_ALL_REG] = 0U;

        /* 清空四轨单点命令，避免全复位后又执行残留的单点复位。 */
        for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; ++track)
        {
            u_tab_registers[DISCARD_SINGLE_RESET_CUP_BASE_REG +
                            (uint16_t)track * DISCARD_SINGLE_RESET_REG_STRIDE] = 0U;
            u_tab_registers[DISCARD_SINGLE_RESET_CONFIRM_BASE_REG +
                            (uint16_t)track * DISCARD_SINGLE_RESET_REG_STRIDE] = 0U;
        }
        return;
    }

    /* 逐轨处理单蛋杯复位。 */
    for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; ++track)
    {
        uint16_t cup_register = DISCARD_SINGLE_RESET_CUP_BASE_REG +
                                (uint16_t)track * DISCARD_SINGLE_RESET_REG_STRIDE;
        uint16_t confirm_register = DISCARD_SINGLE_RESET_CONFIRM_BASE_REG +
                                    (uint16_t)track * DISCARD_SINGLE_RESET_REG_STRIDE;
        uint16_t cup = u_tab_registers[cup_register];

        if (u_tab_registers[confirm_register] != 1U)
        {
            continue;
        }

        /*
         * 仅在当前已确认总杯数，且输入杯号属于本轨有效范围时清零。
         * 防止屏幕误填 0、超出实际产线杯数或数组容量的蛋杯号。
         */
        if ((cup != DISCARD_CHANNEL_INVALID_CUP) &&
            (cup <= s_status.confirmed_cup_count) &&
            (cup <= DISCARD_CHANNEL_MAX_CUPS_PER_TRACK))
        {
            s_unopened_count[track][cup - 1U] = 0U;
            s_kick_failure_count[track][cup - 1U] = 0U;
        }
        /*
         * 无论杯号有效与否，确认按钮均自动复位，防止按钮保持 1 后每周期重复执行。
         * 同时清掉输入杯号，通知屏幕本次命令已被消费。
         */
        u_tab_registers[cup_register] = 0U;
        u_tab_registers[confirm_register] = 0U;
    }
}

void discard_channel_reset(void)
{
    uint8_t track; // 遍历四条轨道。

    s_status.confirmed_cup_count = 0U; // 人工复位后必须重新通过完整原点周期确认实际总杯数。
    s_status.last_measured_cup_count = 0U; // 清除上一次原点周期测量结果。
    s_home_seen = 0U; // 复位后下一次原点重新作为完整一圈计数的起点。
    s_steps_since_home = 0U; // 丢弃复位前未完成一圈的步进计数。

    for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; ++track) // 显式人工复位才取消任务。
    {
        discard_channel_cancel_track(track); // 清空尚未到位的踢蛋任务并关闭继电器。
        s_runtime[track].has_last_egg = 0U; // 复位连续蛋判断基准。
        s_runtime[track].consecutive_egg_count = 0U; // 清除连续段序号。
        s_status.track[track].current_detect_cup = DISCARD_CHANNEL_INVALID_CUP; // 复位后必须重新等待原点。
        s_status.track[track].last_magnet_cup = DISCARD_CHANNEL_INVALID_CUP; // 清除最近软件任务显示。
        s_status.track[track].synchronized = 0U; // 原点前禁止创建带错误杯号的任务。
    }
}

void discard_channel_clear_statistics(void)
{
    discard_channel_init(); // 固定参数没有外部配置需要保留；完整初始化同时清除统计和运行状态。
}
static void discard_channel_write_magnet(uint8_t track, uint8_t output_level)
{
	/*
    switch (track)
    {
        case 0U:
            set_output_io_sta(masterOperate.outputData, eOutputIO43, output_level); // 轨道 1 踢蛋继电器
            break;

        case 1U:
            set_output_io_sta(masterOperate.outputData, eOutputIO44, output_level); // 轨道 2 踢蛋继电器
            break;

        case 2U:
            set_output_io_sta(masterOperate.outputData, eOutputIO45, output_level); // 轨道 3 踢蛋继电器
            break;

        case 3U:
            set_output_io_sta(masterOperate.outputData, eOutputIO46, output_level); // 轨道 4 踢蛋继电器
            break;

        default:
            break;
    }
	*/
}

void discard_channel_process(int32_t encoder_position)
{
	DiscardChannelInput input;		
	DiscardChannelOutput output;	
	
	uint8_t track;
	uint8_t home_edge;
	uint8_t cup_step_edge;
	uint8_t egg_flag = false;
	
	input.home_pulse = get_input_io_sta(masterOperate.inputData, eInputIO15);  // 一号杯原点
	input.cup_step_pulse = get_input_io_sta(masterOperate.inputData, eInputIO16); // 蛋杯步进
	input.egg_detect[0] = get_input_io_sta(masterOperate.inputData, eInputIO24); // 主轨 1 反射
	input.egg_detect[1] = get_input_io_sta(masterOperate.inputData, eInputIO17); // 主轨 2 反射
	input.egg_detect[2] = get_input_io_sta(masterOperate.inputData, eInputIO18); // 主轨 3 反射
	input.egg_detect[3] = get_input_io_sta(masterOperate.inputData, eInputIO19); // 主轨 4 反射
	input.machine_running = 1U; //masterOperate.eSysState// 需要改为实际的设备运行状态
//	rt_kprintf("%d,%d,%d,%d,%d\n", input.egg_detect[0],input.egg_detect[1],input.egg_detect[2],input.egg_detect[3],encoder_position);
    home_edge = ((s_home_previous == 1U) && (input.home_pulse == 0U)) ? 1U : 0U;  
    cup_step_edge = ((s_cup_step_previous == 1U) && (input.cup_step_pulse == 0U)) ? 1U : 0U;  
    s_home_previous = input.home_pulse;  
    s_cup_step_previous = input.cup_step_pulse;  

    if (cup_step_edge != 0U)  
    {
        s_steps_since_home++;  
    }

    if (home_edge != 0U) // 一号杯经过原点
    {
				egg_flag = true;
        discard_channel_sync_home();  //同步杯号
    }
    else if (cup_step_edge != 0U) //原点和步进同周期时原点优先 
    {
				egg_flag = true;
        discard_channel_advance_cup(); //杯号递增
    }

    for (track = 0U; track < DISCARD_CHANNEL_TRACK_COUNT; track++)  
    {
//        if ((s_runtime[track].egg_previous != DISCARD_CHANNEL_EGG_ACTIVE_LEVEL) &&  
			if ((egg_flag) &&  
            (input.egg_detect[track] == DISCARD_CHANNEL_EGG_ACTIVE_LEVEL)) //有蛋
        {
					//	rt_kprintf("egg---%d\n", track);
            discard_channel_handle_egg_detect(track, encoder_position);  
        }
        s_runtime[track].egg_previous = input.egg_detect[track]; 

        if (input.machine_running != 0U) 
        {
            discard_channel_update_magnet(track, encoder_position); //机器运行时持续检查编码器 
        }
        else 
        {
            s_runtime[track].magnet_active = 0U; 
            s_status.track[track].magnet_active = 0U; //队列保留
        }

        output.magnet_output[track] = (s_runtime[track].magnet_active != 0U) ? 
                                       DISCARD_CHANNEL_MAGNET_ACTIVE_LEVEL :
                                       (uint8_t)(DISCARD_CHANNEL_MAGNET_ACTIVE_LEVEL == 0U ? 1U : 0U); 
        //位置需要确定，目前未知                              
		discard_channel_write_magnet(track, output.magnet_output[track]);
    }
    //检测复位命令
    discard_channel_process_reset_command();
    //再加一个与HMI交互更新的函数，用于更新显示
    discard_channel_update_report_registers();
   
}

const DiscardChannelStatus *discard_channel_get_status(void)
{
    return &s_status; // 返回模块内部只读状态，供诊断或上位机查询。
}
