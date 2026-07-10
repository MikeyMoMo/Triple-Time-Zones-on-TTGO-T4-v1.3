#define CONFIG4MIKE
// Original author: (must be left in any copied or modified code)
//  Mike Morrow (GitHub: MikeyMoMo) 5/16/2024

// This is written for a display size of 240x320 in landscape orientation.

#include "Definitions.h"
/***************************************************************************/
void setup()
/***************************************************************************/
{
  Serial.begin(115200);
  delay(2000);

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
  displayW_Header("Start WiFiMulti");
  startWiFiMulti();
  displayW_Header("Start OTA");
  initOTA();
  displayW_Header("Allocate Sprites");
  allocateSprites();
  displayW_Header("Fetch NTP Time");
  initTime();

  // Get current (24) hour
  strftime(cCharWork, 100, "%H", localtime(&workTime));
  iPrevHour = atoi(cCharWork);
  Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                iPrevHour, ihourlyBrilliance[iPrevHour]);
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
  iCurrHour = timeinfo->tm_hour;
  iCurrDay = timeinfo->tm_yday;
  iCurrDOW = timeinfo->tm_wday;
  iCurrSecond = timeinfo->tm_sec;

  //  lLastHeapSize = esp_get_free_heap_size();
  //  liFirstHeapSize = lLastHeapSize;
  //  Serial.printf("Starting Heap Size: %i\r\n",  lLastHeapSize);
  Serial.printf("Starting Heap Size: %i\r\n", esp_get_free_heap_size());

  // The begin() method opens a “storage space” with a defined namespace.
  // The false argument means that we’ll use it in read/write mode.
  // Use true to open or create the namespace in read-only mode.
  // Name the "folder" we will use and set for read only.
  preferences.begin("TripleTime", RO_MODE);

  // Get the two values for XRate continuance across reboot.
  fPHP_Rate = preferences.getFloat("XRateValue", 0);
  ulLastXRateFetchEpoch = preferences.getUInt("XRateEpoch", 0);
  iWhichClock = preferences.getInt("defaultShow", iDigitalClock);
  // Default of 2 shows all on analog clock face.
  uiAC_Custom = preferences.getInt("defaultAC", 2);
  if (preferences.isKey("XRateHist")) {
    Serial.println("Reading XRate History table.");
    preferences.getBytes("XRateHist", XRateHist, sizeof(XRateHist));
    preferences.getBytes("XRateJulian", XRateJulian, sizeof(XRateJulian));
  }
  Serial.printf("Preferences for iWhichClock is %i, "
                "for uiAC_Custom is %i\r\n",
                iWhichClock, uiAC_Custom);
  Serial.printf("There are %i entries left in preferences storage.\r\n",
                preferences.freeEntries());
  preferences.end();

  /* Temporary Fixup in case the board breaks to reload a new board.
   
    XRate for element 0 value 56.675, Julian 2460852, 06/25/2025
    XRate for element 1 value 56.637, Julian 2460853, 06/26/2025
    XRate for element 2 value 56.635, Julian 2460854, 06/27/2025
    XRate for element 3 value 56.610, Julian 2460855, 06/28/2025
    XRate for element 4 value 56.610, Julian 2460856, 06/29/2025
    XRate for element 5 value 56.433, Julian 2460857, 06/30/2025
    XRate for element 6 value 56.209, Julian 2460858, 07/01/2025
    XRate for element 7 value 56.375, Julian 2460859, 07/02/2025
    XRate for element 8 value 56.291, Julian 2460860, 07/03/2025
    XRate for element 9 value 56.505, Julian 2460861, 07/04/2025
    XRate for element 10 value 56.515, Julian 2460862, 07/05/2025
    XRate for element 11 value 56.515, Julian 2460863, 07/06/2025
    XRate for element 12 value 56.557, Julian 2460864, 07/07/2025
    XRate for element 13 value 56.399, Julian 2460865, 07/08/2025
    XRate for element 14 value 56.585, Julian 2460866, 07/09/2025
    XRate for element 15 value 56.435, Julian 2460867, 07/10/2025
    XRate for element 16 value 56.471, Julian 2460868, 07/11/2025
    XRate for element 17 value 56.489, Julian 2460869, 07/12/2025
    XRate for element 18 value 56.489, Julian 2460870, 07/13/2025
    XRate for element 19 value 56.564, Julian 2460871, 07/14/2025
    XRate for element 20 value 56.764, Julian 2460872, 07/15/2025
    XRate for element 21 value 57.018, Julian 2460873, 07/16/2025
    XRate for element 22 value 56.966, Julian 2460874, 07/17/2025
    XRate for element 23 value 57.132, Julian 2460875, 07/18/2025
    XRate for element 24 value 57.110, Julian 2460876, 07/19/2025
    XRate for element 25 value 57.110, Julian 2460877, 07/20/2025
    XRate for element 26 value 57.090, Julian 2460878, 07/21/2025
    XRate for element 27 value 56.901, Julian 2460879, 07/22/2025
    XRate for element 28 value 56.896, Julian 2460880, 07/23/2025
    XRate for element 29 value 56.626, Julian 2460881, 07/24/2025
    XRate for element 30 value 57.152, Julian 2460882, 07/25/2025
    XRate for element 31 value 57.150, Julian 2460883, 07/26/2025
    XRate for element 32 value 57.150, Julian 2460884, 07/27/2025
    XRate for element 33 value 57.058, Julian 2460885, 07/28/2025
    XRate for element 34 value 57.408, Julian 2460886, 07/29/2025
    XRate for element 35 value 57.894, Julian 2460887, 07/30/2025
    XRate for element 36 value 58.380, Julian 2460888, 07/31/2025
    XRate for element 37 value 58.265, Julian 2460889, 08/01/2025
    XRate for element 38 value 57.766, Julian 2460890, 08/02/2025
    XRate for element 39 value 57.766, Julian 2460891, 08/03/2025
    XRate for element 40 value 57.324, Julian 2460892, 08/04/2025
    XRate for element 41 value 57.326, Julian 2460893, 08/05/2025
    XRate for element 42 value 57.326, Julian 2460894, 08/06/2025
    XRate for element 43 value 57.131, Julian 2460895, 08/07/2025
    XRate for element 44 value 56.866, Julian 2460896, 08/08/2025
    XRate for element 45 value 56.750, Julian 2460897, 08/09/2025
    XRate for element 46 value 56.750, Julian 2460898, 08/10/2025
    XRate for element 47 value 57.046, Julian 2460899, 08/11/2025
    XRate for element 48 value 57.193, Julian 2460900, 08/12/2025
    XRate for element 49 value 56.617, Julian 2460901, 08/13/2025
    XRate for element 50 value 56.682, Julian 2460902, 08/14/2025
    XRate for element 51 value 56.992, Julian 2460903, 08/15/2025
    XRate for element 52 value 56.553, Julian 2460904, 08/16/2025
    XRate for element 53 value 56.553, Julian 2460905, 08/17/2025
    XRate for element 54 value 56.977, Julian 2460906, 08/18/2025
    XRate for element 55 value 57.099, Julian 2460907, 08/19/2025
    XRate for element 56 value 57.058, Julian 2460908, 08/20/2025
    XRate for element 57 value 57.093, Julian 2460909, 08/21/2025
    XRate for element 58 value 56.735, Julian 2460910, 08/22/2025
    XRate for element 59 value 56.500, Julian 2460911, 08/23/2025
    XRate for element 60 value 56.500, Julian 2460912, 08/24/2025
    XRate for element 61 value 56.655, Julian 2460913, 08/25/2025
    XRate for element 62 value 56.929, Julian 2460914, 08/26/2025
    XRate for element 63 value 57.277, Julian 2460915, 08/27/2025
    XRate for element 64 value 57.043, Julian 2460916, 08/28/2025
    XRate for element 65 value 57.183, Julian 2460917, 08/29/2025
    XRate for element 66 value 57.088, Julian 2460918, 08/30/2025
    XRate for element 67 value 57.088, Julian 2460919, 08/31/2025
    XRate for element 68 value 57.160, Julian 2460920, 09/01/2025
    XRate for element 69 value 57.216, Julian 2460921, 09/02/2025
    XRate for element 70 value 57.216, Julian 2460922, 09/03/2025
    XRate for element 71 value 57.191, Julian 2460923, 09/04/2025
    XRate for element 72 value 56.684, Julian 2460924, 09/05/2025
    XRate for element 73 value 56.704, Julian 2460925, 09/06/2025
    XRate for element 74 value 56.704, Julian 2460926, 09/07/2025
    XRate for element 75 value 56.656, Julian 2460927, 09/08/2025
    XRate for element 76 value 57.025, Julian 2460928, 09/09/2025
    XRate for element 77 value 57.111, Julian 2460929, 09/10/2025
    XRate for element 78 value 57.118, Julian 2460930, 09/11/2025
    XRate for element 79 value 57.145, Julian 2460931, 09/12/2025
    XRate for element 80 value 57.170, Julian 2460932, 09/13/2025
    XRate for element 81 value 57.170, Julian 2460933, 09/14/2025
    XRate for element 82 value 57.130, Julian 2460934, 09/15/2025
    XRate for element 83 value 56.940, Julian 2460935, 09/16/2025
    XRate for element 84 value 56.796, Julian 2460936, 09/17/2025
    XRate for element 85 value 57.268, Julian 2460937, 09/18/2025
    XRate for element 86 value 57.061, Julian 2460938, 09/19/2025
    XRate for element 87 value 56.947, Julian 2460939, 09/20/2025
    XRate for element 88 value 56.947, Julian 2460940, 09/21/2025
    XRate for element 89 value 56.946, Julian 2460941, 09/22/2025
    XRate for element 90 value 57.528, Julian 2460943, 09/24/2025
    XRate for element 91 value 58.186, Julian 2460944, 09/25/2025
    XRate for element 92 value 58.151, Julian 2460945, 09/26/2025
    XRate for element 93 value 58.070, Julian 2460946, 09/27/2025
    XRate for element 94 value 58.070, Julian 2460947, 09/28/2025
    XRate for element 95 value 58.136, Julian 2460948, 09/29/2025
    XRate for element 96 value 58.228, Julian 2460949, 09/30/2025
    XRate for element 97 value 58.110, Julian 2460950, 10/01/2025
    XRate for element 98 value 58.162, Julian 2460951, 10/02/2025
    XRate for element 99 value 57.940, Julian 2460952, 10/03/2025
    XRate for element 100 value 57.904, Julian 2460953, 10/04/2025
    XRate for element 101 value 57.904, Julian 2460954, 10/05/2025
    XRate for element 102 value 58.242, Julian 2460955, 10/06/2025
    XRate for element 103 value 58.109, Julian 2460956, 10/07/2025
    XRate for element 104 value 58.010, Julian 2460957, 10/08/2025
    XRate for element 105 value 58.359, Julian 2460958, 10/09/2025
    XRate for element 106 value 58.236, Julian 2460959, 10/10/2025
    XRate for element 107 value 58.325, Julian 2460960, 10/11/2025
    XRate for element 108 value 58.325, Julian 2460961, 10/12/2025
    XRate for element 109 value 58.234, Julian 2460962, 10/13/2025
    XRate for element 110 value 58.212, Julian 2460963, 10/14/2025
    XRate for element 111 value 58.198, Julian 2460964, 10/15/2025
    XRate for element 112 value 58.049, Julian 2460965, 10/16/2025
    XRate for element 113 value 58.211, Julian 2460966, 10/17/2025
    XRate for element 114 value 58.121, Julian 2460967, 10/18/2025
    XRate for element 115 value 58.121, Julian 2460968, 10/19/2025
    XRate for element 116 value 58.163, Julian 2460969, 10/20/2025
    XRate for element 117 value 58.324, Julian 2460970, 10/21/2025
    XRate for element 118 value 58.471, Julian 2460971, 10/22/2025
    XRate for element 119 value 58.619, Julian 2460972, 10/23/2025
    XRate for element 120 value 58.540, Julian 2460973, 10/24/2025
    XRate for element 121 value 58.755, Julian 2460974, 10/25/2025
    XRate for element 122 value 58.755, Julian 2460975, 10/26/2025
    XRate for element 123 value 58.883, Julian 2460976, 10/27/2025
    XRate for element 124 value 59.227, Julian 2460977, 10/28/2025
    XRate for element 125 value 58.688, Julian 2460978, 10/29/2025
    XRate for element 126 value 59.004, Julian 2460979, 10/30/2025
    XRate for element 127 value 58.708, Julian 2460980, 10/31/2025
    XRate for element 128 value 58.693, Julian 2460981, 11/01/2025
    XRate for element 129 value 58.693, Julian 2460982, 11/02/2025
    XRate for element 130 value 58.783, Julian 2460983, 11/03/2025
    XRate for element 131 value 58.668, Julian 2460984, 11/04/2025
    XRate for element 132 value 58.772, Julian 2460985, 11/05/2025
    XRate for element 133 value 58.832, Julian 2460986, 11/06/2025
    XRate for element 134 value 59.097, Julian 2460987, 11/07/2025
    XRate for element 135 value 58.806, Julian 2460988, 11/08/2025
    XRate for element 136 value 58.806, Julian 2460989, 11/09/2025
    XRate for element 137 value 58.903, Julian 2460990, 11/10/2025
    XRate for element 138 value 58.954, Julian 2460991, 11/11/2025
    XRate for element 139 value 59.202, Julian 2460992, 11/12/2025
    XRate for element 140 value 58.871, Julian 2460993, 11/13/2025
    XRate for element 141 value 58.969, Julian 2460994, 11/14/2025
    XRate for element 142 value 59.015, Julian 2460995, 11/15/2025
    XRate for element 143 value 59.015, Julian 2460996, 11/16/2025
    XRate for element 144 value 59.009, Julian 2460997, 11/17/2025
    XRate for element 145 value 58.790, Julian 2460998, 11/18/2025
    XRate for element 146 value 58.936, Julian 2460999, 11/19/2025
    XRate for element 147 value 59.032, Julian 2461000, 11/20/2025
    XRate for element 148 value 58.811, Julian 2461001, 11/21/2025
    XRate for element 149 value 58.792, Julian 2461002, 11/22/2025
    XRate for element 150 value 58.792, Julian 2461003, 11/23/2025
    XRate for element 151 value 58.853, Julian 2461004, 11/24/2025
    XRate for element 152 value 58.836, Julian 2461005, 11/25/2025
    XRate for element 153 value 58.831, Julian 2461006, 11/26/2025
    XRate for element 154 value 58.765, Julian 2461007, 11/27/2025
    XRate for element 155 value 58.657, Julian 2461008, 11/28/2025
    XRate for element 156 value 58.635, Julian 2461009, 11/29/2025
    XRate for element 157 value 58.635, Julian 2461010, 11/30/2025
    XRate for element 158 value 58.522, Julian 2461011, 12/01/2025
    XRate for element 159 value 58.639, Julian 2461012, 12/02/2025
    XRate for element 160 value 58.989, Julian 2461013, 12/03/2025
    XRate for element 161 value 59.053, Julian 2461014, 12/04/2025
    XRate for element 162 value 58.988, Julian 2461015, 12/05/2025
    XRate for element 163 value 58.965, Julian 2461016, 12/06/2025
    XRate for element 164 value 58.965, Julian 2461017, 12/07/2025
    XRate for element 165 value 59.117, Julian 2461018, 12/08/2025
    XRate for element 166 value 59.287, Julian 2461019, 12/09/2025
    XRate for element 167 value 59.313, Julian 2461020, 12/10/2025
    XRate for element 168 value 58.938, Julian 2461021, 12/11/2025
    XRate for element 169 value 59.041, Julian 2461022, 12/12/2025
    XRate for element 170 value 59.115, Julian 2461023, 12/13/2025
    XRate for element 171 value 59.115, Julian 2461024, 12/14/2025
    XRate for element 172 value 58.811, Julian 2461025, 12/15/2025
    XRate for element 173 value 58.595, Julian 2461026, 12/16/2025
    XRate for element 174 value 58.563, Julian 2461027, 12/17/2025
    XRate for element 175 value 58.547, Julian 2461028, 12/18/2025
    XRate for element 176 value 58.630, Julian 2461029, 12/19/2025
    XRate for element 177 value 58.571, Julian 2461030, 12/20/2025
    XRate for element 178 value 58.571, Julian 2461031, 12/21/2025
    XRate for element 179 value 58.765, Julian 2461032, 12/22/2025
    XRate for element 180 value 58.868, Julian 2461033, 12/23/2025
    XRate for element 181 value 58.692, Julian 2461034, 12/24/2025
    XRate for element 182 value 58.787, Julian 2461035, 12/25/2025
    XRate for element 183 value 58.751, Julian 2461036, 12/26/2025
    XRate for element 184 value 58.710, Julian 2461037, 12/27/2025
    XRate for element 185 value 58.710, Julian 2461038, 12/28/2025
    XRate for element 186 value 58.889, Julian 2461039, 12/29/2025
    XRate for element 187 value 58.888, Julian 2461040, 12/30/2025
    XRate for element 188 value 58.953, Julian 2461041, 12/31/2025
    XRate for element 189 value 58.878, Julian 2461042, 01/01/2026
    XRate for element 190 value 58.818, Julian 2461043, 01/02/2026
    XRate for element 191 value 58.813, Julian 2461044, 01/03/2026
    XRate for element 192 value 58.813, Julian 2461045, 01/04/2026
    XRate for element 193 value 59.196, Julian 2461046, 01/05/2026
    XRate for element 194 value 59.284, Julian 2461047, 01/06/2026
    XRate for element 195 value 59.347, Julian 2461048, 01/07/2026
    XRate for element 196 value 59.190, Julian 2461049, 01/08/2026
    XRate for element 197 value 59.250, Julian 2461050, 01/09/2026
    XRate for element 198 value 59.296, Julian 2461051, 01/10/2026
    XRate for element 199 value 59.296, Julian 2461052, 01/11/2026
    XRate for element 200 value 59.266, Julian 2461053, 01/12/2026
    XRate for element 201 value 59.388, Julian 2461054, 01/13/2026
    XRate for element 202 value 59.526, Julian 2461055, 01/14/2026
    XRate for element 203 value 59.548, Julian 2461056, 01/15/2026
    XRate for element 204 value 59.422, Julian 2461057, 01/16/2026
    XRate for element 205 value 59.430, Julian 2461058, 01/17/2026
    XRate for element 206 value 59.430, Julian 2461059, 01/18/2026
    XRate for element 207 value 59.447, Julian 2461060, 01/19/2026
    XRate for element 208 value 59.339, Julian 2461061, 01/20/2026
    XRate for element 209 value 59.220, Julian 2461062, 01/21/2026
    XRate for element 210 value 59.100, Julian 2461063, 01/22/2026
    XRate for element 211 value 59.104, Julian 2461064, 01/23/2026
    XRate for element 212 value 58.967, Julian 2461065, 01/24/2026
    XRate for element 213 value 58.967, Julian 2461066, 01/25/2026
    XRate for element 214 value 59.106, Julian 2461067, 01/26/2026
    XRate for element 215 value 58.988, Julian 2461068, 01/27/2026
    XRate for element 216 value 58.804, Julian 2461069, 01/28/2026
    XRate for element 217 value 59.030, Julian 2461070, 01/29/2026
    XRate for element 218 value 58.917, Julian 2461071, 01/30/2026
    XRate for element 219 value 58.915, Julian 2461072, 01/31/2026
    XRate for element 220 value 58.915, Julian 2461073, 02/01/2026
    XRate for element 221 value 58.913, Julian 2461074, 02/02/2026
    XRate for element 222 value 59.093, Julian 2461075, 02/03/2026
    XRate for element 223 value 58.981, Julian 2461076, 02/04/2026
    XRate for element 224 value 58.770, Julian 2461077, 02/05/2026
    XRate for element 225 value 58.581, Julian 2461078, 02/06/2026
    XRate for element 226 value 58.511, Julian 2461079, 02/07/2026
    XRate for element 227 value 58.511, Julian 2461080, 02/08/2026
    XRate for element 228 value 58.441, Julian 2461081, 02/09/2026
    XRate for element 229 value 58.517, Julian 2461082, 02/10/2026
    XRate for element 230 value 58.317, Julian 2461083, 02/11/2026
    XRate for element 231 value 58.038, Julian 2461084, 02/12/2026
    XRate for element 232 value 57.866, Julian 2461085, 02/13/2026
    XRate for element 233 value 57.849, Julian 2461086, 02/14/2026
    XRate for element 234 value 57.849, Julian 2461087, 02/15/2026
    XRate for element 235 value 57.968, Julian 2461088, 02/16/2026
    XRate for element 236 value 58.192, Julian 2461089, 02/17/2026
    XRate for element 237 value 57.910, Julian 2461090, 02/18/2026
    XRate for element 238 value 58.069, Julian 2461091, 02/19/2026
    XRate for element 239 value 58.040, Julian 2461092, 02/20/2026
    XRate for element 240 value 57.959, Julian 2461093, 02/21/2026
    XRate for element 241 value 57.959, Julian 2461094, 02/22/2026
    XRate for element 242 value 57.663, Julian 2461095, 02/23/2026
    XRate for element 243 value 57.734, Julian 2461096, 02/24/2026
    XRate for element 244 value 57.595, Julian 2461097, 02/25/2026
    XRate for element 245 value 57.633, Julian 2461098, 02/26/2026
    XRate for element 246 value 57.746, Julian 2461099, 02/27/2026
    XRate for element 247 value 57.741, Julian 2461100, 02/28/2026
    XRate for element 248 value 57.741, Julian 2461101, 03/01/2026
    XRate for element 249 value 59.045, Julian 2461108, 03/08/2026
    XRate for element 250 value 59.468, Julian 2461109, 03/09/2026
    XRate for element 251 value 58.977, Julian 2461110, 03/10/2026
    XRate for element 252 value 59.120, Julian 2461111, 03/11/2026
    XRate for element 253 value 59.428, Julian 2461112, 03/12/2026
    XRate for element 254 value 59.644, Julian 2461113, 03/13/2026
    XRate for element 255 value 59.590, Julian 2461114, 03/14/2026
    XRate for element 256 value 59.590, Julian 2461115, 03/15/2026
    XRate for element 257 value 59.577, Julian 2461116, 03/16/2026
    XRate for element 258 value 59.578, Julian 2461117, 03/17/2026
    XRate for element 259 value 59.887, Julian 2461118, 03/18/2026
    XRate for element 260 value 60.089, Julian 2461119, 03/19/2026
    XRate for element 261 value 60.026, Julian 2461120, 03/20/2026
    XRate for element 262 value 60.150, Julian 2461121, 03/21/2026
    XRate for element 263 value 60.150, Julian 2461122, 03/22/2026
    XRate for element 264 value 59.718, Julian 2461123, 03/23/2026
    XRate for element 265 value 60.064, Julian 2461124, 03/24/2026
    XRate for element 266 value 60.040, Julian 2461125, 03/25/2026
    XRate for element 267 value 60.134, Julian 2461126, 03/26/2026
    XRate for element 268 value 60.558, Julian 2461127, 03/27/2026
    XRate for element 269 value 60.550, Julian 2461128, 03/28/2026
    XRate for element 270 value 60.550, Julian 2461129, 03/29/2026
    XRate for element 271 value 60.550, Julian 2461130, 03/30/2026
    XRate for element 272 value 60.195, Julian 2461132, 04/01/2026
    XRate for element 273 value 60.692, Julian 2461133, 04/02/2026
    XRate for element 274 value 60.423, Julian 2461134, 04/03/2026
    XRate for element 275 value 60.410, Julian 2461135, 04/04/2026
    XRate for element 276 value 60.410, Julian 2461136, 04/05/2026
    XRate for element 277 value 60.046, Julian 2461137, 04/06/2026
    XRate for element 278 value 60.198, Julian 2461138, 04/07/2026
    XRate for element 279 value 59.538, Julian 2461139, 04/08/2026
    XRate for element 280 value 59.835, Julian 2461140, 04/09/2026
    XRate for element 281 value 59.921, Julian 2461141, 04/10/2026
    XRate for element 282 value 59.877, Julian 2461142, 04/11/2026
    XRate for element 283 value 59.877, Julian 2461143, 04/12/2026
    XRate for element 284 value 60.131, Julian 2461144, 04/13/2026
    XRate for element 285 value 59.762, Julian 2461145, 04/14/2026
    XRate for element 286 value 60.070, Julian 2461146, 04/15/2026
    XRate for element 287 value 59.980, Julian 2461147, 04/16/2026
    XRate for element 288 value 59.597, Julian 2461148, 04/17/2026
    XRate for element 289 value 59.626, Julian 2461149, 04/18/2026
    XRate for element 290 value 59.564, Julian 2461150, 04/19/2026
    XRate for element 291 value 59.815, Julian 2461151, 04/20/2026
    XRate for element 292 value 59.871, Julian 2461152, 04/21/2026
    XRate for element 293 value 60.178, Julian 2461153, 04/22/2026
    XRate for element 294 value 60.429, Julian 2461154, 04/23/2026
    XRate for element 295 value 60.723, Julian 2461155, 04/24/2026
    XRate for element 296 value 60.695, Julian 2461156, 04/25/2026
    XRate for element 297 value 60.695, Julian 2461157, 04/26/2026
    XRate for element 298 value 60.748, Julian 2461158, 04/27/2026
    XRate for element 299 value 61.114, Julian 2461159, 04/28/2026
    XRate for element 300 value 61.671, Julian 2461160, 04/29/2026
    XRate for element 301 value 61.336, Julian 2461161, 04/30/2026
    XRate for element 302 value 61.327, Julian 2461162, 05/01/2026
    XRate for element 303 value 61.290, Julian 2461163, 05/02/2026
    XRate for element 304 value 61.275, Julian 2461164, 05/03/2026
    XRate for element 305 value 61.581, Julian 2461165, 05/04/2026
    XRate for element 306 value 61.497, Julian 2461166, 05/05/2026
    XRate for element 307 value 60.762, Julian 2461167, 05/06/2026
    XRate for element 308 value 60.275, Julian 2461168, 05/07/2026
    XRate for element 309 value 60.486, Julian 2461169, 05/08/2026
    XRate for element 310 value 60.515, Julian 2461170, 05/09/2026
    XRate for element 311 value 60.515, Julian 2461171, 05/10/2026
    XRate for element 312 value 60.983, Julian 2461172, 05/11/2026
    XRate for element 313 value 61.553, Julian 2461173, 05/12/2026
    XRate for element 314 value 61.434, Julian 2461174, 05/13/2026
    XRate for element 315 value 61.471, Julian 2461175, 05/14/2026
    XRate for element 316 value 61.641, Julian 2461176, 05/15/2026
    XRate for element 317 value 61.608, Julian 2461177, 05/16/2026
    XRate for element 318 value 61.608, Julian 2461178, 05/17/2026
    XRate for element 319 value 61.508, Julian 2461179, 05/18/2026
    XRate for element 320 value 61.729, Julian 2461180, 05/19/2026
    XRate for element 321 value 61.758, Julian 2461181, 05/20/2026
    XRate for element 322 value 61.601, Julian 2461182, 05/21/2026
    XRate for element 323 value 61.738, Julian 2461183, 05/22/2026
    XRate for element 324 value 61.474, Julian 2461184, 05/23/2026
    XRate for element 325 value 61.474, Julian 2461185, 05/24/2026
    XRate for element 326 value 61.306, Julian 2461186, 05/25/2026
    XRate for element 327 value 61.548, Julian 2461187, 05/26/2026
    XRate for element 328 value 61.390, Julian 2461188, 05/27/2026
    XRate for element 329 value 61.563, Julian 2461189, 05/28/2026
    XRate for element 330 value 61.559, Julian 2461190, 05/29/2026
    XRate for element 331 value 61.474, Julian 2461191, 05/30/2026
    XRate for element 332 value 61.474, Julian 2461192, 05/31/2026
    XRate for element 333 value 61.857, Julian 2461193, 06/01/2026
    XRate for element 334 value 61.641, Julian 2461194, 06/02/2026
    XRate for element 335 value 61.789, Julian 2461195, 06/03/2026
    XRate for element 336 value 61.470, Julian 2461196, 06/04/2026
    XRate for element 337 value 61.561, Julian 2461197, 06/05/2026
    XRate for element 338 value 61.799, Julian 2461198, 06/06/2026
    XRate for element 339 value 61.799, Julian 2461199, 06/07/2026
    XRate for element 340 value 61.546, Julian 2461200, 06/08/2026
    XRate for element 341 value 61.325, Julian 2461201, 06/09/2026
    XRate for element 342 value 61.341, Julian 2461202, 06/10/2026
    XRate for element 343 value 61.366, Julian 2461203, 06/11/2026
    XRate for element 344 value 60.775, Julian 2461204, 06/12/2026
    XRate for element 345 value 60.771, Julian 2461205, 06/13/2026
    XRate for element 346 value 60.771, Julian 2461206, 06/14/2026
    XRate for element 347 value 60.327, Julian 2461207, 06/15/2026
    XRate for element 348 value 60.321, Julian 2461208, 06/16/2026
    XRate for element 349 value 60.373, Julian 2461209, 06/17/2026
    XRate for element 350 value 60.373, Julian 2461210, 06/18/2026
    XRate for element 351 value 60.611, Julian 2461211, 06/19/2026
    XRate for element 352 value 60.717, Julian 2461212, 06/20/2026
    XRate for element 353 value 60.717, Julian 2461213, 06/21/2026
    XRate for element 354 value 61.139, Julian 2461214, 06/22/2026
    XRate for element 355 value 61.216, Julian 2461215, 06/23/2026
    XRate for element 356 value 61.042, Julian 2461216, 06/24/2026
    XRate for element 357 value 61.226, Julian 2461217, 06/25/2026
    XRate for element 358 value 61.304, Julian 2461218, 06/26/2026
    XRate for element 359 value 61.312, Julian 2461219, 06/27/2026
    XRate for element 360 value 61.312, Julian 2461220, 06/28/2026
    XRate for element 361 value 61.194, Julian 2461221, 06/29/2026
    XRate for element 362 value 61.369, Julian 2461222, 06/30/2026
    XRate for element 363 value 61.556, Julian 2461223, 07/01/2026
    XRate for element 364 value 61.712, Julian 2461224, 07/02/2026

    End Temporary Fixup */

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
  iCurrHour = timeinfo->tm_hour;
  iCurrDay = timeinfo->tm_yday;
  iCurrDOW = timeinfo->tm_wday;
  iCurrSecond = timeinfo->tm_sec;
  if (iPrevSecond != iCurrSecond) {
    //    Serial.printf("Now %02i:%02i:%02i\r\n",
    //                  iCurrHour, iCurrMinute, iCurrSecond);
    secStartMillis = millis();
  }
  if ((timeinfo->tm_wday == 0) && (iCurrHour == 4) && (iCurrMinute == 0) && (iCurrSecond == 0)) {
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
  if (digitalRead(changeACbackground) == LOW) {  // Pressed?
    delay(50);
    if (digitalRead(changeACbackground) == LOW)  // Still pressed?
      bChangeACBG = true;
    while (digitalRead(changeACbackground) == LOW)
      ;  // Wait for unpress
  }

  if (digitalRead(changeClockShowPin) == LOW) {  // Pressed?
    delay(50);
    if (digitalRead(changeClockShowPin) == LOW) {  // Still pressed?
      iWhichClock++;                               // Change to next screen.
      // Remember: Reset iMaxShow when more are added
      if (iWhichClock == iMaxShow) iWhichClock = 0;
      Serial.printf("iWhichClock now %i (%s)\r\n",
                    iWhichClock, clockNames[iWhichClock]);
    }
    // Wait for unpress.
    while (digitalRead(changeClockShowPin) == LOW)
      ;  // Wait for unpress
    Serial.printf("Default clock now %s\r\n", clockNames[iWhichClock]);
    if (iWhichClock == iRotate) {  // The others are unique, rotate isn't.
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextDatum(TC_DATUM);
      tft.drawString("Rotate", iXCenter, iDisplayLine3);
      tft.drawString("Screens", iXCenter, iDisplayLine5);
      delay(3000);
    }
  }
  // I need to avoid taking the second button input if the screen it
  //  affects is not showing so...
  //  I check that bAnalogClock is showing before I check the button.
  if (iWhichClock == iAnalogClock) {               // Only change it if it is showing!
    if (digitalRead(changeACShowingPin) == LOW) {  // Pressed?
      delay(50);
      if (digitalRead(changeACShowingPin) == LOW)  // Still pressed?
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
      while (digitalRead(changeACShowingPin) == LOW)
        ;  // Wait for unpress
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
  strftime(cCharWork, sizeof(cCharWork), "%H", localtime(&workTime));

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
    if (iWhichClock != preferences.getInt("defaultShow", iDigitalClock)) {
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
  if (iWhichClock == iDigitalClock) {
    //    refreshGraph = true;  // Reenable updating of the graph screen.
    updateDigitalDisplay();
  } else if (iWhichClock == iAnalogClock) {
    //    refreshGraph = true;  // Reenable updating of the graph screen.
    updateAnalogClock(workTime);
  } else if (iWhichClock == iXGraph) {
    // Since I put time on the graph screen, I must update it.
    //    if (iPrevMinute != iCurrMinute)
    //      refreshGraph = true;  // Reenable updating of the graph screen.
    showXGraph();
  } else if (iWhichClock == iRotate) {
    if (iIN_RANGE(iCurrSecond, 0, 9) || iIN_RANGE(iCurrSecond, 30, 39))
      updateDigitalDisplay();  // Do the dirty!
    if (iIN_RANGE(iCurrSecond, 10, 19) || iIN_RANGE(iCurrSecond, 40, 49))
      updateAnalogClock(workTime);
    if (iIN_RANGE(iCurrSecond, 20, 29) || iIN_RANGE(iCurrSecond, 50, 59)) {
      //      refreshGraph = true;
      showXGraph();
    }
  }
  if (iCurrSecond == 0) {
    if (iCurrMinute == 0) {  // The second is 0. See if the minute is 0 also.
      // This takes 1.6 seconds to complete.
      //  That guarantees that the new second will be upon us.
      for (i = 0; i < 4; i++) {  // Do HourDance #1
        tft.invertDisplay(true);
        delay(200);
        tft.invertDisplay(false);
        delay(200);
      }
    }
  }
  // Avoid congestion. Here is the first check 10 or 20 minute check delay.
  if ((iCurrMinute - 2) % iXRateFetchCheckInterval == 0 && iCurrSecond == 12)
    getXchangeRate();
  // If new day, may be end of month so reset it.
  if (iPrevDay != iCurrDay) iUseAPIkey = 0;
  iPrevSecond = iCurrSecond;
  iPrevMinute = iCurrMinute;
  iPrevDay = iCurrDay;
}
