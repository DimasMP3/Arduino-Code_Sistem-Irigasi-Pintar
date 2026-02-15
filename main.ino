#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "";
const char* pass = "";

const char* mqtt_server = "";
const int mqtt_port = 8883;
const char* mqtt_user = "";
const char* mqtt_pass = "";
const char* client_id = "";

const char* topic_data = "sensor/tanah/data";
const char* topic_kontrol = "kontrol/pompa";
const char* topic_status = "pompa/status";

const int soilMoisturePin = A0;
const int relayPin = D0;

const int soilWet = 350;
const int soilDry = 950;

int moisturePercent = 0;
bool manualMode = false;
bool pompaStatus = false;

unsigned long lastMsgTime = 0;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.printf("Pesan diterima di topik [%s]: %s\n", topic, message.c_str());

  if (String(topic) == topic_kontrol) {
    if (message == "MANUAL_ON") {
      manualMode = true;
      digitalWrite(relayPin, LOW);
      pompaStatus = true;
      Serial.println("Mode Manual: Pompa dinyalakan.");
    } else if (message == "MANUAL_OFF") {
      manualMode = true;
      digitalWrite(relayPin, HIGH);
      pompaStatus = false;
      Serial.println("Mode Manual: Pompa dimatikan.");
    } else if (message == "AUTO") {
      manualMode = false;
      Serial.println("Mode diubah ke Otomatis.");
    }
    client.publish(topic_status, pompaStatus ? "ON" : "OFF", true);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba terhubung ke MQTT Broker...");
    if (client.connect(client_id, mqtt_user, mqtt_pass)) {
      Serial.println(" Terhubung!");
      client.subscribe(topic_kontrol);
    } else {
      Serial.printf(" Gagal, rc=%d. Coba lagi dalam 5 detik\n", client.state());
      delay(5000);
    }
  }
}

void checkMoistureAndControl() {
  int sensorValue = analogRead(soilMoisturePin);
  moisturePercent = map(sensorValue, soilDry, soilWet, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.printf("Kelembapan Terbaca: %d%% (Nilai mentah: %d)\n", moisturePercent, sensorValue);

  String jsonData = "{\"persen\":" + String(moisturePercent) + "}";
  client.publish(topic_data, jsonData.c_str());

  if (!manualMode) {
    bool newPompaStatus = (moisturePercent < 60);

    if (newPompaStatus != pompaStatus) {
      pompaStatus = newPompaStatus;
      digitalWrite(relayPin, pompaStatus ? LOW : HIGH);
      Serial.printf("Mode Otomatis: Pompa -> %s\n", pompaStatus ? "ON" : "OFF");

      client.publish(topic_status, pompaStatus ? "ON" : "OFF", true);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  Serial.printf("Menyambungkan ke WiFi: %s\n", ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());

  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentTime = millis();
  if (currentTime - lastMsgTime > 1000) {
    lastMsgTime = currentTime;
    checkMoistureAndControl();
  }
}