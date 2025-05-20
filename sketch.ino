// Librerías WiFi
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>

// Librerías sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Inicializaciones

OneWire ourWire(18);  // GPIO18 - D9
DallasTemperature sensors(&ourWire);
unsigned long lastRequest = 0;
const unsigned long requestInterval = 3000;

void setup() {

  // Inicialización de WiFiManager
  WiFiManager wm;
  wm.resetSettings();

  // Inicialización de Serial y librería sensors
  Serial.begin(115200);
  sensors.begin();

  // En caso de no lograrse la conexión automática, se crea el Access Point del ARDUINO
  if(!wm.autoConnect("ESP32_AP","hola1234")) {
    Serial.println("❌ No se pudo conectar, reiniciando");
    ESP.restart();
  }

  Serial.println("✅ Conectado a WiFi");
  
}
 
void loop() {

  if (millis() - lastRequest >= requestInterval) {
    
    lastRequest = millis();

    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);

    if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String serverUrl = "http://192.168.101.88:3000/temperatures";

      http.begin(serverUrl);

      http.addHeader("Content-Type","application/json");

      String payload = "{\"degrees\":" + String(temp, 2) + ",\"source\":\"ESP32 DS18B20\"}";

      int httpCode = http.POST(payload);

      if (httpCode > 0) {
        Serial.print("📨 POST enviado. Código: ");
        Serial.println(httpCode);
        Serial.println("Respuesta:");
        Serial.println(http.getString());
      } else {
        Serial.print("❌ Error en POST: ");
        Serial.println(http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.println("WiFi desconectado!");
    }
  }
}
