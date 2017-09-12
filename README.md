# Example for tickless support on Nuvoton targets

This is an example to demonstrate [tickless](https://en.wikipedia.org/wiki/Tickless_kernel) support on
Nuvoton Mbed-enabled boards.
With tickless support, user program would enter **Idle** mode (shallow sleep) or **Power-down** mode (deep sleep)
automatically when CPU is idle.

## Supported platforms
- [NuMaker-PFM-NUC472](https://developer.mbed.org/platforms/Nuvoton-NUC472/)
- [NuMaker-PFM-M453](https://developer.mbed.org/platforms/Nuvoton-M453/)
- [NuMaker-PFM-M487](https://developer.mbed.org/platforms/NUMAKER-PFM-M487/)

## Supported wake-up source
- Button(s)
- lp_ticker (internal with tickless)
- WDT timeout
- RTC alarm
- UART CTS state change (TODO)
- I2C address match (TODO)

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
1. `Ticker`, `Timeout`, and `Timer` will misbehave with this example. These timers rely on 
internal `us_ticker`, which rely on high-res clocks HXT/HIRC, which are stopped in idle time to save power.
The issue is to be addressed in next mbed OS version.

1. Interrupt callback and asynchronous transfer in `Serial`, `SPI`, `CAN`, etc. won't work with this example.
These peripherals rely on high-res clocks HXT/HIRC to operate, which are stopped in idle time to save power.
Same as above, the issue will be addressed in next mbed OS version.
