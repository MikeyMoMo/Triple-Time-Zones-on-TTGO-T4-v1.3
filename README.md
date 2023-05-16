# Triple-Time-Zones-on-TTGO-T4-v1.3
This version shows a scrolling UTC/Zulu time, 2 more time zones and has an Analog clock of local time.

This code is free for anyone to use, modify and redistrubute as long as original author atribution is left in.

This program came from the base code used in the SSD1306 version, also in my GitHub project collection (MikeyMoMo).  It shows 3 different times from, obviously, 3 different time zones.  On top is UTC(Zulu).  It is in a scroll (crawl) that includes the name of the program and version number.  The value added part is that I can update the scroll as it goes by.  It uses the tft_eSPI library function "scroll".  What else?!?!  It moves the entire sprite left or right and/or up or down.  It can move the sprite 8 ways.  The trick is tracking where the text is on the sprite, even if it has scrolled off.

The rest of the program is not that remarkable.  Press the button and see the analog clock face.  On this version, I am not so happy with the hands.  I might change them to use another new tft_eSPI routine "wedge" that draws wedge-shaped shapes.

Enjoy.  As always, I gladly accept bug reports or program function enhancements (unlike some other software providers on here).

Mike Morrow

![20230516_202321](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/409fce5d-b7ee-4d6a-811c-a9e0ac65b3ad)

![20230516_202333](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/a534056b-cd94-4a4d-9136-94f67c945639)
