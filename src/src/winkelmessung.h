#pragma once

#include <Arduino.h>

static bool ESPNow_init_done;
static unsigned long WinkelmessungMills;
static float M1_offset;
static float M2_offset;
static float M3_offset;
static float M4_offset;
float M1_GyroLagewert;
float M2_GyroLagewert;
float M3_GyroLagewert;
float M4_GyroLagewert;
static float M1_GyroLagewert_old;
static float M2_GyroLagewert_old;
static float M3_GyroLagewert_old;
static float M4_GyroLagewert_old;
static float M1_GyroDrift;
static float M2_GyroDrift;
static float M3_GyroDrift;
static float M4_GyroDrift;
static bool WM_heatup[4];
static unsigned long int WM_heatuptime_start[4];



bool m1_init, m2_init, m3_init, m4_init;
static unsigned long ESPNowRecMills;
//#define DRIFT_COMP
#define GYRO_DRIFT 0.25
unsigned char anz_sens; // wieviele Sensoren sind angemeldet?

//#define MPU_DEBUG

void wm_init (void) {
    // Show manual
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, operationString[LANGUAGE]);
    display.drawString(0, 12, wm_info1[LANGUAGE]);
    display.drawString(0, 24, wm_info2[LANGUAGE]);
    display.drawString(0, 36, wm_info3[LANGUAGE]);
    display.drawString(0, 48, wm_info4[LANGUAGE]);
    display.display (); // Anzeige auf OLED

    // ESPNow Init
    WiFi.mode (WIFI_AP);
    configDeviceAP ();
    Serial.print ("AP MAC: "); 
    Serial.println (WiFi.softAPmacAddress ());
    InitESPNow ();
    esp_now_register_recv_cb(OnDataRecv);

    delay(3000);

    M1_GyroLagewert_old = M2_GyroLagewert_old = M3_GyroLagewert_old = M4_GyroLagewert_old = 0;
}

// Anzeige Batt-Symbol und Ladezustands-Balken
void wm_anz_batt (unsigned char pos, unsigned char symsize, float lipo_U) {
    static unsigned long int battblink_old[4];
    static bool invers_disp[4];

    if (lipo_U > 4.19)  // Korrektur wenn USC-C angeschlosen ist und der Lipo off ist
        lipo_U = 4.19;

    if (lipo_U < 3.4) { // low voltage: Batt-Symbol blinkend
        if ((millis () - battblink_old[pos]) > 500) { // alle 500ms blinken
            battblink_old[pos] = millis ();
            if (invers_disp[pos]) { invers_disp[pos] = false; }
            else { invers_disp[pos] = true; }
        }
    }
    else
        invers_disp[pos] = false;

    if (symsize) {  // 4 Zeilen-Anzeige
        #define BATT1_X 20
        #define BATT1_Y 12
        #define BATT1_L 12
        if (invers_disp[pos])
            display.setColor (BLACK);
        else
            display.setColor (WHITE);
        float lipobar = map_float (lipo_U < 3.4 ? 3.4 : lipo_U, 3.4, 4.2, BATT1_X+1, BATT1_X+BATT1_L-1);
        display.drawRect (BATT1_X, BATT1_Y+pos*16, BATT1_L, 4);
        display.setPixel (BATT1_X+BATT1_L, BATT1_Y+1+pos*16);   // Batt.Knob
        display.setPixel (BATT1_X+BATT1_L, BATT1_Y+2+pos*16);
        display.setPixel (BATT1_X+BATT1_L+1, BATT1_Y+1+pos*16);   // Batt.Knob
        display.setPixel (BATT1_X+BATT1_L+1, BATT1_Y+2+pos*16);
        display.drawLine (BATT1_X+1, BATT1_Y+1+pos*16, lipobar, BATT1_Y+1+pos*16);
        display.drawLine (BATT1_X+1, BATT1_Y+2+pos*16, lipobar, BATT1_Y+2+pos*16);
    }
    else {  // 1 od. 2-Zeilen Anzeige
        if (invers_disp[pos])
            display.setColor (BLACK);
        else
            display.setColor (WHITE);
        display.drawProgressBar (0, 18+pos*32, 16, 6, map_float (lipo_U < 3.4 ? 3.4 : lipo_U, 3.4, 4.2, 0, 100));
        display.drawLine (0, 18+pos*32, 0, 24+pos*32);    // Batt-Symbol
        display.setPixel (1, 18+pos*32);
        display.setPixel (1, 24+pos*32);
        display.drawLine (17, 20+pos*32, 17, 22+pos*32);  // Batt.Knob
        display.drawLine (15, 18+pos*32, 15, 24+pos*32);
        if (!invers_disp[pos]) {
            display.clearPixel (1, 19+pos*32);         // links eckig
            display.clearPixel (1, 23+pos*32);
        }
        display.setPixel (2, 20+pos*32);
        display.setPixel (2, 22+pos*32);
        display.setPixel (3, 20+pos*32);
        display.setPixel (3, 22+pos*32);
    }
    display.setColor (WHITE);
}

void wm_anz_rudderlenght (unsigned char pos, unsigned char textsize, int rudder_length) {
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);  // 10, 16, 24
    switch (textsize) {
        case 1: // ejne Zeile Anzeige
            if (SHOW_ANGLE) {
                display.drawString (36, 14+pos*16, "L = ");
                display.setTextAlignment(TEXT_ALIGN_RIGHT);
                display.drawString (83, 14+pos*16, String (rudder_length));
                display.drawString (100, 14+pos*16, "mm");
            }
            else {
                display.drawString (28, 14+pos*16, "L = ");
                display.drawString (45, 14+pos*16, String (rudder_length) + "mm");
            }
            break;
        case 2: // 2 Zeilen Anzeige
            display.drawString (20, 5+pos*32, "L:" + String (rudder_length) + "mm");
            break;
        case 3: // 3 Zeilen Anzeige
        case 4: // 4 Zeilen Anzeige
            display.drawString (19, 0+pos*16, "L:");
            display.drawString (29, 0+pos*16, String (rudder_length));
            if (rudder_length < 99.9)
                display.drawString (41, 0+pos*16, "mm");
            break;
        default:
            break;
    }
}

/*  
    zeilen: 1-, 2-, 4-Zeilen-Anzeige; >=10: Fokus Winkelmessung
    pos: 1., 2., 3., 4. Zeile
*/
void wm_anz_ruderausschlag (unsigned char pos, unsigned char zeilen, int rudderlength, float angle) {
    float ruderausschlag = rudderlength * sinf (angle*PI/180);
    if ((ruderausschlag < 0.0) && (ruderausschlag > -0.05)) // toggeln des "-" Zeichens vermeiden
        ruderausschlag = 0.0;

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    switch (zeilen) {
        case 1:
            display.setFont(ArialMT_Plain_16);  // 10, 16, 24
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString (10, 40, "s =");
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.drawString (128, 40, "mm");
            display.setFont(ArialMT_Plain_24);  // 10, 16, 24
            display.drawString (100, 33, String (ruderausschlag, 1)); // eine Nachkommastelle
            break;
        case 2:
            if (ruderausschlag > -9.9) {
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.drawString (66, 15+pos*16, "s:");
            }
            if ((ruderausschlag > 99.9) || (ruderausschlag < -99.9)) {
                display.setFont(ArialMT_Plain_16);  // 10, 16, 24
                display.drawString (112, 10+pos*16, String (ruderausschlag, 1));
            }
            else {
                display.setFont(ArialMT_Plain_24);  // 10, 16, 24
                display.drawString (112, 3+pos*16, String (ruderausschlag, 1));
            }
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            display.drawString (128, 15+pos*16, "mm");
            break;
        case 4:
            display.setFont(ArialMT_Plain_16);  // 10, 16, 24
            if ((ruderausschlag > -100.0) && (ruderausschlag < 100))
                display.drawString (74, 0+pos*16, "s:");
            display.drawString (110, 0+pos*16, String (ruderausschlag, 1)); // eine Nachkommastelle
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            display.drawString (128, 5+pos*16, "mm");
            break;
        case 10:    // einzeilige Anzeige, Winkelfokus
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString (37, 25, "s =");
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.drawString (100, 25, "mm");
            display.drawString (83, 25, String (ruderausschlag, 1)); // eine Nachkommastelle
            break;
        case 11:    // 2-zeilige Anzeige, Winkelfokus
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            display.drawString (21, 15+pos*16, "s:" + String (ruderausschlag, 1)); // eine Nachkommastelle
            break;
        case 12:    // 3- oder 4-zeilige Anzeige, Winkelmodus
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            if ((ruderausschlag < 99.9) && (ruderausschlag > -100))
                display.drawString (19, 0+pos*16, "s:" + String (ruderausschlag, 1) + "mm");
            else
                display.drawString (19, 0+pos*16, "s:" + String (ruderausschlag, 1));
            break;
        default:
            break;
    }
    #ifdef MPU_DEBUG
        Serial.printf ("RUDDER_LENGTH_1: %dmm | Ruderausschlag: %5.2fmm\n", RUDDER_LENGTH_1, tanf (M1_GyroLagewert*PI/180) * RUDDER_LENGTH_1);
    #endif
}

/*
    zeilen2: 0, 1: 2-Zeilen Anzeige
    zeilen2: 10: 1-Zeilen Anzeige, Winkelanzeile groß
    zeilen2: 11: 2-Zeilen Anzeige, Winkelanzeile groß
*/
void wm_anz_angle (unsigned char pos, unsigned char zeilen2, float angle) {

    if ((angle < 0.0) && (angle > -0.05))   // toggeln des "-" Zeichens vermeiden
        angle = 0.0;

    switch (zeilen2) {
        case 0:
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            display.drawString (128, 14+pos*16, "> " + String (angle, 1) + "°"); // eine Nachkommastelle
            break;
        case 1:
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            if (angle < 100.0)
                display.drawString (20, 15+pos*16, "> " + String (angle, 1) + "°"); // eine Nachkommastelle
            else
                display.drawString (20, 15+pos*16, "> " + String (angle, 1)); // eine Nachkommastelle
            break;
        case 10:    // Anzeige 1 Zeile, Fokus Winkel
            display.setFont(ArialMT_Plain_24);  // 10, 16, 24
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString (30, 40, ">");
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.drawString (128, 40, "°");
            display.setFont(ArialMT_Plain_24);  // 10, 16, 24
            display.drawString (115, 40, String (angle, 1)); // eine Nachkommastelle
            break;
        case 11:    // Anzeige 2 Zeilen, Fokus Winkel
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            display.drawString (67, 15+pos*16, ">:");
            if (angle < -99.9) {
                display.setFont(ArialMT_Plain_16);  // 10, 16, 24
                display.drawString (122, 10+pos*16, String (angle, 1));
                display.setFont(ArialMT_Plain_16);  // 10, 16, 24
                display.drawString (128, 10+pos*16, "°");
            }
            else {
                display.setFont(ArialMT_Plain_24);  // 10, 16, 24
                display.drawString (122, 3+pos*16, String (angle, 1));
                display.setFont(ArialMT_Plain_16);  // 10, 16, 24
                display.drawString (128, 3+pos*16, "°");
            }
            break;
        case 12:    // Anzeige 3, 4 Zeilen, Fokus Winkel
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.setFont(ArialMT_Plain_16);  // 10, 16, 24
            if (angle > -100)
                display.drawString (84, 0+pos*16, ">:");
            display.drawString (123, 0+pos*16, String (angle, 1)); // eine Nachkommastelle
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            display.drawString (128, 0+pos*16, "°");
            break;
        default:
            break;
    }
}

void wm_anz_modul (unsigned char pos, unsigned char modul_nr) {
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);  // 10, 16, 24
    display.drawString (0, 5+pos*16, "#");
    display.setFont(ArialMT_Plain_16);  // 10, 16, 24
    display.drawString (6, 0+pos*16, String (modul_nr));
}

// die ersten 20 Sek. nach Einschalten eines WM-Moluls: Anzeige "heatup" + Fortschrittsbalken
void wm_anz_heatup (unsigned char pos, unsigned char module, unsigned char size) { // module: 0..3; size: 1, 2, 4
    #define HEATUP_TIME 20000   // 20Sek.
    static unsigned long int WM_heatuptime_old[4];

    if (WM_heatuptime_start[module] == 0) {   // erster Aufruf
        WM_heatuptime_start[module] = WM_heatuptime_old[module] = millis();
    }
    if ((millis() - WM_heatuptime_old[module]) > HEATUP_TIME) { // 20 Sek, Zeit abgelaufen?
        switch (module) {   // Winkelwerte nullen
            case 0: // Modul #1
                M1_offset += M1_GyroLagewert;
                tone (BUZZER_PIN, 4500, 50);
                break;
            case 1: // Modul #2
                M2_offset += M2_GyroLagewert;
                tone (BUZZER_PIN, 4500, 50);
                break;
            case 2: // Modul #3
                M3_offset += M3_GyroLagewert;
                tone (BUZZER_PIN, 4500, 50);
                break;
            case 3: // Modul #4
                M4_offset += M4_GyroLagewert;
                tone (BUZZER_PIN, 4500, 50);
                break;
            default:
                break;
        }
        WM_heatup[module] = true;
    }
    else {
        switch (size) {
            case 1: // einzeilige Anzeige
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.drawString (35, 30, "sensor heatup");
                display.setFont(ArialMT_Plain_16);  // 10, 16, 24
                display.drawString (10, 40, "s =");
                display.setTextAlignment(TEXT_ALIGN_RIGHT);
                display.drawString (128, 40, "mm");
                display.drawProgressBar (50, 45, 40, 10, ::map(millis(), WM_heatuptime_start[module], WM_heatuptime_start[module]+HEATUP_TIME, 0, 100));
                break;
            case 2: // zweizeilige Anzeige
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.drawString (75, 4+pos*16, "heatup");
                display.setTextAlignment(TEXT_ALIGN_RIGHT);
                display.drawString (66, 15+pos*16, "s:");
                display.drawString (128, 15+pos*16, "mm");
                display.drawProgressBar (75, 18+pos*16, 32, 8, ::map(millis(), WM_heatuptime_start[module], WM_heatuptime_start[module]+HEATUP_TIME, 0, 100));
                break;
            case 4: // vierzeilige Anzeige
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.setTextAlignment(TEXT_ALIGN_RIGHT);
                display.setFont(ArialMT_Plain_16);  // 10, 16, 24
                display.drawString (74, 0+pos*16, "s:");
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.drawString (128, 5+pos*16, "mm");
                display.drawProgressBar (76, 6+pos*16, 30, 8, ::map(millis(), WM_heatuptime_start[module], WM_heatuptime_start[module]+HEATUP_TIME, 0, 100));
                break;
            case 10:    // EWD Messung
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.drawString (32, 0+pos*16, "heatup");
                display.setTextAlignment(TEXT_ALIGN_RIGHT);
                display.drawProgressBar (32, 14+pos*16, 32, 8, ::map(millis(), WM_heatuptime_start[module], WM_heatuptime_start[module]+HEATUP_TIME, 0, 100));
                break;
            default:
                break;
        }
    }
}

/* ---- wm_anzeige_display ---------------------------------------------------------------------------------------------------------------- */
void wm_anzeige_display (void) {
    anz_sens = 0;
    if (m1_init)
        anz_sens++;
    if (m2_init)
        anz_sens++;
    if (m3_init)
        anz_sens++;
    if (m4_init)
        anz_sens++;

    display.clear();
    if (anz_sens == 1) {
        if (m1_init) {
            wm_anz_modul (0, ESPNowData_modul1_Nr); // Modul#
            wm_anz_batt (0, 0, ESPNowData_U1_Lipo);    // Lipo %
            if (!WM_heatup[0])
                wm_anz_heatup (0, 0, 1);   // Modul1 ([0]), einzeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (0, 10, M1_GyroLagewert);  // Ruderausschlag Winkel (groß)
                    wm_anz_ruderausschlag (0, 10, RUDDER_LENGTH_1, M1_GyroLagewert); // Ruderausschlag Weg (klein)
                }
                else {
                    wm_anz_angle (0, 0, M1_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (0, 1, RUDDER_LENGTH_1, M1_GyroLagewert);
                }
                wm_anz_rudderlenght (0, 1, RUDDER_LENGTH_1);   // Ruderlänge
            }
        }
        if (m2_init) {
            wm_anz_modul (0, ESPNowData_modul2_Nr); // Modul#
            wm_anz_batt (0, 0, ESPNowData_U2_Lipo);    // Lipo %
            if (!WM_heatup[1])
                wm_anz_heatup (0, 1, 1);   // Modul2 ([1]), einzeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (0, 10, M2_GyroLagewert);  // Ruderausschlag Winkel (groß)
                    wm_anz_ruderausschlag (0, 10, RUDDER_LENGTH_2, M2_GyroLagewert); // Ruderausschlag Weg (klein)
                }
                else {
                    wm_anz_angle (0, 0, M2_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (0, 1, RUDDER_LENGTH_2, M2_GyroLagewert);
                }
            }
            wm_anz_rudderlenght (0, 1, RUDDER_LENGTH_2);   // Ruderlänge
        }
        if (m3_init) {
            wm_anz_modul (0, ESPNowData_modul3_Nr); // Modul#
            wm_anz_batt (0, 0, ESPNowData_U3_Lipo);    // Lipo %
            if (!WM_heatup[2])
                wm_anz_heatup (0, 2, 1);   // Modul3 ([2]), einzeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (0, 10, M3_GyroLagewert);  // Ruderausschlag Winkel (groß)
                    wm_anz_ruderausschlag (0, 10, RUDDER_LENGTH_3, M3_GyroLagewert); // Ruderausschlag Weg (klein)
                }
                else {
                    wm_anz_angle (0, 0, M3_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (0, 1, RUDDER_LENGTH_3, M3_GyroLagewert);
                }
            }
            wm_anz_rudderlenght (0, 1, RUDDER_LENGTH_3);   // Ruderlänge
        }
        if (m4_init) {
            wm_anz_modul (0, ESPNowData_modul4_Nr); // Modul#
            wm_anz_batt (0, 0, ESPNowData_U4_Lipo);    // Lipo %
            if (!WM_heatup[3])
                wm_anz_heatup (0, 3, 1);   // Modul4 ([3]), einzeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (0, 10, M4_GyroLagewert);  // Ruderausschlag Winkel (groß)
                    wm_anz_ruderausschlag (0, 10, RUDDER_LENGTH_4, M4_GyroLagewert); // Ruderausschlag Weg (klein)
                }
                else {
                    wm_anz_angle (0, 0, M4_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (0, 1, RUDDER_LENGTH_4, M4_GyroLagewert);
                }
            }
            wm_anz_rudderlenght (0, 1, RUDDER_LENGTH_4);   // Ruderlänge
        }
    }
    if (anz_sens == 2) {
        unsigned char pos_offset=0;
        if (m1_init) {
            wm_anz_modul (pos_offset*2, ESPNowData_modul1_Nr); // Modul#
            wm_anz_batt (pos_offset, 0, ESPNowData_U1_Lipo);    // Lipo %
            if (!WM_heatup[0])
                wm_anz_heatup (pos_offset*2, 0, 2);   // Position, Modul1 ([0]), zweizeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (pos_offset*2, 11, M1_GyroLagewert);
                    wm_anz_ruderausschlag (pos_offset*2, 11, RUDDER_LENGTH_1, M1_GyroLagewert);
                }
                else {
                    wm_anz_angle (pos_offset*2, 1, M1_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (pos_offset*2, 2, RUDDER_LENGTH_1, M1_GyroLagewert);    // Ruderausschlag mm
                }
            }
            wm_anz_rudderlenght (pos_offset, 2, RUDDER_LENGTH_1);   // Ruderlänge
            pos_offset++;
        }
        if (m2_init) {
            wm_anz_modul (pos_offset*2, ESPNowData_modul2_Nr); // Modul#
            wm_anz_batt (pos_offset, 0, ESPNowData_U2_Lipo);    // Lipo %
            if (!WM_heatup[1])
                wm_anz_heatup (pos_offset*2, 1, 2);   // Modul2 ([1]), zweizeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (pos_offset*2, 11, M2_GyroLagewert);
                    wm_anz_ruderausschlag (pos_offset*2, 11, RUDDER_LENGTH_2, M2_GyroLagewert);
                }
                else {
                    wm_anz_angle (pos_offset*2, 1, M2_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (pos_offset*2, 2, RUDDER_LENGTH_2, M2_GyroLagewert);    // Ruderausschlag mm
                }
            }
            wm_anz_rudderlenght (pos_offset, 2, RUDDER_LENGTH_2);   // Ruderlänge
            pos_offset++;
        }
        if (m3_init) {
            wm_anz_modul (pos_offset*2, ESPNowData_modul3_Nr); // Modul#
            wm_anz_batt (pos_offset, 0, ESPNowData_U3_Lipo);    // Lipo %
            if (!WM_heatup[2])
                wm_anz_heatup (pos_offset*2, 2, 2);   // Modul3 ([2]), zweizeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (pos_offset*2, 11, M3_GyroLagewert);
                    wm_anz_ruderausschlag (pos_offset*2, 11, RUDDER_LENGTH_3, M3_GyroLagewert);
                }
                else {
                    wm_anz_angle (pos_offset*2, 1, M3_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (pos_offset*2, 2, RUDDER_LENGTH_3, M3_GyroLagewert);    // Ruderausschlag mm
                }
            }
            wm_anz_rudderlenght (pos_offset, 2, RUDDER_LENGTH_3);   // Ruderlänge
            pos_offset++;
        }
        if (m4_init) {
            wm_anz_modul (pos_offset*2, ESPNowData_modul4_Nr); // Modul#
            wm_anz_batt (pos_offset, 0, ESPNowData_U4_Lipo);    // Lipo %
            if (!WM_heatup[3])
                wm_anz_heatup (pos_offset*2, 3, 2);   // Modul4 ([3]), zweizeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (pos_offset*2, 11, M4_GyroLagewert);
                    wm_anz_ruderausschlag (pos_offset*2, 11, RUDDER_LENGTH_4, M4_GyroLagewert);
                }
                else {
                    wm_anz_angle (pos_offset*2, 1, M4_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (pos_offset*2, 2, RUDDER_LENGTH_4, M4_GyroLagewert);    // Ruderausschlag mm
                }
            }
            wm_anz_rudderlenght (pos_offset, 2, RUDDER_LENGTH_4);   // Ruderlänge
            //pos_offset++;
        }
    }
    if ((anz_sens == 3) || (anz_sens == 4)) {
        // static unsigned long int scrollbar_blink;
        unsigned char pos_offset=0;

        // // scrollbalken blinken
        // if ((millis() - scrollbar_blink) > 1000) {  // Scrollbar Blinkfrequenz 1s
        //     scrollbar_blink = millis();
        // }

        // Display: 128*64, 64/4=16
        if (m1_init) {
            wm_anz_modul (pos_offset, ESPNowData_modul1_Nr); // Modul#
            wm_anz_batt (pos_offset, 1, ESPNowData_U1_Lipo);    // Lipo %
            if (!WM_heatup[0])
                wm_anz_heatup (pos_offset, 0, 4);   // Position, Modul1 ([0]), vierzeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (pos_offset, 12, M1_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (pos_offset, 12, RUDDER_LENGTH_1, M1_GyroLagewert);    // Ruderausschlag mm
                }
                else {
                    wm_anz_ruderausschlag (pos_offset, 4, RUDDER_LENGTH_1, M1_GyroLagewert);    // Ruderausschlag mm
                    wm_anz_rudderlenght (pos_offset, 4, RUDDER_LENGTH_1);   // Ruderlänge
                }
            }
            pos_offset++;
        }
        if (m2_init) {
            wm_anz_modul (pos_offset, ESPNowData_modul2_Nr); // Modul#
            wm_anz_batt (pos_offset, 1, ESPNowData_U2_Lipo);    // Lipo %
            if (!WM_heatup[1])
                wm_anz_heatup (pos_offset, 1, 4);   // Position, Modul2 ([1]), vierzeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (pos_offset, 12, M2_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (pos_offset, 12, RUDDER_LENGTH_2, M2_GyroLagewert);    // Ruderausschlag mm
                }
                else {
                    wm_anz_ruderausschlag (pos_offset, 4, RUDDER_LENGTH_2, M2_GyroLagewert);    // Ruderausschlag mm
                    wm_anz_rudderlenght (pos_offset, 4, RUDDER_LENGTH_2);   // Ruderlänge
                }
            }
            pos_offset++;
        }
        if (m3_init) {
            wm_anz_modul (pos_offset, ESPNowData_modul3_Nr); // Modul#
            wm_anz_batt (pos_offset, 1, ESPNowData_U3_Lipo);    // Lipo %
            if (!WM_heatup[2])
                wm_anz_heatup (pos_offset, 2, 4);   // Position, Modul3 ([2]), vierzeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (pos_offset, 12, M3_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (pos_offset, 12, RUDDER_LENGTH_3, M3_GyroLagewert);    // Ruderausschlag mm
                }
                else {
                    wm_anz_ruderausschlag (pos_offset, 4, RUDDER_LENGTH_3, M3_GyroLagewert);    // Ruderausschlag mm
                    wm_anz_rudderlenght (pos_offset, 4, RUDDER_LENGTH_3);   // Ruderlänge
                }
            }
            pos_offset++;
        }
        if (m4_init) {
            wm_anz_modul (pos_offset, ESPNowData_modul4_Nr); // Modul#
            wm_anz_batt (pos_offset, 1, ESPNowData_U4_Lipo);    // Lipo %
            if (!WM_heatup[3])
                wm_anz_heatup (pos_offset, 3, 4);   // Position, Modul4 ([3]), vierzeilige Anzeige
            else {
                if (SHOW_ANGLE) {
                    wm_anz_angle (pos_offset, 12, M4_GyroLagewert);  // Ruderausschlag Winkel
                    wm_anz_ruderausschlag (pos_offset, 12, RUDDER_LENGTH_4, M4_GyroLagewert);    // Ruderausschlag mm
                }
                else {
                    wm_anz_ruderausschlag (pos_offset, 4, RUDDER_LENGTH_4, M4_GyroLagewert);    // Ruderausschlag mm
                    wm_anz_rudderlenght (pos_offset, 4, RUDDER_LENGTH_4);   // Ruderlänge
                }
            }
            //pos_offset++;
        }
    }
    display.display (); // Anzeige auf OLED
}

/* ---- EWD_anzeige_display --------------------------------------------------------------------------------------------------------------- */
void EWD_anzeige_display (void) {
    static unsigned long int displayinv_old;
    static unsigned char invers;
    float EWD_angle [2];
    float ewd;

    anz_sens = 0;
    if (m1_init)
        anz_sens++;
    if (m2_init)
        anz_sens++;
    if (m3_init)
        anz_sens++;
    if (m4_init)
        anz_sens++;

    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    if (anz_sens < 2) { // 2 Sensoren sollen angemeldet sein
        // if ((millis () - displayinv_old) > 500) { // alle 500ms blinken
        //     displayinv_old = millis ();
        //     if (invers) { display.setColor (WHITE); invers = false; }
        //     else { display.setColor (BLACK); invers = true; }
        //     display.drawString(0, 36, EWD_info2[LANGUAGE]); // 2 Sensoren eben platzieren
        //     display.display (); // Anzeige auf OLED
        // }
        display.setFont(ArialMT_Plain_10);  // 10, 16, 24
        display.drawString(0, 36, EWD_info2[LANGUAGE]); // 2 Sensoren eben platzieren
    }
    else if (anz_sens > 2) {    // max 2 Sensoren sollen angemeldet sein
        // if ((millis () - displayinv_old) > 500) { // alle 500ms blinken
        //     displayinv_old = millis ();
        //     if (invers) { display.setColor (WHITE); invers = false; }
        //     else { display.setColor (BLACK); invers = true; }
        //     display.drawString(0, 36, EWD_info5[LANGUAGE]); // max. 2 Sensoren platzieren
        //     display.display (); // Anzeige auf OLED
        // }
        display.setFont(ArialMT_Plain_10);  // 10, 16, 24
        display.drawString(0, 36, EWD_info5[LANGUAGE]); // max. 2 Sensoren platzieren
    }
    else {
        unsigned char pos_offset=0;
        if (m1_init) {
            wm_anz_modul (pos_offset*2, ESPNowData_modul1_Nr); // Modul#
            wm_anz_batt (pos_offset, 0, ESPNowData_U1_Lipo);    // Lipo %
            if (!WM_heatup[0])
                wm_anz_heatup (pos_offset*2, 0, 10);   // Position, Modul1 ([0]), EWD Messung
            else
                wm_anz_angle (pos_offset*2, 1, M1_GyroLagewert);  // Ruderausschlag Winkel
            EWD_angle [pos_offset] = M1_GyroLagewert;
            pos_offset++;
        }
        if (m2_init) {
            wm_anz_modul (pos_offset*2, ESPNowData_modul2_Nr); // Modul#
            wm_anz_batt (pos_offset, 0, ESPNowData_U2_Lipo);    // Lipo %
            if (!WM_heatup[1])
                wm_anz_heatup (pos_offset*2, 1, 10);   // Modul2 ([1]), EWD Messung
            else
                wm_anz_angle (pos_offset*2, 1, M2_GyroLagewert);  // Ruderausschlag Winkel
            EWD_angle [pos_offset] = M2_GyroLagewert;
            pos_offset++;
        }
        if (m3_init) {
            wm_anz_modul (pos_offset*2, ESPNowData_modul3_Nr); // Modul#
            wm_anz_batt (pos_offset, 0, ESPNowData_U3_Lipo);    // Lipo %
            if (!WM_heatup[2])
                wm_anz_heatup (pos_offset*2, 2, 10);   // Modul3 ([2]), EWD Messung
            else
                wm_anz_angle (pos_offset*2, 1, M3_GyroLagewert);  // Ruderausschlag Winkel
            EWD_angle [pos_offset] = M3_GyroLagewert;
            pos_offset++;
        }
        if (m4_init) {
            wm_anz_modul (pos_offset*2, ESPNowData_modul4_Nr); // Modul#
            wm_anz_batt (pos_offset, 0, ESPNowData_U4_Lipo);    // Lipo %
            if (!WM_heatup[3])
                wm_anz_heatup (pos_offset*2, 3, 10);   // Modul4 ([3]), EWD Messung
            else
                wm_anz_angle (pos_offset*2, 1, M4_GyroLagewert);  // Ruderausschlag Winkel
            EWD_angle [pos_offset] = M4_GyroLagewert;
        }
        // Anzeige EWD
        ewd = EWD_angle [0] - EWD_angle [1];
        if ((ewd < 0.0) && (ewd > -0.05))   // toggeln des "-" Zeichens vermeiden
            ewd = 0.0;
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.setFont(ArialMT_Plain_24);  // 10, 16, 24
        display.drawString (128, 0, "EWD");
        display.setFont(ArialMT_Plain_24);  // 10, 16, 24
        display.drawString (128, 32, String (ewd, 1) + "°");
    }
    
    display.display (); // Anzeige auf OLED
}

/* ---- wm_found -------------------------------------------------------------------------------------------------------------------------- */
void wm_found (unsigned char module_nr) {
    tone (BUZZER_PIN, 4000, 50);
    tone (BUZZER_PIN, 4500, 50);
    // Winkelmesser Modul1 Lipo-Spannung
    display.clear();
    display.setFont(ArialMT_Plain_10);  // 10, 16, 24
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawRect (13, 15, (128-2*13), (64-2*15));
    display.drawLine (14, 16+(64-2*15), 14+(128-2*13), 16+(64-2*15));
    display.drawLine (14+(128-2*13), 16, 14+(128-2*13), 16+(64-2*15));
    display.drawString (64, 20, "Modul #" + String (module_nr) + " Lipo");
    display.display (); // Anzeige auf OLED
    delay (200);
    switch (module_nr)
    {
    case 1:
        display.drawString (64, 30, "C: " + String (map_float (ESPNowData_U1_Lipo, 3.2, 4.2, 0, 100)) + "%" + " U: " + String (ESPNowData_U1_Lipo) + "V");
        Serial.printf ("Modul%d Lipo: %f\n", module_nr , ESPNowData_U1_Lipo);
        break;
    case 2:
        display.drawString (64, 30, "C: " + String (map_float (ESPNowData_U2_Lipo, 3.2, 4.2, 0, 100)) + "%" + " U: " + String (ESPNowData_U2_Lipo) + "V");
        Serial.printf ("Modul%d Lipo: %f\n", module_nr , ESPNowData_U2_Lipo);
        break;
    case 3:
        display.drawString (64, 30, "C: " + String (map_float (ESPNowData_U3_Lipo, 3.2, 4.2, 0, 100)) + "%" + " U: " + String (ESPNowData_U3_Lipo) + "V");
        Serial.printf ("Modul%d Lipo: %f\n", module_nr , ESPNowData_U3_Lipo);
        break;
    case 4:
        display.drawString (64, 30, "C: " + String (map_float (ESPNowData_U4_Lipo, 3.2, 4.2, 0, 100)) + "%" + " U: " + String (ESPNowData_U4_Lipo) + "V");
        Serial.printf ("Modul%d Lipo: %f\n", module_nr , ESPNowData_U4_Lipo);
        break;
    default:
        break;
    }
    display.display (); // Anzeige auf OLED
    delay (3000);
}

/* ---- wm_measure ------------------------------------------------------------------------------------------------------------------------ */
void wm_measure (unsigned char wm_mode) {    // alle 200ms (def. in src: WinkelmessungMills); wm_mode=0: Winkelmessung, wm_mode=1: EWD Modus
    #define DRIFTSTOP_TIME 5000
    // Modul1?
    if (ESPNowData_modul1_Nr == 1)   // Modul1 gefunden
    {
        if (!m1_init)
        {
            m1_init = true;
            wm_found (1);
        }
    }
    // Modul2?
    if (ESPNowData_modul2_Nr == 2)   // Modul2 gefunden
    {
        if (!m2_init)
        {
            m2_init = true;
            wm_found (2);
        }
    }
    // Modul3?
    if (ESPNowData_modul3_Nr == 3)   // Modul3 gefunden
    {
        if (!m3_init)
        {
            m3_init = true;
            wm_found (3);
        }
    }
    // Modul4?
    if (ESPNowData_modul4_Nr == 4)   // Modul4 gefunden
    {
        if (!m4_init)
        {
            m4_init = true;
            wm_found (4);
        }
    }

    // einer der Winkelsensoren angemeldet?
    if (m1_init || m2_init || m3_init || m4_init) {
        static unsigned long int driftstop1_old;
        static unsigned long int driftstop2_old;
        static unsigned long int driftstop3_old;
        static unsigned long int driftstop4_old;
        static bool drift1_stop;
        static bool drift2_stop;
        static bool drift3_stop;
        static bool drift4_stop;

        if (m1_init) {
            // Warnung bei Lipo-Spannung zu niedrig
            if (ESPNowData_U1_Lipo < 3.2) {
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                display.clear();
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.setTextAlignment(TEXT_ALIGN_CENTER);
                display.drawRect (5, 18, (128-2*5), 38);
                display.drawString (64, 20, "Achtung!");
                display.drawString (64, 30, "Lipo Modul #" + String (ESPNowData_modul1_Nr) + " Spannung");
                display.drawString (64, 40, String (ESPNowData_U1_Lipo, 1) + "V");   // 1 Nachkommastelle
                display.display (); // Anzeige auf OLED
                Serial.printf ("Modul%d Lipo: %f\n", ESPNowData_modul1_Nr, ESPNowData_U1_Lipo);
                delay (5000);
            }
            // Driftkompensation
            #ifdef DRIFT_COMP
                float M1_Gyroaenderung = ESPNowData_Gyro1_Lagewert - M1_GyroLagewert_old;
                if (abs (M1_Gyroaenderung) < GYRO_DRIFT) {
                    if (millis() - driftstop1_old > DRIFTSTOP_TIME)   // bei keiner aktiven Winkeländerung, nach 4 Sek. drift aktivieren
                        drift1_stop = false;
                    if (!drift1_stop)
                        M1_GyroDrift += M1_Gyroaenderung;
                }
                else {  // reale Winkeländerung
                    // 4 Sek. drift stoppen
                    driftstop1_old = millis();
                    drift1_stop = true;
                }
            #endif
            // spike Kompensation
            // t.b.d

            M1_GyroLagewert_old = ESPNowData_Gyro1_Lagewert;
            M1_GyroLagewert = ESPNowData_Gyro1_Lagewert - M1_offset - M1_GyroDrift;
            
            // Nullstellung bei Tasterdruck
            if (buttonState == 2) {
                M1_offset += M1_GyroLagewert;
            }

            // Displayausgabe
            if (wm_mode == 0)
                wm_anzeige_display ();  // Winkelanzeige
            else
                EWD_anzeige_display (); // EWD Anzeige
        }

        if (m2_init) {
            // Warnung bei Lipo-Spannung zu niedrig
            if (ESPNowData_U2_Lipo < 3.2) {
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                display.clear();
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.setTextAlignment(TEXT_ALIGN_CENTER);
                display.drawRect (5, 18, (128-2*5), 38);
                display.drawString (64, 20, "Achtung!");
                display.drawString (64, 30, "Lipo Modul #" + String (ESPNowData_modul2_Nr) + " Spannung");
                display.drawString (64, 40, String (ESPNowData_U2_Lipo, 1) + "V");   // 1 Nachkommastelle
                display.display (); // Anzeige auf OLED
                Serial.printf ("Modul%d Lipo: %f\n", ESPNowData_modul2_Nr, ESPNowData_U2_Lipo);
                delay (5000);
            }
            // Driftkompensation
            #ifdef DRIFT_COMP
                float M2_Gyroaenderung = ESPNowData_Gyro2_Lagewert - M2_GyroLagewert_old;
                if (abs (M2_Gyroaenderung) < GYRO_DRIFT) {
                    if (millis() - driftstop2_old > DRIFTSTOP_TIME)   // bei keiner aktiven Winkeländerung, nach 4 Sek. drift aktivieren
                        drift2_stop = false;
                    if (!drift2_stop)
                        M2_GyroDrift += M2_Gyroaenderung;
                }
                else {  // reale Winkeländerung
                    // 4 Sek. drift stoppen
                    driftstop2_old = millis();
                    drift2_stop = true;
                }
            #endif

            // spike Kompensation
            // t.b.d
        
            M2_GyroLagewert_old = ESPNowData_Gyro2_Lagewert;
            M2_GyroLagewert = ESPNowData_Gyro2_Lagewert - M2_offset - M2_GyroDrift;
            
            // Nullstellung bei Tasterdruck
            if (buttonState == 2) {
                M2_offset += M2_GyroLagewert;
            }

            // Displayausgabe
            if (wm_mode == 0)
                wm_anzeige_display ();  // Winkelanzeige
            else
                EWD_anzeige_display (); // EWD Anzeige
        }

        if (m3_init) {
            // Warnung bei Lipo-Spannung zu niedrig
            if (ESPNowData_U3_Lipo < 3.2) {
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                display.clear();
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.setTextAlignment(TEXT_ALIGN_CENTER);
                display.drawRect (5, 18, (128-2*5), 38);
                display.drawString (64, 20, "Achtung!");
                display.drawString (64, 30, "Lipo Modul #" + String (ESPNowData_modul3_Nr) + " Spannung");
                display.drawString (64, 40, String (ESPNowData_U3_Lipo, 1) + "V");   // 1 Nachkommastelle
                display.display (); // Anzeige auf OLED
                Serial.printf ("Modul%d Lipo: %f\n", ESPNowData_modul3_Nr, ESPNowData_U3_Lipo);
                delay (5000);
            }
            // Driftkompensation
            #ifdef DRIFT_COMP
                float M3_Gyroaenderung = ESPNowData_Gyro3_Lagewert - M3_GyroLagewert_old;
                if (abs (M3_Gyroaenderung) < GYRO_DRIFT) {
                    if (millis() - driftstop3_old > DRIFTSTOP_TIME)   // bei keiner aktiven Winkeländerung, nach 4 Sek. drift aktivieren
                        drift3_stop = false;
                    if (!drift3_stop)
                        M3_GyroDrift += M3_Gyroaenderung;
                }
                else {  // reale Winkeländerung
                    // 4 Sek. drift stoppen
                    driftstop3_old = millis();
                    drift3_stop = true;
                }
            #endif

            // spike Kompensation
            // t.b.d

            M3_GyroLagewert_old = ESPNowData_Gyro3_Lagewert;
            M3_GyroLagewert = ESPNowData_Gyro3_Lagewert - M3_offset - M3_GyroDrift;
            
            // Nullstellung bei Tasterdruck
            if (buttonState == 2) {
                M3_offset += M3_GyroLagewert;
            }

            // Displayausgabe
            if (wm_mode == 0)
                wm_anzeige_display ();  // Winkelanzeige
            else
                EWD_anzeige_display (); // EWD Anzeige
        }

        if (m4_init) {
            // Warnung bei Lipo-Spannung zu niedrig
            if (ESPNowData_U4_Lipo < 3.2) {
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                tone (BUZZER_PIN, 4500, 50);
                tone (BUZZER_PIN, 4000, 100);
                display.clear();
                display.setFont(ArialMT_Plain_10);  // 10, 16, 24
                display.setTextAlignment(TEXT_ALIGN_CENTER);
                display.drawRect (5, 18, (128-2*5), 38);
                display.drawString (64, 20, "Achtung!");
                display.drawString (64, 30, "Lipo Modul #" + String (ESPNowData_modul4_Nr) + " Spannung");
                display.drawString (64, 40, String (ESPNowData_U4_Lipo, 1) + "V");   // 1 Nachkommastelle
                display.display (); // Anzeige auf OLED
                Serial.printf ("Modul%d Lipo: %f\n", ESPNowData_modul4_Nr, ESPNowData_U4_Lipo);
                delay (5000);
            }
            // Driftkompensation
            #ifdef DRIFT_COMP
                float M4_Gyroaenderung = ESPNowData_Gyro4_Lagewert - M4_GyroLagewert_old;
                if (abs (M4_Gyroaenderung) < GYRO_DRIFT) {
                    if (millis() - driftstop4_old > DRIFTSTOP_TIME)   // bei keiner aktiven Winkeländerung, nach 4 Sek. drift aktivieren
                        drift4_stop = false;
                    if (!drift4_stop)
                        M4_GyroDrift += M4_Gyroaenderung;
                }
                else {  // reale Winkeländerung
                    // 4 Sek. drift stoppen
                    driftstop4_old = millis();
                    drift4_stop = true;
                }
            #endif

            // spike Kompensation
            // t.b.d

            M4_GyroLagewert_old = ESPNowData_Gyro4_Lagewert;
            M4_GyroLagewert = ESPNowData_Gyro4_Lagewert - M4_offset - M4_GyroDrift;
            
            // Nullstellung bei Tasterdruck
            if (buttonState == 2) {
                M4_offset += M4_GyroLagewert;
            }

            // Displayausgabe
            if (wm_mode == 0)
                wm_anzeige_display ();  // Winkelanzeige
            else
                EWD_anzeige_display (); // EWD Anzeige
        }

        #ifdef MPU_DEBUG
            Serial.printf ("Modul: %d (%4.2fV (%d)) | ESPNowData.Gyro_Lagewert: %6.2f | M1_GyroDrift: %6.2f | M1_offset: %6.2f | M1_GyroLagewert: %6.2f | ", ESPNowData.modul_Nr, ESPNowData.U_Lipo, ESPNowData.U_Lipo_raw, ESPNowData.Gyro_Lagewert, M1_GyroDrift, M1_offset, M1_GyroLagewert);
        #endif

        // Drehgeber: Umschaltung zwischen Ruderweg- und Winkel-Anzeige
        if ((encoderState == 1) || (encoderState == 2)) {    // Encoder links oder rechts gedreht?
            tone (BUZZER_PIN, 4000, 2);
            if (SHOW_ANGLE) {
                SHOW_ANGLE = 0;
            }
            else  {
                SHOW_ANGLE = 1;
            }

            // // Popup window
            // if (millis() - popupMillis < 1500)
            // {
            //     display.setTextAlignment(TEXT_ALIGN_CENTER);
            //     display.setColor(BLACK);
            //     display.fillRect(25, 22, 78, 29); // Clear area behind window
            //     display.setColor(WHITE);
            //     display.drawRect(25, 22, 78, 29); // Draw window frame
            //     display.drawString(64, 25, "Sampling delay");
            //     display.drawString(64, 35, String(samplingDelay) + " µs"); // Show sampling delay
            //     if (samplingDelay != samplingDelayOld)
            //     {
            //         tone (BUZZER_PIN, 4000, 2);
            //         samplingDelayOld = samplingDelay;
            //     }
            // }
            display.setFont(ArialMT_Plain_10);  // 10, 16, 24
            display.setTextAlignment(TEXT_ALIGN_CENTER);
            display.setColor(BLACK);
            display.fillRect(25, 22, 78, 29); // Clear area behind window
            display.setColor(WHITE);
            display.drawRect(25, 22, 78, 29); // Draw window frame
            display.drawString(64, 25, "Fokus:");
            if (SHOW_ANGLE) {
                display.drawString(64, 35, "Winkel-Anz.");
            }
            else {
                display.drawString(64, 35, "Ruderweg-Anz.");
            }
            display.display ();

            // Serial.printf ("SHOW_ANGLE: %d encoderState: %d\n", SHOW_ANGLE, encoderState);

            // EEPROM
            eepromWrite(); // Safe changes
            delay (1000);
            ButtonRead();
            encoderState = 0;
        }
    }
    else {
        display.clear();
        display.setFont(ArialMT_Plain_10);  // 10, 16, 24
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawRect (15, 15, (128-2*15), (64-2*15));
        display.drawString (64, 20, "kein Winkelsensor");
        display.drawString (64, 30, "angemeldet");
        display.display (); // Anzeige auf OLED
    }
}

// neue ESPNow_sSender vorhanden, vorhandene disconnected?
void wm_check_for_conn_change (void) {
    static unsigned int ESPNow1_WD_old;
    static unsigned int ESPNow2_WD_old;
    static unsigned int ESPNow3_WD_old;
    static unsigned int ESPNow4_WD_old;
    static unsigned int ESPNow_WD_old;

    if (ESPNow1_WD == ESPNow1_WD_old) {
        m1_init = false;
        ESPNowData_modul1_Nr = ESPNowData_Gyro1_Lagewert = M1_GyroLagewert = M1_GyroDrift = ESPNowData_U1_Lipo = 0;
        WM_heatup[0] = false;
        WM_heatuptime_start[0] = 0;
    }
    if (ESPNow2_WD == ESPNow2_WD_old) {
        m2_init = false;
        ESPNowData_modul2_Nr = ESPNowData_Gyro2_Lagewert = M2_GyroLagewert = M2_GyroDrift = ESPNowData_U2_Lipo = 0;
        WM_heatup[1] = false;
        WM_heatuptime_start[1] = 0;
    }
    if (ESPNow3_WD == ESPNow3_WD_old) {
        m3_init = false;
        ESPNowData_modul3_Nr = ESPNowData_Gyro3_Lagewert = M3_GyroLagewert = M3_GyroDrift = ESPNowData_U3_Lipo = 0;
        WM_heatup[2] = false;
        WM_heatuptime_start[2] = 0;
    }
    if (ESPNow4_WD == ESPNow4_WD_old) {
        m4_init = false;
        ESPNowData_modul4_Nr = ESPNowData_Gyro4_Lagewert = M4_GyroLagewert = M4_GyroDrift = ESPNowData_U4_Lipo = 0;
        WM_heatup[3] = false;
        WM_heatuptime_start[3] = 0;
    }
    if (ESPNow_WD == ESPNow_WD_old) {
        m1_init = m2_init = m3_init = m4_init = false;
        ESPNowData_modul1_Nr = ESPNowData_modul2_Nr = ESPNowData_modul3_Nr = ESPNowData_modul4_Nr = 0;
        ESPNowData.modul_Nr = 0;
    }
    ESPNow1_WD_old = ESPNow1_WD;
    ESPNow2_WD_old = ESPNow2_WD;
    ESPNow3_WD_old = ESPNow3_WD;
    ESPNow4_WD_old = ESPNow4_WD;
    ESPNow_WD_old = ESPNow_WD;
}

void wm_edit_rudderlength (void) {
    static unsigned long int displayinv_old;
    static unsigned long int timeout_t;
    static bool invers;
    unsigned char pos=0;
    bool timeout = false;
    unsigned char ix;
    unsigned char iy;
    unsigned char pos_offset=0;

    // bei Drehgeber Impuls: Ruderlänge erhöhen, veringern, pip bei Änderung
    
    // Anzeige Ruderausschlag löschen
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    switch (anz_sens) {
        case 1:
            display.setFont(ArialMT_Plain_24);  // 10, 16, 24
            // Ruderausschlag- oder Winkelanzeige löschen, -.- anzeigen
            if (SHOW_ANGLE) {
                if (!!WM_heatup[0] || !WM_heatup[1] || !WM_heatup[2] || !WM_heatup[3]) {
                    for (ix = 30; ix < 128; ix++) {
                        for (iy = 28; iy < 64; iy++) {
                            display.clearPixel (ix, iy);
                        }
                    }
                }
                else {
                    for (ix = 50; ix < 118; ix++) {
                        for (iy = 28; iy < 64; iy++) {
                            display.clearPixel (ix, iy);
                        }
                    }
                }
                display.drawString (113, 40, "-.-");
            }
            else {
                for (ix = 32; ix < 103; ix++) {
                    for (iy = 32; iy < 64; iy++) {
                        display.clearPixel (ix, iy);
                    }
                }
                display.drawString (100, 33, "-.-");            
            }
            break;
        case 2:
            // Ruderausschlag löschen
            display.setColor (BLACK); 
            invers = true;
            if (m1_init) {
                if (SHOW_ANGLE) {
                    if (!WM_heatup[0])
                        wm_anz_heatup (pos_offset*2, 0, 2);   // Position, Modul1 ([0]), zweizeilige Anzeige
                    else
                        wm_anz_angle (pos_offset*2, 11, M1_GyroLagewert);
                }
                else {
                    if (!WM_heatup[0])
                        wm_anz_heatup (pos_offset*2, 0, 2);   // Position, Modul1 ([0]), zweizeilige Anzeige
                    else
                        wm_anz_ruderausschlag (pos_offset*2, 2, RUDDER_LENGTH_1, M1_GyroLagewert);
                }
                pos_offset++;
            }
            if (m2_init) {
                if (SHOW_ANGLE) {
                    if (!WM_heatup[1])
                        wm_anz_heatup (pos_offset*2, 1, 2);   // Position, Modul2 ([1]), zweizeilige Anzeige
                    else
                       wm_anz_angle (pos_offset*2, 11, M2_GyroLagewert);
                }
                else {
                    if (!WM_heatup[1])
                        wm_anz_heatup (pos_offset*2, 1, 2);   // Position, Modul2 ([1]), zweizeilige Anzeige
                    else
                        wm_anz_ruderausschlag (pos_offset*2, 2, RUDDER_LENGTH_2, M2_GyroLagewert);
                }
                pos_offset++;
            }
            if (m3_init) {
                if (SHOW_ANGLE) {
                    if (!WM_heatup[2])
                        wm_anz_heatup (pos_offset*2, 2, 2);   // Position, Modul3 ([2]), zweizeilige Anzeige
                    else
                        wm_anz_angle (pos_offset*2, 11, M3_GyroLagewert);
                }
                else {
                    if (!WM_heatup[2])
                        wm_anz_heatup (pos_offset*2, 2, 2);   // Position, Modul3 ([2]), zweizeilige Anzeige
                    else
                        wm_anz_ruderausschlag (pos_offset*2, 2, RUDDER_LENGTH_3, M3_GyroLagewert);
                }
                pos_offset++;
            }
            if (m4_init) {
                if (SHOW_ANGLE) {
                    if (!WM_heatup[3])
                        wm_anz_heatup (pos_offset*2, 3, 2);   // Position, Modul4 ([3]), zweizeilige Anzeige
                    else
                        wm_anz_angle (pos_offset*2, 11, M4_GyroLagewert);
                }
                else {
                    if (!WM_heatup[3])
                        wm_anz_heatup (pos_offset*2, 3, 2);   // Position, Modul4 ([3]), zweizeilige Anzeige
                    else
                        wm_anz_ruderausschlag (pos_offset*2, 2, RUDDER_LENGTH_4, M4_GyroLagewert);
                }
                // pos_offset++;
            }
            display.setColor (WHITE); 
            invers = false;
            display.setFont(ArialMT_Plain_24);  // 10, 16, 24
            for (iy = 0; iy < anz_sens; iy++)
                display.drawString (112, 0+iy*32, "-.-");            
            break;
        case 3:
        case 4:
            if (SHOW_ANGLE) {
                for (ix = 85; ix < 124; ix++) {     // angezeigte Ruderausschläge löschen
                    for (iy = 0; iy < 64; iy++) {
                        display.clearPixel (ix, iy);
                    }
                }
                // Ruderausschlag löschen
                display.setColor (BLACK); 
                invers = true;
                if (m1_init) {
                    if (!WM_heatup[0])
                        wm_anz_heatup (pos_offset, 0, 4);   // Position, Modul1 ([0]), vierzeilige Anzeige
                    else
                        wm_anz_ruderausschlag (pos_offset, 12, RUDDER_LENGTH_1, M1_GyroLagewert);
                    pos_offset++;
                }
                if (m2_init) {
                    if (!WM_heatup[1])
                        wm_anz_heatup (pos_offset, 1, 4);   // Position, Modul2 ([1]), vierzeilige Anzeige
                    else
                        wm_anz_ruderausschlag (pos_offset, 12, RUDDER_LENGTH_2, M2_GyroLagewert);
                    pos_offset++;
                }
                if (m3_init) {
                    if (!WM_heatup[2])
                        wm_anz_heatup (pos_offset, 2, 4);   // Position, Modul3 ([2]), vierzeilige Anzeige
                    else
                        wm_anz_ruderausschlag (pos_offset, 12, RUDDER_LENGTH_3, M3_GyroLagewert);
                    pos_offset++;
                }
                if (m4_init) {
                    if (!WM_heatup[3])
                        wm_anz_heatup (pos_offset, 3, 4);   // Position, Modul4 ([3]), vierzeilige Anzeige
                    else
                        wm_anz_ruderausschlag (pos_offset, 12, RUDDER_LENGTH_4, M4_GyroLagewert);
                    // pos_offset++;
                }
                display.setColor (WHITE); 
                invers = false;

                display.setFont(ArialMT_Plain_16);  // 10, 16, 24
                display.setTextAlignment(TEXT_ALIGN_RIGHT);
                for (iy = 0; iy < anz_sens; iy++)
                    display.drawString (120, 0+iy*16, "-.-");

                // aktuelle Ruderlängen anzeigen
                if (m1_init) {
                    wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_1);
                    pos++;
                }
                if (m2_init) {
                    wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_2);
                    pos++;
                }
                if (m3_init) {
                    wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_3);
                    pos++;
                }
                if (m4_init) {
                    wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_4);
                    // pos++;
                }
            }
            else {
                for (ix = 72; ix < 110; ix++) {     // angezeigte Ruderausschläge löschen
                    for (iy = 0; iy < 64; iy++) {
                        display.clearPixel (ix, iy);
                    }
                }
                display.setFont(ArialMT_Plain_16);  // 10, 16, 24
                for (iy = 0; iy < anz_sens; iy++)
                    display.drawString (110, 0+iy*16, "-.-");            
            }
            break;
        default:
            break;
    }
    pos = 0;

    display.setFont(ArialMT_Plain_10);  // 10, 16, 24
    timeout_t = millis();
    if (m1_init) {
        do {
            if ((millis () - displayinv_old) > 500) { // alle 500ms blinken
                displayinv_old = millis ();
                if (invers) { display.setColor (WHITE); invers = false; }
                else { display.setColor (BLACK); invers = true; }
            }
            // Drehgeber
            if (encoderState == 2) {    //rechtsrum
                timeout_t = millis();     // timeout reset
                displayinv_old = millis ();
                // alten Wert auf OLED löschen
                display.setColor (BLACK);
                wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_1);
                display.setColor (WHITE);
                invers = false;
                RUDDER_LENGTH_1++;
                if (RUDDER_LENGTH_1 > 500)
                    RUDDER_LENGTH_1 = 500;
                tone (BUZZER_PIN, 4000, 2);
            }
            if (encoderState == 1) {    //linksrum
                timeout_t = millis();     // timeout reset
                displayinv_old = millis ();
                display.setColor (BLACK);
                wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_1);
                display.setColor (WHITE);
                invers = false;
                RUDDER_LENGTH_1--;
                if (RUDDER_LENGTH_1 < 10)
                    RUDDER_LENGTH_1 = 10;
                tone (BUZZER_PIN, 4000, 2);
            }
            wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_1);
            display.display ();
            if (((millis() - timeout_t) > 30000) || (buttonState == 4)) {  // nach 30 Sek. Timeout oder "zurück"
                timeout = true;
            }
            ButtonRead ();
        } while ((buttonState != 2) && !timeout); // bis enter
        display.setColor (WHITE);
        wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_1);
        pos++;
    }
    timeout_t = millis();

    if (m2_init) {
        do {
            if ((millis () - displayinv_old) > 500) { // alle 500ms blinken
                displayinv_old = millis ();
                if (invers) { display.setColor (WHITE); invers = false; }
                else { display.setColor (BLACK); invers = true; }
            }
            // Drehgeber
            if (encoderState == 2) {    //rechtsrum
                timeout_t = millis();     // timeout reset
                displayinv_old = millis ();
                // alten Wert auf OLED löschen
                display.setColor (BLACK);
                wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_2);
                display.setColor (WHITE);
                invers = false;
                RUDDER_LENGTH_2++;
                if (RUDDER_LENGTH_2 > 500)
                    RUDDER_LENGTH_2 = 500;
                tone (BUZZER_PIN, 4000, 2);
            }
            if (encoderState == 1) {    //linksrum
                timeout_t = millis();     // timeout reset
                displayinv_old = millis ();
                display.setColor (BLACK);
                wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_2);
                display.setColor (WHITE);
                invers = false;
                RUDDER_LENGTH_2--;
                if (RUDDER_LENGTH_2 < 10)
                    RUDDER_LENGTH_2 = 10;
                tone (BUZZER_PIN, 4000, 2);
            }
            wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_2);
            display.display ();
            if (((millis() - timeout_t) > 30000) || (buttonState == 4)) {  // nach 30 Sek. Timeout oder "zurück"
                timeout = true;
            }
            ButtonRead ();
        } while ((buttonState != 2) && !timeout); // bis enter
        display.setColor (WHITE);
        wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_2);
        pos++;
    }
    timeout_t = millis();

    if (m3_init) {
        do {
            if ((millis () - displayinv_old) > 500) { // alle 500ms blinken
                displayinv_old = millis ();
                if (invers) { display.setColor (WHITE); invers = false; }
                else { display.setColor (BLACK); invers = true; }
            }
            // Drehgeber
            if (encoderState == 2) {    //rechtsrum
                timeout_t = millis();     // timeout reset
                displayinv_old = millis ();
                // alten Wert auf OLED löschen
                display.setColor (BLACK);
                wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_3);
                display.setColor (WHITE);
                invers = false;
                RUDDER_LENGTH_3++;
                if (RUDDER_LENGTH_3 > 500)
                    RUDDER_LENGTH_3 = 500;
                tone (BUZZER_PIN, 4000, 2);
            }
            if (encoderState == 1) {    //linksrum
                timeout_t = millis();     // timeout reset
                displayinv_old = millis ();
                display.setColor (BLACK);
                wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_3);
                display.setColor (WHITE);
                invers = false;
                RUDDER_LENGTH_3--;
                if (RUDDER_LENGTH_3 < 10)
                    RUDDER_LENGTH_3 = 10;
                tone (BUZZER_PIN, 4000, 2);
            }
            wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_3);
            display.display ();
            if (((millis() - timeout_t) > 30000) || (buttonState == 4)) {  // nach 30 Sek. Timeout oder "zurück"
                timeout = true;
            }
            ButtonRead ();
        } while ((buttonState != 2) && !timeout); // bis enter
        display.setColor (WHITE);
        wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_3);
        pos++;
    }
    timeout_t = millis();

    if (m4_init) {
        do {
            if ((millis () - displayinv_old) > 500) { // alle 500ms blinken
                displayinv_old = millis ();
                if (invers) { display.setColor (WHITE); invers = false; }
                else { display.setColor (BLACK); invers = true; }
            }
            // Drehgeber
            if (encoderState == 2) {    //rechtsrum
                timeout_t = millis();     // timeout reset
                displayinv_old = millis ();
                // alten Wert auf OLED löschen
                display.setColor (BLACK);
                wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_4);
                display.setColor (WHITE);
                invers = false;
                RUDDER_LENGTH_4++;
                if (RUDDER_LENGTH_4 > 500)
                    RUDDER_LENGTH_4 = 500;
                tone (BUZZER_PIN, 4000, 2);
            }
            if (encoderState == 1) {    //linksrum
                timeout_t = millis();     // timeout reset
                displayinv_old = millis ();
                display.setColor (BLACK);
                wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_4);
                display.setColor (WHITE);
                invers = false;
                RUDDER_LENGTH_4--;
                if (RUDDER_LENGTH_4 < 10)
                    RUDDER_LENGTH_4 = 10;
                tone (BUZZER_PIN, 4000, 2);
            }
            wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_4);
            display.display ();
            if (((millis() - timeout_t) > 30000) || (buttonState == 4)) {  // nach 30 Sek. Timeout oder "zurück"
                timeout = true;
            }
            ButtonRead ();
        } while ((buttonState != 2) && !timeout); // bis enter
        display.setColor (WHITE);
        wm_anz_rudderlenght (pos, anz_sens, RUDDER_LENGTH_4);
        pos++;
    }

    tone (BUZZER_PIN, 4500, 50);

    display.setColor (WHITE);
    wm_anzeige_display ();

    // bei Enter (buttonstate==2): Ruderlänge in EEPROM anspeichern
    eepromWrite(); // Safe changes
}

void EWD_init (void) {
    // Show manual
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, operationString[LANGUAGE]);    // Bedienung
    display.drawString(0, 12, EWD_info1[LANGUAGE]);         // EWD Messung

    // ESPNow Init
    WiFi.mode (WIFI_AP);
    configDeviceAP ();
    Serial.print ("AP MAC: "); 
    Serial.println (WiFi.softAPmacAddress ());
    InitESPNow ();
    esp_now_register_recv_cb (OnDataRecv);
    
    display.setColor (WHITE);
    display.drawString(0, 24, EWD_info2[LANGUAGE]); // 2 Sensoren eben platzieren
    display.drawString(0, 36, EWD_info3[LANGUAGE]); // Sensoren Reset
    display.drawString(0, 48, EWD_info4[LANGUAGE]); // 1x Klick: Winkel reset
    display.display (); // Anzeige auf OLED

    delay(3000);

    M1_GyroLagewert_old = M2_GyroLagewert_old = M3_GyroLagewert_old = M4_GyroLagewert_old = 0;
}
