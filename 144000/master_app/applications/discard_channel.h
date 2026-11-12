#ifndef __DISCARD_CHANNEL_H__
#define __DISCARD_CHANNEL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DISCARD_CHANNEL_TRACK_COUNT        4U    	// 固定四条抛弃轨道。
#define DISCARD_CHANNEL_MAX_CUPS_PER_TRACK 512U  	// 允许的最大蛋杯总数，同时也是统计数组容量；实际总杯数由相邻原点间的步进脉冲确认。

/* 每次实时扫描传入的原始输入。 */
typedef struct
{
    uint8_t home_pulse;                                      // 一号杯原点 
    uint8_t cup_step_pulse;                                  // 蛋杯单个
    uint8_t egg_detect[DISCARD_CHANNEL_TRACK_COUNT];         // 四路反射,高电平有效
    uint8_t machine_running;                                 // 仅运行时允许继电器输出
} DiscardChannelInput;

/* 调用方将四路物理输出电平写到继电器。 */
typedef struct
{
    uint8_t magnet_output[DISCARD_CHANNEL_TRACK_COUNT];     // 高电平为继电器有效。
} DiscardChannelOutput;

/* 单轨诊断及生产统计；杯号均从 1 开始。 */
typedef struct
{
    uint32_t cup_step_count;                                 // 本轨已接收的蛋杯步进次数。
    uint32_t egg_detect_count;                               // 反射开关有效边沿次数。
    uint32_t magnet_kick_count;                              // 已实际发出的继电器踢蛋次数。
    uint32_t ignored_before_home_count;                      // 未经过原点校准时忽略的检测次数。
    uint32_t queue_overflow_count;                           // 同一轨待踢任务槽已占用时丢弃的检测次数。
    uint32_t resync_count;                                   // 一号杯原点重新校准次数。
    uint16_t current_detect_cup;                             // 当前蛋杯号
    uint16_t last_magnet_cup;                                // 最近一次登记软件踢蛋任务的蛋杯号。
    uint8_t synchronized;                                    // 非零表示已由原点得到有效杯号。
    uint8_t task_pending;                                    // 非零表示存在等待编码器到位的踢蛋任务。
    uint8_t magnet_active;                                   // 非零表示继电器当前正在输出踢蛋脉冲。
    uint32_t discard_rule_mismatch_count;                    // 规则不匹配抛弃次数
    uint32_t discard_kick_failure_count;                     // 踢蛋失败抛弃次数
    uint32_t discard_power_loss_count;                       // 断电信息丢失抛弃
} DiscardChannelTrackStatus;

typedef struct
{
    DiscardChannelTrackStatus track[DISCARD_CHANNEL_TRACK_COUNT];
    uint16_t confirmed_cup_count;                            // 已由完整原点周期确认的实际蛋杯总数；0 表示尚未确认。
    uint16_t last_measured_cup_count;                        // 最近一个原点周期内累计到的 eInputIO16 步进脉冲数。
    uint32_t home_period_error_count;                        // 异常次数
} DiscardChannelStatus;

/* 初始化固定参数模块并清除所有运行状态和统计；无需外部配置。 */
void discard_channel_init(void);

/* 关闭继电器、清除在途任务及原点同步状态；保留累计统计。 */
void discard_channel_reset(void);

/* 清除所有统计、运行状态、在途任务及原点同步状态。 */
void discard_channel_clear_statistics(void);

/*
 * 实时周期接口：
 * 1. 相邻两个一号杯原点下降沿之间累计的 eInputIO16 脉冲数，确认本圈实际蛋杯总数；
 * 2. eInputIO16 下降沿使检测位杯号递增，到本圈已确认的实际总杯数后回到 1；
 * 3. 一号杯原点下降沿将各轨计算杯号校验并修正至各自固定的检测位杯号；
 * 4. 反射开关高电平有效时，连续蛋杯仅软件踢第 1、3、5... 杯，间隔杯交给后方硬件兜底；
 * 5. 模块直接调用 firstPulseEncoderGet()，到达检测位置加固定距离后开继电器，
 *    再经过固定编码器脉冲数后关闭继电器。
 */
void discard_channel_process(int32_t encoder_position);
/*
 * 主轨累计编码器坐标整体重定位时调用。
 * 当前主状态机在 tickEncoderAdd > 50000 时减 40000，
 * 因此调用 discard_channel_encoder_rebase(-40000)。
 */
void discard_channel_encoder_rebase(int32_t delta);

/* 获取四条轨道的实时状态及累计统计。 */
const DiscardChannelStatus *discard_channel_get_status(void);

#ifdef __cplusplus
}
#endif

#endif
