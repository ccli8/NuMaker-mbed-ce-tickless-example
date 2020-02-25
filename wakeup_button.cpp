#include "mbed.h"
#include "wakeup.h"

#if defined(TARGET_NUMAKER_PFM_NANO130)
// SW
#define BUTTON1     SW1
#define BUTTON2     SW2

#elif defined(TARGET_NUMAKER_PFM_NUC472)
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

#elif defined(TARGET_NUMAKER_IOT_M487)
// SW
#define BUTTON1     SW2
#define BUTTON2     SW3

#elif defined(TARGET_NUMAKER_IOT_M263A)
// SW
#define BUTTON1     SW2
#define BUTTON2     SW3

#endif

#if defined(BUTTON1) && defined(BUTTON2)

static InterruptIn button1(BUTTON1);
static InterruptIn button2(BUTTON2);
static void button1_release(void);
static void button2_release(void);

#if defined(TARGET_NUMAKER_PFM_NANO130)
static void button1_press(void);
static void button2_press(void);
#endif

void config_button_wakeup(void)
{
    button1.rise(&button1_release);
    button2.rise(&button2_release);
    
    /* KNOWN ISSUE: On NANO130 (NANO100 series), there's H/W issue with GPIO wake-up from 
     *              Power-down (deep sleep). Workaround by enabling both edge triggers. */
#if defined(TARGET_NUMAKER_PFM_NANO130)
    button1.fall(&button1_press);
    button2.fall(&button2_press);
#endif
}

void button1_release(void)
{
    wakeup_eventflags.set(EventFlag_Wakeup_Button1);
}

void button2_release(void)
{
    wakeup_eventflags.set(EventFlag_Wakeup_Button2);
}

#if defined(TARGET_NUMAKER_PFM_NANO130)
void button1_press(void)
{
    wakeup_eventflags.set(EventFlag_Wakeup_Button1);
}

void button2_press(void)
{
    wakeup_eventflags.set(EventFlag_Wakeup_Button2);
}
#endif

#else

void config_button_wakeup(void)
{
    printf("Disable button wake-up on this target\n\n");
}

#endif /* #if defined(BUTTON1) && defined(BUTTON2) */
