#include "include.h"
#include "arm_arch.h"

#include "wdt_pub.h"
#include "fake_clock_pub.h"

#if CFG_ENABLE_WDG

#define WDG_MIN_THRESHOLD 1000

enum wdg_status {
    WDG_STATUS_STOP,
    WDG_STATUS_REBOOT,
    WDG_STATUS_WATCH,
    WDG_STATUS_MAX,
};

struct wdg_context {
    enum wdg_status wdg_flag;
    int consumed_in_tick;
    uint32_t threshold_in_tick;
    uint32_t last_fresh_in_tick; /* add time to dealing with power save mode */
};

static struct wdg_context g_wdg_context = { WDG_STATUS_STOP, 0, 0, 0 };

extern UINT32 wdt_ctrl(UINT32 cmd, void *param);

/**
 * start dog's time-out in milli-second
 * period_ms should greater than WDG_MIN_THRESHOLD
 */
void bk_wdt_start(uint32_t period_ms)
{
    uint32_t time_ms = 10000; /* 10000 * 1ms = 10000ms = 10s */

    if (WDG_STATUS_REBOOT == g_wdg_context.wdg_flag)
    {
        return;
    }

    if (period_ms > WDG_MIN_THRESHOLD)
    {
        time_ms = period_ms;
    }

    wdt_ctrl(WCMD_SET_PERIOD, &time_ms);
    wdt_ctrl(WCMD_POWER_UP, NULL);

    g_wdg_context.threshold_in_tick = (time_ms * 3) >> 2;
    /* convert ms to tick */
#if CFG_SUPPORT_RTT
#elif CFG_SUPPORT_ALIOS
#elif CFG_OS_FREERTOS
    //g_wdg_context.threshold_in_tick = g_wdg_context.threshold_in_tick * configTICK_RATE_HZ / 1000;
    g_wdg_context.threshold_in_tick = g_wdg_context.threshold_in_tick / 2;
#endif
    g_wdg_context.consumed_in_tick = 0;
    g_wdg_context.last_fresh_in_tick = fclk_get_tick();
    bk_printf("%s start wdg time_ms=%d threshold=%d\n", __FUNCTION__, time_ms, g_wdg_context.threshold_in_tick);

    g_wdg_context.wdg_flag = WDG_STATUS_WATCH;
}

/**
 * refresh dog's time-out
 */
static void bk_wdt_refresh(void)
{
    if (WDG_STATUS_WATCH == g_wdg_context.wdg_flag)
    {
#if CFG_SUPPORT_RTT
        bk_printf("refresh wdg\n");
#elif CFG_SUPPORT_ALIOS
#elif CFG_OS_FREERTOS
        extern size_t xPortGetMinimumEverFreeHeapSize( void );
        bk_printf("wdg(%d)\n", xPortGetMinimumEverFreeHeapSize());
#endif
        wdt_ctrl(WCMD_RELOAD_PERIOD, NULL);
    }
    g_wdg_context.consumed_in_tick = 0;
    g_wdg_context.last_fresh_in_tick = fclk_get_tick();
}

void bk_wdt_update_period(uint32_t period_ms)
{
    /* wdt reboot should using small period */
    if (period_ms <= WDG_MIN_THRESHOLD)
    {
        g_wdg_context.wdg_flag = WDG_STATUS_REBOOT;
        bk_printf("%s reboot\n", __FUNCTION__);
    }
}


/**
 * stop dog's time-out
 */
void bk_wdt_stop(void)
{
    if (WDG_STATUS_REBOOT == g_wdg_context.wdg_flag)
    {
        return;
    }

    bk_printf("%s stop wdg\n", __FUNCTION__);
    wdt_ctrl(WCMD_POWER_DOWN, NULL);

    g_wdg_context.wdg_flag = WDG_STATUS_STOP;
    g_wdg_context.threshold_in_tick = 0;
}

void bk_wdt_tick_proc()
{
    if (WDG_STATUS_WATCH != g_wdg_context.wdg_flag)
    {
        return;
    }
    g_wdg_context.consumed_in_tick++;
    /**
      * normal mode: using 120M RTC, 1 tick = 1ms
      * ps mode: using 32K RTC, rt_tick is adapted
      */
    if ((g_wdg_context.consumed_in_tick >= (int)g_wdg_context.threshold_in_tick)
      || (fclk_get_tick() - g_wdg_context.last_fresh_in_tick >= g_wdg_context.threshold_in_tick))
    {
        bk_wdt_refresh();
    }
}
#endif

