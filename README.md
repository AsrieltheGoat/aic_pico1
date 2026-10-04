# AIC Pico for Waveshare RP2040-Zero

A board-specific AIC reader firmware for the **Waveshare RP2040-Zero** using a **PN532 NFC reader** and the board's onboard **WS2812B RGB LED**.

This project is a hardware-specific fork of [WHoweChina/AIC Pico](https://github.com/whowechina/aic_pico). It is intentionally simplified for this board: the keypad, PN5180, LCD, touch controller, accelerometer, GUI, graphics, and their related support have been removed.

> **Important:** This firmware is for the **Waveshare RP2040-Zero**. Do not use the pinout below as a generic Raspberry Pi Pico pinout.

## Hardware

Required:

- Waveshare RP2040-Zero
- PN532 NFC reader/breakout with I2C support
- USB-C cable
- Jumper wires

![Waveshare RP2040 Zero](./img/swpico.png)
![PN532 (Shopee Vietnam](https://shopee.vn/Pn532-NFC-RFID-M%C3%B4-%C4%91un-kh%C3%B4ng-d%C3%A2y-V3-B%E1%BB%99-d%E1%BB%A5ng-c%E1%BB%A5-ng%C6%B0%E1%BB%9Di-d%C3%B9ng-%C4%90%E1%BA%A7u-%C4%91%E1%BB%8Dc-Ch%E1%BA%BF-%C4%91%E1%BB%99-ghi-IC-S50-Th%E1%BA%BB-PCB-Attenna-I2C-IIC-SPI-HSU-i.578443443.29463868507?extraParams=%7B%22display_model_id%22%3A197873309568%2C%22model_selection_logic%22%3A3%7D)
![RP2040 Zero Original](https://www.waveshare.com/rp2040-zero.htm)
![Buy RP2040 Zero Clone (Shopee Vietnam)](https://shopee.vn/Rp2040-zero-RP2040-D%C3%A0nh-Cho-Raspberry-Pi-Vi-%C4%90i%E1%BB%81u-Khi%E1%BB%83n-PICO-M%C3%B4-%C4%90un-Ban-Ph%C3%A1t-Tri%E1%BB%83n-L%C3%B5i-K%C3%A9p-Cortex-M0-B%E1%BB%99-X%E1%BB%AD-L%C3%BD-2MB-Flash-i.148048328.25572194412)

Optional:

- An external WS2812B is not required. The RP2040-Zero already has an onboard WS2812B connected to **GPIO16**.

The PN532 should be configured for **I2C mode (1-0)** using whatever switch/jumper configuration your particular PN532 breakout provides.

## Pinout

### PN532

| PN532 | RP2040-Zero | GPIO | Notes |
|---|---|---:|---|
| SDA | P26  | **GPIO26** | I2C SDA |
| SCL | P27  | **GPIO27** | I2C SCL |
| GND | PGND | GND | Common ground |
| VCC | P3V3 | 3V3 | Use the voltage appropriate for your PN532 board; 3.3 V logic is required |

The firmware uses:

- I2C peripheral: **Jumper set to 1-0**
- SDA: **GPIO26**
- SCL: **GPIO27**
- Bus speed: **400 kHz**
- PN532 I2C address: **0x24**

## Firmware features

The current build provides:

- PN532 NFC polling over I2C
- MIFARE / Type A detection
- FeliCa detection
- AIME/CardIO functionality from the upstream project
- Bandai Namco reader support from the upstream project
- USB CDC CLI
- USB AIME port
- USB HID CardIO interface
- USB HID lighting control
- WS2812B status lighting
- PN532 diagnostics

The following have been deliberately removed from this fork:

- Physical keypad
- Auto PIN entry
- NKRO keyboard HID interface
- PN5180 NFC reader
- ST7789 LCD
- CST816T touch controller
- LIS3DH accelerometer
- Touch GUI / graphics / animation assets

## Building locally

### 1. Install dependencies

On Debian/Ubuntu/Linux Mint, install the ARM toolchain, CMake, Ninja, Git, and Python:

```bash
sudo apt update
sudo apt install -y \
  git cmake ninja-build python3 \
  gcc-arm-none-eabi \
  libnewlib-arm-none-eabi
```

### 2. Install the Raspberry Pi Pico SDK

Clone the SDK and its submodules:

```bash
cd ~
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init --recursive
```

The included `build.sh` expects the SDK at `~/pico-sdk` by default.

For another location, set `PICO_SDK_PATH` explicitly:

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
```

### 3. Clone this repository

```bash
git clone https://github.com/AsrieltheGoat/aic_pico1.git
cd aic_pico1
```

### 4. Build

```bash
chmod +x build.sh
./build.sh
```

The output firmware is:

```text
aic_pico.uf2
```

You can also build with an SDK path supplied for just this command:

```bash
PICO_SDK_PATH=/path/to/pico-sdk ./build.sh
```

## Flashing the firmware

The RP2040-Zero supports drag-and-drop UF2 programming over USB. Waveshare provides a BOOT button for entering download mode.

1. Unplug the RP2040-Zero.
2. Hold the **BOOT** button.
3. Connect the USB-C cable to the computer.
4. Release the BOOT button.
5. A USB mass-storage drive should appear.
6. Copy `aic_pico.uf2` to that drive.
7. The board will reboot automatically.

## First-time wiring checklist

Before powering the reader, check:

```text
RP2040-Zero            PN532
--------------------------------
GPIO26 (P26)   ----> SDA
GPIO27 (P27)   ----> SCL
3V3    (P3V3)  ----> VCC*
GND    (PGND)  ----> GND

* Use the supply voltage required by your specific PN532 breakout.
```

## Testing from the CLI

After plugging the board into USB, the firmware exposes a **CLI CDC serial port**. Open that port at **115200 baud**. The prompt is:

```text
aic_pico>
```

Run:

```text
i2cscan
```

Expected result with a connected PN532:

```text
I2C scan: GPIO27=SCL GPIO26=SDA @ 400 kHz
Found device at 0x24
```

`i2cscan` only scans the GPIO26/27 I2C bus used by this board-specific build.

Then run:

```text
nfcdiag
```

This checks PN532 initialization and attempts Type A and FeliCa polling.

You can also use:

```text
nfc
```

which polls the NFC reader and prints the detected card and UID.

Other useful commands can be listed with:

```text
?
```

## USB interfaces

The firmware exposes multiple USB interfaces. The important ones are:

| Interface | Purpose |
|---|---|
| AIC Pico CLI Port | Serial CLI and diagnostics |
| AIC Pico AIME Port | AIME reader protocol |

For **AIME / Sinmai / Segatools**, use the USB serial interface named **AIC Pico AIME Port**, not the CLI port. The COM number assigned by Windows can change between machines and reconnects.

## Game Interface

### Configure the AIME Port

1. Open **Device Manager** by running `devmgmt.msc`.
2. If **Communications Port (COM1)** is present and in use, disable it.
3. Locate **AIC Pico AIME Port** and double-click it to open its properties.
4. Go to **Port Settings --> Advanced**.
5. Set the **COM Port Number** to **COM1**.
6. Click **OK** to apply the changes.

### Configure Segatool

1. Open `segatool.ini` with a text editor such as Notepad.
2. Locate the `[aime]` section and disable AIME support:

```text
[aime]
Enable=0
```

3. Save the file.

The game should recognize the AIC Pico immediately once the AIME port is configured correctly.
Do not enable another AimeIO reader implementation at the same time unless your specific setup requires it.

## Troubleshooting

### `i2cscan` says `No I2C devices found`

Check these first:

```text
PN532 SDA -> GPIO26
PN532 SCL -> GPIO27
PN532 GND -> GND
PN532 powered correctly
PN532 configured for I²C mode
```

The firmware expects the PN532 at 7-bit address `0x24`. If `i2cscan` does not find `0x24`, fix the electrical/interface configuration before troubleshooting the AIME software.

### `nfc` says the command is ambiguous

The CLI accepts abbreviated commands. An exact command name takes priority, so:

```text
nfc     -> NFC command
nfcdiag -> NFC diagnostic command
```

Typing `nf` can still be ambiguous because both commands begin with that prefix.

### AIME software cannot read the reader

First verify the PN532 with:

```text
i2cscan
nfcdiag
nfc
```

Then make sure the game/software is connected to the **AIC Pico AIME Port**.

## GitHub Actions build

This repository includes a manual GitHub Actions workflow at `.github/workflows/release.yml`.

Open **GitHub → Actions → Build Firmware → Run workflow**. The workflow builds the firmware and creates a GitHub Release containing:

```text
aic_pico.uf2
```

The workflow is `workflow_dispatch` only, so it does not build automatically on every push.

## Project layout

The important files are:

```text
src/board_defs.h      Board-specific GPIO configuration
src/lib/pn532.c       PN532 driver
src/lib/nfc.c         NFC abstraction and card detection
src/cardio.c          CardIO handling
src/lib/aime.c        AIME protocol
src/lib/bana.c        Bandai Namco protocol
src/light.c           WS2812B/status LED control
src/commands.c        CLI commands
src/cli.c             CLI parser
build.sh              Local build script
.github/workflows/    Manual GitHub Actions build/release
```

## Credits

This project is based on **AIC Pico** by WHoweChina and the upstream contributors:

https://github.com/whowechina/aic_pico

Hardware-specific modifications in this fork are maintained for the Waveshare RP2040-Zero.
