#include "mbed.h"
#include "wakeup.h"

#if defined(TARGET_NUMAKER_PFM_NUC472)
// SW
#define BUTTON1     SW1
#define BUTTON2     SW2

#elif defined(TARGET_NUMAKER_PFM_M453)
// SW
#define BUTTON1     SW2
#define BUTTON2     SW3

#elif defined(TARGET_NUMAKER_PFM_M487)
// SW
#define BUTTON1     SW2
#define BUTTON2     SW3

#endif

static InterruptIn button1(BUTTON1);
static InterruptIn button2(BUTTON2);
static void button1_release(void);
static void button2_release(void);

void config_button_wakeup(void)
{
    button1.rise(&button1_release);
    button2.rise(&button2_release);
}

void button1_release(void)
{
    wakeup_eventflags.set(EventFlag_Wakeup_Button1);
}

void button2_release(void)
{
    wakeup_eventflags.set(EventFlag_Wakeup_Button2);
}
