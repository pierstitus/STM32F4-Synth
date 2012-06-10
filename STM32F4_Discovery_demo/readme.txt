*****************************************************************************
** STM32F4 Discovery GLCD UI WavePlayer Demo                               **
*****************************************************************************

** TARGET **

The demo runs on an ST STM32F4-Discovery board.

** The Demo **

The demo shows how to use a 2.4" LCD with the STM32F4-Discovery board. Also
included is a small GUI Library I implemented using the touch panel.

Demo Applications called by menus include:
 - 3 RGB Test Patterns
 - A simple 'doodle application' - that lets you draw on the screen
 - Wave Player
 
The end result of my exercise will be to design a full-fledged mp3 audio player
with all bells and whistles!

** Notes **

This code runs on ChibiOS 2.4.1

The UI code as a whole, is based on the UI framework provided on
this website: http://reifel.org/PICUserInterface/ . It was actually the
first piece of code from where I learnt UI fundamentals, like event handler
mechanism.

It also uses Chan's xprintf library.