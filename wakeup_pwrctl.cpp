#include "mbed.h"
#include "wakeup.h"

/* Power-down wake-up interrupt handler */
void PWRWU_IRQHandler(void)
{
    CLK->PWRCTL |= CLK_PWRCTL_PDWKIF_Msk;
    
    wakeup_eventflags.set(EventFlag_Wakeup_UnID);
}

void config_pwrctl(void)
{
    SYS_UnlockReg();
    CLK->PWRCTL |= CLK_PWRCTL_PDWKIEN_Msk;
    SYS_LockReg();
    /* NOTE: The name of symbol PWRWU_IRQHandler is mangled in C++ and cannot override that in startup file in C.
     *       So the NVIC_SetVector call cannot be left out. */
    NVIC_SetVector(PWRWU_IRQn, (uint32_t) PWRWU_IRQHandler);
    NVIC_EnableIRQ(PWRWU_IRQn);
}
