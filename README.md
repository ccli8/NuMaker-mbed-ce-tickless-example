# Example for tickless mode on Nuvoton's Mbed CE enabled boards

This is an example to demo [Mbed tickless mode](https://os.mbed.com/docs/mbed-os/v6.16/porting/tickless-mode.html)
on Nuvoton's Mbed CE enabled boards.

Check out [Mbed CE](https://github.com/mbed-ce)
for details on Mbed OS community edition.

## Support development tools

Use cmake-based build system.
Check out [hello world example](https://github.com/mbed-ce/mbed-ce-hello-world) for getting started.

> **⚠️ Warning**
>
> Legacy development tools below are not supported anymore.
> - [Arm's Mbed Studio](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/mbed-studio.html)
> - [Arm's Mbed CLI 2](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/mbed-cli-2.html)
> - [Arm's Mbed CLI 1](https://os.mbed.com/docs/mbed-os/v6.15/tools/developing-mbed-cli.html)

For [VS Code development](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-VS-Code)
or [OpenOCD as upload method](https://github.com/mbed-ce/mbed-os/wiki/Upload-Methods#openocd),
install below additionally:

-   [NuEclipse](https://github.com/OpenNuvoton/Nuvoton_Tools#numicro-software-development-tools): Nuvoton's fork of Eclipse
-   Nuvoton forked OpenOCD: Shipped with NuEclipse installation package above.
    Checking openocd version `openocd --version`, it should fix to `0.10.022`.

## Support targets

- [NuMaker-PFM-NANO130](https://www.nuvoton.com/products/iot-solution/iot-platform/numaker-pfm-nano130/)
- [NuMaker-PFM-NUC472](https://www.nuvoton.com/products/iot-solution/iot-platform/numaker-pfm-nuc472/)
- [NuMaker-PFM-M453](https://www.nuvoton.com/products/iot-solution/iot-platform/numaker-pfm-m453/)
- [NuMaker-PFM-M487](https://www.nuvoton.com/products/iot-solution/iot-platform/numaker-pfm-m487/)
- [NuMaker-IoT-M487](https://www.nuvoton.com/products/iot-solution/iot-platform/numaker-iot-m487/)
- [NuMaker-IoT-M467](https://www.nuvoton.com/board/numaker-iot-m467/)
- [NuMaker-IoT-M263A](https://www.nuvoton.com/board/numaker-iot-m263a/)
- NuMaker-IoT-M252

## Support wake-up sources

- Button(s)
- lp_ticker (internal with tickless)
- WDT timeout
- RTC alarm
- UART CTS state change (TODO)
- I2C address match (TODO)

## Customize idle handler

Application can choose to use Mbed OS internal idle handler or customize its own one.

1.  To use Mbed OS internal idle handler, ensure the `MBED_TICKLESS` macro
    is defined and `tickless-from-us-ticker` is configured to false.

    ```json5
    "target_overrides": {
        "NUMAKER_IOT_M467": {
            "target.macros_add": ["MBED_TICKLESS"],
            "target.tickless-from-us-ticker": false,
        },
    },
    ```

1.  To customize idle handler, ensure the `MBED_TICKLESS` macro is not defined.
    This gives flexibility for providing platform-dependent idle handler.
    The `idle_hdlr` in `main.cpp` is a trivial example.

> **⚠️ Warning**
>
> TF-M enabled target e.g. [NuMaker-M2354](https://www.nuvoton.com/board/numaker-m2354/)
> doesn't support tickless mode via `MBED_TICKLESS`. This is because TF-M doesn't allow
> NS secure call in SVC context except
> [TZ_InitContextSystem_S and friends](https://github.com/mbed-ce/mbed-os/blob/8a8bc9ca361d1cc8590832c35298551ec2d265cc/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_kernel.c#L108-L115),
> but `MBED_TICKLESS` mode needs it for
> [us_ticker initialization](https://github.com/mbed-ce/mbed-os/blob/25b05a10ec8e1ce25318817587bc77a9cca61418/targets/TARGET_NUVOTON/TARGET_M2354/us_ticker.c#L77-L81)
> in [SVC context](https://github.com/mbed-ce/mbed-os/blob/8a8bc9ca361d1cc8590832c35298551ec2d265cc/cmsis/CMSIS_5/CMSIS/RTOS2/RTX/Source/rtx_kernel.c#L263-L271).
> TF-M will trap this error and reboot the system.
> However, it is still feasible to go tickless mode by disabling `MBED_TICKLESS` and customizing idle handler as above.

## Developer guide

In the following, we take **NuMaker-IoT-M467** board as an example for Mbed CE support.

### Hardware requirements

-   NuMaker-IoT-M467 board
-   Host OS: Windows or others

### Build the example

1.  Clone the example and navigate into it
    ```
    $ git clone https://github.com/mbed-nuvoton/NuMaker-mbed-ce-tickless-example
    $ cd NuMaker-mbed-ce-tickless-example
    $ git checkout -f master
    ```

1.  Deploy necessary libraries
    ```
    $ git submodule update --init
    ```
    Or for fast install:
    ```
    $ git submodule update --init --filter=blob:none
    ```

1.  Compile with cmake/ninja
    ```
    $ mkdir build; cd build
    $ cmake .. -GNinja -DCMAKE_BUILD_TYPE=Develop -DMBED_TARGET=NUMAKER_IOT_M467
    $ ninja
    $ cd ..
    ```

### Flash the image

Flash by drag-n-drop built image `NuMaker-mbed-ce-tickless-example.bin` or `NuMaker-mbed-ce-tickless-example.hex` onto **NuMaker-IoT-M467** board

### Verify the result

On host terminal (115200/8-N-1), you should see below:

```
I am going to shallow/deep sleep
Wake up by RTC alarm from deep sleep

I am going to shallow/deep sleep
Wake up by Button1 from deep sleep

I am going to shallow/deep sleep
Wake up by Button2 from deep sleep
```

> **⚠️ Warning**
>
> To run this tickless example, target board must run with no ICE connected.
> With ICE connected, system will enter **Idle** mode (shallow sleep) rather
> than **Power-down** mode (deep sleep) even though `CLK_PowerDown()` is called.

> **ℹ️ Information**
>
> If you see `shallow sleep` rather than `deep sleep`, you just enter **Idle**
> mode (shallow sleep) rather than **Power-down** mode (deep sleep). And you need
> to check your environment.

> **ℹ️ Information**
>
> If you see `Unidentified`, it means chip is waken up from **Power-down** mode
> but its wake-up source is unidentified. Usually it is `lp_ticker` which is
> internal with the tickless mechanism.
