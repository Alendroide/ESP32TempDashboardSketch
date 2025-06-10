// Librerías WiFi
#include <WiFi.h>
#include <WiFiManager.h>

// Librería MQTT
#include <PubSubClient.h>

// Librerías sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Pines para el sensor MQ135
const int mq135Pin = 1;  // GPIO1 (A0) para el MQ135
const int ds18b20Pin = 18; // GPIO18 (D9) para el DS18B20

// Pines y objetos
OneWire ourWire(ds18b20Pin);
DallasTemperature sensors(&ourWire);
WiFiClient espClient;
PubSubClient client(espClient);

// Configuración MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* topic_temp = "pepe/esp32/temperatura";
const char* topic_air = "pepe/esp32/aire";

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

    // Leer temperatura DS18B20
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);

    // Leer valor MQ135
    int mq135Value = analogRead(mq135Pin);  // Lee el valor analógico
    float airQuality = mq135Value * (3.1 / 4095.0);  // Convertir a voltaje (0-3.1V)

    // Preparar los datos a enviar
    String tempPayload = "{\"degrees\":" + String(temp, 2) + ",\"source\":\"ESP32 DS18B20\"}";
    String airPayload = "{\"airQuality\":" + String(airQuality, 2) + ",\"source\":\"ESP32 MQ135\"}";

    // Publicar el dato en el tópico de temperatura
    if (client.publish(topic_temp, tempPayload.c_str())) {
      Serial.print("📡 Temperatura publicada: ");
      Serial.println(tempPayload);
    } else {
      Serial.println("❌ Error al publicar temperatura en MQTT");
    }

    // Publicar el dato en el tópico de calidad de aire
    if (client.publish(topic_air, airPayload.c_str())) {
      Serial.print("📡 Calidad de aire publicada: ");
      Serial.println(airPayload);
    } else {
      Serial.println("❌ Error al publicar calidad de aire en MQTT");
    }
  }
}
