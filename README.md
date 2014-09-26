mutebox
=======
![](https://github.com/antila/mutebox/blob/master/schematics/mutebox_bb.png)

Software and hardware schematics for a 3 button USB media keyboard with volume knob.

## Folders
### box/
Contains laser cutter diagrams. Made for printing on a 384.0 mm long x 384.0 mm wide x 5.0 mm high square of black acrylic. I ordered mine to be cut from [Ponoko](http://ponoko.com), but it's probably a lot cheaper to do so at your local hackerspace. (Support your local makers!)

### schematics/
Instructions on how to solder all the components together.

mutebox.fzz is made with [Fritzing](http://fritzing.org/home/) and the [Adafruit Fritzing Library](https://github.com/adafruit/Fritzing-Library)

### src/
Arduino IDE project. Has the C code for flashing on the Adafruit Trinket. Instructions for setting up the IDE can be found [on their page](https://learn.adafruit.com/introducing-trinket?view=all)



Special shout out to
[this tutorial](https://learn.adafruit.com/trinket-usb-volume-knob/code), which I based most of my work on.
