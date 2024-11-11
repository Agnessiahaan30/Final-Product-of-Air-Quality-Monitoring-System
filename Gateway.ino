#include <PubSubClient.h>
#include <WiFi.h> // Jika Anda menggunakan WiFi
#include <Wire.h> // Jika Anda menggunakan Ethernet

#define RXp2 18
#define TXp2 19

const char* ssid = "ta5";
const char* password = "kelompok5";
const char* mqtt_server = "broker.emqx.io"; // Ganti dengan alamat server MQTT yang sesuai

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages here
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ArduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2);
  setup_wifi(); // Menginisialisasi koneksi WiFi
  client.setServer(mqtt_server, 1883); // Atur alamat server MQTT dan portnya
  client.setCallback(callback); // Atur fungsi yang akan dipanggil saat pesan masuk
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Membaca data dari Serial2 dan mengirimkannya melalui MQTT
  if (Serial2.available()) {
    String message = Serial2.readString();
    client.publish("testtopic/kelompok5TA", message.c_str());
  }
}