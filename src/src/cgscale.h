#pragma once

#include <Arduino.h>

void cg_init(void) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, operationString[LANGUAGE]);
  display.drawString(0, 12, wm_info1[LANGUAGE]);
  display.drawString(0, 24, wm_info2[LANGUAGE]);
  display.drawString(0, 36, wm_info3[LANGUAGE]);
  display.drawString(0, 48, wm_info4[LANGUAGE]);
  display.display();

  WiFi.mode(WIFI_AP);
  configDeviceAP();
  Serial.print("AP MAC: ");
  Serial.println(WiFi.softAPmacAddress());
  InitESPNow();
  esp_now_register_recv_cb(OnDataRecv);

  delay(3000);
}

void cg_measure() {
  if (EspNowCGLastUpdate > millis() - 1000) {
    display.clear();

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    if (ESPNowData_CG_Bat_Type > 0) {
      // if(ESPNowData_CG_Bat_Type > 0){
      int factor = ESPNowData_CG_Bat_Type > 1 ? 0 : 2;
      display.drawString(128, 0, String(ESPNowData_CG_U_Lipo, factor) + (ESPNowData_CG_Bat_Type > 1 ? "%" : "V"));
    }

    display.setFont(ArialMT_Plain_10);

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, CgScaleTitle[LANGUAGE]);

    display.setFont(ArialMT_Plain_24);

    display.drawXbm(0, 23, 18, 18, weightImage);
    display.drawXbm(0, 43, 18, 18, CGImage);

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(101, 20, String(ESPNowData_CG_Model_Weight, 1));
    display.drawString(101, 40, String(ESPNowData_CG_Model_CG, 1));

    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(103, 28, "g");
    display.drawString(103, 48, "mm");

    display.display();
  } else {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawRect(15, 15, (128 - 2 * 15), (64 - 2 * 15));
    display.drawString(64, 20, CG_Scale_info1[LANGUAGE]);
    display.drawString(64, 30, CG_Scale_Not_Connected1[LANGUAGE]);
    display.display();
  }
}

void cg_scale_init(void) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, operationString[LANGUAGE]);
  display.drawString(0, 12, CG_Scale_info2[LANGUAGE]);

  WiFi.mode(WIFI_AP);
  configDeviceAP();
  Serial.print("AP MAC: ");
  Serial.println(WiFi.softAPmacAddress());
  InitESPNow();
  esp_now_register_recv_cb(OnDataRecv);

  display.setColor(WHITE);
  display.drawString(0, 24, CG_Scale_info3[LANGUAGE]);
  display.drawString(0, 36, CG_Scale_info4[LANGUAGE]);
  display.drawString(0, 50, CG_Scale_info5[LANGUAGE]);
  display.display();

  delay(3000);
}
