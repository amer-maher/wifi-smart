#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

// ===== إعدادات GitHub =====
const char* version_file_url = "https://raw.githubusercontent.com/amer-maher/wifi-smart/main/version.txt";
const char* firmware_url     = "https://raw.githubusercontent.com/amer-maher/wifi-smart/main/wifi.ino.bin";

// النسخة الحالية للجهاز
const char* CURRENT_VERSION = "1.0";

// ===== WiFi Manager =====
WiFiManager wm;

// ==================== دوال ====================

// جلب النسخة الجديدة من GitHub
String getRemoteVersion(const char* url) {
  WiFiClientSecure client;
  client.setInsecure(); // لتخطي شهادة SSL أثناء الاختبار
  HTTPClient http;
  http.begin(client, url);

  int httpCode = http.GET();
  String payload = "";
  if (httpCode == 200) {
    payload = http.getString();
    payload.trim();
    Serial.print("Fetched remote version: ");
    Serial.println(payload);
  } else {
    Serial.printf("Failed to get version, HTTP code: %d\n", httpCode);
  }

  http.end();
  return payload;
}

// تنفيذ التحديث OTA
void performUpdate(const char* url) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);

  int httpCode = http.GET();
  if (httpCode == 200) {
    int contentLength = http.getSize();
    Serial.printf("Firmware size: %d bytes\n", contentLength);

    if (Update.begin(contentLength)) {
      WiFiClient* stream = http.getStreamPtr();
      size_t written = Update.writeStream(*stream);

      if (written == contentLength) Serial.println("Firmware written successfully.");
      else Serial.println("Written size mismatch!");

      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("OTA finished successfully. Rebooting...");
          delay(2000);
          ESP.restart();
        } else {
          Serial.println("OTA not finished. Something went wrong.");
        }
      } else {
        Serial.printf("OTA error: %s\n", Update.errorString());
      }
    } else {
      Serial.println("Not enough space for OTA!");
    }
  } else {
    Serial.printf("HTTP error while downloading firmware: %d\n", httpCode);
  }

  http.end();
}

// ==================== setup ====================
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("ESP32 booting...");

  // اتصال بالواي فاي
  if (!wm.autoConnect("ESP32-Setup", "12345678")) {
    Serial.println("Failed to connect WiFi, restarting...");
    ESP.restart();
  }
  Serial.println("Connected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // جلب النسخة الجديدة
  String remoteVersion = getRemoteVersion(version_file_url);
  Serial.print("Current version: ");
  Serial.println(CURRENT_VERSION);
  Serial.print("Remote version: ");
  Serial.println(remoteVersion);

  // مقارنة النسخ وتنفيذ التحديث
  if (remoteVersion != "" && remoteVersion != CURRENT_VERSION) {
    Serial.println("New version available! Starting OTA...");
    performUpdate(firmware_url);
  } else {
    Serial.println("No update needed. Running current version.");
  }
}

// ==================== loop ====================
void loop() {
  // ضع هنا كود الجهاز العادي
  // مثال تجريبي لطباعة كل 5 ثواني
  Serial.println("Device running...");
  delay(5000);
}
