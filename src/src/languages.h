#pragma once

#include <Arduino.h>
#include <map>
// Language ressources strings for OLED menus

int noOfLanguages = 2;  // 0 = English, 1 = Deutsch, 2 = Francais

// Einstellung
String languagesString[]{"English", "Deutsch", "Français"};
String settingsString[]{"Settings", "Einstellung", "Paramètres"};
String onString[]{"On", "Ein", "activé"};
String offString[]{"Off", "Aus", "désactivé"};
String noString[]{"No", "Nein", "Non"};
String yesString[]{"Yes", "Ja", "Oui"};
String factoryResetString[]{"! Factory Reset !", "! Werksreset !", "! Réinitialiser !"};
String servoStepsString[]{"Servo Steps µs", "Servo Schritte µs", "étape servo µs"};
String servoMaxString[]{"Servo Max. µs", "Servo Max. µs", "Servo Max. µs"};
String servoMinString[]{"Servo Min. µs", "Servo Min. µs", "Servo Min. µs"};
String servoCenterString[]{"Servo Center µs", "Servo Mitte µs", "Servo centre µs"};
String servoHzString[]{"Servo Hz", "Servo Hz", "Servo Hz"};
String PowerScaleString[]{"POWER Scale", "POWER Skala", "échelle POWER"};
String inversedString[]{"Inversed", "Invertiert", "Inversé"};
String standardString[]{"Standard", "Standard", "Défaut"};
String encoderDirectionString[]{"Encoder direction", "Encoder Richtung", "Encodeur direct."};
String languageString[]{"Language", "Sprache", "Langue"};
String pongBallRateString[]{"Pong ball speed", "Pong Ball Gesch.", "Pong V. de balle"};
String manualOnStartupString[]{"Manual on startup", "Bedien. bei. Start", "Manual on startup"};
String functionsString[]{"Enabled", "Aktiviert", "Enabled"};
String functionsInfo1String[]{"Display", "Funktionen", "Display"};
String functionsInfo2String[]{"functions", "Anzeigen", "functions"};

// Impuls lesen
String impulseString[]{"Impulse", "Impuls", "Impulsion"};
String impulseSignalString[]{"No signal", "Kein Signal", "Pas de signal"};

// Automatik
String delayString[]{"Delay", "Verz.", "Ret."};

// Rechner
String calculatorString[]{"Calculator", "Rechner", "Calculateur"};

// Auswahl
String pongString[]{"Pong", "Pong", "Pong"};
String flappyBirdsString[]{"Flappy Birds", "Flappy Birds", "Flappy Birds"};
String servotesterString[]{"Servotester", "Servotester", "Testeur de servos"};
String readIbusString[]{"Read IBUS", "IBUS lesen", "Lire IBUS"};
String readSbusString[]{"Read SBUS", "SBUS lesen", "Lire SBUS"};
String readCh5String[]{"read CH5", "lesen CH5", "lire CH5"};
String readCh1Ch5String[]{"read CH1 - 5", "lesen CH1 - 5", "lire CH1 - 5"};
String PwmImpulseString[]{"PWM Impulse", "PWM Impuls", "PWM Impulsion"};
String MultiSwitchString[]{"Multiswitch", "Multiswitch", "Multiswitch"};
String automaticModeString[]{"Automatic Mode", "Automatik Modus", "Mode automat."};
String oscillateServoString[]{"(Oscillate Servo)", "(Servo pendeln)", "(Osciller Servo)"};
String angleModeString[]{"Angle Mode", "Winkelmessung", "Angle Mode"};
String rudderMeasureString[]{"Ruderausschläge", "Ruderausschläge", "Ruderausschläge"};
String EWDModeString[]{"EWD Mode", "EWD Messung", "EWD Mode"};
String EWDMeasureString[]{"Einst.Winkel Diff.", "Einst.Winkel Diff.", "Einst.Winkel Diff."};
String CgScaleMenuString1[]{"CG Scale", "Schwerpunkt", "Cg Scale"};
String CgScaleMenuString2[]{"", "waage", ""};
String readOscilloscopeString[]{"Oscilloscope CH5", "Oszilloskop CH5", "Oscilloscope CH5"};
String readOscilloscopeString2[]{"0- 3.3V Signals only", "Nur 0 - 3.3V Signale", "Signaux 0 - 3.3V uniquem."};
String signalGeneratorString[]{"Signal Generator", "Signal Generator", "Générat. de signal"};
String signalGeneratorString2[]{"0 - 3.3V GPIO 26", "0 - 3.3V  GPIO 26", "0 - 3.3V  GPIO 26"};

// Setup
String passwordString[]{"Password:", "Passwort:", "Mot de passe:"};
String ipAddressString[]{"IP address:", "IP Adresse:", "IP Adresse:"};
String WiFiOnString[]{"WiFi On", "WiFi Ein", "WiFi activé"};
String WiFiOffString[]{"WiFi Off", "WiFi Aus", "WiFi desactivé"};
String apIpAddressString[]{"AP-IP-Adress: ", "AP-IP-Adresse: ", "AP-IP-Adresse: "};
String connectingAccessPointString[]{"AP (Accesspoint) connecting…", "AP (Zugangspunkt) einstellen…", "connecter le point d'accès…"};

// Instructions
String operationString[]{"******** Operation: ********", "******** Bedienung: ********", "****** Mode d'emploi : ******"};
String shortPressString[]{"Short Press = Select", "Kurz Drücken = Auswahl", "Appui brièvem. = Sélection"};
String longPressString[]{"Long Press = Back", "Lang Drücken = Zurück", "Appui long   = Retour"};
String doubleclickString[]{"Doubleclick = CH Change", "Doppelklick = CH Wechsel", "Double-clique = CH Changem."};
String RotateKnobString[]{"Rotate = Scroll / Adjust", "Drehen = Blättern / Einstell.", "Tourner     = Défiler"};

// EEPROM
String eepromReadString[]{"EEPROM read.", "EEPROM gelesen.", "EEPROM lire."};
String eepromWrittenString[]{"EEPROM written.", "EEPROM geschrieben.", "EEPROM écrit."};
String eepromInitString[]{"EEPROM initialized.", "EEPROM initialisiert.", "EEPROM initialisé."};

// Battery
String eepromVoltageString[]{"Battery voltage: ", "Akkuspannung: ", "Voltage de batterie: "};
String noBatteryString[]{"No Battery connected", "Kein Akku angeschlossen", "Pas de batterie connectée"};

// Rudersensoren
String wm_info1[]{"1x click: angle reset", "1x Klick: Winkel reset", "1x click: angle reset"};
String wm_info2[]{"2x click: set rudder length", "2x Klick: Rüderlänge", "2x click: set rudder length"};
String wm_info3[]{"s: Ruderausschlag", "s: Ruderausschlag", "s: Ruderausschlag"};
String wm_info4[]{"L: Ruderlänge", "L: Ruderlänge", "L: Ruderlänge"};

// EWD Messung
String EWD_info1[]{"EWD Messung", "EWD Messung", "EWD Messung"};
String EWD_info2[]{"2 Sensoren eben platzieren", "2 Sensoren eben platzieren", "2 Sensoren eben platzieren"};
String EWD_info3[]{"sensor reset", "Sensoren Reset:", "sensor reset"};
String EWD_info4[]{"1x click: angle reset", "1x Klick: Winkel reset", "1x click: angle reset"};
String EWD_info5[]{"max. 2 Sensoren platzieren", "max. 2 Sensoren platzieren", "max. 2 Sensoren platzieren"};

// CG Scale
String CgScaleTitle[]{"CG Scale", "Schwerpunktwaage", "Cg Scale"};
String CG_Scale_info1[]{"CG Scale", "Schwerpunktwaage", "CG Scale"};
String CG_Scale_info2[]{"1: Open this menu", "1: Dieses Menü öffnen", "1: Open this menu"};
String CG_Scale_info3[]{"2: Turn on CG Scale", "2: Schwerpunktwaage", "2: Turn on CG Scale"};
String CG_Scale_info4[]{"", "    einschalten", ""};
String CG_Scale_info5[]{"Implemented by Tjark :)", "Implemented by Tjark :)", "Implemented by Tjark :)"};
String CG_Scale_Not_Connected1[]{"not connected", "nicht verbunden", "not connected"};


std::map<String, String *> menuKeyMap = {
    {"menu_servo_tester", servotesterString},
    {"menu_auto_mode", automaticModeString},
    {"menu_angle_measurement", angleModeString},
    {"menu_ewd", EWDModeString},
    {"menu_cg_scale", CgScaleMenuString1},
    {"menu_pulse_read", PwmImpulseString},
    {"menu_multiswitch_read", MultiSwitchString},
    {"menu_sbus_read", readSbusString},
    {"menu_ibus_read", readIbusString},
    {"menu_oscilloscope", readOscilloscopeString},
    {"menu_signal_generator", signalGeneratorString},
    {"menu_calculator", calculatorString},
    {"menu_pong", pongString},
    {"menu_flappy_birds", flappyBirdsString},
    {"menu_settings", settingsString}};