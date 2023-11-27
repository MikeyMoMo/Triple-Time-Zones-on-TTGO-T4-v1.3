# Triple-Time-Zones-on-TTGO-T4-v1.3
This version shows a scrolling UTC/Zulu time, 2 more time zones and has an Analog clock of local time.

Get the api key at apilayer.com.  Free for 250 pulls per month of any information they have available.  I get a currency exchange rate with this program.  You can just put in a return at the top of the getXchangeRate routine to stop it updating.  It is already there, just uncomment it.

This code is free for anyone to use, modify and redistrubute as long as original author atribution is left in.

This program came from the base code used in the SSD1306 version, also in my GitHub project collection (MikeyMoMo).  This version uses Open Font Render so multiple sizes can be displayed from a single font file.  

There is a #define to change the hands appearance.  One has wire frame hands, the other has wedge hands.  Change the #define to get the one you want to see.

This version does NOT use ezTime.  There are too many problems with it and what I figured out to do makes it quite easy without it, even for DST and zones.  It shows 3 different times from, obviously, 3 different time zones.  On top is UTC(Zulu).  It is in a scroll (crawl) that includes the name of the program and version number.  The value added part is that I can update the scroll as it goes by.  It uses the tft_eSPI library function "scroll".  What else?!?!  It moves the entire sprite left or right and/or up or down.  It can move the sprite 8 ways.  The trick is tracking where the text is on the sprite, even if it has scrolled off.

The rest of the program is not that remarkable.  Press the button and see the analog clock face.  I didn't really like the wire frame hands.  The code is still there so you can choose between wire frame and wedge hands.

Lately added is a second switch to cycle between 6 variations of presentation of the analog clock.  With and without digital display (with or without seconds) and show or hide the second hand.  It checks the setting each 10 minutes and, if different, saves it with preferences.h.  Same for selection of analog or digital face selection.  This saves the flash life just a little bit.  Version 12 is the latest.  Older versions have been removed.  They may not be compatable with the latest support code modules.

And the 3rd button changes the background color on the analog clock display.  Put in for testing but left it in since I had no other need for a button.  I don't use the button2 library on this one since my needs are simple.

Enjoy.  As always, I gladly accept bug reports or program function enhancements (unlike some other software providers on here).  Or just ask a question if you want.  I won't bite your head off like some developers.

Mike Morrow

![20230516_202321](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/409fce5d-b7ee-4d6a-811c-a9e0ac65b3ad)
![20230516_202333](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/a534056b-cd94-4a4d-9136-94f67c945639)
![20230528_185141](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/a1ad1587-dd5a-477c-8ace-ad4ed38f5add)
![20230528_185102](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/f7260822-7fd4-4696-a631-0d8ea94eb2c6)
![20230528_185108](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/44cdb627-8d7d-4e54-ac2f-a93b081381b7)
![20230528_185113](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/e4b57fc8-39b1-4c85-b730-049683f299cc)
![20230528_185118](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/d87970da-7f6a-4019-8d41-ffb9d8209a6e)
![20230528_185123](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/19c587ae-c22e-4063-8e94-e5ab9a44205a)
