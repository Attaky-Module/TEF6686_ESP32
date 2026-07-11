<div align="center">

  <a href="https://attaky.com">
    <img alt="Attaky" src="./.github/attaky-logo-horizontal.png" width="32%" />
  </a>

  <h2>FM/AM Radio firmware for the Attaky Radio series</h2>

  <p>
    <img src="https://img.shields.io/badge/series-Attaky_Radio_Series-C0252B?style=flat-square" alt="Attaky Radio Series" />
    <a href="./LICENSE"><img src="https://img.shields.io/badge/license-GPL_3.0-blue?style=flat-square" alt="License" /></a>
    <a href="https://discord.attaky.com"><img src="https://img.shields.io/badge/Discord-attaky-5865F2?style=flat-square&logo=discord&logoColor=white" alt="Discord" /></a>
  </p>

  <a href="https://attaky.com">Attaky home</a>
  &nbsp;·&nbsp;
  <a href="https://discord.attaky.com">Discord</a>

</div>

> This repository is a fork of [TEF6686_ESP32](https://github.com/PE5PVB/TEF6686_ESP32) by PE5PVB,
> ported by Attaky from upstream **v2.20** to run on the **Attaky Radio series**.
> **Modified by Attaky in 2026** to add the `ATK_COMBO_V1` target, a touchscreen tuning UI,
> on-screen battery level, and Attaky hardware bring-up fixes. Inherits the project's
> **GPL-3.0** license.

## Compatible devices

Full-band FM and AM reception with a large touchscreen tuner. Runs on the **Attaky Radio series**:

- **Attaky Core + FM/AM Radio module** — the receiver front end (NXP **TEF668x** tuner), with
  speaker and 3.5 mm headphone output
- **2.0-inch 320×240 touchscreen** — tune by frequency, step through the band, store favourite stations
- *(optional)* **battery module** to go portable

Reception features (FM/AM, RDS, memory channels, signal readouts, Wi-Fi extras) come from
upstream TEF6686_ESP32.

## What this fork adds on top of TEF6686_ESP32

- **`ATK_COMBO_V1` build target** — for the Core + FM/AM Radio module hardware
- **Touchscreen tuning UI** — capacitive touch on the 320×240 display
- **On-screen battery level**
- **Attaky default theme and branding**
- **Bring-up fixes** for the Attaky hardware (sleep / backlight blackout, interrupt-pin conflicts, touch I²C pins)

Upstream reception, RDS, memory-channel and FM-DX features are unchanged.

## Build & flash

This is an Arduino project — clone **this** repository (not upstream `PE5PVB/TEF6686_ESP32`) to
get the `ATK_COMBO_V1` target.

1. Clone this fork:
   ```sh
   git clone https://github.com/Attaky-Module/TEF6686_ESP32.git
   ```
2. Set up the Arduino / ESP32 toolchain per the upstream
   [install guide](https://github.com/PE5PVB/TEF6686_ESP32/wiki/Installing-the-software).
3. Enable the Attaky target — uncomment `#define ATK_COMBO_V1 1` in:
   `TEF6686_ESP32-2.20/TEF6686_ESP32.ino`, `TEF6686_ESP32-2.20/src/TEF6686.cpp`,
   `TEF6686_ESP32-2.20/src/Tuner_Interface.cpp` and `TEF6686_ESP32-2.20/src/gui.h`.
4. Replace `Touch.cpp` and `User_Setup.h` in your `TFT_eSPI` library folder with the copies in `TFT_espi/`.
5. Build and flash to the device.

## Support

- **Attaky Radio series / this fork** — Attaky [Discord](https://discord.attaky.com)
- **TEF6686_ESP32 tuner software / upstream** — file at [`PE5PVB/TEF6686_ESP32`](https://github.com/PE5PVB/TEF6686_ESP32/issues)

---

# ↓ Upstream TEF6686_ESP32 README ↓

[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/PE5PVB/TEF6686_ESP32#contributing)
[![HitCount](https://hits.dwyl.com/PE5PVB/TEF6686_ESP32.svg?style=flat-square&show=unique)](http://hits.dwyl.com/PE5PVB/TEF6686_ESP32)
[![License](https://img.shields.io/badge/license%20-%20GNU_GPLv3-GPLv3?color=blue)](https://github.com/PE5PVB/TEF6686_ESP32/blob/main/LICENSE)

# Note:
The version in the repository is an ongoing development. It could and will contain bugs. To make sure you use the latest fully tested firmware, check the releases!



# TEF6686_ESP32
Advanced Tuner software for NXP TEF668x tuners with ESP32 board\
More information: https://www.pe5pvb.nl/tef6686/

## Wiki
- [About the software & features](https://github.com/PE5PVB/TEF6686_ESP32/wiki)
- [Getting started](https://github.com/PE5PVB/TEF6686_ESP32/wiki#getting-started)
- [How to install the software](https://github.com/PE5PVB/TEF6686_ESP32/wiki/Installing-the-software)
- [How to use memory channels](https://github.com/PE5PVB/TEF6686_ESP32/wiki/How-to-use-memory-channels)
- [How to use the FM DX scanner](https://github.com/PE5PVB/TEF6686_ESP32/wiki/How-to-use-the-FM-DX-scanner)
- [Menu options explained](https://github.com/PE5PVB/TEF6686_ESP32/wiki/Menu-options-explained)

## Contributing
We are open for a new ideas in our project. Feel free to share your thoughts in [Discussions](https://github.com/PE5PVB/TEF6686_ESP32/discussions).\
You can also contribute your own code using [Pull Requests](https://github.com/PE5PVB/TEF6686_ESP32/pulls). We will review it and merge into main branch.

You are using our software and you found a difficulty? Please create new [issue](https://github.com/PE5PVB/TEF6686_ESP32/issues) and describe your problem.

We also would like to invite you to join our Discord community where we share our ideas and help each other with some issues.\
[<img alt="Join the TEF6686 Discord community!" src="https://i.imgur.com/lI9Tuxf.png" height="120">](https://discord.gg/ZAVNdS74mC)  

Special thanks to all [contributors](https://github.com/PE5PVB/TEF6686_ESP32/graphs/contributors). You are awesome! ❤️
## License
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 

## If you like this software
<a href="https://www.buymeacoffee.com/pe5pvb"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=☕&slug=pe5pvb&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
