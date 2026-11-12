
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>



rt_uint32_t hal_crc32(uint8_t *buf,uint16_t len)
{
    struct rt_hwcrypto_ctx *ctx;
    static struct hwcrypto_crc_cfg cfg =
		{
        .last_val = 0xFFFFFFFF,
        .poly = 0x04C11DB7,
        .width = 32,
        .xorout = 0x00000000,
        .flags = 0,
 };
    rt_uint32_t result = 0;
	  ctx = rt_hwcrypto_crc_create(rt_hwcrypto_dev_default(), HWCRYPTO_CRC_CRC32);/* ?? CRC ?? */
    rt_hwcrypto_crc_cfg(ctx, &cfg);
    result = rt_hwcrypto_crc_update(ctx, buf, len);
    rt_hwcrypto_crc_destroy(ctx);
		return result;
}

