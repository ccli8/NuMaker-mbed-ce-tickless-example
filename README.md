# Example for tickless support on Nuvoton targets

This is an example to demonstrate [tickless](https://en.wikipedia.org/wiki/Tickless_kernel) support on
Nuvoton Mbed-enabled boards.
With tickless support, user program would enter **Idle** mode (shallow sleep) or **Power-down** mode (deep sleep)
automatically when CPU is idle.

## Supported platforms
- [NuMaker-PFM-NUC472](https://os.mbed.com/platforms/Nuvoton-NUC472/)
- [NuMaker-PFM-M453](https://os.mbed.com/platforms/Nuvoton-M453/)
- [NuMaker-PFM-M487](https://os.mbed.com/platforms/NuMaker-PFM-M487/)
- [NuMaker-PFM-NANO130](https://os.mbed.com/platforms/NuMaker-PFM-NANO130/)

## Supported wake-up source
- Button(s)
- lp_ticker (internal with tickless)
- WDT timeout
- RTC alarm
- UART CTS state change (TODO)
- I2C address match (TODO)

## Choose idle handler
Application user can choose to use Mbed OS internal idle handler or customize its own one.
1. To use Mbed OS internal idle handler, ensure the `MBED_TICKLESS` macro is defined in `mbed_app.json`.
1. To customize idle handler, ensure the `MBED_TICKLESS` macro is not defined in `mbed_app.json`.
   This gives flexibility for providing platform-dependent idle handler.
   The `idle_hdlr` in `main.cpp` is a trivial example.

<pre>
{
    "target_overrides": {
        "NUMAKER_PFM_NUC472": {
            "target.macros_add": ["OS_IDLE_THREAD_STACK_SIZE=512", <b>"MBED_TICKLESS"</b>],
            "target.gpio-irq-debounce-enable-list": "SW1, SW2",
            "target.gpio-irq-debounce-clock-source": "GPIO_DBCTL_DBCLKSRC_IRC10K",
            "target.gpio-irq-debounce-sample-rate": "GPIO_DBCTL_DBCLKSEL_256"
        },
</pre>

## Execution
To run this tickless example, target board must run with no ICE connected.
With ICE connected, system will enter **Idle** mode (shallow sleep) rather than **Power-down** mode (deep sleep)
even though `deepsleep()` is called.

### Modes
There are two modes on Nuvoton Mbed-enabled boards:

#### MASS mode
This mode supports drap-n-drop programming and VCOM. For retrieving chip information, ICE is connected and system cannot enter
**Power-down** mode.

#### Debug mode
This mode supports debugger and VCOM. But without entering debug session for debugging, ICE isn't connected and system can enter
**Power-down** mode (deep sleep).

So to run this tickless example, user must switch to **MASS** mode first for drag-n-drop programming and 
then switch to **Debug** mode without entering debug session for running the code.

### Note for terminal output
With your terminal program configured with 9600/8-N-1, you would see output with:
<pre>
I am going to shallow/deep sleep
Wake up by WDT timeout from <b>deep sleep</b>

I am going to shallow/deep sleep
Wake up by <b>Unidentified</b> from <b>deep sleep</b>

I am going to shallow/deep sleep
Wake up by Button2 from <b>deep sleep</b>

I am going to shallow/deep sleep
Wake up by RTC alarm from <b>deep sleep</b>

I am going to shallow/deep sleep
Wake up by Button1 from <b>deep sleep</b>
</pre>

1. If you see `shallow sleep` rather than `deep sleep`, you just enter **Idle** mode (shallow sleep) rather than
**Power-down** mode (deep sleep). And you need to check your environment.
1. If you see `Unidentified`, it means chip is waken up from **Power-down** mode but its wake-up source is unidentified.
Usually it is `lp_ticker` which is internal with the tickless mechanism.

## Known issues
1. Since mbed OS **5.6.2**, `Ticker`, `Timeout`, and `Timer` could work correctly with this example.
But use of them would lock deep sleep and cause the system from entering into deep sleep mode.
It is because these timers rely on internal `us_ticker`, which relies on high-res clocks HXT/HIRC,
which are forced to stop in deep sleep mode to save more power.
To ensure these timers work correctly, the system is guarded from entering into deep sleep mode during their use.

1. Use of interrupt callback and asynchronous transfer in `Serial`, `SPI`, `CAN`, etc. has the same issue as above.

1. Use of wait series functions (`wait`/`wait_ms`/`wait_us`) has the same issue as above.
During the wait time, the system is guarded from entering deep sleep mode.

1. Use of `EventQueue` has the same issue as above because `EventQueue` relies on `Timer`/`Ticker` for its internal counting.

1. Use of `RtosTimer` can work seamlessly with this example because `RtosTimer` relies on system timer rather than `us_ticker`.
