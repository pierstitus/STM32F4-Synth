 STM32F4-Discovery Faust Synthesizer Target
============================================

This is an example of running [Faust](http://faust.grame.fr/) code on the [STM32F4-Discovery](http://www.chibios.org/dokuwiki/doku.php?id=chibios:articles:stm32f4_discovery) board.
The synthesizer is controlled by the onboard accelerometer and the onboard
audio codec is used for output.

Requirements
------------
- ChibiOS 2.6.3, must be in `../ChibiOS`
- dfu-util for direct uploading (or other way of uploading)
- gcc-arm-none-eabi compiler
- Faust

Usage
-----
Build and upload to STM32F4-Discovery board using dfu-util:

    make prog

Make sure the MCU is in upload mode by putting a jumper over the BOOT0-VDD pins 
and pressing the reset button.

The synthesizer is written in the [Faust language](http://faust.grame.fr/) and is
called `synth.dsp`.
The controllable parameters in this example are the absolute acceleration and normalized x, y, and z acceleration. They are now hard-coded on several locations: `synth.h`, `main.c`, `faust2stm32f4.py`.

The code for controlling the onboard CS43L22 DAC and sending data to it via
DMA is based on an implementation by [Abhishek](https://github.com/abhishek-kakkar/STM32F4Player).

This Faust target has been developed as part of the development of the Striso,
a self-contained battery powered music instrument, and is written by Piers
Titus van der Torren. See http://www.striso.org/.
