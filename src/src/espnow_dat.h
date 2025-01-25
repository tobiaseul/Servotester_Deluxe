#define CHANNEL 1
#define LED_INTERN 22
#define LED_INTERN_AN 0
#define LED_INTERN_AUS 1

static float EspNowCGLastUpdate = 0;
static float ESPNowData_CG_U_Lipo;
static float ESPNowData_CG_Bat_Type;
static float ESPNowData_CG_Model_Weight;
static float ESPNowData_CG_Model_CG;
static float ESPNowData_CG_Model_CG_Trans;

static unsigned char ESPNowData_modul1_Nr;
static unsigned char ESPNowData_modul2_Nr;
static unsigned char ESPNowData_modul3_Nr;
static unsigned char ESPNowData_modul4_Nr;
static unsigned char ESPNowData_modul5_Nr;
static unsigned char ESPNowData_modul6_Nr;
static unsigned char ESPNowData_modul7_Nr;
static unsigned char ESPNowData_modul8_Nr;
static float ESPNowData_U1_Lipo;
static float ESPNowData_U2_Lipo;
static float ESPNowData_U3_Lipo;
static float ESPNowData_U4_Lipo;
static float ESPNowData_U5_Lipo;
static float ESPNowData_U6_Lipo;
static float ESPNowData_U7_Lipo;
static float ESPNowData_U8_Lipo;
static float ESPNowData_Gyro1_Lagewert;
static float ESPNowData_Gyro2_Lagewert;
static float ESPNowData_Gyro3_Lagewert;
static float ESPNowData_Gyro4_Lagewert;
static float ESPNowData_Gyro5_Lagewert;
static float ESPNowData_Gyro6_Lagewert;
static float ESPNowData_Gyro7_Lagewert;
static float ESPNowData_Gyro8_Lagewert;
static unsigned int ESPNow1_WD;
static unsigned int ESPNow2_WD;
static unsigned int ESPNow3_WD;
static unsigned int ESPNow4_WD;
static unsigned int ESPNow5_WD;
static unsigned int ESPNow6_WD;
static unsigned int ESPNow7_WD;
static unsigned int ESPNow8_WD;

// empfangene Daten (max. 256 bytes!)
typedef struct struct_message {
  unsigned char modul_Nr;
  float U_Lipo;
  float Gyro_Lagewert;
  unsigned int U_Lipo_raw;
  // unsigned char type; // type 1 = angle measurement; type 2 = cg
} struct_ESPNowMessage;

// empfangene Daten (max. 256 bytes!)
typedef struct struct_cg {
  float U_Lipo;
  int Bat_Type;
  float Model_Weight;
  float Model_CG;
  float Model_CG_Trans;
  unsigned char type;  // type 1 = angle measurement; type 2 = cg
} struct_ESPNowMessage_CG;

struct_ESPNowMessage ESPNowData;
struct_ESPNowMessage_CG CGData;

unsigned int ESPNow_WD;

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  } else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL ");
    Serial.println(WiFi.channel());
  }
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  // char macStr[18];

  // disable interrupts?

  if (data_len == sizeof(struct_ESPNowMessage_CG)) {
    memcpy(&CGData, data, sizeof(struct_ESPNowMessage_CG));
    if (CGData.type == 2) {
      // Verarbeite Daten vom Typ 2 (CG-Daten)
      EspNowCGLastUpdate = millis();
      ESPNowData_CG_U_Lipo = CGData.U_Lipo;
      ESPNowData_CG_Bat_Type = CGData.Bat_Type;
      ESPNowData_CG_Model_Weight = CGData.Model_Weight;
      ESPNowData_CG_Model_CG = CGData.Model_CG;
      ESPNowData_CG_Model_CG_Trans = CGData.Model_CG_Trans;
    }
    return;
  }

  memcpy(&ESPNowData, data, sizeof(ESPNowData));

  switch (ESPNowData.modul_Nr) {
    case 1:
      ESPNowData_modul1_Nr = 1;
      ESPNowData_U1_Lipo = ESPNowData.U_Lipo;
      ESPNowData_Gyro1_Lagewert = ESPNowData.Gyro_Lagewert;
      ESPNow1_WD++;
      break;
    case 2:
      ESPNowData_modul2_Nr = 2;
      ESPNowData_U2_Lipo = ESPNowData.U_Lipo;
      ESPNowData_Gyro2_Lagewert = ESPNowData.Gyro_Lagewert;
      ESPNow2_WD++;
      break;
    case 3:
      ESPNowData_modul3_Nr = 3;
      ESPNowData_U3_Lipo = ESPNowData.U_Lipo;
      ESPNowData_Gyro3_Lagewert = ESPNowData.Gyro_Lagewert;
      ESPNow3_WD++;
      break;
    case 4:
      ESPNowData_modul4_Nr = 4;
      ESPNowData_U4_Lipo = ESPNowData.U_Lipo;
      ESPNowData_Gyro4_Lagewert = ESPNowData.Gyro_Lagewert;
      ESPNow4_WD++;
      break;
    case 5:
      ESPNowData_modul5_Nr = 5;
      ESPNowData_U5_Lipo = ESPNowData.U_Lipo;
      ESPNowData_Gyro5_Lagewert = ESPNowData.Gyro_Lagewert;
      ESPNow5_WD++;
      break;
    case 6:
      ESPNowData_modul6_Nr = 6;
      ESPNowData_U6_Lipo = ESPNowData.U_Lipo;
      ESPNowData_Gyro6_Lagewert = ESPNowData.Gyro_Lagewert;
      ESPNow6_WD++;
      break;
    case 7:
      ESPNowData_modul7_Nr = 7;
      ESPNowData_U7_Lipo = ESPNowData.U_Lipo;
      ESPNowData_Gyro7_Lagewert = ESPNowData.Gyro_Lagewert;
      ESPNow7_WD++;
      break;
    case 8:
      ESPNowData_modul8_Nr = 8;
      ESPNowData_U8_Lipo = ESPNowData.U_Lipo;
      ESPNowData_Gyro8_Lagewert = ESPNowData.Gyro_Lagewert;
      ESPNow8_WD++;
      break;
    default:
      Serial.printf("modulX??\n");
      break;
  }

  ESPNow_WD++;  // kein ESPNow angemeldet?
  // Serial.printf ("Modul_%d:  Lipo %5.2fV (raw: %d)  Winkel %5.2f˚\n", ESPNowData.modul_Nr,  ESPNowData.U_Lipo, ESPNowData.U_Lipo_raw, ESPNowData.Gyro_Lagewert);

  // enable interrupts
}
