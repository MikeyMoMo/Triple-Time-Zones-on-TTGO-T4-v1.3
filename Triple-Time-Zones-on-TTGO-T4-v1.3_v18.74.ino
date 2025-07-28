#define CONFIG4MIKE
// Original author: (must be left in any copied or modified code)
//  Mike Morrow (GitHub: MikeyMoMo) 5/16/2024

// This is written for a display size of 240x320 in landscape orientation.

#include "Definitions.h"
/***************************************************************************/
void setup()
/***************************************************************************/
{
  Serial.begin(115200); delay(2000);

  Serial.println(F("\n\nThis is the Triple TZ on T4 v1.3."));
  printVers();
  // Get the number of entries in the Time Zones table.  In case on-the-fly
  //  zone changing via buttons is ever implemented.

  //  Serial.printf("Table size:\t%i\r\n", sizeof(ENVName));
  //  Serial.printf("Element size:\t%i\r\n", sizeof(*ENVName));
  //  int t = sizeof(ENVName) / sizeof(*ENVName);
  //  Serial.printf("Time ENV Table elements: %i\r\n", t);
  //  for (int i = 0; i < t; i++) Serial.println(ENVName[i]);

  initDisplay();

  iScrollSpriteW = tft.width() * 2;  // Will be reallocated, later.
  iScrollSpriteH = 42;

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector

  //displayW_Header("startWiFiManager"); startWiFiManager();
  //displayW_Header("startWiFi");        startWiFi();
  displayW_Header("Start WiFiMulti");    startWiFiMulti();
  displayW_Header("Start OTA");          initOTA();
  displayW_Header("Allocate Sprites");   allocateSprites();
  displayW_Header("Fetch NTP Time");     initTime();

  // Get current (24) hour
  strftime (cCharWork, 100, "%H", localtime(&workTime));
  iPrevHour = atoi(cCharWork);
  Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                iPrevHour, ihourlyBrilliance[iPrevHour]);
  // Set display brightness for local hour.
  //  ledcWrite(iPWM_LedChannelTFT, ihourlyBrilliance[iPrevHour]);
  ledcWrite(TFT_BL, ihourlyBrilliance[iPrevHour]);
  pinMode(changeClockShowPin, INPUT_PULLUP);
  pinMode(changeACShowingPin, INPUT_PULLUP);
  pinMode(changeACbackground, INPUT_PULLUP);

  time(&UTC);  // Get UTC epoch number.
#if defined CONFIG4MIKE
  // Assuming, for now, that the bottom clock is local time for Mike.
  workTime = UTC + iBotOffset;
#else
  // Assuming, for now, that the top clock is local time for Joe.
  workTime = UTC + iTopOffset;
#endif
  // Load up the timeinfo struct for immediate use.
  timeinfo = localtime(&workTime);
  iCurrMinute = timeinfo->tm_min;
  iCurrHour   = timeinfo->tm_hour;
  iCurrDay    = timeinfo->tm_yday;
  iCurrDOW    = timeinfo->tm_wday;
  iCurrSecond = timeinfo->tm_sec;

  //  lLastHeapSize = esp_get_free_heap_size();
  //  liFirstHeapSize = lLastHeapSize;
  //  Serial.printf("Starting Heap Size: %i\r\n",  lLastHeapSize);
  Serial.printf("Starting Heap Size: %i\r\n", esp_get_free_heap_size());

  // The begin() method opens a “storage space” with a defined namespace.
  // The false argument means that we’ll use it in read/write mode.
  // Use true to open or create the namespace in read-only mode.
  // Name the "folder" we will use and set for read/write.
  preferences.begin("TripleTime", RO_MODE);

  // Get the two values for XRate continuance across reboot.
  fPHP_Rate = preferences.getFloat("XRateValue", 0);
  ulLastXRateFetchEpoch = preferences.getUInt("XRateEpoch", 0);
  iWhichClock = preferences.getInt("defaultShow", iDigitalClock);
  // Default of 2 shows all on analog clock face.
  uiAC_Custom   = preferences.getInt("defaultAC", 2);
  if (preferences.isKey("XRateHist")) {
    Serial.println("Reading XRate History table.");
    preferences.getBytes("XRateHist", XRateHist, sizeof(XRateHist));
    preferences.getBytes("XRateJulian", XRateJulian, sizeof(XRateJulian));
  }
  /* Temporary Fixup */

  //  Starts at 199 on 06/22/2025

  //  XRateHist[227] = 58.870;  XRateJulian[227] = 2460666;  //  12/21/2024
  //  XRateHist[228] = 58.870;  XRateJulian[228] = 2460667;  //  12/22/2024
  //  XRateHist[229] = 58.870;  XRateJulian[229] = 2460668;  //  12/23/2024
  //  XRateHist[230] = 58.556;  XRateJulian[220] = 2460669;  //  12/24/2024
  //  XRateHist[231] = 58.492;  XRateJulian[221] = 2460670;  //  12/25/2024
  //  XRateHist[232] = 58.660;  XRateJulian[232] = 2460671;  //  12/26/2024
  //  XRateHist[233] = 58.000;  XRateJulian[233] = 2460672;  //  12/27/2024
  //  XRateHist[234] = 57.918;  XRateJulian[234] = 2460673;  //  12/28/2024
  //  XRateHist[235] = 57.918;  XRateJulian[235] = 2460674;  //  12/29/2024
  //  XRateHist[236] = 57.918;  XRateJulian[236] = 2460675;  //  12/30/2024
  //  XRateHist[237] = 57.859;  XRateJulian[237] = 2460676;  //  12/31/2024
  //  XRateHist[238] = 58.077;  XRateJulian[238] = 2460677;  //  01/01/2025
  //  XRateHist[239] = 58.196;  XRateJulian[239] = 2460678;  //  01/02/2025
  //  XRateHist[240] = 58.021;  XRateJulian[240] = 2460679;  //  01/03/2025
  //  XRateHist[241] = 58.202;  XRateJulian[241] = 2460680;  //  01/04/2025
  //  XRateHist[242] = 58.208;  XRateJulian[242] = 2460681;  //  01/05/2025
  //  XRateHist[243] = 58.208;  XRateJulian[243] = 2460682;  //  01/06/2025
  //  XRateHist[244] = 58.059;  XRateJulian[244] = 2460683;  //  01/07/2025
  //  XRateHist[245] = 58.150;  XRateJulian[245] = 2460684;  //  01/08/2025
  //  XRateHist[246] = 58.547;  XRateJulian[246] = 2460685;  //  01/09/2025
  //  XRateHist[247] = 58.491;  XRateJulian[247] = 2460686;  //  01/10/2025
  //  XRateHist[248] = 58.680;  XRateJulian[248] = 2460687;  //  01/11/2025
  //  XRateHist[249] = 59.009;  XRateJulian[249] = 2460688;  //  01/12/2025
  //  XRateHist[250] = 59.009;  XRateJulian[250] = 2460689;  //  01/13/2025
  //  XRateHist[251] = 58.660;  XRateJulian[251] = 2460690;  //  01/14/2025
  //  XRateHist[252] = 58.728;  XRateJulian[252] = 2460691;  //  01/15/2025
  //  XRateHist[253] = 58.473;  XRateJulian[253] = 2460692;  //  01/16/2025
  //  XRateHist[254] = 58.603;  XRateJulian[254] = 2460693;  //  01/17/2025
  //  XRateHist[255] = 58.404;  XRateJulian[255] = 2460694;  //  01/18/2025
  //  XRateHist[256] = 58.545;  XRateJulian[256] = 2460695;  //  01/19/2025
  //  XRateHist[257] = 58.443;  XRateJulian[257] = 2460697;  //  01/21/2025
  //  XRateHist[258] = 58.353;  XRateJulian[258] = 2460698;  //  01/22/2025
  //  XRateHist[259] = 58.563;  XRateJulian[259] = 2460699;  //  01/23/2025
  //  XRateHist[260] = 58.646;  XRateJulian[260] = 2460700;  //  01/24/2025
  //  XRateHist[261] = 58.306;  XRateJulian[261] = 2460701;  //  01/25/2025
  //  XRateHist[262] = 58.250;  XRateJulian[262] = 2460702;  //  01/26/2025
  //  XRateHist[263] = 58.250;  XRateJulian[263] = 2460703;  //  01/27/2025
  //  XRateHist[264] = 58.425;  XRateJulian[264] = 2460705;  //  01/29/2025
  //  XRateHist[265] = 58.445;  XRateJulian[265] = 2460706;  //  01/30/2025
  //  XRateHist[266] = 58.333;  XRateJulian[266] = 2460707;  //  01/31/2025
  //  XRateHist[267] = 58.431;  XRateJulian[267] = 2460708;  //  02/01/2025
  //  XRateHist[268] = 58.421;  XRateJulian[268] = 2460709;  //  02/02/2025
  //  XRateHist[269] = 58.421;  XRateJulian[269] = 2460710;  //  02/03/2025
  //  XRateHist[270] = 58.494;  XRateJulian[270] = 2460711;  //  02/04/2025
  //  XRateHist[271] = 58.015;  XRateJulian[271] = 2460712;  //  02/05/2025
  //  XRateHist[272] = 57.878;  XRateJulian[272] = 2460713;  //  02/06/2025
  //  XRateHist[273] = 58.060;  XRateJulian[273] = 2460714;  //  02/07/2025
  //  XRateHist[274] = 58.129;  XRateJulian[274] = 2460715;  //  02/08/2025
  //  XRateHist[275] = 58.060;  XRateJulian[275] = 2460716;  //  02/09/2025
  //  XRateHist[276] = 58.060;  XRateJulian[276] = 2460717;  //  02/10/2025
  //  XRateHist[277] = 58.120;  XRateJulian[277] = 2460718;  //  02/11/2025
  //  XRateHist[278] = 58.148;  XRateJulian[278] = 2460719;  //  02/12/2025
  //  XRateHist[279] = 58.199;  XRateJulian[279] = 2460720;  //  02/13/2025
  //  XRateHist[280] = 57.871;  XRateJulian[280] = 2460721;  //  02/14/2025
  //  XRateHist[281] = 57.793;  XRateJulian[281] = 2460722;  //  02/15/2025
  //  XRateHist[282] = 57.695;  XRateJulian[282] = 2460723;  //  02/16/2025
  //  XRateHist[283] = 57.695;  XRateJulian[283] = 2460724;  //  02/17/2025
  //  XRateHist[284] = 57.737;  XRateJulian[284] = 2460725;  //  02/18/2025
  //  XRateHist[285] = 58.025;  XRateJulian[285] = 2460726;  //  02/19/2025
  //  XRateHist[286] = 58.169;  XRateJulian[286] = 2460727;  //  02/20/2025
  //  XRateHist[287] = 58.127;  XRateJulian[287] = 2460728;  //  02/21/2025
  //  XRateHist[288] = 57.877;  XRateJulian[288] = 2460729;  //  02/22/2025
  //  XRateHist[289] = 57.881;  XRateJulian[289] = 2460730;  //  02/23/2025
  //  XRateHist[290] = 57.877;  XRateJulian[290] = 2460731;  //  02/24/2025
  //  XRateHist[291] = 57.875;  XRateJulian[291] = 2460732;  //  02/25/2025
  //  XRateHist[292] = 57.853;  XRateJulian[292] = 2460733;  //  02/26/2025
  //  XRateHist[293] = 57.899;  XRateJulian[293] = 2460734;  //  02/27/2025
  //  XRateHist[294] = 57.885;  XRateJulian[294] = 2460735;  //  02/28/2025
  //  XRateHist[295] = 57.940;  XRateJulian[295] = 2460736;  //  03/01/2025
  //  XRateHist[296] = 57.940;  XRateJulian[296] = 2460737;  //  03/02/2025
  //  XRateHist[297] = 57.965;  XRateJulian[297] = 2460738;  //  03/03/2025
  //  XRateHist[298] = 57.806;  XRateJulian[298] = 2460739;  //  03/04/2025
  //  XRateHist[299] = 57.684;  XRateJulian[299] = 2460740;  //  03/05/2025
  //  XRateHist[300] = 57.288;  XRateJulian[300] = 2460741;  //  03/06/2025
  //  XRateHist[301] = 57.187;  XRateJulian[301] = 2460742;  //  03/07/2025
  //  XRateHist[302] = 57.332;  XRateJulian[302] = 2460743;  //  03/08/2025
  //  XRateHist[303] = 57.205;  XRateJulian[303] = 2460744;  //  03/09/2025
  //  XRateHist[304] = 57.205;  XRateJulian[304] = 2460745;  //  03/10/2025
  //  XRateHist[305] = 57.469;  XRateJulian[305] = 2460746;  //  03/11/2025
  //  XRateHist[306] = 57.457;  XRateJulian[306] = 2460747;  //  03/12/2025
  //  XRateHist[307] = 57.390;  XRateJulian[307] = 2460748;  //  03/13/2025
  //  XRateHist[308] = 57.415;  XRateJulian[308] = 2460749;  //  03/14/2025
  //  XRateHist[309] = 57.359;  XRateJulian[309] = 2460750;  //  03/15/2025
  //  XRateHist[310] = 57.229;  XRateJulian[310] = 2460751;  //  03/16/2025
  //  XRateHist[311] = 57.269;  XRateJulian[311] = 2460752;  //  03/17/2025
  //  XRateHist[312] = 57.260;  XRateJulian[312] = 2460753;  //  03/18/2025
  //  XRateHist[313] = 57.229;  XRateJulian[313] = 2460754;  //  03/19/2025
  //  XRateHist[314] = 57.174;  XRateJulian[314] = 2460755;  //  03/20/2025
  //  XRateHist[315] = 57.265;  XRateJulian[315] = 2460756;  //  03/21/2025
  //  XRateHist[316] = 57.341;  XRateJulian[316] = 2460757;  //  03/22/2025
  //  XRateHist[317] = 57.339;  XRateJulian[317] = 2460758;  //  03/23/2025
  //  XRateHist[318] = 57.335;  XRateJulian[318] = 2460759;  //  03/24/2025
  //  XRateHist[319] = 57.335;  XRateJulian[319] = 2460760;  //  03/25/2025
  //  XRateHist[320] = 57.352;  XRateJulian[320] = 2460761;  //  03/26/2025
  //  XRateHist[321] = 57.602;  XRateJulian[321] = 2460762;  //  03/27/2025
  //  XRateHist[322] = 57.757;  XRateJulian[322] = 2460763;  //  03/28/2025
  //  XRateHist[323] = 57.360;  XRateJulian[323] = 2460764;  //  03/29/2025
  //  XRateHist[324] = 57.378;  XRateJulian[324] = 2460765;  //  03/30/2025
  //  XRateHist[325] = 57.279;  XRateJulian[325] = 2460766;  //  03/31/2025
  //  XRateHist[326] = 57.271;  XRateJulian[326] = 2460767;  //  04/01/2025
  //  XRateHist[327] = 57.218;  XRateJulian[327] = 2460768;  //  04/02/2025
  //  XRateHist[328] = 57.238;  XRateJulian[328] = 2460769;  //  04/03/2025
  //  XRateHist[329] = 56.907;  XRateJulian[329] = 2460770;  //  04/04/2025
  //  XRateHist[330] = 57.270;  XRateJulian[330] = 2460771;  //  04/05/2025
  //  XRateHist[331] = 57.385;  XRateJulian[331] = 2460772;  //  04/06/2025
  //  XRateHist[332] = 57.385;  XRateJulian[332] = 2460773;  //  04/07/2025
  //  XRateHist[333] = 55.659;  XRateJulian[333] = 2460798;  //  05/02/2025
  //  XRateHist[334] = 55.510;  XRateJulian[334] = 2460799;  //  05/03/2025
  //  XRateHist[335] = 55.510;  XRateJulian[335] = 2460800;  //  05/04/2025
  //  XRateHist[336] = 55.492;  XRateJulian[336] = 2460801;  //  05/05/2025
  //  XRateHist[337] = 55.828;  XRateJulian[337] = 2460802;  //  05/06/2025
  //  XRateHist[338] = 55.278;  XRateJulian[338] = 2460803;  //  05/07/2025
  //  XRateHist[339] = 55.588;  XRateJulian[339] = 2460804;  //  05/08/2025
  //  XRateHist[340] = 55.716;  XRateJulian[340] = 2460805;  //  05/09/2025
  //  XRateHist[341] = 55.367;  XRateJulian[341] = 2460806;  //  05/10/2025
  //  XRateHist[342] = 55.367;  XRateJulian[342] = 2460807;  //  05/11/2025
  //  XRateHist[343] = 55.413;  XRateJulian[343] = 2460808;  //  05/12/2025
  //  XRateHist[344] = 55.687;  XRateJulian[344] = 2460809;  //  05/13/2025
  //  XRateHist[345] = 55.734;  XRateJulian[345] = 2460810;  //  05/14/2025
  //  XRateHist[346] = 55.813;  XRateJulian[346] = 2460811;  //  05/15/2025
  //  XRateHist[347] = 55.595;  XRateJulian[347] = 2460812;  //  05/16/2025
  //  XRateHist[348] = 55.804;  XRateJulian[348] = 2460813;  //  05/17/2025
  //  XRateHist[349] = 55.804;  XRateJulian[349] = 2460814;  //  05/18/2025
  //  XRateHist[350] = 55.788;  XRateJulian[350] = 2460815;  //  05/19/2025
  //  XRateHist[351] = 55.719;  XRateJulian[351] = 2460816;  //  05/20/2025
  //  XRateHist[352] = 55.595;  XRateJulian[352] = 2460817;  //  05/21/2025
  //  XRateHist[353] = 55.579;  XRateJulian[353] = 2460818;  //  05/22/2025
  //  XRateHist[354] = 55.685;  XRateJulian[354] = 2460819;  //  05/23/2025
  //  XRateHist[355] = 55.370;  XRateJulian[355] = 2460820;  //  05/24/2025
  //  XRateHist[356] = 55.370;  XRateJulian[356] = 2460821;  //  05/25/2025
  //  XRateHist[357] = 55.164;  XRateJulian[357] = 2460822;  //  05/26/2025
  //  XRateHist[358] = 55.383;  XRateJulian[358] = 2460823;  //  05/27/2025
  //  XRateHist[359] = 55.602;  XRateJulian[359] = 2460824;  //  05/28/2025
  //  XRateHist[360] = 55.638;  XRateJulian[360] = 2460825;  //  05/29/2025
  //  XRateHist[361] = 55.625;  XRateJulian[361] = 2460826;  //  05/30/2025
  //  XRateHist[362] = 55.770;  XRateJulian[362] = 2460827;  //  05/31/2025
  //  XRateHist[363] = 55.770;  XRateJulian[363] = 2460828;  //  06/01/2025
  //  XRateHist[364] = 55.842;  XRateJulian[364] = 2460829;  //  06/02/2025
  // XRate for element 345 value 55.650, Julian 2460830, 06/03/2025
  // XRate for element 346 value 55.787, Julian 2460831, 06/04/2025
  // XRate for element 347 value 55.606, Julian 2460832, 06/05/2025
  // XRate for element 348 value 55.673, Julian 2460833, 06/06/2025
  // XRate for element 349 value 55.912, Julian 2460834, 06/07/2025
  // XRate for element 350 value 55.912, Julian 2460835, 06/08/2025
  // XRate for element 351 value 55.820, Julian 2460836, 06/09/2025
  // XRate for element 352 value 55.768, Julian 2460837, 06/10/2025
  // XRate for element 353 value 55.873, Julian 2460838, 06/11/2025
  // XRate for element 354 value 55.857, Julian 2460839, 06/12/2025
  // XRate for element 355 value 56.155, Julian 2460840, 06/13/2025
  // XRate for element 356 value 56.090, Julian 2460841, 06/14/2025
  // XRate for element 357 value 56.090, Julian 2460842, 06/15/2025
  // XRate for element 358 value 56.441, Julian 2460843, 06/16/2025
  // XRate for element 359 value 56.533, Julian 2460844, 06/17/2025
  // XRate for element 360 value 56.813, Julian 2460845, 06/18/2025
  // XRate for element 361 value 57.365, Julian 2460846, 06/19/2025
  // XRate for element 362 value 57.226, Julian 2460847, 06/20/2025
  // XRate for element 363 value 57.172, Julian 2460848, 06/21/2025
  // XRate for element 364 value 57.172, Julian 2460849, 06/22/2025
  // XRate for element 340 value 57.585, Julian 2460850, 06/23/2025
  // XRate for element 341 value 56.912, Julian 2460851, 06/24/2025
  // XRate for element 342 value 56.675, Julian 2460852, 06/25/2025
  // XRate for element 343 value 56.637, Julian 2460853, 06/26/2025
  // XRate for element 344 value 56.635, Julian 2460854, 06/27/2025
  // XRate for element 345 value 56.610, Julian 2460855, 06/28/2025
  // XRate for element 346 value 56.610, Julian 2460856, 06/29/2025
  // XRate for element 347 value 56.433, Julian 2460857, 06/30/2025
  // XRate for element 348 value 56.209, Julian 2460858, 07/01/2025
  // XRate for element 349 value 56.375, Julian 2460859, 07/02/2025
  // XRate for element 350 value 56.291, Julian 2460860, 07/03/2025
  // XRate for element 351 value 56.505, Julian 2460861, 07/04/2025
  // XRate for element 352 value 56.515, Julian 2460862, 07/05/2025
  // XRate for element 353 value 56.515, Julian 2460863, 07/06/2025
  // XRate for element 354 value 56.557, Julian 2460864, 07/07/2025
  // XRate for element 355 value 56.399, Julian 2460865, 07/08/2025
  // XRate for element 356 value 56.585, Julian 2460866, 07/09/2025
  // XRate for element 357 value 56.435, Julian 2460867, 07/10/2025
  // XRate for element 358 value 56.471, Julian 2460868, 07/11/2025
  // XRate for element 359 value 56.489, Julian 2460869, 07/12/2025
  // XRate for element 360 value 56.489, Julian 2460870, 07/13/2025
  // XRate for element 361 value 56.564, Julian 2460871, 07/14/2025
  // XRate for element 362 value 56.764, Julian 2460872, 07/15/2025
  // XRate for element 363 value 57.018, Julian 2460873, 07/16/2025
  // XRate for element 364 value 56.966, Julian 2460874, 07/17/2025
  // XRate for element 362 value 57.132, Julian 2460875, 07/18/2025
  // XRate for element 363 value 57.110, Julian 2460876, 07/19/2025
  // XRate for element 364 value 57.110, Julian 2460877, 07/20/2025
  /* End Temporary Fixup */

  Serial.printf("Preferences for iWhichClock is %i, "
                "for uiAC_Custom is %i\r\n", iWhichClock, uiAC_Custom);
  Serial.printf("There are %i entries left in preferences storage.\r\n",
                preferences.freeEntries());
  preferences.end();

#if defined CONFIG4MIKE
  sprintf(caReadingTime, "As of %02i:%02i %s, ",
          iCurrHour, iCurrMinute, cBotDST);
#else
  sprintf(caReadingTime, "As of %02i:%02i %s, ",
          iCurrHour, iCurrMinute, cTopDST);
#endif

  countXRate(true, true);  // Print out all entries and total.

  decodeAC_Bits();  // Which display variation for the Analog Clock display.

  tft.fillScreen(TFT_BLACK);  // Let's go!
}
/***************************************************************************/
void loop()
/***************************************************************************/
{
  ArduinoOTA.handle();
  time(&UTC);
#if defined CONFIG4MIKE
  workTime = UTC + iBotOffset;
#else
  workTime = UTC + iTopOffset;  // Use top as local time for Joe.
#endif

  // Load up the timeinfo struct for immediate use.
  timeinfo = localtime(&workTime);
  iCurrMinute = timeinfo->tm_min;
  iCurrHour   = timeinfo->tm_hour;
  iCurrDay    = timeinfo->tm_yday;
  iCurrDOW    = timeinfo->tm_wday;
  iCurrSecond = timeinfo->tm_sec;
  if (iPrevSecond != iCurrSecond) secStartMillis = millis();
  if ((timeinfo->tm_wday == 0) && (iCurrHour == 4) &&
      (iCurrMinute == 0) && (iCurrSecond == 0)) {
    Serial.printf("%02i:%02i:%02i Doing weekly reboot now.",
                  iCurrHour, iCurrMinute, iCurrSecond);

    // Save the last XRate quote and time before the reboot.
    preferences.begin("TripleTime", RW_MODE);
    preferences.putFloat("XRateValue", fPHP_Rate);
    preferences.putUInt("XRateEpoch", ulLastXRateFetchEpoch);
    preferences.end();

    ESP.restart();
  }

  // Does the user wish to change the analog clock background color?
  if (digitalRead(changeACbackground) == LOW) {      // Pressed?
    delay(50);
    if (digitalRead(changeACbackground) == LOW)      // Still pressed?
      bChangeACBG = true;
    while (digitalRead(changeACbackground) == LOW);  // Wait for unpress
  }

  if (digitalRead(changeClockShowPin) == LOW) {   // Pressed?
    delay(50);
    if (digitalRead(changeClockShowPin) == LOW) {  // Still pressed?
      iWhichClock++;  // Change to next screen.
      // Reset iMaxShow when more are added
      if (iWhichClock == iMaxShow) iWhichClock = 0;
      Serial.printf("iWhichClock now %i (%s)\r\n",
                    iWhichClock, clockNames[iWhichClock]);
    }
    // Wait for unpress.
    while (digitalRead(changeClockShowPin) == LOW); // Wait for unpress
    Serial.printf("Default clock now %s\r\n", clockNames[iWhichClock]);
    if (iWhichClock == iDigitalClock) {
      tft.fillScreen(TFT_BLACK);
      ofr.setFontColor(TFT_WHITE);
      // So there won't be a black band on top during the first few seconds.
      tft.fillRect(0, 0, tft.width(), iScrollSpriteH, DarkBlue);
      updateDigitalDisplay();  // Do the dirty!
    } else if (iWhichClock ==  iAnalogClock) {
      tft.fillScreen(TFT_BLACK);  // Out with the old...
      Serial.println("Showing Analog Clock");
      updateAnalogClock(workTime);  // In with the new!
    } else if (iWhichClock == iXGraph) {
      Serial.println("Going to iXGraph.");
      tft.fillScreen(TFT_BLACK);  // Out with the old...
      Serial.println("Showing XRate Graph");
      showXGraph();
    }
  }
  // I needed to avoid taking the second button input if the screen it
  //  affects is not showing so...
  //  I check that bAnalogClock is showing before I check the button.
  if (iWhichClock == iAnalogClock) {  // Only change it if it is showing!
    if (digitalRead(changeACShowingPin) == LOW) {  // Pressed?
      delay(50);
      if (digitalRead(changeACShowingPin) == LOW)     // Still pressed?
        uiAC_Custom++;
      // 2 & 3 are essentially identical.  Also 6 and 7.  So, skipping them.
      /*
        // bShowShortTime, bShowLongTime, showsecond hand are the 3 columns.
        // 2 & 3 and 6 & 7 are the same because of the way
        //  decodeAC_Bits is written.
        0 F F F
        1 T F F
        2 F T F  // Same as 3
        3 F T F
        4 F F T
        5 T F T
        6 F T T  // Same as 7
        7 F T T
      */
      if (uiAC_Custom == 2) uiAC_Custom = 3;  // Skip duplicate
      if (uiAC_Custom == 6) uiAC_Custom = 7;  // Skip duplicate
      // Wait for unpress.
      while (digitalRead(changeACShowingPin) == LOW); // Wait for unpress
      Serial.printf("uiAC_Custom now set to %i\r\n", uiAC_Custom);
      decodeAC_Bits();
    }
  }

  // One more shot at a smooth scroll on top.
  if (iWhichClock == iDigitalClock) scrollIt();
  time(&UTC);
#if defined CONFIG4MIKE
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iBotOffset;
#else
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iTopOffset;
#endif
  // Get current hour
  strftime (cCharWork, sizeof(cCharWork), "%H", localtime(&workTime));

  if (iPrevHour != atoi(cCharWork)) {
    iPrevHour = atoi(cCharWork);
    Serial.println("------------\r\n"
                   "Determining zone offsets for new hour.");
    // One more shot at a smooth scroll on top.
    if (iWhichClock == iDigitalClock) scrollIt();
    deduceOffsets();
    // One more shot at a smooth scroll on top.
    if (iWhichClock == iDigitalClock) scrollIt();
    Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                  iPrevHour, ihourlyBrilliance[iPrevHour]);
    // One more shot at a smooth scroll on top.
    if (iWhichClock == iDigitalClock) scrollIt();
    // For board definitions 2.x.x
    //    ledcWrite(iPWM_LedChannelTFT, ihourlyBrilliance[iPrevHour]);
    // For board definitions 3.x.x
    ledcWrite(TFT_BL, ihourlyBrilliance[iPrevHour]);
  }

  // One more shot at a smooth scroll on top.
  if (iWhichClock == iDigitalClock) scrollIt();
#if defined CONFIG4MIKE
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iBotOffset;
#else
  // Assuming, for now, that the bottom clock is local time.
  workTime = UTC + iTopOffset;
#endif

  // Another shot at a smooth scroll on top.
  if (iWhichClock == iDigitalClock) scrollIt();

  // Check if default clock to show has been changed and save it, if so.
  if (iCurrMinute % 10 == 0 && iCurrSecond == 0)  // Only on the 10 minute.
  {
    preferences.begin("TripleTime", RW_MODE);
    if (iWhichClock != preferences.getInt("defaultShow", iDigitalClock))
    {
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      Serial.printf("Saving new clock face choice %i\r\n", iWhichClock);
      preferences.putInt("defaultShow", iWhichClock);
    }
    // Another shot at a smooth scroll for the top sprite.
    if (iWhichClock == iDigitalClock) scrollIt();
    if (uiAC_Custom != preferences.getInt("defaultAC", 2)) {
      Serial.printf("uiAC_Custom %i, pref setting %i\r\n",
                    uiAC_Custom, preferences.getInt("defaultAC", 2));
      Serial.print(localtime(&workTime), "%a %m-%d-%Y %T %Z - ");
      // Another shot at a smooth scroll for the top sprite.
      if (iWhichClock == iDigitalClock) scrollIt();
      Serial.printf("Saving analog show choice %i\r\n", uiAC_Custom);
      preferences.putInt("defaultAC", uiAC_Custom);
    }
    preferences.end();
  }

  // <<<********************************>>>
  // Here is where all of the work is done.
  // <<<********************************>>>
  if (iWhichClock == iDigitalClock)
  {
    refreshGraph = true;  // Reenable updating of the graph screen.
    updateDigitalDisplay();
  }
  else if (iWhichClock == iAnalogClock) {
    refreshGraph = true;  // Reenable updating of the graph screen.
    updateAnalogClock(workTime);
  }
  else if (iWhichClock == iXGraph) {
    // Since I put time on the graph screen, I must update it.
    if (iPrevMinute != iCurrMinute)
      refreshGraph = true;  // Reenable updating of the graph screen.
    showXGraph();
  }

  if (iCurrSecond == 0) {
    if (iCurrMinute == 0) {  // The second is 0. See if the minute is 0 also.
      // This takes 1.6 seconds to complete.
      //  That guarantees that the new second will be upon us.
      for (i = 0; i < 4; i++) {  // Do HourDance #1
        tft.invertDisplay(true); delay(200);
        tft.invertDisplay(false); delay(200);
      }
    }
  }
  // Avoid congestion. Here is the first check 10 or 20 minute check delay.
  if ((iCurrMinute - 2) % iXRateFetchCheckInterval == 0 && iCurrSecond == 12)
    getXchangeRate();
  // If new day, may be end of month so reset it.
  if (iPrevDay != iCurrDay) iUseAPIkey = 0;
  iPrevSecond = iCurrSecond; iPrevMinute = iCurrMinute; iPrevDay = iCurrDay;
}
