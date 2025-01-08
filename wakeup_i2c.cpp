#include "mbed.h"
#include "wakeup.h"
#include "PeripheralPins.h"

#define I2C_ADDR    (0x90)

#if defined(TARGET_NUMAKER_PFM_NUC472)
// I2C
#define I2C_SDA     D14
#define I2C_SCL     D15

#elif defined(TARGET_NUMAKER_PFM_M453)
// I2C
#define I2C_SDA     D14
#define I2C_SCL     D15

#elif defined(TARGET_NUMAKER_PFM_M487)
// I2C
#define I2C_SDA     D9
#define I2C_SCL     D8

#elif defined(TARGET_NUMAKER_PFM_NANO130)
// I2C
#define I2C_SDA     D14
#define I2C_SCL     D15

#endif

/* NOTE: Per test (on NUC472/M453/M487), we could handle in time from idle mode (shallow sleep) wake-up, 
 *       but fail from power-down mode (deep sleep). */

/* Support wake-up by I2C traffic */
static Semaphore sem_i2c(0, 1);

static void poll_i2c(void);
/* This handler is to be called in I2C interrupt context (which is extended by Nuvoton's I2C HAL implementation 
 * on mbed OS) to support wake-up by I2C traffic. */
extern "C" void nu_i2c_wakeup_handler(I2C_T *i2c_base);

void config_i2c_wakeup(void)
{
    /* I2C engine is clocked by external I2C bus clock, so its support for wake-up is irrespective of HXT/HIRC
     * which are disabled during deep sleep (power-down). */
    
    static Thread thread_i2c;
    
    Callback<void()> callback(&poll_i2c);
    thread_i2c.start(callback);
}

static void poll_i2c(void)
{
    static I2CSlave i2c_slave(I2C_SDA, I2C_SCL);

    static char i2c_buf[32];
    
    i2c_slave.address(I2C_ADDR);
    
    while (true) {
        sem_i2c.acquire();

        bool has_notified_wakeup = false;
        /* This timer is to check if there is I2C traffic remaining. */
        Timer timer;
        timer.start();
        
        /* With no I2C traffic for 5 ms, we go back to wait on next I2C traffic. */
        while (timer.read_high_resolution_us() < 5000) {
            /* We shall call I2CSlave::receive to enable I2C interrupt again which may be disabled in handling 
             * I2C interrupt in Nuvoton's I2C HAL implementation on mbed OS. */
            int addr_status = i2c_slave.receive();
            switch (addr_status) {
                case I2CSlave::ReadAddressed:
                    if (! has_notified_wakeup) {
                        has_notified_wakeup = true;
                        wakeup_eventflags.set(EventFlag_Wakeup_I2C_AddrMatch);
                    }
                    i2c_slave.write(i2c_buf, sizeof (i2c_buf));
                    timer.reset();
                    break;
                
                case I2CSlave::WriteAddressed:
                    if (! has_notified_wakeup) {
                        has_notified_wakeup = true;
                        wakeup_eventflags.set(EventFlag_Wakeup_I2C_AddrMatch);
                    }
                    i2c_slave.read(i2c_buf, sizeof (i2c_buf));
                    timer.reset();
                    break;
            }
        }
    }
}

void nu_i2c_wakeup_handler(I2C_T *i2c_base)
{
    (void) i2c_base;

    /* FIXME: Clear wake-up event to enable re-entering Power-down mode */

    sem_i2c.release();
}
