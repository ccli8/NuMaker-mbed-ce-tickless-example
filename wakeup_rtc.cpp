#include "mbed.h"
#include "wakeup.h"
#include "rtc_api.h"
#include "mbed_mktime.h"

#define YEAR0       1900

static void schedule_rtc_alarm(void *secs_);
Event<void()> *schedule_rtc_alarm_event;

void RTC_IRQHandler(void)
{
    /* Check if RTC alarm interrupt has occurred */
#if defined(TARGET_NUMAKER_PFM_NUC472)
    if (RTC->INTSTS & RTC_INTSTS_ALMIF_Msk) {
        /* Clear RTC alarm interrupt flag */
        RTC->INTSTS = RTC_INTSTS_ALMIF_Msk;

        wakeup_eventflags.set(EventFlag_Wakeup_RTC_Alarm);
    }
#else
    if (RTC_GET_ALARM_INT_FLAG()) {
        /* Clear RTC alarm interrupt flag */
        RTC_CLEAR_ALARM_INT_FLAG();

        wakeup_eventflags.set(EventFlag_Wakeup_RTC_Alarm);
    }
#endif
}

void config_rtc_wakeup(void)
{
    Callback<void()> callback(&schedule_rtc_alarm, (void *) 3);
    static RtosTimer timer(callback, osTimerPeriodic);
    timer.start(4000);
}

void schedule_rtc_alarm(void *secs_)
{
    uint32_t secs = (uint32_t) secs_;
    
    /* time() will call set_time(0) internally to set timestamp if rtc is not yet enabled, where the 0 timestamp 
     * corresponds to 00:00 hours, Jan 1, 1970 UTC. But Nuvoton mcu's rtc supports calendar since 2000 and 1970 
     * is not supported. For this test, a timestamp after 2000 is explicitly set. */ 
    {
        static bool time_inited = false;
    
        if (! time_inited) {
            time_inited = true;
        
            #define CUSTOM_TIME  1256729737
            set_time(CUSTOM_TIME);  // Set RTC time to Wed, 28 Oct 2009 11:35:37
        }
    }
    
    RTC_DisableInt(RTC_INTEN_ALMIEN_Msk);
    
    time_t t = time(NULL);
    t += secs; // Schedule RTC alarm after secs
    
    // Convert timestamp to struct tm
    struct tm timeinfo;
    if (_rtc_localtime(t, &timeinfo) == false) {
        printf("config_rtc_alarm() fails\n");
        return;
    }

    S_RTC_TIME_DATA_T rtc_datetime;
    
    // Convert struct tm to S_RTC_TIME_DATA_T
    rtc_datetime.u32Year        = timeinfo.tm_year + YEAR0;
    rtc_datetime.u32Month       = timeinfo.tm_mon + 1;
    rtc_datetime.u32Day         = timeinfo.tm_mday;
    rtc_datetime.u32DayOfWeek   = timeinfo.tm_wday;
    rtc_datetime.u32Hour        = timeinfo.tm_hour;
    rtc_datetime.u32Minute      = timeinfo.tm_min;
    rtc_datetime.u32Second      = timeinfo.tm_sec;
    rtc_datetime.u32TimeScale   = RTC_CLOCK_24;
    
    RTC_SetAlarmDateAndTime(&rtc_datetime);
    
    /* NOTE: The Mbed RTC HAL implementation of Nuvoton's targets doesn't use interrupt, so we can override vector
             handler (via NVIC_SetVector). */
    /* NOTE: The name of symbol PWRWU_IRQHandler is mangled in C++ and cannot override that in startup file in C.
     *       So the NVIC_SetVector call cannot be left out. */
    NVIC_SetVector(RTC_IRQn, (uint32_t) RTC_IRQHandler);
    NVIC_EnableIRQ(RTC_IRQn);
    /* Enable RTC alarm interrupt and wake-up function will be enabled also */
    RTC_EnableInt(RTC_INTEN_ALMIEN_Msk);
}
