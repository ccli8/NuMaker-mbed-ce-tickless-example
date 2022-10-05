#include "mbed.h"
#include "wakeup.h"

#if defined(TARGET_NUMAKER_PFM_NANO130)
// Serial
#define SERIAL_RX   D0
#define SERIAL_TX   D1
#define SERIAL_CTS  PB_7
#define SERIAL_RTS  PB_6

#elif defined(TARGET_NUMAKER_PFM_NUC472)
// Serial
#define SERIAL_RX   PF_0
#define SERIAL_TX   PD_15
#define SERIAL_CTS  PD_13
#define SERIAL_RTS  PD_14

#elif defined(TARGET_NUMAKER_PFM_M453)
// Serial
#define SERIAL_RX   A2
#define SERIAL_TX   A3
#define SERIAL_CTS  A4
#define SERIAL_RTS  A5

#elif defined(TARGET_NUMAKER_PFM_M487)
// Serial
#define SERIAL_RX   D13
#define SERIAL_TX   D10
#define SERIAL_CTS  D12
#define SERIAL_RTS  D11

#elif defined(TARGET_NUMAKER_IOT_M487)
// Serial
#define SERIAL_RX   D13
#define SERIAL_TX   D10
#define SERIAL_CTS  D12
#define SERIAL_RTS  D11

#elif defined(TARGET_NUMAKER_IOT_M263A)
// Serial
#define SERIAL_RX   D0
#define SERIAL_TX   D1
#define SERIAL_CTS  PB_9
#define SERIAL_RTS  PB_8

#elif defined(TARGET_NUMAKER_IOT_M252)
// Serial
#define SERIAL_RX   D0
#define SERIAL_TX   D1
#define SERIAL_CTS  PB_9
#define SERIAL_RTS  PB_8

#endif

/* This handler is to be called in UART interrupt context (which is extended by Nuvoton's UART HAL implementation 
 * on mbed OS) to support wake-up by UART CTS state change. */
extern "C" void nu_uart_cts_wakeup_handler(UART_T *uart_base);

static void poll_serial(void);
#if MBED_MAJOR_VERSION >= 6
static void serial_tx_callback(UnbufferedSerial *serial_);
#else
static void serial_tx_callback(Serial *serial_);
#endif

/* Support wake-up by UART CTS state change */
static Semaphore sem_serial(0, 1);

void config_uart_wakeup(void)
{
    static Thread thread_serial;
    
    Callback<void()> callback(&poll_serial);
    thread_serial.start(callback);
}

static void poll_serial(void)
{
#if MBED_MAJOR_VERSION >= 6
    static UnbufferedSerial serial(SERIAL_TX, SERIAL_RX);
#else
    static Serial serial(SERIAL_TX, SERIAL_RX);
#endif
    /* UART CTS wake-up: clock source is not limited.
     * UART data wake-up: clock source is required to be LXT/LIRC. */
    serial.set_flow_control(SerialBase::RTSCTS, SERIAL_RTS, SERIAL_CTS);

    /* We need to register one interrupt handler to enable interrupt. */
#if MBED_MAJOR_VERSION >= 6
    Callback<void()> callback((void (*)(UnbufferedSerial *)) &serial_tx_callback, (UnbufferedSerial *) &serial);
#else
    Callback<void()> callback((void (*)(Serial *)) &serial_tx_callback, (Serial *) &serial);
#endif
    serial.attach(callback, mbed::SerialBase::TxIrq);
    
    while (true) {
        sem_serial.acquire();

        wakeup_eventflags.set(EventFlag_Wakeup_UART_CTS);
    }
}
#if MBED_MAJOR_VERSION >= 6
static void serial_tx_callback(UnbufferedSerial *serial_)
#else
static void serial_tx_callback(Serial *serial_)
#endif
{
    (void) serial_;
}

void nu_uart_cts_wakeup_handler(UART_T *uart_base)
{
    (void) uart_base;

    /* FIXME: Clear wake-up event to enable re-entering Power-down mode */

    sem_serial.release();
}
