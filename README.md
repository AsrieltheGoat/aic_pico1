# AIC Pico1

Firmware for the **Waveshare RP2040 Zero**.

![Waveshare RP2040 Zero](./img/swpico.png)
![Buy (Shopee Vietnam)](https://shopee.vn/Rp2040-zero-RP2040-D%C3%A0nh-Cho-Raspberry-Pi-Vi-%C4%90i%E1%BB%81u-Khi%E1%BB%83n-PICO-M%C3%B4-%C4%90un-Ban-Ph%C3%A1t-Tri%E1%BB%83n-L%C3%B5i-K%C3%A9p-Cortex-M0-B%E1%BB%99-X%E1%BB%AD-L%C3%BD-2MB-Flash-i.148048328.25572194412)

This project is a modified build of [AIC Pico](https://github.com/whowechina/aic_pico), adapted for the Waveshare RP2040 Zero.

## Hardware

- Waveshare RP2040 Zero
- PN532 NFC reader
- Add: IC: SDA GPIO 0, SCL GPIO 1 to PN532 GPIOs

## Build

```bash
./build.sh
```

The resulting firmware is `aic_pico.uf2`.

## Credits

Original AIC Pico project and upstream authors:

- https://github.com/whowechina/aic_pico

This repository contains modifications for Waveshare RP2040 Zero hardware.
