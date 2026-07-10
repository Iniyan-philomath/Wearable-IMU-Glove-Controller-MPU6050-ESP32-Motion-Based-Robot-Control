/*
  Wearable IMU Glove Controller — Bot-Side ESP32 Firmware
  --------------------------------------------------------
  Identical to the bot firmware used in the camera-based gesture
  project. Subscribes to MQTT commands (published here by the glove
  controller instead of a webcam script) and drives two DC motors
  via an L298N driver.

  Commands accepted on topic "gesturebot/command":
    "forward", "reverse", "left", "right", "stop"

  Author: Iniyan
*/

#include <WiFi.h>
#include <PubSubClient.h>

// ---- WiFi & MQTT config ----
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "192.168.x.x";  // your laptop's IP running Mosquitto
const int mqtt_port = 1883;
const char* mqtt_topic = "gesturebot/command";

WiFiClient espClient;
PubSubClient client(espClient);

// ---- Motor pins ----
#define ENA 32
#define ENB 33
#define IN1 26
#define IN2 27
#define IN3 14
#define IN4 25

int speedVal = 200; // 0-255

void setSpeed(int s) {
  ledcWrite(ENA, s);
  ledcWrite(ENB, s);
}

void forward() {
  setSpeed(speedVal);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void reverse() {
  setSpeed(speedVal);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeft() {
  setSpeed(speedVal);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight() {
  setSpeed(speedVal);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void stopCar() {
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void setup_wifi() {
  delay(100);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();
  Serial.println("Command: " + msg);

  if (msg == "forward") forward();
  else if (msg == "reverse") reverse();
  else if (msg == "left") turnLeft();
  else if (msg == "right") turnRight();
  else if (msg == "stop") stopCar();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32GestureBot")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  ledcAttach(ENA, 5000, 8);
  ledcAttach(ENB, 5000, 8);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
