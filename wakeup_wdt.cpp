#include "mbed.h"
#include "wakeup.h"

void WDT_IRQHandler(void)
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
    CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);

    SYS_UnlockReg();
    /* Alarm every 2^14 LIRC clocks, disable system reset, enable system wake-up */
    WDT_Open(WDT_TIMEOUT_2POW14, 0, FALSE, TRUE);
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
