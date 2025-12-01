#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <Update.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// ===== إعدادات WiFi و MQTT =====
WiFiManager wm;
const char* mqtt_server = "192.168.1.14"; // عدّل حسب سيرفرك
const char* mqtt_topic = "esp32/update";
WiFiClient espClient;
PubSubClient client(espClient);

// ===== روابط الملفات =====
const char* version_file_url = "https://raw.githubusercontent.com/amer-maher/wifi-smart/main/version.txt";
const char* firmware_url = "https://github.com/amer-maher/wifi-smart/releases/download/mqqt/wifi.ino.bin"; 

// النسخة الحالية
String CURRENT_VERSION = "1.0";

// ===== دوال =====
void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("Failed MQTT, rc=");
      Serial.print(client.state());
      delay(1000);
    }
  }
}

String getRemoteVersion() {
  WiFiClientSecure secureClient;
  secureClient.setInsecure(); // تجاهل التحقق من الشهادة
  HTTPClient http;
  http.begin(secureClient, version_file_url);
  int httpCode = http.GET();
  String payload = "";
  if (httpCode == 200) {
    payload = http.getString();
    payload.trim();
    Serial.println("Fetched remote version: " + payload);
  } else {
    Serial.printf("Failed to get version, HTTP code: %d\n", httpCode);
  }
  http.end();
  return payload;
}

void performOTA() {
  Serial.println("Starting OTA...");

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);   // مهم جدا
  http.begin(secureClient, firmware_url);

  int httpCode = http.GET();

  if (httpCode == 200) {
    int contentLength = http.getSize();

    if (Update.begin(contentLength)) {
      WiFiClient * stream = http.getStreamPtr();

      size_t written = Update.writeStream(*stream);

      if (written == contentLength) {
        Serial.println("Firmware written successfully");
      } else {
        Serial.println("Written size mismatch!");
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("OTA finished successfully. Rebooting...");
          ESP.restart();
        } else {
          Serial.println("OTA not finished.");
        }
      } else {
        Serial.printf("OTA failed. Error: %s\n", Update.errorString());
      }

    } else {
      Serial.println("Not enough space for OTA");
    }

  } else {
    Serial.printf("HTTP error while downloading firmware: %d\n", httpCode);
  }

  http.end();
}


// ===== setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);

  // الاتصال بالواي فاي
  if (!wm.autoConnect("ESP32-Setup", "12345678")) {
    Serial.println("Failed to connect WiFi, restarting...");
    ESP.restart();
  }

  Serial.println("Connected!");
  Serial.println(WiFi.localIP());

  // الاتصال بـ MQTT
  client.setServer(mqtt_server, 1883);
  reconnectMQTT();
  client.publish(mqtt_topic, "test...");

  // تحقق من النسخة
  String remoteVersion = getRemoteVersion();
  Serial.println("Current version: " + CURRENT_VERSION);
  Serial.println("Remote version: " + remoteVersion);

  if (remoteVersion != "" && remoteVersion != CURRENT_VERSION) {
    Serial.println("New version available! Starting OTA...");
    client.publish(mqtt_topic, "New version available! Starting OTA...");
    performOTA();
  } else {
    Serial.println("No update needed. Running current version.");
    client.publish(mqtt_topic, "No update needed. Running current version.");
  }
}

// ===== loop =====
void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  // هنا ضع كود الجهاز العادي
}
