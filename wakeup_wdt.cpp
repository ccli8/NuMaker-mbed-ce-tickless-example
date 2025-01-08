/* Avoid conflict with Mbed OS watchdog API
 *
 * Mbed OS has defined watchdog API. To avoid conflict, disable this code when target has implemented Mbed OS watchdog API.
 */
#if !defined(DEVICE_WATCHDOG) || !DEVICE_WATCHDOG

#include "mbed.h"
#include "wakeup.h"

#if defined(TARGET_NANO100)
/* This target doesn't support relocating vector table and requires overriding 
 * vector handler at link-time. */
extern "C" void WDT_IRQHandler(void)
#else
void WDT_IRQHandler(void)
#endif
{
    /* Check WDT interrupt flag */
    if (WDT_GET_TIMEOUT_INT_FLAG()) {
        WDT_CLEAR_TIMEOUT_INT_FLAG();
        WDT_RESET_COUNTER();
    }

    // Check WDT wake-up flag
    if (WDT_GET_TIMEOUT_WAKEUP_FLAG()) {
        WDT_CLEAR_TIMEOUT_WAKEUP_FLAG();
        
        wakeup_eventflags.set(EventFlag_Wakeup_WDT_Timeout);
    }
}

void config_wdt_wakeup()
{
    /* Enable IP module clock */
    CLK_EnableModuleClock(WDT_MODULE);

    /* Select IP clock source */
#if defined(TARGET_NANO100)
    CLK_SetModuleClock(WDT_MODULE, 0, 0);
#else
    CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);
#endif

    SYS_UnlockReg();
#if defined(TARGET_M251)
    /* Alarm every 2^16 LIRC clocks, disable system reset, enable system wake-up */
    /* LIRC higher than other targets, so enlarge clock count */
    WDT_Open(WDT_TIMEOUT_2POW16, 0, FALSE, TRUE);
#else
    /* Alarm every 2^14 LIRC clocks, disable system reset, enable system wake-up */
    WDT_Open(WDT_TIMEOUT_2POW14, 0, FALSE, TRUE);
#endif
    SYS_LockReg();
    
    /* NOTE: The name of symbol WDT_IRQHandler is mangled in C++ and cannot override that in startup file in C.
     *       So the NVIC_SetVector call cannot be left out. */
    NVIC_SetVector(WDT_IRQn, (uint32_t) WDT_IRQHandler);
    NVIC_EnableIRQ(WDT_IRQn);
    SYS_UnlockReg();
    /* Enable WDT timeout interrupt */
    WDT_EnableInt();
    SYS_LockReg();
}

#else

#include "mbed.h"

void config_wdt_wakeup()
{
    printf("Disable WDT timeout wake-up on this target\n\n");
}

#endif  /* #if !defined(DEVICE_WATCHDOG) || !DEVICE_WATCHDOG */
