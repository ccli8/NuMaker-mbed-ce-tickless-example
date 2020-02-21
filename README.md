# Example for tickless support on Nuvoton targets

This is an example to demonstrate [tickless](https://en.wikipedia.org/wiki/Tickless_kernel) support on
Nuvoton Mbed-enabled boards.
With tickless support, user program would enter **Idle** mode (shallow sleep) or **Power-down** mode (deep sleep)
automatically when CPU is idle.

## Supported platforms

- [NuMaker-PFM-NANO130](https://os.mbed.com/platforms/NuMaker-PFM-NANO130/)
- [NuMaker-PFM-NUC472](https://os.mbed.com/platforms/Nuvoton-NUC472/)
- [NuMaker-PFM-M453](https://os.mbed.com/platforms/Nuvoton-M453/)
- [NuMaker-PFM-M487](https://os.mbed.com/platforms/NuMaker-PFM-M487/)

## Supported wake-up source
- Button(s)
- lp_ticker (internal with tickless)
- WDT timeout
- RTC alarm
- UART CTS state change (TODO)
- I2C address match (TODO)

## Choose idle handler
Application user can choose to use Mbed OS internal idle handler or customize its own one.
1.  To use Mbed OS internal idle handler, ensure the `MBED_TICKLESS` macro is defined and `tickless-from-us-ticker` is configured to false in `mbed_app.json`.

    ```json
    "target_overrides": {
        ......
        "NUMAKER_PFM_NUC472": {
            "target.macros_add": ["MBED_TICKLESS"],
            "target.tickless-from-us-ticker": false,
        ......
    ```

    **Note**: To enable tickless on existent project, just define `MBED_TICKLESS` and configure `tickless-from-us-ticker` to false as above.

1.  To customize idle handler, ensure the `MBED_TICKLESS` macro is not defined in `mbed_app.json`.
    This gives flexibility for providing platform-dependent idle handler.
    The `idle_hdlr` in `main.cpp` is a trivial example.

## Execution
To run this tickless example, target board must run with no ICE connected.
With ICE connected, system will enter **Idle** mode (shallow sleep) rather than **Power-down** mode (deep sleep)
even though `CLK_PowerDown()` is called.

**Note:** With old Nu-Link1 firmware version, ICE is unexpected connected in MASS mode. Update to the latest Nu-Link firmware version to fix the issue.

### Terminal output
With your terminal program configured with 115200/8-N-1, you would see output with:
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

## Reference

-   [Tickless kernel](https://en.wikipedia.org/wiki/Tickless_kernel)
-   [Mbed tickless](https://os.mbed.com/docs/mbed-os/v5.15/porting/tickless.html)
