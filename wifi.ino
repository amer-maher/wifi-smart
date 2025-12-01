#include <WiFi.h>
#include <WiFiManager.h>

#define RESET_PIN 0  
WiFiManager wm;

void setup() {
  Serial.begin(115200);
  pinMode(RESET_PIN, INPUT_PULLUP);

  // إذا تم الضغط على الزر → امسح الإعدادات
  if (digitalRead(RESET_PIN) == LOW) {
    Serial.println("زر المسح مضغوط! جاري مسح إعدادات WiFi...");
    wm.resetSettings();
    delay(1000);
    Serial.println("تم مسح الإعدادات. إعادة التشغيل...");
    ESP.restart();
  }

  // اتصال تلقائي أو فتح AP عند الحاجة
  if (!wm.autoConnect("ESP32-Setup", "12345678")) {
    Serial.println("فشل الاتصال! إعادة تشغيل...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("✔️ تم الاتصال بنجاح!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // لا شيء هنا
}
