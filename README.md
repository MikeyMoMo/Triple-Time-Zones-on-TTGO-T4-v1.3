# Triple-Time-Zones-on-TTGO-T4-v1.3
This application has a scrolling UTC/Zulu time incorporating the currency exchange rate, 2 more time zones and has an Analog clock of local time with smooth movement wedge hands.  It was a pain to do the scrolling sprite and the smooth moving hands because of the original structure that did not support it.  I hope you enjoy the hard work.  (It was fun!)  Recently added was a bar graph of the history of the currency exchange rate for the last 280 days.

Get your api key(2) at apilayer.com.  Free for 250 pulls per month of any information they have available.  If you want to pull more than 250 per month (every 3 hours), get a second api key under a different email address.  I automatically switch to additional ones when I get back a code 429 telling me that a key is all used up for the month.  I get a currency exchange rate with this program from apilayer.com and that's where you get the key.  You can just put in a return at the top of the getXchangeRate routine to stop it fetching and updating the exchange rate.  It is already there, just uncomment it.

This code is free for anyone to use, modify and redistrubute as long as original author atribution is left in.

This program came from the base code used in the SSD1306 version, also in my GitHub project collection (MikeyMoMo).  This version uses Open Font Render so multiple sizes can be displayed from a single font file.  It is too slow to use everywhere so I had to back off of its usage.  Sad.  It is easy to use (with a few bugs, yet) but too slow.  It translates the font every time.  It has to so it can do different sized on the fly.  I hope he can find a way to make it faster but that may not be possible.  For non-time-sensitive applications, it is great (except for the few bugs).

There is a #define to change the hands appearance.  One has wire frame hands, the other has wedge hands.  Change the #define to get the one you want to see.

This version does NOT use ezTime any longer.  There are too many problems with it and what I figured out to do makes it quite easy without it, even for DST and zones.  It shows 3 different times from, obviously, 3 different time zones.  On top is UTC(Zulu).  It is in a scrolling (crawling, actually) sprite that includes the name of the program and version number.  The value added part is that I can update the scroll as it goes by.  I do all of the scrolling myself including appending the front of the sprite to the end of the same sprite so it appears seamless.  That's a cute trick.  Look at the code.  The scrollSprite is resized any time there is a change of text so it will continue to appear seamless scrolling by.  It was a pain to do but I finally accomplished it.

The rest of the program is not that remarkable except for the smooth movement of the hands.  Press the button to see the analog clock face.  I didn't really like the wire frame hands but there is a bug in tft_espi that won't allow wedge hands if you are not using sprites and I was on a WROOM without PSRAM so had no room for create a display-sized sprite.  The code is still there so you can choose between wire frame and wedge hands.  Look for #define WEDGE.

Lately added is a second switch to cycle between 6 variations of presentation of the analog clock.  With and without digital display (with or without seconds) and show or hide the second hand.  It checks the setting each 10 minutes and, if different, saves it with preferences.h.  Same for selection of analog or digital face selection.  This saves the flash life just a little bit.  Version 12 is the latest.  Older versions have been removed.  They may not be compatable with the latest support code modules.

And the 3rd button changes the background color on the analog clock display.  Put in for testing but left it in since I had no other need for a button.  I don't use the button2 library on this one since my needs are simple.

Even later added is a bar graph showing the last 280 days (if you have run the program long enough to have collected that many) exchange rate values.  I am still not entirely happy with it.  It may change, slightly, in a future update.  It features dates along and under the Y axis showing the dates so you can keep up and a highlight color of the bar cooresponding to that date for ease of tracking.

Enjoy.  As always, I gladly accept bug reports or program function enhancements (unlike some other software providers on here who get upset with bug reports).  Or just ask a question if you want.  I won't bite your head off like some developers -- I promise.

Mike Morrow

![20230516_202321](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/409fce5d-b7ee-4d6a-811c-a9e0ac65b3ad)
![20230516_202333](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/a534056b-cd94-4a4d-9136-94f67c945639)
![20230528_185141](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/a1ad1587-dd5a-477c-8ace-ad4ed38f5add)
![20230528_185102](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/f7260822-7fd4-4696-a631-0d8ea94eb2c6)
![20230528_185108](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/44cdb627-8d7d-4e54-ac2f-a93b081381b7)
![20230528_185113](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/e4b57fc8-39b1-4c85-b730-049683f299cc)
![20230528_185118](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/d87970da-7f6a-4019-8d41-ffb9d8209a6e)
![20230528_185123](https://github.com/MikeyMoMo/Triple-Time-Zones-on-TTGO-T4-v1.3/assets/15792417/19c587ae-c22e-4063-8e94-e5ab9a44205a)
