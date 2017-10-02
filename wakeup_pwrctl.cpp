#include "mbed.h"
#include "wakeup.h"


#if defined(TARGET_NUMAKER_PFM_NANO130)
/* Power-down wake-up interrupt handler */
/* This target doesn't support relocating vector table and requires overriding 
 * vector handler at link-time. */
extern "C" void PDWU_IRQHandler(void)
{
    CLK->WK_INTSTS = CLK_WK_INTSTS_PD_WK_IS_Msk;
    
    wakeup_eventflags.set(EventFlag_Wakeup_UnID);
}

void config_pwrctl(void)
{
    SYS_UnlockReg();
    CLK->PWRCTL |= CLK_PWRCTL_PD_WK_IE_Msk;
    SYS_LockReg();
    /* NOTE: The name of symbol PDWU_IRQHandler is mangled in C++ and cannot override that in startup file in C.
     *       So the NVIC_SetVector call cannot be left out. */
    NVIC_SetVector(PDWU_IRQn, (uint32_t) PDWU_IRQHandler);
    NVIC_EnableIRQ(PDWU_IRQn);
}

#else

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

#endif
