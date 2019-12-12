#ifndef _WDT_PUB_H_
#define _WDT_PUB_H_

#define WDT_FAILURE                (1)
#define WDT_SUCCESS                (0)

#define WDT_DEV_NAME                "wdt"

#define WDT_CMD_MAGIC              (0xe330000)
enum
{
    WCMD_POWER_UP = WDT_CMD_MAGIC + 1,
	WCMD_SET_PERIOD,
	WCMD_RELOAD_PERIOD,
    WCMD_POWER_DOWN
};

/*******************************************************************************
* Function Declarations
*******************************************************************************/
extern void wdt_init(void);
extern void wdt_exit(void);

/**
 * start dog's time-out in milli-second
 * period_ms should greater than WDG_MIN_THRESHOLD
 */
void bk_wdt_start(uint32_t period_ms);

/**
 * check updated period_ms to deal with user reboot option
 */
void bk_wdt_update_period(uint32_t period_ms);

/**
 * fresh wdt in tick proc if needed
 */
void bk_wdt_tick_proc();

/**
 * stop dog's time-out
 */
void bk_wdt_stop(void);

#endif //_WDT_PUB_H_ 

