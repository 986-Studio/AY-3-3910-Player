/*******************************************************************
 *                     MYM Player to Serial port                   *
 *                 (c) 2014 Manoel "Godzil" Trapier                *
 *******************************************************************/
 
First for the format things, this project is licensed under the

                    WTFPL v2 Postal Card Edition:

             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
  1. If you like this software you can send me a (virtual) postals
     card. Details bellow:

             < godzil-nospambot at godzil dot net >

 If you wan't to send a real postal card, send me an email, I'll
 give you my address. Of course remove the -nospambot from my
 e-mail address.
 
*********************************************************************

Now the formal things are done.

The small project is to provide a simple and easy way to play YM
files on a computer using the less possible component.

It is currently only using a resistor, a AY-3-3910 (but any version
is fine for playing music), a femal audio jack connector to let plug
something to ear the nice sound, an Arduino and a buch of wire,
nothing more.

Most project I saw before was using a quartz to provide a clock to
the AY, but this is just useless and expensive when you have a µC
with timers :)

As the Arduino does not have enought memory to store teh YM file,
it is somewhat streamed from the PC trought the USB cable.

A Fritzing schematic is not ready yet but will come ASAP.

This project may evolve in a fully autonoous Arduino capable of playing
YM/MYM file without the need of a PC streaming. I may use the
Gameduino2 for it's the display and SD part.

More information on this project can be found here:
http://www.986-studio.com/category/electronic/ay-3-3910/

*********************************************************************

I've included a simple MYM file with this project: Thrust.mym
It's the Title music of the game Thrust on the Atari ST, music
composed by Rob Hubbard.

To convert ym to mym I recommend the ym2mym tool that come with the
OSDK (Oric SDK) that can be found here: http://osdk.defence-force.org

Enjoy!

Godzil / Manoel
