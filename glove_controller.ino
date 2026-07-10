/*
  Wearable IMU Glove Controller — Glove-Side ESP32 Firmware
  --------------------------------------------------------
  Reads pitch/roll from a wrist-mounted MPU6050, classifies the
  hand's tilt into a drive gesture, and publishes the corresponding
  MQTT command to the bot's ESP32.

  Gesture mapping (physical action -> MQTT command):
    palm tilt up    -> forward
    palm tilt down  -> reverse
    roll left       -> left
    roll right      -> right
    flat / neutral  -> stop

  Author: Iniyan
*/

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---- WiFi & MQTT config ----
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "192.168.x.x"; // laptop IP running Mosquitto
const int mqtt_port = 1883;
const char* mqtt_topic = "gesturebot/command";

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_MPU6050 mpu;

String lastCommand = "";
unsigned long lastSentTime = 0;
const unsigned long COOLDOWN = 500; // ms

float getPitch(sensors_event_t a) {
  return atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;
}

float getRoll(sensors_event_t a) {
  return atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
}

// Classification thresholds — calibrate for your own sensor mount.
// Pitch is checked BEFORE roll: forward/reverse gestures can produce
// large roll values near +-180 deg that would otherwise be misread
// as left/right if roll were checked first.
String getPhysicalAction(float pitch, float roll) {
  if (pitch > 50) return "forward";
  else if (pitch < -50) return "reverse";
  else if (roll > 50) return "left";
  else if (roll < -50) return "right";
  else return "stop";
}

String getMqttCommand(String action) {
  // Maps physical action directly to the MQTT command name.
  // If your bot's motor wiring produces mismatched physical motion,
  // adjust this mapping rather than rewiring hardware.
  if (action == "forward") return "forward";
  if (action == "reverse") return "reverse";
  if (action == "left") return "left";
  if (action == "right") return "right";
  return "stop";
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

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32GloveController")) {
      Serial.println("connected");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(); // default SDA=GPIO21, SCL=GPIO22

  if (!mpu.begin()) {
    Serial.println("Error: Could not find MPU6050 sensor! Check wiring.");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.println("Sensor ready.");

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float pitch = getPitch(a);
  float roll = getRoll(a);

  String physicalAction = getPhysicalAction(pitch, roll);
  String mqttCommand = getMqttCommand(physicalAction);

  Serial.print("Pitch: "); Serial.print(pitch, 1);
  Serial.print("  Roll: "); Serial.print(roll, 1);
  Serial.print("  -> Action: "); Serial.println(physicalAction);

  unsigned long now = millis();
  if (mqttCommand != lastCommand || (now - lastSentTime) > COOLDOWN) {
    client.publish(mqtt_topic, mqttCommand.c_str());
    lastCommand = mqttCommand;
    lastSentTime = now;
  }

  delay(150);
}
