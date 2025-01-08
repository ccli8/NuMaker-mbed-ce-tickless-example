#ifndef __WAKEUP_H__
#define __WAKEUP_H__

#include <vector>
#include "mbed.h"

enum EventFlag_Wakeup {
    EventFlag_Wakeup_Button1        = (1 << 0),
    EventFlag_Wakeup_Button2        = (1 << 1),
    EventFlag_Wakeup_LPTicker       = (1 << 2),
    EventFlag_Wakeup_WDT_Timeout    = (1 << 3),
    EventFlag_Wakeup_RTC_Alarm      = (1 << 4),
    EventFlag_Wakeup_UART_CTS       = (1 << 5),
    EventFlag_Wakeup_I2C_AddrMatch  = (1 << 6),
    
    EventFlag_Wakeup_UnID           = (1 << 7),
    
    EventFlag_Wakeup_All            = 0xFF,
};

extern EventFlags wakeup_eventflags;

void config_pwrctl(void);
void config_button_wakeup(void);
void config_wdt_wakeup(void);
void config_rtc_wakeup(void);
void config_uart_wakeup(void);
void config_i2c_wakeup(void);

#endif  // target-power.h
