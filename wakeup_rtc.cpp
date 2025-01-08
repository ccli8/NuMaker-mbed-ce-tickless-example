#include "mbed.h"
#include "wakeup.h"
#include "rtc_api.h"
#include "mbed_mktime.h"

/* Micro seconds per second */
#define NU_US_PER_SEC               1000000

/* Timer clock per second
 *
 * NOTE: This dependents on real hardware.
 */
#if defined(TARGET_NANO100)
#define NU_RTCCLK_PER_SEC           (__LXT)
#else
#define NU_RTCCLK_PER_SEC           ((CLK->CLKSEL3 & CLK_CLKSEL3_SC0SEL_Msk) ? __LIRC : __LXT)
#endif

/* Start year of struct TM*/
#define NU_TM_YEAR0         1900
/* Start year of POSIX time (set_time()/time()) */
#define NU_POSIX_YEAR0      1970
/* Start year of H/W RTC */
#define NU_HWRTC_YEAR0      2000

static Semaphore sem_rtc(0, 1);

static void rtc_loop(void);
static void schedule_rtc_alarm(uint32_t secs);

/* Convert date time from H/W RTC to struct TM */
static void rtc_convert_datetime_hwrtc_to_tm(struct tm *datetime_tm, const S_RTC_TIME_DATA_T *datetime_hwrtc);
/* Convert date time from struct TM to H/W RTC */
static void rtc_convert_datetime_tm_to_hwrtc(S_RTC_TIME_DATA_T *datetime_hwrtc, const struct tm *datetime_tm);

#if defined(TARGET_NANO100)
/* This target doesn't support relocating vector table and requires overriding 
 * vector handler at link-time. */
extern "C" void RTC_IRQHandler(void)
#else
void RTC_IRQHandler(void)
#endif
{
    /* Check if RTC alarm interrupt has occurred */
#if defined(TARGET_NANO100)
    if (RTC->RIIR & RTC_RIIR_AIF_Msk) {
        /* Clear RTC alarm interrupt flag */
        RTC->RIIR = RTC_RIIR_AIF_Msk;
        
        wakeup_eventflags.set(EventFlag_Wakeup_RTC_Alarm);
    }
#elif defined(TARGET_NUC472)
    if (RTC->INTSTS & RTC_INTSTS_ALMIF_Msk) {
        /* Clear RTC alarm interrupt flag */
        RTC->INTSTS = RTC_INTSTS_ALMIF_Msk;

        wakeup_eventflags.set(EventFlag_Wakeup_RTC_Alarm);
    }
#elif defined(TARGET_M451) || defined(TARGET_M460) || defined(TARGET_M480) || defined(TARGET_M251)
    if (RTC_GET_ALARM_INT_FLAG()) {
        /* Clear RTC alarm interrupt flag */
        RTC_CLEAR_ALARM_INT_FLAG();

        wakeup_eventflags.set(EventFlag_Wakeup_RTC_Alarm);
    }
#else
    if (RTC_GET_ALARM_INT_FLAG(RTC)) {
        /* Clear RTC alarm interrupt flag */
        RTC_CLEAR_ALARM_INT_FLAG(RTC);

        wakeup_eventflags.set(EventFlag_Wakeup_RTC_Alarm);
    }
#endif

    /* Wake up RTC loop to schedule another RTC alarm */
    sem_rtc.release();
}

void config_rtc_wakeup(void)
{
    static Thread thread_rtc(osPriorityNormal, 2048);
    
    Callback<void()> callback(&rtc_loop);
    thread_rtc.start(callback);
}

void rtc_loop(void)
{
    /* Schedule RTC alarm in 3 secs */
    schedule_rtc_alarm(3);
    
    while (true) {
        sem_rtc.acquire();

        /* Re-schedule RTC alarm in 3 secs */
        schedule_rtc_alarm(3);
    }
}

void schedule_rtc_alarm(uint32_t secs)
{
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
    
#if defined(TARGET_NANO100)
    RTC_DisableInt(RTC_RIER_AIER_Msk);
#else
    RTC_DisableInt(RTC_INTEN_ALMIEN_Msk);
#endif
    
    /* Schedule RTC alarm
     *
     * Mbed OS RTC API doesn't support RTC alarm function. To enable it, we need to control RTC H/W directly.
     * Because RTC H/W doesn't inevitably record real date time, we cannot calculate alarm time based on time()
     * timestamp. Instead, we fetch date time recorded in RTC H/W for our calculation of scheduled alarm time.
     *
     * Control flow would be:
     * 1. Fetch RTC H/W date time.
     * 2. Convert RTC H/W date time to timestamp: S_RTC_TIME_DATA_T > struct tm > time_t
     * 3. Calculate alarm time based on timestamp above
     * 4. Convert calculated timestamp back to RTC H/W date time: time_t > struct tm > S_RTC_TIME_DATA_T
     * 5. Control RTC H/W to schedule alarm
     */
    time_t t_alarm;
    struct tm datetime_tm_alarm;
    S_RTC_TIME_DATA_T datetime_hwrtc_alarm;

    /* Fetch RTC H/W date time */
    RTC_GetDateAndTime(&datetime_hwrtc_alarm);
    
    /* Convert date time from H/W RTC to struct TM */
    rtc_convert_datetime_hwrtc_to_tm(&datetime_tm_alarm, &datetime_hwrtc_alarm);
    
    /* Convert date time of struct TM to POSIX time */
    if (! _rtc_maketime(&datetime_tm_alarm, &t_alarm, RTC_FULL_LEAP_YEAR_SUPPORT)) {
        printf("%s: _rtc_maketime failed\n", __func__);
        return;
    }

    /* Calculate RTC alarm time */
    t_alarm += secs; 

    /* Convert POSIX time to date time of struct TM */
    if (! _rtc_localtime(t_alarm, &datetime_tm_alarm, RTC_FULL_LEAP_YEAR_SUPPORT)) {
        printf("%s: _rtc_localtime failed\n", __func__);
        return;
    }

    /* Convert date time from struct TM to H/W RTC */
    rtc_convert_datetime_tm_to_hwrtc(&datetime_hwrtc_alarm, &datetime_tm_alarm);
    
    /* Control RTC H/W to schedule alarm */
    RTC_SetAlarmDateAndTime(&datetime_hwrtc_alarm);
    /* NOTE: When engine is clocked by low power clock source (LXT/LIRC), we need to wait for 3 engine clocks. */
    wait_us((NU_US_PER_SEC / NU_RTCCLK_PER_SEC) * 3);
    
    /* NOTE: The Mbed RTC HAL implementation of Nuvoton's targets doesn't use interrupt, so we can override vector
             handler (via NVIC_SetVector). */
    /* NOTE: The name of symbol PWRWU_IRQHandler is mangled in C++ and cannot override that in startup file in C.
     *       So the NVIC_SetVector call cannot be left out. */
    NVIC_SetVector(RTC_IRQn, (uint32_t) RTC_IRQHandler);
    NVIC_EnableIRQ(RTC_IRQn);
    /* Enable RTC alarm interrupt and wake-up function will be enabled also */
#if defined(TARGET_NANO100)
    RTC_EnableInt(RTC_RIER_AIER_Msk);
#else
    RTC_EnableInt(RTC_INTEN_ALMIEN_Msk);
#endif
}

/*
 struct tm
   tm_sec      seconds after the minute 0-61
   tm_min      minutes after the hour 0-59
   tm_hour     hours since midnight 0-23
   tm_mday     day of the month 1-31
   tm_mon      months since January 0-11
   tm_year     years since 1900
   tm_wday     days since Sunday 0-6
   tm_yday     days since January 1 0-365
   tm_isdst    Daylight Saving Time flag
*/
static void rtc_convert_datetime_hwrtc_to_tm(struct tm *datetime_tm, const S_RTC_TIME_DATA_T *datetime_hwrtc)
{
    datetime_tm->tm_year = datetime_hwrtc->u32Year - NU_TM_YEAR0;
    datetime_tm->tm_mon  = datetime_hwrtc->u32Month - 1;
    datetime_tm->tm_mday = datetime_hwrtc->u32Day;
    datetime_tm->tm_wday = datetime_hwrtc->u32DayOfWeek;
    datetime_tm->tm_hour = datetime_hwrtc->u32Hour;
    if (datetime_hwrtc->u32TimeScale == RTC_CLOCK_12 && datetime_hwrtc->u32AmPm == RTC_PM) {
        datetime_tm->tm_hour += 12;
    }
    datetime_tm->tm_min  = datetime_hwrtc->u32Minute;
    datetime_tm->tm_sec  = datetime_hwrtc->u32Second;
}

static void rtc_convert_datetime_tm_to_hwrtc(S_RTC_TIME_DATA_T *datetime_hwrtc, const struct tm *datetime_tm)
{
    datetime_hwrtc->u32Year = datetime_tm->tm_year + NU_TM_YEAR0;
    datetime_hwrtc->u32Month = datetime_tm->tm_mon + 1;
    datetime_hwrtc->u32Day = datetime_tm->tm_mday;
    datetime_hwrtc->u32DayOfWeek = datetime_tm->tm_wday;
    datetime_hwrtc->u32Hour = datetime_tm->tm_hour;
    datetime_hwrtc->u32TimeScale = RTC_CLOCK_24;
    datetime_hwrtc->u32Minute = datetime_tm->tm_min;
    datetime_hwrtc->u32Second = datetime_tm->tm_sec;
}
