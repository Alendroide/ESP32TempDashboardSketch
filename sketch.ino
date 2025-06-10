// Librerías WiFi
#include <WiFi.h>
#include <WiFiManager.h>

// Librería MQTT
#include <PubSubClient.h>

// Librerías sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Pines y objetos
OneWire ourWire(18);  // GPIO18
DallasTemperature sensors(&ourWire);
WiFiClient espClient;
PubSubClient client(espClient);

// Configuración MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic = "pepe/esp32/temperatura";

// Temporizador
unsigned long lastRequest = 0;
const unsigned long requestInterval = 3000;

void reconnectMQTT() {
  // Reconectar si es necesario
  while (!client.connected()) {
    Serial.print("🔄 Intentando conectar al broker MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("✅ Conectado al broker MQTT");
    } else {
      Serial.print("❌ fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  // Serial y sensor
  Serial.begin(115200);
  sensors.begin();

  // WiFiManager
  WiFiManager wm;
  wm.resetSettings();
  if (!wm.autoConnect("ESP32_AP", "hola1234")) {
    Serial.println("❌ No se pudo conectar, reiniciando...");
    ESP.restart();
  }

  Serial.println("✅ Conectado a WiFi");

  // MQTT setup
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  if (millis() - lastRequest >= requestInterval) {
    lastRequest = millis();

    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);

    String payload = "{\"degrees\":" + String(temp, 2) + ",\"source\":\"ESP32 DS18B20\"}";

    // Publicar el dato en el topic
    if (client.publish(topic, payload.c_str())) {
      Serial.print("📡 Publicado en MQTT: ");
      Serial.println(payload);
    } else {
      Serial.println("❌ Error al publicar en MQTT");
    }
  }
}
