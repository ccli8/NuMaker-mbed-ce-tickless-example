#include "mbed.h"
#include <limits.h>
#include <vector>

#include "wakeup.h"

static void flush_stdio_uart_fifo(void);
static void check_wakeup_source(uint32_t, bool deepsleep);
#if (! defined(MBED_TICKLESS))
static void idle_hdlr(void);
#endif

EventFlags wakeup_eventflags;

int main() {
#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\r\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif
    config_pwrctl();
    config_button_wakeup();
    config_wdt_wakeup();
    config_rtc_wakeup();
    /* TODO */
    //config_uart_wakeup();
    //config_i2c_wakeup();
    
#if defined(MBED_TICKLESS)
    /* Run Mbed OS internal idle handler */
#else
    /* Register idle handler which supports tickless */
    rtos::Kernel::attach_idle_hook(idle_hdlr);
#endif

    while (true) {
        
        printf("I am going to shallow/deep sleep\n");
        /* Flush STDIO UART before entering Idle/Power-down mode */
        fflush(stdout);
        flush_stdio_uart_fifo();
        
        /* Wait for any wake-up event */
        uint32_t flags = wakeup_eventflags.wait_any(EventFlag_Wakeup_All, osWaitForever, true);
        if (flags & osFlagsError) {
            if (flags != osFlagsErrorTimeout) {
                printf("OS error code: 0x%08lX\n", flags);
            }
            continue;
        }

        /* EventFlag_Wakeup_UnID is set in PWRWU_IRQHandler to indicate unidentified wake-up from power-down.
         * It could be deferred. Wait for it. */
        uint32_t flags2 = wakeup_eventflags.wait_any(EventFlag_Wakeup_UnID, 100, true);
        if (flags2 & osFlagsError) {
            if (flags2 != osFlagsErrorTimeout) {
                printf("OS error code: 0x%08lX\n", flags2);
            }
            else {
                flags2 = 0;
            }
        }
        flags |= flags2;
        
        /* EventFlag_Wakeup_UnID means unidentified wake-up source from power-down.
         *
         * If EventFlag_Wakeup_UnID is set, we wake up from power-down mode (deep sleep) but its wake-up
         * source is not yet identified. Wait for it to be identified if no other wake-up sources have been 
         * identified.
         * 
         * If EventFlag_Wakeup_UnID is not set, we wake up from idle mode (shallow sleep). Wake-up source
         * would have been identified.
         */
        if (flags == EventFlag_Wakeup_UnID) {
            uint32_t flags3 = wakeup_eventflags.wait_any(EventFlag_Wakeup_All & ~EventFlag_Wakeup_UnID, 100, true);
            if (flags3 & osFlagsError) {
                if (flags3 != osFlagsErrorTimeout) {
                    printf("OS error code: 0x%08lX\n", flags3);
                }
                else {
                    flags3 = 0;
                }
            }
            flags |= flags3;
        }
        
        /* Clear wake-up flags caused by short time EventFlags::wait_any() above. These wake-up flags are 
         * just for program control and not actual wake-up events we want to track. This has a side effect of
         * losing actual wake-up events in this short time. */
        wakeup_eventflags.clear();
        
        bool deepsleep = (flags & EventFlag_Wakeup_UnID) ? true : false;

        /* Remove EventFlag_Wakeup_UnID if any other wake-up source is identified */
        if ((flags & ~EventFlag_Wakeup_UnID)) {
            flags &= ~EventFlag_Wakeup_UnID;
        }
        check_wakeup_source(flags, deepsleep);
        
        printf("\n");
    }
    
    return 0;
}

void flush_stdio_uart_fifo(void)
{
    UART_T *uart_base = (UART_T *) NU_MODBASE(STDIO_UART);
    
    while (! UART_IS_TX_EMPTY(uart_base));
}

void check_wakeup_source(uint32_t flags, bool deepsleep)
{
    typedef std::pair<uint32_t, const char *> WakeupName;
    
    static const WakeupName wakeup_name_arr[] = {
        WakeupName(EventFlag_Wakeup_Button1, "Button1"),
        WakeupName(EventFlag_Wakeup_Button2, "Button2"),
        WakeupName(EventFlag_Wakeup_LPTicker, "lp_ticker"),
        WakeupName(EventFlag_Wakeup_WDT_Timeout, "WDT timeout"),
        WakeupName(EventFlag_Wakeup_RTC_Alarm, "RTC alarm"),
        WakeupName(EventFlag_Wakeup_UART_CTS, "UART CTS"),
        WakeupName(EventFlag_Wakeup_I2C_AddrMatch, "I2C address match"),
        
        WakeupName(EventFlag_Wakeup_UnID, "Unidentified"),
    };
    
    const char *sleep_mode = deepsleep ? "deep sleep" : "shallow sleep";
    
    if (flags) {
        const WakeupName *wakeup_name_pos = wakeup_name_arr;
        const WakeupName *wakeup_name_end = wakeup_name_arr + (sizeof (wakeup_name_arr) / sizeof (wakeup_name_arr[0]));
        
        for (; wakeup_name_pos != wakeup_name_end; wakeup_name_pos ++) {
            if (flags & wakeup_name_pos->first) {
                printf("Wake up by %s from %s\n", wakeup_name_pos->second, sleep_mode);
            }
        }
    }
}

#if (! defined(MBED_TICKLESS))

#define US_PER_SEC              (1000 * 1000)
#define US_PER_OS_TICK          (US_PER_SEC / OS_TICK_FREQ)

void dummy_cb(void) {}

void idle_hdlr(void) {
    
    const int max_us_sleep = (INT_MAX / OS_TICK_FREQ) * OS_TICK_FREQ; 
    /* Keep track of the time asleep */
    LowPowerTimer asleep_watch;
    /* Will wake up the system (by lp_ticker) if no other wake-up event */
    LowPowerTimeout alarm_clock;

    /* Suspend the system */
    uint32_t ticks_to_sleep = osKernelSuspend();
    uint32_t elapsed_ticks = 0;

    if (ticks_to_sleep) {
        uint64_t us_to_sleep = ticks_to_sleep * US_PER_OS_TICK;

        if (us_to_sleep > max_us_sleep) { 
            us_to_sleep = max_us_sleep;
        }

        /* Start the asleep_watch and setup the alarm_clock to wake up the system in us_to_sleep */
        asleep_watch.start();
        alarm_clock.attach_us(dummy_cb, us_to_sleep);

        /* Go to deep sleep */
        hal_deepsleep();
            
        /* Woken up by lp_ticker or other wake-up event */
        int us_asleep = asleep_watch.read_us();

        /* Clean up asleep_watch and alarm_clock */
        asleep_watch.stop();
        asleep_watch.reset();
        alarm_clock.detach();

        /* Translate us_asleep into ticks */
        elapsed_ticks = us_asleep / US_PER_OS_TICK;
    }

    /* Resume the system */
    osKernelResume(elapsed_ticks);
}

#endif
