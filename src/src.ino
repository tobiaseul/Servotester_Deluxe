/*
   Servotester Deluxe
   Ziege-One (Der RC-Modelbauer)
   https://www.youtube.com/watch?v=YNLPCft2qjg&list=PLS6SFYu711FpxCNO_j4_ig4ey0NwVQPcW

   Modified by TheDIYGuy999
   https://www.youtube.com/channel/UCqWO3PNCSjHmYiACDMLr23w

   It is recommended to use VS Code instead of Arduino IDE, because board and library management is way easier this way!
   Requirements:
   - git is installed (allows to download libraries and boards automatically): https://git-scm.com/downloads
   - PlatformIO plugin is installed in VS Code
   - Espressif32 platform is up to date in Platformio > Platforms > Updates

   If you are using Arduino IDE: Select the following board: "ESP32 Dev Module"

 ESP32 + Encoder + OLED

 /////Pin Belegung////
 GPIO 4: active piezo buzzer

 GPIO 13: Servo1
 GPIO 14: Servo2
 GPIO 27: Servo3
 GPIO 33: Servo4
 GPIO 32: Servo5 & BUS Input
 GPIO 15: Ecoder Taster
 GPIO 16: Ecoder Richtung 1
 GPIO 17: Ecoder Richtung 2

 GPIO 21: SDA OLED
 GPIO 22: SDL OLED
 */

char codeVersion[] = "0.17.0";  // Software revision.
//
// =======================================================================================================
// ! ! I M P O R T A N T ! !   ALL USER SETTINGS ARE DONE IN THE FOLLOWING TABS, WHICH ARE DISPLAYED ABOVE
// (ADJUST THEM BEFORE CODE UPLOAD), DO NOT CHANGE ANYTHING IN THIS TAB
// =======================================================================================================
//

// All the required user settings are done in the following .h files:
#include "0_generalSettings.h"  // <<------- general settings

//
// =======================================================================================================
// LIRBARIES & HEADER FILES, REQUIRED ESP32 BOARD DEFINITION
// =======================================================================================================
//

// Libraries (you have to install all of them in the "Arduino sketchbook"/libraries folder)
// !! Do NOT install the libraries in the sketch folder.
// No manual library download is required in Visual Studio Code IDE (see platformio.ini)

/* Boardversion
ESP32                                         2.0.5 or 2.0.6 <<--- (make sure your Espressif32 platform is up to date in Platformio > Platforms > Updates)
 */

/* Required Libraries / Benötigte Bibliotheken
ESP32Encoder                                  0.10.1
IBusBM                                        1.1.5
ESP8266 and ESP OLED driver SSD1306 displays  4.3.0
ESP32AnalogRead                               0.2.1
Array                                         1.0.0
 */

#include <Array.h>            //https://github.com/TheDIYGuy999/arduino-array
#include <ESP32AnalogRead.h>  // https://github.com/madhephaestus/ESP32AnalogRead <<------- required for calibrated battery voltage measurement
#include <ESP32Encoder.h>     // https://github.com/madhephaestus/ESP32Encoder/archive/refs/tags/0.10.1.tar.gz
#include <IBusBM.h>           // https://github.com/bmellink/IBusBM/archive/refs/tags/v1.1.5.tar.gz
#include <SH1106Wire.h>       //1.3"
#include <SSD1306Wire.h>      //0.96" https://github.com/ThingPulse/esp8266-oled-ssd1306/archive/refs/tags/4.3.0.tar.gz

#include "defaults.h"

// No need to install these, they come with the ESP32 board definition
#include <EEPROM.h>  // for non volatile storage
#include <Esp.h>     // for displaying memory information
#include <WiFi.h>
#include <esp_now.h>
#include <soc/sens_reg.h>     // for custom ADC function
#include <soc/sens_struct.h>  // for custom ADC function

#include <string>  // std::string, std::stof
#include <vector>

#include "driver/mcpwm.h"  // for servo PWM output
#include "rom/rtc.h"       // for displaying reset reason
#include "soc/rtc_wdt.h"   // for watchdog timer
using namespace std;

// Project specific includes
#if defined ALTERNATIVE_LOGO
#include "src/images2.h"  // Alternative Logo
#else
#if defined WM_LOGO
#include "src/images3.h"  // WM Logo
#else
#if defined LOGO4
#include "src/images4.h"  // WM Logo
#else
#include "src/images.h"  // Startlogo Der RC Modellbauer
#endif
#endif
#endif

#include "src/languages.h"  // Menu language ressources
#include <sbus.h>       // For SBUS interface

// EEPROM
#define EEPROM_SIZE 115

int RESET_EEPROM;  // WIFI 1 = Reset 0 = No Reset

#define adr_eprom_WIFI_ON 0                      // WIFI 1 = Ein 0 = Aus
#define adr_eprom_SERVO_STEPS 4                  // Deprecated, calculated automaticallly
#define adr_eprom_SERVO_MAX 8                    // Deprecated, controlled by servoModes.h
#define adr_eprom_SERVO_MIN 12                   // Deprecated, controlled by servoModes.h
#define adr_eprom_SERVO_CENTER 16                // Deprecated, controlled by servoModes.h
#define adr_eprom_SERVO_Hz 20                    // Deprecated, controlled by servoModes.h
#define adr_eprom_POWER_SCALE 24                 // Skalierung für Akkuspannungs-Messung
#define adr_eprom_SBUS_INVERTED 28               // SBUS inverted
#define adr_eprom_ENCODER_INVERTED 32            // Encoder inverted
#define adr_eprom_LANGUAGE 36                    // Gewählte Sprache
#define adr_eprom_PONG_BALL_RATE 40              // Pong Ball Geschwindigkeit
#define adr_eprom_SERVO_MODE 44                  // Servo operation mode
#define adr_eprom_SERVO_MAX_STD 48               // SERVO µs Max Wert im Servotester Modus (Standard)
#define adr_eprom_SERVO_MIN_STD 52               // SERVO µs Min Wert im Servotester Modus (Standard)
#define adr_eprom_SERVO_CENTER_STD 56            // SERVO µs Mitte Wert im Servotester Modus (Standard)
#define adr_eprom_SERVO_MAX_SANWA 60             // SERVO µs Max Wert im Servotester Modus (Sanwa)
#define adr_eprom_SERVO_MIN_SANWA 64             // SERVO µs Min Wert im Servotester Modus (Sanwa)
#define adr_eprom_SERVO_CENTER_SANWA 68          // SERVO µs Mitte Wert im Servotester Modus (Sanwa)
#define adr_eeprom_RUDDER_LENGTH_1 72            // rudder length 1
#define adr_eeprom_RUDDER_LENGTH_2 76            // rudder length 2
#define adr_eeprom_RUDDER_LENGTH_3 80            // rudder length 3
#define adr_eeprom_RUDDER_LENGTH_4 84            // rudder length 4
#define adr_eeprom_SHOW_ANGLE 88                 // Winkel-Anzeige (anstatt Ruderausschlag-Anzeige)
#define adr_eeprom_DISPLAY_MANUAL_ON_STARTUP 92  // WIFI 1 = Ein 0 = Aus

// Eine Adresse pro Menüpunkt; aktuell 15
#define adr_eeprom_menuEnabled_0 100
#define adr_eeprom_menuEnabled_1 101
#define adr_eeprom_menuEnabled_2 102
#define adr_eeprom_menuEnabled_3 103
#define adr_eeprom_menuEnabled_4 104
#define adr_eeprom_menuEnabled_5 105
#define adr_eeprom_menuEnabled_6 106
#define adr_eeprom_menuEnabled_7 107
#define adr_eeprom_menuEnabled_8 108
#define adr_eeprom_menuEnabled_9 109
#define adr_eeprom_menuEnabled_10 110
#define adr_eeprom_menuEnabled_11 111
#define adr_eeprom_menuEnabled_12 112
#define adr_eeprom_menuEnabled_13 113
#define adr_eeprom_menuEnabled_14 114

// EEPROM Speicher der Einstellungen
int WIFI_ON;             // WIFI 1 = Ein 0 = Aus
int SERVO_STEPS;         // Deprecated, calculated automaticallly
int SERVO_MAX;           // Deprecated, controlled by servoModes.h
int SERVO_MIN;           // Deprecated, controlled by servoModes.h
int SERVO_CENTER;        // Deprecated, controlled by servoModes.h
int SERVO_Hz;            // Deprecated, controlled by servoModes.h
int POWER_SCALE;         // Skalierung für Akkuspannungs-Messung
int SBUS_INVERTED;       // SBUS inverted
int ENCODER_INVERTED;    // Encoder inverted
int LANGUAGE;            // Gewählte Sprache
int PONG_BALL_RATE;      // Pong Ball Geschwindigkeit
int SERVO_MODE;          // New servo parameters starting here
int SERVO_MAX_STD;       // SERVO µs Max Wert im Servotester Modus (Standard)
int SERVO_MIN_STD;       // SERVO µs Min Wert im Servotester Modus (Standard)
int SERVO_CENTER_STD;    // SERVO µs Mitte Wert im Servotester Modus (Standard)
int SERVO_MAX_SANWA;     // SERVO µs Max Wert im Servotester Modus (Sanwa)
int SERVO_MIN_SANWA;     // SERVO µs Min Wert im Servotester Modus (Sanwa)
int SERVO_CENTER_SANWA;  // SERVO µs Mitte Wert im Servotester Modus (Sanwa)
int RUDDER_LENGTH_1;     // rudder length 1
int RUDDER_LENGTH_2;     // rudder length 2
int RUDDER_LENGTH_3;     // rudder length 3
int RUDDER_LENGTH_4;     // rudder length 4
int SHOW_ANGLE;          // Winkel- oder Ruderausschlag-Anzeige
bool DISPLAY_MANUAL_ON_STARTUP;

bool WiFiChanged;
bool SettingChanged;

// Encoder + button
ESP32Encoder encoder;

#define SHOW_BACK
#define BUTTON_PIN 15     // Hardware Pin Button
#define ENCODER_PIN_1 16  // Hardware Pin1 Encoder
#define ENCODER_PIN_2 17  // Hardware Pin2 Encoder
long prev1 = 0;           // Zeitspeicher für Taster
long prev2 = 0;           // Zeitspeicher für Taster
#ifdef SHOW_BACK
long prev3 = 0;  // Zeitspeicher für Taster
bool zurueck = 0;
#endif
long previousDebouncTime = 0;  // Speicher Entprellzeit für Taster
int buttonState = 0;           // 0 = Taster nicht betätigt; 1 = Taster langer Druck; 2 = Taster kurzer Druck; 3 = Taster Doppelklick
int encoderState = 0;          // 1 = Drehung nach Links (-); 2 = Drehung nach Rechts (+)
int encoderStateOld = 0;
int Duration_long = 400;    // Zeit für langen Druck
int Duration_double = 200;  // Zeit für Doppelklick
int bouncing = 50;          // Zeit für Taster Entprellung
int encoder_last;           // Speicher letzer Wert Encoder
int encoder_read;           // Speicher aktueller Wert Encoder
int encoderSpeed;           // Speicher aktuelle Encoder Geschwindigkeit für Beschleunigung
bool disableButtonRead;     // In gewissen Situationen soll der Encoder Button nicht von der blockierenden Funktion gelesen werden

bool buttonUsedForNavigation = false;
// 3 pin connectors, used as input and output
#define SERVO_CONNECTOR_1 13
#define SERVO_CONNECTOR_2 14
#define SERVO_CONNECTOR_3 27
#define SERVO_CONNECTOR_4 33
#define SERVO_CONNECTOR_5 32

// Servo
uint8_t servopin[5] = {SERVO_CONNECTOR_1, SERVO_CONNECTOR_2, SERVO_CONNECTOR_3, SERVO_CONNECTOR_4, SERVO_CONNECTOR_5};  // Pins Servoausgang 1 - 5
int servo_pos[5];                                                                                                       // Speicher für Servowerte
int selectedServo = 0;                                                                                                  // Das momentan angesteuerte Servo
int selectedServoOld = 0;
String servoMode = "";  // Servo operation mode text
int servo_posOld;

// Servo operation modes (see servoModes.h)
enum {
  STD,  // Std = 50Hz   1000 - 1500 - 2000µs = gemäss ursprünglichem Standard
  NOR,  // NOR = 100Hz  1000 - 1500 - 2000µs = normal = für die meisten analogen Servos
  SHR,  // SHR = 333Hz  1000 - 1500 - 2000µs = Sanwa High Response = für alle Digitalservos
  SSR,  // SSR = 400Hz   130 -  300 - 470µs  = Sanwa Super Response = nur für Sanwa Servos der SRG-Linie
  SUR,  // SUR = 800Hz   130 -  300 - 470µs  = Sanwa Ultra Response
  SXR   // SXR = 1600Hz  130 -  300 - 470µs  = Sanwa Xtreme Response
};

// Sound
#define BUZZER_PIN 4  // Active 3V buzzer
int beepDuration;     // how long the beep will be
bool playTone = 1;

// Oscilloscope pin
#define OSCILLOSCOPE_PIN 32  // ADC 1 pin only! Don't change it, oscilloscope is hardcoded!

// Serial command pins for SBUS, IBUS -----
#define COMMAND_RX 32  // pin 13
#define COMMAND_TX -1  // -1 is just a dummy

// SBUS
bfs::SbusRx sBus(&Serial2, COMMAND_RX, COMMAND_TX, SBUS_INVERTED);

// IBUS
IBusBM IBus;  // IBus object

// Externe Spannungsversorgung
#define BATTERY_DETECT_PIN 36   // Hardware Pin Externe Spannung in. ADC channel 2 only!
bool batteryDetected;           // Akku vorhanden
int numberOfBatteryCells;       // Akkuzellen
float batteryVoltage;           // Akkuspannung in V
float batteryChargePercentage;  // Akkuspannung in Prozent

// Menüstruktur
/*
 * 1 = Servotester_Auswahl       Auswahl -> 51 Servotester_Menu
 * 2 = Automatik_Modus_Auswahl   Auswahl -> 52 Automatik_Modus_Menu
 * 3 = Impuls_lesen_Auswahl      Auswahl -> 53 Impuls_lesen_Menu
 * 4 = Multiswitch_lesen_Auswahl Auswahl -> 54 Multiswitch_lesen_Menu
 * 5 = SBUS_lesen_Auswahl        Auswahl -> 55 SBUS_lesen_Menu
 * 6 = Einstellung_Auswahl       Auswahl -> 56 Einstellung_Menu
 * etc.
 */
// enum
// {
//   Servotester_Auswahl = 1,
//   Automatik_Modus_Auswahl = 2,
//   Winkelmessung_Auswahl = 3,
//   EWD_Auswahl = 4,
//   CG_Scale_Auswahl = 5,
//   Impuls_lesen_Auswahl = 6,
//   Multiswitch_lesen_Auswahl = 7,
//   SBUS_lesen_Auswahl = 8,
//   IBUS_lesen_Auswahl = 9,
//   Oscilloscope_Auswahl = 10,
//   SignalGenerator_Auswahl = 11,
//   Rechner_Auswahl = 12,
//   Pong_Auswahl = 13,
//   Flappy_Birds_Auswahl = 14,
//   Einstellung_Auswahl = 15,
//   //
//   Servotester_Menu = 51,
//   Automatik_Modus_Menu = 52,
//   Winkelmessung_Menu = 53,
//   EWD_Menu = 54,
//   CG_Scale_Menu = 55,
//   Impuls_lesen_Menu = 56,
//   Multiswitch_lesen_Menu = 57,
//   SBUS_lesen_Menu = 58,
//   IBUS_lesen_Menu = 59,
//   Oscilloscope_Menu = 60,
//   SignalGenerator_Menu = 61,
//   Rechner_Menu = 62,
//   Flappy_Birds_Menu = 63,
//   Pong_Menu = 64,
//   Einstellung_Menu = 65
// };

struct MenuEntry {
  String key;     // Key für Menüeinträge
  String name;    // Name des Menüeintrags
  int auswahlId;  // ID für "Auswahl"
  int menuId;     // ID für das zugehörige Menü
  bool enabled;   // Status: Aktiviert oder deaktiviert
};

// Dynamische Menüstruktur
std::vector<MenuEntry> menuEntries = {
    {"menu_servo_tester", "Servotester", 1, 51, true},
    {"menu_auto_mode", "Automatik_Modus", 2, 52, true},
    {"menu_angle_measurement", "Winkelmessung", 3, 53, true},
    {"menu_ewd", "EWD", 4, 54, true},
    {"menu_cg_scale", "CG_Scale", 5, 55, true},
    {"menu_pulse_read", "Impuls_lesen", 6, 56, true},
    {"menu_multiswitch_read", "Multiswitch_lesen", 7, 57, true},
    {"menu_sbus_read", "SBUS_lesen", 8, 58, true},
    {"menu_ibus_read", "IBUS_lesen", 9, 59, true},
    {"menu_oscilloscope", "Oscilloscope", 10, 60, true},
    {"menu_signal_generator", "SignalGenerator", 11, 61, true},
    {"menu_calculator", "Rechner", 12, 62, true},
    {"menu_pong", "Pong", 13, 63, true},
    {"menu_flappy_birds", "Flappy_Birds", 14, 64, true},
    {"menu_settings", "Einstellung", 15, 65, true}};

String getMenuEntryName(const MenuEntry &entry) {
  if (menuKeyMap.find(entry.key) != menuKeyMap.end()) {
    return menuKeyMap[entry.key][LANGUAGE];
  }
  return entry.key;
}

//-Menu 52 Automatik Modus
int Autopos[5];       // Speicher
bool Auto_Pause = 0;  // Pause im Auto Modus

//-Menu 54 Impuls lesen
int Impuls_min = 1000;

int Impuls_max = 2000;
int pwmFreq = 0;

//-Menu 55 Multiwitch Futaba lesen
#define kanaele 9     // Anzahl der Multiswitch Kanäle
int value1[kanaele];  // Speicher Multiswitch Werte
//-Menu
int Menu = 1;  // Aktives Menu
// int Menu = CG_Scale_Menu; // ONLY FOR DEBUG
// int MenuOld = Menu;

int SetupSubmenuVisibilityIndex = 0;

bool SetupMenu = false;  // Zustand Setupmenu
int Einstellung = 5;     // Aktives Einstellungsmenu
int EinstellungOld = 0;
bool Edit = false;  // Einstellungen ausgewählt

// Battery voltage
ESP32AnalogRead battery;

// OLED
#ifdef OLED1306
SSD1306Wire display(0x3c, SDA, SCL);  // Oled Hardware an SDA 21 und SCL 22
#else
SH1106Wire display(0x3c, SDA, SCL);  // Oled Hardware an SDA 21 und SCL 22
#endif

// Webserver auf Port 80
WiFiServer server(80);

// Speicher HTTP request
String header;

// Für HTTP GET value
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;

// These are used to print the reset reason on startup
const char *RESET_REASONS[] = {"POWERON_RESET", "NO_REASON", "SW_RESET", "OWDT_RESET", "DEEPSLEEP_RESET", "SDIO_RESET", "TG0WDT_SYS_RESET", "TG1WDT_SYS_RESET", "RTCWDT_SYS_RESET", "INTRUSION_RESET", "TGWDT_CPU_RESET", "SW_CPU_RESET", "RTCWDT_CPU_RESET", "EXT_CPU_RESET", "RTCWDT_BROWN_OUT_RESET", "RTCWDT_RTC_RESET"};

unsigned long currentTime = millis();      // Aktuelle Zeit
unsigned long currentTimeAuto = millis();  // Aktuelle Zeit für Auto Modus
unsigned long currentTimeSpan = millis();  // Aktuelle Zeit für Externe Spannung
unsigned long previousTime = 0;            // Previous time
unsigned long previousTimeAuto = 0;        // Previous time für Auto Modus
unsigned long previousTimeSpan = 0;        // Previous time für Externe Spannung
int TimeAuto = 50;                         // Auto time
int TimeAutoOld;
const long timeoutTime = 2000;  // Define timeout time in milliseconds (example: 2000ms = 2s)

//
// =======================================================================================================
// SUB FUNCTIONS & ADDITIONAL HEADERS
// =======================================================================================================
//

// Map as Float --------------------------------------------------------------------------------
float map_float(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Convert µs to degrees (°)
float us2degree(uint16_t value) {
  return map_float(float(value), float(SERVO_MIN), float(SERVO_MAX), -45.0, 45.0);
}

// buzzer control ------------------------------------------------------------------------------
void beep() {
  static unsigned long buzzerTriggerMillis;
  if (beepDuration > 0 && !digitalRead(BUZZER_PIN)) {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerTriggerMillis = millis();
  }

  if (millis() - buzzerTriggerMillis >= beepDuration) {
    digitalWrite(BUZZER_PIN, LOW);
    beepDuration = 0;
  }
}

// Loop time measurement -----------------------------------------------------------------------
unsigned long loopDuration() {
  static unsigned long timerOld;
  unsigned long loopTime;
  unsigned long timer = millis();
  loopTime = timer - timerOld;
  timerOld = timer;
  return loopTime;
}

// Frequency counter ---------------------------------------------------------------------------
#define WAIT_FOR_PIN_STATE(state)                                         \
  while (digitalRead(pin) != (state)) {                                   \
    if (cpu_hal_get_cycle_count() - start_cycle_count > timeout_cycles) { \
      return 0;                                                           \
    }                                                                     \
  }

unsigned long readFreq(uint8_t pin, uint8_t state, unsigned long timeout) {
  const uint32_t max_timeout_us = clockCyclesToMicroseconds(UINT_MAX);
  if (timeout > max_timeout_us) {
    timeout = max_timeout_us;
  }
  const uint32_t timeout_cycles = microsecondsToClockCycles(timeout);
  const uint32_t start_cycle_count = cpu_hal_get_cycle_count();
  WAIT_FOR_PIN_STATE(!state);
  WAIT_FOR_PIN_STATE(state);                                           // Signal going high
  const uint32_t pulse_start_cycle_count = cpu_hal_get_cycle_count();  // Store start time
  WAIT_FOR_PIN_STATE(!state);                                          // Signal going low
  WAIT_FOR_PIN_STATE(state);                                           // Signal going high again

  return 1000000 / (clockCyclesToMicroseconds(cpu_hal_get_cycle_count() - pulse_start_cycle_count));
}

// Super fast analogRead() alternative ---------------------------------------------------------
// See: https://www.toptal.com/embedded/esp32-audio-sampling
#ifdef FAST_ADC
int IRAM_ATTR local_adc1_read(int channel) {
  uint16_t adc_value;
  SENS.sar_meas_start1.sar1_en_pad = (1 << channel);  // only one channel is selected
  while (SENS.sar_slave_addr1.meas_status != 0);
  SENS.sar_meas_start1.meas1_start_sar = 0;
  SENS.sar_meas_start1.meas1_start_sar = 1;
  while (SENS.sar_meas_start1.meas1_done_sar == 0);
  adc_value = SENS.sar_meas_start1.meas1_data_sar;
  return adc_value;
}
#endif

#include "src/espnow_dat.h"  // ESPNow required for winkel/ewd and cg scale. Must be includes before the following includes

// Additional headers --------------------------------------------------------------------------
#include "src/calculator.h"       // A handy calculator
#include "src/cgscale.h"          // Anzeige der Daten einer Schwerpunktwaage, Übertragung per ESPNow
#include "src/flappyBirds.h"      // A little flappy birds game :-)
#include "src/oscilloscope.h"     // A handy oscilloscope
#include "src/pong.h"             // A little pong game :-)
#include "src/servoModes.h"       // Servo operation profiles
#include "src/signalGenerator.h"  // A handy signal generator
#include "src/systemImages.h"     // Symbols
#include "src/webInterface.h"     // Configuration website
#include "src/winkelmessung.h"    // Ruderausschlag Messung mit MPU6050, Übertragung per ESPNow
//
// =======================================================================================================
// mcpwm unit SETUP for servos (1x during startup)
// =======================================================================================================
//
// See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/mcpwm.html#configure

void setupMcpwm() {
  // Unit 0 ---------------------------------------------------------------------
  // 1. set our servo 1 - 4 output pins
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_CONNECTOR_1);  // Set steering as PWM0A
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, SERVO_CONNECTOR_2);  // Set shifting as PWM0B
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, SERVO_CONNECTOR_3);  // Set coupling as PWM1A
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, SERVO_CONNECTOR_4);  // Set winch  or beacon as PWM1B

  // 2. configure MCPWM parameters
  mcpwm_config_t pwm_config;
  pwm_config.frequency = SERVO_Hz;  // frequency
  pwm_config.cmpr_a = 0;            // duty cycle of PWMxa = 0
  pwm_config.cmpr_b = 0;            // duty cycle of PWMxb = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;  // 0 = not inverted, 1 = inverted

  // 3. configure channels with settings above
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);  // Configure PWM0A & PWM0B
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);  // Configure PWM1A & PWM1B

  // Unit 1 ---------------------------------------------------------------------
  // 1. set our servo 5 output pin
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, SERVO_CONNECTOR_5);  // Set ESC as PWM0A

  // 2. configure MCPWM parameters
  mcpwm_config_t pwm_config1;
  pwm_config1.frequency = SERVO_Hz;  // frequency
  pwm_config1.cmpr_a = 0;            // duty cycle of PWMxa = 0
  pwm_config1.cmpr_b = 0;            // duty cycle of PWMxb = 0
  pwm_config1.counter_mode = MCPWM_UP_COUNTER;
  pwm_config1.duty_mode = MCPWM_DUTY_MODE_0;  // 0 = not inverted, 1 = inverted

  // 3. configure channels with settings above
  mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config1);  // Configure PWM0A & PWM0B
}

//
// =======================================================================================================
// WiFi SETUP
// =======================================================================================================
//

void waitWiFiconnected() {
  long timeoutWiFi = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      Serial.print("\nWifi: No SSID available");
      break;
    } else if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.print("\nWifi: Connection failed");
      break;
    } else if ((millis() - timeoutWiFi) > TIMEOUT_CONNECT) {
      Serial.print("\nWifi: Timeout");
      break;
    }
  }
}

void wifiSetup(bool displayWifiStartupOnDisplay = true) {
  if (WIFI_ON == 1) {  // Wifi Ein
    // Print local IP address and start web server
    // Serial.println(connectingAccessPointString[LANGUAGE]);
    // WiFi.mode(WIFI_STA);
    // WiFi.softAP(ssid, password);

    // IPAddress IP = WiFi.softAPIP();
    // Serial.print(apIpAddressString[LANGUAGE]);
    // Serial.println(IP);

    // Serial.println("Wifi: STA mode - connecting with: " + String(SSID_STA));

    // // Start by connecting to a WiFi network
    // WiFi.persistent(false);
    // WiFi.mode(WIFI_STA);
    // WiFi.begin(SSID_STA, PASSWORD_STA);

    // waitWiFiconnected();

    // if (WiFi.status() != WL_CONNECTED)
    // {
    // if WiFi not connected, switch to access point mode
    // wifiSTAmode = false;
    Serial.print("Wifi: AP mode - create access point: " + String(SSID_AP));
    WiFi.mode(WIFI_STA);
    // WiFi.mode(WIFI_AP);
    // WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(SSID_AP, PASSWORD_AP);
    Serial.print("Wifi: Connected, IP: " + String(WiFi.softAPIP().toString()));
    // }
    // else
    // {
    //   Serial.print("Wifi: Connected, IP: " + String(WiFi.localIP().toString()));
    // }

    // Set Hostname
    String hostname = "disabled";
#if ENABLE_MDNS
    hostname = device_Name;
    hostname.replace(" ", "");
    hostname.toLowerCase();
    if (!MDNS.begin(hostname, WiFi.localIP())) {
      hostname = "mDNS failed";
      printConsole(T_ERROR, "Wifi: " + hostname);
    } else {
      hostname += ".local";
      printConsole(T_RUN, "Wifi hostname: " + hostname);
    }
#endif

    digitalWrite(BUZZER_PIN, LOW);  // Buzzer off

    if (displayWifiStartupOnDisplay) {
      // Show IP address
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
#ifdef SHOW_SMALLER_US
      display.setFont(ArialMT_Plain_10);
#else
      display.setFont(ArialMT_Plain_16);
#endif
      display.drawString(0, 10, WiFiOnString[LANGUAGE]);
      display.drawString(0, 26, ipAddressString[LANGUAGE]);
      display.drawString(0, 42, WiFi.softAPIP().toString());
      display.display();
      delay(1500);

      // Show SSID & Password
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
#ifdef SHOW_SMALLER_US
      display.setFont(ArialMT_Plain_10);
#else
      display.setFont(ArialMT_Plain_16);
#endif
      display.drawString(0, 0, "SSID: ");
      display.drawString(0, 16, ssid);
      display.drawString(0, 32, passwordString[LANGUAGE]);
      display.drawString(0, 48, password);
      display.display();
      delay(2000);
    }
    Serial.printf("\nWiFi Tx Power Level: %u", WiFi.getTxPower());
    WiFi.setTxPower(cpType);  // WiFi and ESP-Now power according to "0_generalSettings.h"
    Serial.printf("\nWiFi Tx Power Level changed to: %u\n\n", WiFi.getTxPower());

    server.begin();  // Start Webserver
  }

  // WiFi off
  else {
    server.end();
    WiFi.mode(WIFI_OFF);
    Serial.println("");
    Serial.println(WiFiOffString[LANGUAGE]);

    digitalWrite(BUZZER_PIN, LOW);  // Buzzer off

    if (displayWifiStartupOnDisplay) {
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      display.drawString(0, 10, "WiFi");
      display.drawString(0, 26, offString[LANGUAGE]);
      display.display();
      delay(700);
    }
  }
}

//
// =======================================================================================================
// MAIN ARDUINO SETUP (1x during startup)
// =======================================================================================================
//
void setup() {
  // Watchdog timers need to be disabled, if task 1 is running without delay(1)
  // disableCore0WDT();

  // Serial setup
  Serial.begin(115200);  // USB serial (for DEBUG)

  // Print some system and software info to serial monitor
  delay(1000);  // Give serial port/connection some time to get ready
  Serial.printf("\n**************************************************************************************************\n");
  Serial.printf("Sevotester Deluxe for ESP32 software version %s\n", codeVersion);
  Serial.printf("Original version: https://github.com/Ziege-One/Servotester_Deluxe\n");
  Serial.printf("Modified version: https://github.com/TheDIYGuy999/Servotester_Deluxe\n");
  Serial.printf("Modded Modified version: https://github.com/shockyfan/Servotester_Deluxe\n");
  Serial.printf("XTAL Frequency: %i MHz, CPU Clock: %i MHz, APB Bus Clock: %i Hz\n", getXtalFrequencyMhz(), getCpuFrequencyMhz(), getApbFrequency());
  Serial.printf("Internal RAM size: %i Byte, Free: %i Byte\n", ESP.getHeapSize(), ESP.getFreeHeap());
  Serial.printf("WiFi MAC address: %s\n", WiFi.macAddress().c_str());
  for (uint8_t coreNum = 0; coreNum < 2; coreNum++) {
    uint8_t resetReason = rtc_get_reset_reason(coreNum);
    if (resetReason <= (sizeof(RESET_REASONS) / sizeof(RESET_REASONS[0]))) {
      Serial.printf("Core %i reset reason: %i: %s\n", coreNum, rtc_get_reset_reason(coreNum), RESET_REASONS[resetReason - 1]);
    }
  }

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);

  eepromRead();  // Read EEPROM

  eepromInit();  // Initialize EEPROM (store defaults, if new or erased EEPROM is detected)

  // Setup Encoder
  ESP32Encoder::useInternalWeakPullResistors = UP;
  encoder.attachHalfQuad(ENCODER_PIN_1, ENCODER_PIN_2);
  encoder.setFilter(1023);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // BUTTON_PIN = Eingang

  // Speaker setup
  pinMode(BUZZER_PIN, OUTPUT);

  // Battery
  battery.attach(BATTERY_DETECT_PIN);

  tone(BUZZER_PIN, 4000, 50);

  // Setup OLED
  display.init();
  display.flipScreenVertically();

  // Show logo
  display.setFont(ArialMT_Plain_10);
  display.drawXbm(0, 0, Logo_width, Logo_height, Logo_bits);
  display.display();
  delay(1250);

  // Show software version
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 10, "Software Version:");
  display.drawString(64, 26, String(codeVersion));
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 45, "ESPNow Edition");
  display.display();
  delay(1250);

  // Show manual
  if (DISPLAY_MANUAL_ON_STARTUP) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, operationString[LANGUAGE]);
    display.drawString(0, 12, shortPressString[LANGUAGE]);
    display.drawString(0, 24, longPressString[LANGUAGE]);
    display.drawString(0, 36, doubleclickString[LANGUAGE]);
    display.drawString(0, 48, RotateKnobString[LANGUAGE]);
    display.display();
    delay(4000);
  }

  wifiSetup();

  encoder.setCount(Menu);
  servo_pos[0] = 1500;
  servo_pos[1] = 1500;
  servo_pos[2] = 1500;
  servo_pos[3] = 1500;
  servo_pos[4] = 1500;

  beepDuration = 10;  // Short beep = device ready
  tone(BUZZER_PIN, 4500, 50);

  /*
    // Task 1 setup (running on core 0) TODO, testing only
    TaskHandle_t Task1;
    // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
        Task1code, // Task function
        "Task1",   // name of task
        8192,      // Stack size of task (8192)
        NULL,      // parameter of the task
        1,         // priority of the task (1 = low, 3 = medium, 5 = highest)
        &Task1,    // Task handle to keep track of created task
        0);        // pin task to core 0
        */
}

//
// =======================================================================================================
// ENCODER & BUTTON
// =======================================================================================================
//
void ButtonRead() {
  if (!disableButtonRead) {
    buttonState = 0;
    zurueck = 0;
    if (!(digitalRead(BUTTON_PIN))) {  // Button gedrückt 0
      delay(bouncing);                 // Taster entprellen
      prev1 = millis();
      buttonState = 1;
      while ((millis() - prev1) <= Duration_long) {
        if (digitalRead(BUTTON_PIN)) {  // Button losgelassen 1 innerhalb Zeit
          delay(bouncing);              // Taster entprellen
          buttonState = 2;
          prev2 = millis();
          while ((millis() - prev2) <= Duration_double) {  // Doppelkick abwarten
            if (!(digitalRead(BUTTON_PIN))) {              // Button gedrückt 0 innerhalb Zeit Doppelklick
              delay(bouncing);                             // Taster entprellen
              buttonState = 3;
              if (digitalRead(BUTTON_PIN)) {  // Button losgelassen 1
                break;
              }
            }
          }
          break;
        }
      }
#ifdef SHOW_BACK
      prev3 = millis();
      while (!(digitalRead(BUTTON_PIN))) {  // Warten bis Button nicht gedückt ist = 1
        // Serial.println("warten bis Taster losgelassen wird");
        if ((millis() - prev3) > Duration_long) {
          // Serial.println("Zeit abgelaufen");
          if (playTone) {
            playTone = 0;
            tone(BUZZER_PIN, 4500, 20);
          }
          display.clear();
          display.setColor(WHITE);
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_24);
          display.drawString(64, 0, "< zurück");  // todo: languages
          display.setFont(ArialMT_Plain_16);
          display.drawString(64, 25, "");
          display.display();
          zurueck = 1;
          Edit = false;
          buttonState = 4;  // zurück
        }
      }
#else
      while (!(digitalRead(BUTTON_PIN))) {  // Warten bis Button nicht gedückt ist = 1
      }
#endif
      // Serial.printf("Buttonstate: %u\n", buttonState);

      if (buttonState == 1) {  //
        beepDuration = 20;
      }
      if (buttonState == 2) {  // einmal gedrückt
        beepDuration = 10;
        if (playTone) {
          playTone = 0;
          tone(BUZZER_PIN, 4000, 20);
        }
      }
      if (buttonState == 3) {  // doppelt gedrückt
        beepDuration = 30;
        if (playTone) {
          playTone = 0;
          tone(BUZZER_PIN, 4000, 20);
          delay(60);
          tone(BUZZER_PIN, 4000, 20);
        }
      }
      if (buttonState == 4) {  // zurück
        beepDuration = 20;
      }
    }
    playTone = 1;
  }

  // Encoder -------------------------------------------------------------------------------------------------
  encoder_read = encoder.getCount() / 2;  // Read encoder --------------

  if (previousDebouncTime + 10 > millis())  // Debouncing 10ms -------------
  {
    encoder_last = encoder_read;
  }

  static unsigned long encoderSpeedMillis;
  static int lastEncoderSpeed;

  if (millis() - encoderSpeedMillis > 150)  // Encoder speed detection (150 was 100) -----------------
  {
    encoderSpeedMillis = millis();
    encoderSpeed = abs(encoder_read - lastEncoderSpeed);
    encoderSpeed = constrain(encoderSpeed, 1, 15);  // (15 was 4)

    // Serial.println(encoderSpeed); // For encoder speed debuggging

    lastEncoderSpeed = encoder_read;
  }

  if (encoder_last > encoder_read)  // Left turn detected --------------
  {
    if (encoder_last > encoder_read) {
      if (ENCODER_INVERTED) {
        encoderState = 2;  // right
      } else {
        encoderState = 1;  // left
      }
      encoder_last = encoder_read;
      previousDebouncTime = millis();
    }
  } else if (encoder_last < encoder_read)  // Right turn detected --------------
  {
    if (encoder_last < encoder_read) {
      if (ENCODER_INVERTED) {
        encoderState = 1;  // left
      } else {
        encoderState = 2;  // right
      }
      encoder_last = encoder_read;
      previousDebouncTime = millis();
    }
  } else {
    encoderState = 0;
  }

  // Serial.printf ("encoderState_src: %d\n", encoderState);

  // // Bip bei Pin drehen
  // if (encoderState != encoderStateOld) {
  //   if (Menu != Servotester_Menu)
  //     tone (BUZZER_PIN, 4000, 2);
  //   encoderStateOld = encoderState;
  // }
}

void navigateBack() {
  const MenuEntry *entry = nullptr;
  for (const auto &menuEntry : menuEntries) {
    if (menuEntry.auswahlId == Menu || menuEntry.menuId == Menu) {
      entry = &menuEntry;
      break;
    }
  }

  if (entry == nullptr) {
    return;
  }

  Menu = entry->auswahlId;
}

void navigateMenu() {
  if (encoderState == 1 || encoderState == 2) {
    int increment = encoderState == 2;
    do {
      if (increment) {
        Menu++;
        if (Menu > menuEntries.size()) {
          for (size_t i = menuEntries.size(); i > 0; i--) {
            if (menuEntries[i - 1].enabled) {
              Menu = i;
              break;
            }
          }
        }
      } else {
        Menu--;
        if (Menu < 1) {
          for (size_t i = 0; i < menuEntries.size(); i++) {
            if (menuEntries[i].enabled) {
              Menu = i + 1;
              break;
            }
          }
        }
      }
    } while (!menuEntries[Menu - 1].enabled);
  }

  if (buttonState == 2) {
    Menu = menuEntries[Menu - 1].menuId;
    buttonUsedForNavigation = true;
  }
}

void checkButtonUsedForNavigation() {
  if (buttonUsedForNavigation && buttonState == 0) {
    buttonUsedForNavigation = false;
  }
}

//
// =======================================================================================================
// MENU
// =======================================================================================================
//
/*
 * 1 = Servotester       Auswahl -> 10 Servotester
 * 2 = Automatik Modus   Auswahl -> 20 Automatik Modus
 * 3 = Impuls lesen      Auswahl -> 30 Impuls lesen
 * 4 = Multiswitch lesen Auswahl -> 40 Multiswitch lesen
 * 5 = SBUS lesen        Auswahl -> 50 SBUS lesen
 * 6 = Einstellung       Auswahl -> 60 Einstellung
 */
void MenuUpdate() {
  const MenuEntry *entry = nullptr;
  for (const auto &menuEntry : menuEntries) {
    if (menuEntry.auswahlId == Menu || menuEntry.menuId == Menu) {
      entry = &menuEntry;
      break;
    }
  }

  if (entry == nullptr) {
    return;
  }

  // Servotester Auswahl *********************************************************
  if (entry->name == "Servotester" && entry->auswahlId == Menu) {
    servoModes();    // Refresh servo operation mode
    batteryVolts();  // Read battery voltage
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "  Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, servotesterString[LANGUAGE]);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Hz");
    display.drawString(0, 10, String(SERVO_Hz));
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 0, servoMode);
    drawWiFi();
    if (batteryDetected) {
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.drawString(30, 50, String(batteryVoltage));
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(30, 50, "V");

      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.drawString(64, 50, String(numberOfBatteryCells));
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(64, 50, "S");

      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.drawString(115, 50, String(batteryChargePercentage, 0));
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.drawString(115, 50, "%");
    } else {
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.drawString(64, 50, noBatteryString[LANGUAGE]);  // No battery
      display.setTextAlignment(TEXT_ALIGN_LEFT);
    }

    display.display();

    navigateMenu();
  }

  // Automatikmodus Auswahl *********************************************************
  if (entry->name == "Automatik_Modus" && entry->auswahlId == Menu) {
    servoModes();  // Refresh servo operation mode
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, automaticModeString[LANGUAGE]);
    display.drawString(64, 45, oscillateServoString[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Winkelmessung Auswahl *********************************************************
  if (entry->name == "Winkelmessung" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, angleModeString[LANGUAGE]);
    display.drawString(64, 45, rudderMeasureString[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // EWD Auswahl *********************************************************
  if (entry->name == "EWD" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, EWDModeString[LANGUAGE]);
    display.drawString(64, 45, EWDMeasureString[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // CG Scale Auswahl *********************************************************
  if (entry->name == "CG_Scale" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, CgScaleMenuString1[LANGUAGE]);
    display.drawString(64, 45, CgScaleMenuString2[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Impuls lesen Auswahl *********************************************************
  if (entry->name == "Impuls_lesen" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, PwmImpulseString[LANGUAGE]);
    display.drawString(64, 45, readCh1Ch5String[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Multiswitch lesen Auswahl *********************************************************
  if (entry->name == "Multiswitch_lesen" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, "PPM Multiswitch");
    display.drawString(64, 45, readCh5String[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // SBUS lesen Auswahl *********************************************************
  if (entry->name == "SBUS_lesen" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, readSbusString[LANGUAGE]);
    display.drawString(64, 45, "CH5");
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // IBUS lesen Auswahl *********************************************************
  if (entry->name == "IBUS_lesen" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, readIbusString[LANGUAGE]);
    display.drawString(64, 45, "CH5");
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Oszilloskop Auswahl *********************************************************
  if (entry->name == "Oscilloscope" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, readOscilloscopeString[LANGUAGE]);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 45, readOscilloscopeString2[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Signal Generator Auswahl *********************************************************
  if (entry->name == "SignalGenerator" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, signalGeneratorString[LANGUAGE]);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 45, signalGeneratorString2[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Rechner Auswahl *********************************************************
  if (entry->name == "Rechner" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, calculatorString[LANGUAGE]);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 45, "No Warranty Edition");
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Pong Auswahl *********************************************************
  if (entry->name == "Pong" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, "P  O  N  G");
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 45, "TheDIYGuy999 Edition");
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Flappy Birds Auswahl *********************************************************
  if (entry->name == "Flappy_Birds" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu >");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, "Flappy Birds");
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 45, "TheDIYGuy999 Edition");
    drawWiFi();
    display.display();

    disableButtonRead = false;  // Re enable regular button read function

    navigateMenu();
  }

  // Einstellung Auswahl *********************************************************
  // case Einstellung_Auswahl:
  if (entry->name == "Einstellung" && entry->auswahlId == Menu) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawString(64, 0, "< Menu  ");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 25, settingsString[LANGUAGE]);
    drawWiFi();
    display.display();

    navigateMenu();
  }

  // Untermenus ================================================================

  // Servotester *********************************************************
  if (entry->name == "Servotester" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 10, "Hz");
      display.drawString(0, 20, String(SERVO_Hz));
      display.drawString(0, 35, servoMode);
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.drawString(128, 20, "°");
      display.drawString(123, 20, String(us2degree(servo_pos[selectedServo]), 1));
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.setFont(ArialMT_Plain_24);
      display.drawString(64, 0, "Servo" + String(selectedServo + 1));
#ifdef SHOW_SMALLER_US
      display.setFont(ArialMT_Plain_16);
#endif
      display.drawString(64, 25, String(servo_pos[selectedServo]) + "µs");
      display.drawProgressBar(8, 50, 112, 10, (((servo_pos[selectedServo] - SERVO_MIN) * 100) / (SERVO_MAX - SERVO_MIN)));
      display.display();
      if (!SetupMenu) {
        servo_pos[0] = SERVO_CENTER;
        servo_pos[1] = SERVO_CENTER;
        servo_pos[2] = SERVO_CENTER;
        servo_pos[3] = SERVO_CENTER;
        servo_pos[4] = SERVO_CENTER;
        setupMcpwm();
        SetupMenu = true;
      }
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_pos[0]);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, servo_pos[1]);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, servo_pos[2]);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, servo_pos[3]);
      mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A, servo_pos[4]);

      if (encoderState == 1)  // Left turn
      {
        servo_pos[selectedServo] = servo_pos[selectedServo] - (SERVO_STEPS * encoderSpeed);  // Variable encoder speed
      }
      if (encoderState == 2)  // Right turn
      {
        servo_pos[selectedServo] = servo_pos[selectedServo] + (SERVO_STEPS * encoderSpeed);
      }

      if (servo_pos[selectedServo] > SERVO_MAX)  // Servo MAX
      {
        servo_pos[selectedServo] = SERVO_MAX;
      } else if (servo_pos[selectedServo] < SERVO_MIN)  // Servo MIN
      {
        servo_pos[selectedServo] = SERVO_MIN;
      }

      if (servo_pos[selectedServo] != servo_posOld) {
        servo_posOld = servo_pos[selectedServo];
        // tone (BUZZER_PIN, servo_posOld+2500, 5);  // 3500..4500
        tone(BUZZER_PIN, ::map(servo_posOld, 1000, 2000, 2500, 4500), 5);  // 2500..5500
      }
    }

    if (buttonState == 4)  // zurück?
    {
      navigateBack();
      SetupMenu = false;
      selectedServo = 0;
    }

    if (buttonState == 2) {
      servo_pos[selectedServo] = SERVO_CENTER;  // Servo Mitte
    }

    if (buttonState == 3) {
      selectedServo++;  // Servo +
    }

    if (selectedServo > 4) {
      selectedServo = 0;
    }
  }

  // Automatik Modus *********************************************************
  if (entry->name == "Automatik_Modus" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      static unsigned long autoMenuMillis;
      int autoChange;
      if (millis() - autoMenuMillis > 20) {  // Every 20ms (slow screen refresh down, servo movement is too slow otherwise!)
        autoMenuMillis = millis();
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 0, delayString[LANGUAGE]);
        display.drawString(0, 10, String(TimeAuto));
        display.drawString(0, 20, "ms");
        display.drawString(0, 35, servoMode);
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.drawString(128, 20, "°");
        display.drawString(123, 20, String(us2degree(servo_pos[selectedServo]), 1));
        display.drawString(128, 35, String(SERVO_Hz));
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setFont(ArialMT_Plain_24);
        display.drawString(64, 0, "Servo" + String(selectedServo + 1));
#ifdef SHOW_SMALLER_US
        display.setFont(ArialMT_Plain_16);
#endif
        display.drawString(64, 25, String(servo_pos[selectedServo]) + "µs");
        if (Auto_Pause) {
          display.setFont(ArialMT_Plain_16);
          display.drawString(64, 48, "Pause");
        } else {
          display.drawProgressBar(8, 50, 112, 10, (((servo_pos[selectedServo] - SERVO_MIN) * 100) / (SERVO_MAX - SERVO_MIN)));
        }
        display.display();
      }

      if (!SetupMenu) {
        servo_pos[0] = SERVO_CENTER;
        servo_pos[1] = SERVO_CENTER;
        servo_pos[2] = SERVO_CENTER;
        servo_pos[3] = SERVO_CENTER;
        servo_pos[4] = SERVO_CENTER;
        setupMcpwm();
        TimeAuto = 50;       // Zeit für SERVO Steps +-
        Auto_Pause = false;  // Pause aus
        SetupMenu = true;
      }

      currentTimeAuto = millis();
      if (!Auto_Pause) {
        if ((currentTimeAuto - previousTimeAuto) > TimeAuto) {
          previousTimeAuto = currentTimeAuto;
          if (Autopos[selectedServo] > ((SERVO_MIN + SERVO_MAX) / 2)) {
            servo_pos[selectedServo] = servo_pos[selectedServo] + SERVO_STEPS;
          } else {
            servo_pos[selectedServo] = servo_pos[selectedServo] - SERVO_STEPS;
          }
        }
      }

      if (servo_pos[selectedServo] < SERVO_MIN) {
        Autopos[selectedServo] = SERVO_MAX;
        servo_pos[selectedServo] = SERVO_MIN;
      }
      if (servo_pos[selectedServo] > SERVO_MAX) {
        Autopos[selectedServo] = SERVO_MIN;
        servo_pos[selectedServo] = SERVO_MAX;
      }

      autoChange = ::map(TimeAuto, 0, 100, 1, 10);  // Calculate adjustment step
      if (encoderState == 1) {
        if (Auto_Pause) {
          servo_pos[selectedServo] = servo_pos[selectedServo] - SERVO_STEPS;
        } else {
          TimeAuto -= autoChange;
        }
      }
      if (encoderState == 2) {
        if (Auto_Pause) {
          servo_pos[selectedServo] = servo_pos[selectedServo] + SERVO_STEPS;
        } else {
          TimeAuto += autoChange;
        }
      }

      if (Auto_Pause && (servo_pos[selectedServo] != servo_posOld)) {
        servo_posOld = servo_pos[selectedServo];
        tone(BUZZER_PIN, 4000, 2);
      }

      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_pos[0]);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, servo_pos[1]);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, servo_pos[2]);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, servo_pos[3]);
      mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A, servo_pos[4]);

      if (TimeAuto > 100)  // TimeAuto MAX
      {
        TimeAuto = 100;
      } else if (TimeAuto < 0)  // TimeAuto MIN
      {
        TimeAuto = 0;
      }

      if (selectedServo > 4) {
        selectedServo = 4;
      } else if (selectedServo < 0) {
        selectedServo = 0;
      }
    }

    if (TimeAuto != TimeAutoOld) {
      TimeAutoOld = TimeAuto;
      tone(BUZZER_PIN, ::map(TimeAutoOld, 100, 0, 2500, 4500), 5);
      delay(5);
    }

    if (buttonState == 4)  // Long press = back
    {
      navigateBack();
      SetupMenu = false;
      selectedServo = 0;
    }

    if (buttonState == 2)  // Short press = pause
    {
      Auto_Pause = !Auto_Pause;
    }

    if (buttonState == 3)  // Doubleclick = Change channel
    {
      selectedServo++;  // Servo +
    }

    if (selectedServo > 4) {
      selectedServo = 0;
    }
  }

  // Winkelmessung Modus *********************************************************
  if (entry->name == "Winkelmessung" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      // ESPNow init
      if (!ESPNow_init_done) {
        ESPNow_init_done = true;
        // Hilfetext und Init_ESPNow
        wm_init();
      }
      wm_measure(0);

      // check for connected sensors
      if ((millis() - ESPNowRecMills) > 4000) {  // alle 4s
        ESPNowRecMills = millis();
        wm_check_for_conn_change();  // neue ESPNow_sSender vorhanden, vorhandene disconnected?
      }
    }

    // edit mode
    if (buttonState == 3) {  // doppelklick
      wm_edit_rudderlength();
    }

    // zurück
    if (buttonState == 4) {
      navigateBack();
      ESPNow_init_done = false;
      wifiSetup(false);  // WLAN wieder an
      SetupMenu = false;
    }
  }

  // EWD Modus *********************************************************
  if (entry->name == "EWD" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      // ESPNow init
      if (!ESPNow_init_done) {
        ESPNow_init_done = true;
        // Hilfetext und Init_ESPNow
        EWD_init();
      }

      // Anzeige der Messwerte der Sender
      if (millis() - WinkelmessungMills > 200) {  // alle 200ms
        WinkelmessungMills = millis();
        wm_measure(1);  // EWD Modus
      }

      // check for connected sensors
      if ((millis() - ESPNowRecMills) > 4000) {  // alle 4s
        ESPNowRecMills = millis();
        wm_check_for_conn_change();  // neue ESPNow_sSender vorhanden, vorhandene disconnected?
      }
    }

    // edit mode
    if (buttonState == 3) {  // doppelklick
    }

    // zurück
    if (buttonState == 4) {
      navigateBack();
      ESPNow_init_done = false;
      wifiSetup(false);  // WLAN wieder an
      SetupMenu = false;
    }
  }

  // EWD Modus *********************************************************
  if (entry->name == "CG_Scale" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      if (!ESPNow_init_done) {
        ESPNow_init_done = true;
        cg_scale_init();
      }

      if (millis() - WinkelmessungMills > 200) {  // alle 200ms
        WinkelmessungMills = millis();
        cg_measure();  // EWD Modus
      }
    }

    // edit mode
    if (buttonState == 3) {  // doppelklick
    }

    // zurück
    if (buttonState == 4) {
      navigateBack();
      ESPNow_init_done = false;
      wifiSetup(false);  // WLAN wieder an
      SetupMenu = false;
    }
    // break;
  }

  // PWM Impuls lesen *********************************************************
  if (entry->name == "Impuls_lesen" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      bool parameterSet;  // See servoModes.h

      if (!SetupMenu) {
        pinMode(servopin[0], INPUT);
        pinMode(servopin[1], INPUT);
        pinMode(servopin[2], INPUT);
        pinMode(servopin[3], INPUT);
        pinMode(servopin[4], INPUT);
        SetupMenu = true;
      }

      servo_pos[selectedServo] = pulseIn(servopin[selectedServo], HIGH, 50000);  // Read PWM signal
      pwmFreq = readFreq(servopin[selectedServo], HIGH, 50000);                  // Read PWM frequency

      // Switch progress bar range
      if (servo_pos[selectedServo] > 750)  // Normal pulsewidth range
      {
        Impuls_min = SERVO_MIN_STD;
        Impuls_max = SERVO_MAX_STD;
      } else  // Sanwa pulsewidth range
      {
        Impuls_min = SERVO_MIN_SANWA;
        Impuls_max = SERVO_MAX_SANWA;
      }

      // Enlarge range, if required
      if (servo_pos[selectedServo] > Impuls_max)
        Impuls_max = servo_pos[selectedServo];
      if (servo_pos[selectedServo] < Impuls_min)
        Impuls_min = servo_pos[selectedServo];

      static unsigned long pwmMenuMillis;
      if (millis() - pwmMenuMillis > 100) {  // Every 100ms (slow screen refresh down, display is too nervous otherwise!)
        pwmMenuMillis = millis();
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setFont(ArialMT_Plain_24);
        display.drawString(64, 0, impulseString[LANGUAGE] + String(selectedServo + 1));
#ifdef SHOW_SMALLER_US
        display.setFont(ArialMT_Plain_16);
#endif
        display.drawString(64, 25, String(servo_pos[selectedServo]) + "µs");

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);

        display.drawString(0, 25, "Hz");
        display.drawString(0, 35, String(pwmFreq));

        if (pwmFreq > 1)  // Only show progress bar, if we have a signal
        {
          display.drawProgressBar(8, 50, 112, 10, (((servo_pos[selectedServo] - Impuls_min) * 100) / (Impuls_max - Impuls_min)));
        } else {
          display.setTextAlignment(TEXT_ALIGN_CENTER);
          display.setFont(ArialMT_Plain_10);
          display.drawString(64, 50, impulseSignalString[LANGUAGE]);
        }

        display.display();
      }

      if (encoderState == 1) {
        selectedServo--;
      }
      if (encoderState == 2) {
        selectedServo++;
      }

      if (selectedServo > 4) {
        selectedServo = 4;
      } else if (selectedServo < 0) {
        selectedServo = 0;
      }

      if (selectedServo != selectedServoOld) {
        tone(BUZZER_PIN, 4000, 2);
        selectedServoOld = selectedServo;
      }
    }

    if (buttonState == 4) {
      navigateBack();
      SetupMenu = false;
      selectedServo = 0;
    }
  }

  // Multiswitch lesen *********************************************************
  // https: // www.modelltruck.net/showthread.php?54795-Futaba-Robbe-Multiswitch-Decoder-mit-Arduino
  if (entry->name == "Multiswitch_lesen" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      static unsigned long multiswitchMenuMillis;
      if (millis() - multiswitchMenuMillis > 20) {  // Every 20ms (slow screen refresh down)
        multiswitchMenuMillis = millis();
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(20, 0, String(value1[8]));
        display.drawString(64, 0, String(value1[0]) + "  " + String(value1[1]));
        display.drawString(64, 15, String(value1[2]) + "  " + String(value1[3]));
        display.drawString(64, 30, String(value1[4]) + "  " + String(value1[5]));
        display.drawString(64, 45, String(value1[6]) + "  " + String(value1[7]));
        display.drawString(5, 30, "Multiswitch");
        display.display();
      }

      if (!SetupMenu) {
        pinMode(servopin[4], INPUT);
        SetupMenu = true;
      }

      value1[8] = 1500;

      while (value1[8] > 1000)  // Wait for the beginning of the frame (1000)
      {
        value1[8] = pulseIn(servopin[4], HIGH, 50000);
        if (!(digitalRead(BUTTON_PIN)))  // Taster gedrückt?
        {
          // Menu = Multiswitch_lesen_Auswahl;
          navigateBack();
          SetupMenu = false;
          break;
        }
      }

      for (int x = 0; x <= kanaele - 1; x++)  // Loop to store all the channel positions
      {
        value1[x] = pulseIn(servopin[4], HIGH, 50000);
        if (!(digitalRead(BUTTON_PIN)))  // Taster gedrückt?
        {
          // Menu = Multiswitch_lesen_Auswahl;
          navigateBack();
          SetupMenu = false;
          break;
        }
      }
    }

    if (buttonState == 4)  // zurück?
    {
      navigateBack();
      SetupMenu = false;
    }
  }

  // SBUS lesen *********************************************************
  if (entry->name == "SBUS_lesen" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.setFont(ArialMT_Plain_10);

      if (!SetupMenu) {
        pinMode(servopin[0], INPUT);
        pinMode(servopin[1], INPUT);
        pinMode(servopin[2], INPUT);
        pinMode(servopin[3], INPUT);
        pinMode(servopin[4], INPUT);
        sBus.Begin();  // begin SBUS communication with compatible receivers
        SetupMenu = true;
      }

      sBus.Read();
      const auto sBus_data = sBus.data();

      display.drawString(32, 0, String(::map(sBus_data.ch[0], 172, 1811, 1000, 2000)));
      display.drawString(64, 0, String(::map(sBus_data.ch[1], 172, 1811, 1000, 2000)));
      display.drawString(96, 0, String(::map(sBus_data.ch[2], 172, 1811, 1000, 2000)));
      display.drawString(128, 0, String(::map(sBus_data.ch[3], 172, 1811, 1000, 2000)));

      display.drawString(32, 15, String(::map(sBus_data.ch[4], 172, 1811, 1000, 2000)));
      display.drawString(64, 15, String(::map(sBus_data.ch[5], 172, 1811, 1000, 2000)));
      display.drawString(96, 15, String(::map(sBus_data.ch[6], 172, 1811, 1000, 2000)));
      display.drawString(128, 15, String(::map(sBus_data.ch[7], 172, 1811, 1000, 2000)));

      display.drawString(32, 30, String(::map(sBus_data.ch[8], 172, 1811, 1000, 2000)));
      display.drawString(64, 30, String(::map(sBus_data.ch[9], 172, 1811, 1000, 2000)));
      display.drawString(96, 30, String(::map(sBus_data.ch[10], 172, 1811, 1000, 2000)));
      display.drawString(128, 30, String(::map(sBus_data.ch[11], 172, 1811, 1000, 2000)));

      display.drawString(32, 45, String(::map(sBus_data.ch[12], 172, 1811, 1000, 2000)));
      display.drawString(64, 45, String(::map(sBus_data.ch[13], 172, 1811, 1000, 2000)));
      display.drawString(96, 45, String(::map(sBus_data.ch[14], 172, 1811, 1000, 2000)));
      display.drawString(128, 45, String(::map(sBus_data.ch[15], 172, 1811, 1000, 2000)));
      display.display();
    }

    if (buttonState == 4) {
      navigateBack();
      SetupMenu = false;
    }
  }

  // IBUS lesen *********************************************************
  if (entry->name == "IBUS_lesen" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(32, 0, String(IBus.readChannel(0)));
      display.drawString(64, 0, String(IBus.readChannel(1)));
      display.drawString(96, 0, String(IBus.readChannel(2)));
      display.drawString(128, 0, String(IBus.readChannel(3)));

      display.drawString(32, 15, String(IBus.readChannel(4)));
      display.drawString(64, 15, String(IBus.readChannel(5)));
      display.drawString(96, 15, String(IBus.readChannel(6)));
      display.drawString(128, 15, String(IBus.readChannel(7)));

      display.drawString(32, 30, String(IBus.readChannel(8)));
      display.drawString(64, 30, String(IBus.readChannel(9)));
      display.drawString(96, 30, String(IBus.readChannel(10)));
      display.drawString(128, 30, String(IBus.readChannel(11)));

      display.drawString(32, 45, String(IBus.readChannel(12)));
      display.drawString(64, 45, String(IBus.readChannel(13)));
      // display.drawString( 96, 45,  String(IBus.readChannel(14)));
      // display.drawString(128, 45,  String(IBus.readChannel(15)));
      display.display();

      if (!SetupMenu) {
        pinMode(servopin[0], INPUT);
        pinMode(servopin[1], INPUT);
        pinMode(servopin[2], INPUT);
        pinMode(servopin[3], INPUT);
        pinMode(servopin[4], INPUT);
        IBus.begin(Serial2, IBUSBM_NOTIMER, COMMAND_RX, COMMAND_TX);  // iBUS object connected to serial2 RX2 pin and use timer 1
        SetupMenu = true;
      }

      IBus.loop();  // call internal loop function to update the communication to the receiver
    }

    if (buttonState == 4) {
      navigateBack();
      SetupMenu = false;
    }
  }

  // Oszilloskop *********************************************************
  if (entry->name == "Oscilloscope" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      if (!SetupMenu)  // This stuff is only executed once
      {
        pinMode(servopin[0], INPUT);
        pinMode(servopin[1], INPUT);
        pinMode(servopin[2], INPUT);
        pinMode(servopin[3], INPUT);
        pinMode(servopin[4], INPUT);
        pinMode(OSCILLOSCOPE_PIN, INPUT);
        oscilloscopeLoop(true);  // Init oscilloscope
        SetupMenu = true;
      } else {
        oscilloscopeLoop(false);  // Loop oscilloscope code
      }
    }

    if (buttonState == 4)  // Back
    {
      navigateBack();
      SetupMenu = false;
    }
  }

  // Signal Generator *********************************************************
  if (entry->name == "SignalGenerator" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      if (!SetupMenu)  // This stuff is only executed once
      {
        pinMode(servopin[0], INPUT);
        pinMode(servopin[1], INPUT);
        pinMode(servopin[2], INPUT);
        pinMode(servopin[3], INPUT);
        pinMode(servopin[4], INPUT);
        pinMode(OSCILLOSCOPE_PIN, INPUT);
        signalGeneratorLoop(true);  // Init signal generator
        SetupMenu = true;
      } else {
        signalGeneratorLoop(false);  // Loop signal generator
      }
    }

    if (buttonState == 4)  // Back
    {
      navigateBack();
      SetupMenu = false;
    }
  }

  // Rechner *********************************************************
  if (entry->name == "Rechner" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    static bool calculatorLeft;
    static bool calculatorRight;
    static bool calculatorSelect;
    if (!zurueck) {
      calculator(calculatorLeft, calculatorRight, calculatorSelect);  // Run calculator

      if (!SetupMenu)  // This stuff is only executed once
      {
        SetupMenu = true;
      }

      if (encoderState == 1)  // Left
      {
        calculatorLeft = true;
        calculatorRight = false;
      } else if (encoderState == 2)  // Right
      {
        calculatorRight = true;
        calculatorLeft = false;
      } else  // Cursor stop
      {
        calculatorLeft = false;
        calculatorRight = false;
      }
    }

    if (buttonState == 4)  // Back
    {
      navigateBack();
      SetupMenu = false;
    }

    if (buttonState == 2)  // Select
    {
      calculatorSelect = true;
    } else {
      calculatorSelect = false;
    }
  }

  // Pong spielen *********************************************************
  if (entry->name == "Pong" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    static bool paddleUp;
    static bool paddleDown;
    static bool pongReset;
    if (!zurueck) {
      pong(paddleUp, paddleDown, pongReset, encoderSpeed);  // Run pong game

      if (!SetupMenu)  // This stuff is only executed once
      {
        SetupMenu = true;
      }

      if (encoderState == 1)  // Paddle up
      {
        paddleUp = true;
        paddleDown = false;
      } else if (encoderState == 2)  // Paddle down
      {
        paddleDown = true;
        paddleUp = false;
      } else  // Paddle stop
      {
        paddleUp = false;
        paddleDown = false;
      }
    }

    if (buttonState == 4)  // Back
    {
      navigateBack();
      SetupMenu = false;
    }

    if (buttonState == 2)  // Reset
    {
      pongReset = true;
    } else {
      pongReset = false;
    }
  }

  // Flappy Birds spielen *********************************************************
  if (entry->name == "Flappy_Birds" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    static bool flappyClick;
    static unsigned long lastClick = millis();

    disableButtonRead = true;  // Disable button read function for Flappy Bird!

    flappyBirds(flappyClick);  // Run flappy birds game

    if (!digitalRead(BUTTON_PIN))  // Button pressed
    {
      flappyClick = true;
    } else  // Button released
    {
      flappyClick = false;
      lastClick = millis();
    }

    if (millis() - lastClick > Duration_long)  // Back, if pressed long
    {
      // Menu = Flappy_Birds_Auswahl;
      navigateBack();
      SetupMenu = false;
    }
  }

  // Einstellung *********************************************************
  if (entry->name == "Einstellung" && entry->menuId == Menu) {
    checkButtonUsedForNavigation();
    if (!zurueck) {
      batteryVolts();  // Read battery voltage
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.setFont(ArialMT_Plain_24);
      display.drawString(64, 0, settingsString[LANGUAGE]);
      display.setFont(ArialMT_Plain_16);
      switch (Einstellung) {
        case 0:
          display.drawString(64, 25, "Wifi");
          if (WIFI_ON == 1) {
            display.drawString(64, 45, onString[LANGUAGE]);
          } else {
            display.drawString(64, 45, offString[LANGUAGE]);
          }
          break;
        case 1:
          display.drawString(64, 25, factoryResetString[LANGUAGE]);
          if (RESET_EEPROM == 1) {
            display.drawString(64, 45, yesString[LANGUAGE]);
          } else {
            display.drawString(64, 45, noString[LANGUAGE]);
          }
          break;
        case 2:
          display.drawString(64, 25, servoMaxString[LANGUAGE]);
          display.drawString(64, 45, String(SERVO_MAX));
          display.setTextAlignment(TEXT_ALIGN_RIGHT);
          display.setFont(ArialMT_Plain_10);
          display.drawString(128, 50, servoMode);
          break;
        case 3:
          display.drawString(64, 25, servoMinString[LANGUAGE]);
          display.drawString(64, 45, String(SERVO_MIN));
          display.setTextAlignment(TEXT_ALIGN_RIGHT);
          display.setFont(ArialMT_Plain_10);
          display.drawString(128, 50, servoMode);
          break;
        case 4:
          display.drawString(64, 25, servoCenterString[LANGUAGE]);
          display.drawString(64, 45, String(SERVO_CENTER));
          display.setTextAlignment(TEXT_ALIGN_RIGHT);
          display.setFont(ArialMT_Plain_10);
          display.drawString(128, 50, servoMode);
          break;
        case 5:
          display.drawString(64, 25, servoHzString[LANGUAGE]);
          display.drawString(64, 45, String(SERVO_Hz));
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 27, "µs");
          display.drawString(0, 37, String(SERVO_MIN));
          display.setTextAlignment(TEXT_ALIGN_RIGHT);
          display.drawString(128, 27, "µs");
          display.drawString(128, 37, String(SERVO_MAX));
          display.drawString(128, 50, servoMode);
          break;
        case 6:
          display.drawString(64, 25, PowerScaleString[LANGUAGE]);
          display.drawString(64, 45, String(POWER_SCALE));
          display.setFont(ArialMT_Plain_10);
          display.setTextAlignment(TEXT_ALIGN_RIGHT);
          display.drawString(110, 50, String(batteryVoltage));
          display.setTextAlignment(TEXT_ALIGN_LEFT);
          display.drawString(110, 50, "V");
          break;
        case 7:
          display.drawString(64, 25, "SBUS");
          if (SBUS_INVERTED == 1) {
            display.drawString(64, 45, standardString[LANGUAGE]);
          } else {
            display.drawString(64, 45, inversedString[LANGUAGE]);
          }
          break;
        case 8:
          display.drawString(64, 25, encoderDirectionString[LANGUAGE]);
          if (ENCODER_INVERTED == 0) {
            display.drawString(64, 45, standardString[LANGUAGE]);
          } else {
            display.drawString(64, 45, inversedString[LANGUAGE]);
          }
          break;
        case 9:
          display.drawString(64, 25, languageString[LANGUAGE]);
          display.drawString(64, 45, languagesString[LANGUAGE]);
          break;
        case 10:
          display.drawString(64, 25, pongBallRateString[LANGUAGE]);
          display.drawString(64, 45, String(PONG_BALL_RATE));
          break;
        case 11:
          display.drawString(64, 25, manualOnStartupString[LANGUAGE]);
          if (DISPLAY_MANUAL_ON_STARTUP) {
            display.drawString(64, 45, onString[LANGUAGE]);
          } else {
            display.drawString(64, 45, offString[LANGUAGE]);
          }
          break;
        case 12:  // Neues Untermenü für die Menüpunkt-Verwaltung
          if (!Edit) {
            display.setTextAlignment(TEXT_ALIGN_CENTER);
            display.drawString(64, 27, functionsInfo1String[LANGUAGE]);
            display.drawString(64, 45, functionsInfo2String[LANGUAGE]);
          } else {
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(8, 27, functionsString[LANGUAGE]);
            display.setTextAlignment(TEXT_ALIGN_CENTER);
            // display.drawString(64, 45, menuEntries[SetupSubmenuVisibilityIndex].name.c_str());
            display.drawString(64, 45, getMenuEntryName(menuEntries[SetupSubmenuVisibilityIndex]));
            if (menuEntries[SetupSubmenuVisibilityIndex].enabled) {
              int centerX = 120;
              int centerY = 37;
              int lineLength = 8;
              display.drawLine(centerX - lineLength, centerY - lineLength, centerX + lineLength - 1, centerY + lineLength - 1);
              display.drawLine(centerX - lineLength, centerY + lineLength - 1, centerX + lineLength - 1, centerY - lineLength);
            }
          }
          break;
      }
      if (Edit && Einstellung == 12) {
        display.drawRect(128 - 16, 29, 16, 16);
      } else if (Edit) {
        display.drawString(10, 50, "->");
      }
      display.display();

      if (!SetupMenu) {
        SetupMenu = true;
      }
    }

    if (encoderState == 1)  // Encoder left turn -------
    {
      if (!Edit) {
        Einstellung--;
      } else {
        switch (Einstellung) {
          case 0:
            WIFI_ON--;
            WiFiChanged = true;
            SettingChanged = true;
            break;
          case 1:
            RESET_EEPROM--;
            SettingChanged = true;
            break;
          case 2:
            if (SERVO_MODE == STD || SERVO_MODE == NOR || SERVO_MODE == SHR)
              SERVO_MAX_STD--;
            else
              SERVO_MAX_SANWA--;
            SettingChanged = true;
            break;
          case 3:
            if (SERVO_MODE == STD || SERVO_MODE == NOR || SERVO_MODE == SHR)
              SERVO_MIN_STD--;
            else
              SERVO_MIN_SANWA--;
            SettingChanged = true;
            break;
          case 4:
            if (SERVO_MODE == STD || SERVO_MODE == NOR || SERVO_MODE == SHR)
              SERVO_CENTER_STD--;
            else
              SERVO_CENTER_SANWA--;
            SettingChanged = true;
            break;
          case 5:
            SERVO_MODE--;
            SettingChanged = true;
            break;
          case 6:
            POWER_SCALE--;
            SettingChanged = true;
            break;
          case 7:
            SBUS_INVERTED--;
            SettingChanged = true;
            break;
          case 8:
            ENCODER_INVERTED--;
            SettingChanged = true;
            break;
          case 9:
            LANGUAGE--;
            SettingChanged = true;
            break;
          case 10:
            PONG_BALL_RATE--;
            SettingChanged = true;
            break;
          case 11:
            DISPLAY_MANUAL_ON_STARTUP = false;
            SettingChanged = true;
            break;
          case 12:
            SetupSubmenuVisibilityIndex--;
            if (SetupSubmenuVisibilityIndex < 0) {
              SetupSubmenuVisibilityIndex = menuEntries.size() - 2;
            }
            break;
        }
      }
    }
    if (encoderState == 2)  // Encoder right turn ------
    {
      if (!Edit) {
        Einstellung++;
      } else {
        switch (Einstellung) {
          case 0:
            WIFI_ON++;
            WiFiChanged = true;
            SettingChanged = true;
            break;
          case 1:
            RESET_EEPROM++;
            SettingChanged = true;
            break;
          case 2:
            if (SERVO_MODE == STD || SERVO_MODE == NOR || SERVO_MODE == SHR)
              SERVO_MAX_STD++;
            else
              SERVO_MAX_SANWA++;
            SettingChanged = true;
            break;
          case 3:
            if (SERVO_MODE == STD || SERVO_MODE == NOR || SERVO_MODE == SHR)
              SERVO_MIN_STD++;
            else
              SERVO_MIN_SANWA++;
            SettingChanged = true;
            break;
          case 4:
            if (SERVO_MODE == STD || SERVO_MODE == NOR || SERVO_MODE == SHR)
              SERVO_CENTER_STD++;
            else
              SERVO_CENTER_SANWA++;
            SettingChanged = true;
            break;
          case 5:
            SERVO_MODE++;
            SettingChanged = true;
            break;
          case 6:
            POWER_SCALE++;
            SettingChanged = true;
            break;
          case 7:
            SBUS_INVERTED++;
            SettingChanged = true;
            break;
          case 8:
            ENCODER_INVERTED++;
            SettingChanged = true;
            break;
          case 9:
            LANGUAGE++;
            SettingChanged = true;
            break;
          case 10:
            PONG_BALL_RATE++;
            SettingChanged = true;
            break;
          case 11:
            DISPLAY_MANUAL_ON_STARTUP = true;
            SettingChanged = true;
            break;
          case 12:
            SetupSubmenuVisibilityIndex++;
            if (SetupSubmenuVisibilityIndex >= (menuEntries.size() - 1)) {
              SetupSubmenuVisibilityIndex = 0;
            }
            break;
        }
      }
    }

    // Menu range -------------------------------------
    if (Einstellung > 12) {
      Einstellung = 0;
    } else if (Einstellung < 0) {
      Einstellung = 12;
    }

    // Limits -----------------------------------------
    if (PONG_BALL_RATE < 1) {  // Pong ball rate nicht unter 1
      PONG_BALL_RATE = 1;
    }

    if (PONG_BALL_RATE > 4) {  // Pong ball rate nicht über 4
      PONG_BALL_RATE = 4;
    }

    if (LANGUAGE < 0) {  // Language nicht unter 0
      LANGUAGE = 0;
    }

    if (LANGUAGE > noOfLanguages) {  // Language nicht über 1
      LANGUAGE = noOfLanguages;
    }

    if (ENCODER_INVERTED < 0) {  // Encoder inverted nicht unter 0
      ENCODER_INVERTED = 0;
    }

    if (ENCODER_INVERTED > 1) {  // Encoder inverted nicht über 1
      ENCODER_INVERTED = 1;
    }

    if (SBUS_INVERTED < 0) {  // SBUS inverted nicht unter 0
      SBUS_INVERTED = 0;
    }

    if (SBUS_INVERTED > 1) {  // SBUS inverted nicht über 1
      SBUS_INVERTED = 1;
    }

    if (WIFI_ON < 0) {  // Wifi off nicht unter 0
      WIFI_ON = 0;
    }

    if (WIFI_ON > 1) {  // Wifi on nicht über 1
      WIFI_ON = 1;
    }

    if (RESET_EEPROM < 0) {  // Reset eeprom nicht unter 0
      RESET_EEPROM = 0;
    }

    if (RESET_EEPROM > 1) {  // Reset eeprom nicht über 1
      RESET_EEPROM = 1;
    }

    SERVO_MIN_STD = constrain(SERVO_MIN_STD, 750, 1200);
    SERVO_MIN_SANWA = constrain(SERVO_MIN_SANWA, 100, 200);

    SERVO_CENTER_STD = constrain(SERVO_CENTER_STD, 1400, 1600);
    SERVO_CENTER_SANWA = constrain(SERVO_CENTER_SANWA, 250, 350);

    SERVO_MAX_STD = constrain(SERVO_MAX_STD, 1800, 2250);
    SERVO_MAX_SANWA = constrain(SERVO_MAX_SANWA, 400, 500);

    servoModes();  // Refresh servo operation mode

    if (buttonState == 3 && Einstellung == 12 && Edit == true) {
      menuEntries[SetupSubmenuVisibilityIndex].enabled = !menuEntries[SetupSubmenuVisibilityIndex].enabled;
      SettingChanged = true;
    }

    // Buttons ---------------------------------------
    if (buttonState == 4)  // zurück?
    {
      Serial.println("zurück, kein EEPROM Abspeichern");
      eepromRead();  // Abbruch, keinen Wert übernehmen
      navigateBack();
      SetupMenu = false;
      Einstellung = 5;
    }

    if (buttonState == 2 && !buttonUsedForNavigation) {
      if (Edit) {
        Edit = false;
        if (RESET_EEPROM) {
          eepromInit();  // Restore defaults
        } else {
          eepromWrite();  // Safe changes
        }
        if (WiFiChanged) {
          WiFiChanged = false;
          wifiSetup();
        }
      } else {
        Edit = true;
      }
    }

    if ((Einstellung != EinstellungOld) || (SettingChanged)) {
      tone(BUZZER_PIN, 4000, 2);
      EinstellungOld = Einstellung;
      encoderStateOld = encoderState;
      SettingChanged = false;
    }
  }

  // default:
  //   // Tue etwas, im Defaultfall
  //   // Dieser Fall ist optional
  //   Menu = Servotester_Auswahl;
  //   break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
  // }
  // Bip bei Menu Änderung
  // if (Menu != MenuOld)
  // {
  //   if (Menu != Servotester_Menu)
  //     tone(BUZZER_PIN, 4000, 2);
  //   MenuOld = Menu;
  // }
}

// =======================================================================================================
// BATTERY MONITOR
// =======================================================================================================
//
void batteryVolts() {
  currentTimeSpan = millis();
  if ((currentTimeSpan - previousTimeSpan) > 2000) {
    previousTimeSpan = currentTimeSpan;

    batteryVoltage = battery.readVoltage() * 0.825;  // We want the same POWER_SCALE as before with analogRead()

    float scale_value = POWER_SCALE / 100.0;
    batteryVoltage = batteryVoltage * scale_value;

    // Serial.print(eepromVoltageString[LANGUAGE]);
    // Serial.println(batteryVoltage, 3);

    batteryDetected = 1;

    if (batteryVoltage > 21.25)  // 6s Lipo
    {
      numberOfBatteryCells = 6;
    } else if (batteryVoltage > 17.0)  // 5s Lipo
    {
      numberOfBatteryCells = 5;
    } else if (batteryVoltage > 12.75)  // 4s Lipo
    {
      numberOfBatteryCells = 4;
    } else if (batteryVoltage > 8.5)  // 3s Lipo
    {
      numberOfBatteryCells = 3;
    } else if (batteryVoltage > 6.0)  // 2s Lipo
    {
      numberOfBatteryCells = 2;
    } else  // 1s Lipo kein Akku angeschlossen
    {
      numberOfBatteryCells = 1;
      batteryDetected = 0;
    }

    batteryChargePercentage = (batteryVoltage / numberOfBatteryCells);  // Prozentanzeige
    batteryChargePercentage = map_float(batteryChargePercentage, 3.2, 4.2, 0, 100);
    if (batteryChargePercentage < 0) {
      batteryChargePercentage = 0;
    }
    if (batteryChargePercentage > 100) {
      batteryChargePercentage = 100;
    }
  }
}

//
// =======================================================================================================
// EEPROM
// =======================================================================================================
//

// Init new board with the default values you want ------
void eepromInit() {
  Serial.println(SERVO_MIN_STD);
  if (SERVO_MIN_STD < 50 || RESET_EEPROM)  // Automatic or manual reset
  {
    RESET_EEPROM = 0;

    // Restore defaults
    WIFI_ON = 1;  // Wifi on
    // SERVO_STEPS = 10;
    // SERVO_MAX = 2000;
    // SERVO_MIN = 1000;
    // SERVO_CENTER = 1500;
    // SERVO_Hz = 50;
    POWER_SCALE = 948;
    SBUS_INVERTED = 1;  // 1 = Standard signal!
    ENCODER_INVERTED = 0;
    LANGUAGE = 0;
    PONG_BALL_RATE = 1;
    SERVO_MODE = STD;
    SERVO_MAX_STD = 2000;
    SERVO_MIN_STD = 1000;
    SERVO_CENTER_STD = 1500;
    SERVO_MAX_SANWA = 470;
    SERVO_MIN_SANWA = 130;
    SERVO_CENTER_SANWA = 300;
    RUDDER_LENGTH_1 = 40;
    RUDDER_LENGTH_2 = 40;
    RUDDER_LENGTH_3 = 40;
    RUDDER_LENGTH_4 = 40;
    SHOW_ANGLE = 0;
    DISPLAY_MANUAL_ON_STARTUP = true;
    Serial.println(eepromInitString[LANGUAGE]);
    servoModes();  // servoModes() needs to be executed in order to actualize the values TODO
    eepromWrite();
  }
}

// Write new values to EEPROM ------
void eepromWrite() {
  EEPROM.writeInt(adr_eprom_WIFI_ON, WIFI_ON);
  // EEPROM.writeInt(adr_eprom_SERVO_STEPS, SERVO_STEPS);
  //  EEPROM.writeInt(adr_eprom_SERVO_MAX, SERVO_MAX);
  //  EEPROM.writeInt(adr_eprom_SERVO_MIN, SERVO_MIN);
  //  EEPROM.writeInt(adr_eprom_SERVO_CENTER, SERVO_CENTER);
  //  EEPROM.writeInt(adr_eprom_SERVO_Hz, SERVO_Hz);
  EEPROM.writeInt(adr_eprom_POWER_SCALE, POWER_SCALE);
  EEPROM.writeInt(adr_eprom_SBUS_INVERTED, SBUS_INVERTED);
  EEPROM.writeInt(adr_eprom_ENCODER_INVERTED, ENCODER_INVERTED);
  EEPROM.writeInt(adr_eprom_LANGUAGE, LANGUAGE);
  EEPROM.writeInt(adr_eprom_PONG_BALL_RATE, PONG_BALL_RATE);
  EEPROM.writeInt(adr_eprom_SERVO_MODE, SERVO_MODE);
  EEPROM.writeInt(adr_eprom_SERVO_MAX_STD, SERVO_MAX_STD);
  EEPROM.writeInt(adr_eprom_SERVO_MIN_STD, SERVO_MIN_STD);
  EEPROM.writeInt(adr_eprom_SERVO_CENTER_STD, SERVO_CENTER_STD);
  EEPROM.writeInt(adr_eprom_SERVO_MAX_SANWA, SERVO_MAX_SANWA);
  EEPROM.writeInt(adr_eprom_SERVO_MIN_SANWA, SERVO_MIN_SANWA);
  EEPROM.writeInt(adr_eprom_SERVO_CENTER_SANWA, SERVO_CENTER_SANWA);
  EEPROM.writeInt(adr_eeprom_RUDDER_LENGTH_1, RUDDER_LENGTH_1);
  EEPROM.writeInt(adr_eeprom_RUDDER_LENGTH_2, RUDDER_LENGTH_2);
  EEPROM.writeInt(adr_eeprom_RUDDER_LENGTH_3, RUDDER_LENGTH_3);
  EEPROM.writeInt(adr_eeprom_RUDDER_LENGTH_4, RUDDER_LENGTH_4);
  EEPROM.writeInt(adr_eeprom_SHOW_ANGLE, SHOW_ANGLE);
  EEPROM.writeBool(adr_eeprom_DISPLAY_MANUAL_ON_STARTUP, DISPLAY_MANUAL_ON_STARTUP);

  eepromWriteMenuEntries();

  EEPROM.commit();
  Serial.println(eepromWrittenString[LANGUAGE]);
}

void eepromWriteMenuEntries() {
  for (size_t i = 0; i < menuEntries.size(); i++) {
    EEPROM.writeBool(adr_eeprom_menuEnabled_0 + i, menuEntries[i].enabled);
  }
  // EEPROM.commit();
}

void eepromReadMenuEntries() {
  for (size_t i = 0; i < menuEntries.size(); i++) {
    menuEntries[i].enabled = EEPROM.readBool(adr_eeprom_menuEnabled_0 + i);
  }
}

// Read values from EEPROM ------
void eepromRead() {
  WIFI_ON = EEPROM.readInt(adr_eprom_WIFI_ON);
  // SERVO_STEPS = EEPROM.readInt(adr_eprom_SERVO_STEPS);
  //  SERVO_MAX = EEPROM.readInt(adr_eprom_SERVO_MAX);
  //  SERVO_MIN = EEPROM.readInt(adr_eprom_SERVO_MIN);
  //  SERVO_CENTER = EEPROM.readInt(adr_eprom_SERVO_CENTER);
  //  SERVO_Hz = EEPROM.readInt(adr_eprom_SERVO_Hz);
  POWER_SCALE = EEPROM.readInt(adr_eprom_POWER_SCALE);
  SBUS_INVERTED = EEPROM.readInt(adr_eprom_SBUS_INVERTED);
  ENCODER_INVERTED = EEPROM.readInt(adr_eprom_ENCODER_INVERTED);
  LANGUAGE = EEPROM.readInt(adr_eprom_LANGUAGE);
  PONG_BALL_RATE = EEPROM.readInt(adr_eprom_PONG_BALL_RATE);
  SERVO_MODE = EEPROM.readInt(adr_eprom_SERVO_MODE);
  SERVO_MAX_STD = EEPROM.readInt(adr_eprom_SERVO_MAX_STD);
  SERVO_MIN_STD = EEPROM.readInt(adr_eprom_SERVO_MIN_STD);
  SERVO_CENTER_STD = EEPROM.readInt(adr_eprom_SERVO_CENTER_STD);
  SERVO_MAX_SANWA = EEPROM.readInt(adr_eprom_SERVO_MAX_SANWA);
  SERVO_MIN_SANWA = EEPROM.readInt(adr_eprom_SERVO_MIN_SANWA);
  SERVO_CENTER_SANWA = EEPROM.readInt(adr_eprom_SERVO_CENTER_SANWA);
  RUDDER_LENGTH_1 = EEPROM.readInt(adr_eeprom_RUDDER_LENGTH_1);
  RUDDER_LENGTH_2 = EEPROM.readInt(adr_eeprom_RUDDER_LENGTH_2);
  RUDDER_LENGTH_3 = EEPROM.readInt(adr_eeprom_RUDDER_LENGTH_3);
  RUDDER_LENGTH_4 = EEPROM.readInt(adr_eeprom_RUDDER_LENGTH_4);
  SHOW_ANGLE = EEPROM.readInt(adr_eeprom_SHOW_ANGLE);
  DISPLAY_MANUAL_ON_STARTUP = EEPROM.readBool(adr_eeprom_DISPLAY_MANUAL_ON_STARTUP);

  servoModes();  // servoModes() needs to be executed in order to actualize the values

  eepromReadMenuEntries();

  Serial.println(eepromReadString[LANGUAGE]);
  Serial.print("WIFI_ON: ");
  Serial.println(WIFI_ON);
  // Serial.print("SERVO_STEPS: ");
  // Serial.println(SERVO_STEPS);
  // Serial.print("SERVO_MAX: ");
  // Serial.println(SERVO_MAX);
  // Serial.print("SERVO_MIN: ");
  // Serial.println(SERVO_MIN);
  // Serial.print("SERVO_CENTER: ");
  // Serial.println(SERVO_CENTER);
  Serial.print("POWER_SCALE: ");
  Serial.println(POWER_SCALE);
  Serial.print("SBUS_INVERTED: ");
  Serial.println(SBUS_INVERTED);
  Serial.print("ENCODER_INVERTED: ");
  Serial.println(ENCODER_INVERTED);
  if (LANGUAGE < 0)  // Make sure, language is in correct range, otherwise device will crash!
    LANGUAGE = 0;
  if (LANGUAGE > noOfLanguages)
    LANGUAGE = noOfLanguages;
  Serial.print("LANGUAGE: ");
  Serial.println(LANGUAGE);
  Serial.print("PONG_BALL_RATE: ");
  Serial.println(PONG_BALL_RATE);
  Serial.print("SERVO_MODE: ");
  Serial.println(SERVO_MODE);
  Serial.print("SERVO_MAX_STD: ");
  Serial.println(SERVO_MAX_STD);
  Serial.print("SERVO_MIN_STD: ");
  Serial.println(SERVO_MIN_STD);
  Serial.print("SERVO_CENTER_STD: ");
  Serial.println(SERVO_CENTER_STD);
  Serial.print("SERVO_MAX_SANWA: ");
  Serial.println(SERVO_MAX_SANWA);
  Serial.print("SERVO_MIN_SANWA: ");
  Serial.println(SERVO_MIN_SANWA);
  Serial.print("SERVO_CENTER_SANWA: ");
  Serial.println(SERVO_CENTER_SANWA);
  Serial.print("RUDDER_LENGTH_1: ");
  Serial.println(RUDDER_LENGTH_1);
  Serial.print("RUDDER_LENGTH_2: ");
  Serial.println(RUDDER_LENGTH_2);
  Serial.print("RUDDER_LENGTH_3: ");
  Serial.println(RUDDER_LENGTH_3);
  Serial.print("RUDDER_LENGTH_4: ");
  Serial.println(RUDDER_LENGTH_4);
  Serial.print("SHOW_ANGLE: ");
  Serial.println(SHOW_ANGLE);
  Serial.print("DISPLAY_MANUAL_ON_STARTUP: ");
  Serial.println(DISPLAY_MANUAL_ON_STARTUP);
}

//
// =======================================================================================================
// MAIN LOOP, RUNNING ON CORE 1
// =======================================================================================================
//

void loop() {
  ButtonRead();
  // beep();
  MenuUpdate();
  // webInterface();

  // Serial.println (loopDuration());
}

//
// =======================================================================================================
// 1st MAIN TASK, RUNNING ON CORE 0 (Interrupts are running on this core as well)
// =======================================================================================================
//
/*
void Task1code(void *pvParameters) // TODO, testing only!
{
  for (;;)
  {
    // vTaskDelay(1);          // REQUIRED TO RESET THE WATCH DOG TIMER IF WORKFLOW DOES NOT CONTAIN ANY OTHER DELAY
  }
}*/