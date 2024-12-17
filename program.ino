#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Konfigurasi Wi-Fi
const char* ssid = "NAMA_WIFI_ANDA";      // Ganti dengan SSID Wi-Fi Anda
const char* password = "PASSWORD_WIFI";   // Ganti dengan Password Wi-Fi Anda

// Konfigurasi Broker MQTT
const char* mqtt_server = "192.168.1.100"; // IP Address komputer (broker Mosquitto)
const int mqtt_port = 1883;

// Topik untuk switch
const char* switch_topics[] = {
  "switch/1",
  "switch/2",
  "switch/3",
  "switch/4",
  "switch/5",
  "switch/6"
};

// Pin yang digunakan untuk kontrol LED/switch
const int switch_pins[] = {5, 4, 14, 12, 13, 15}; // D1, D2, D5, D6, D7, D8 pada ESP8266

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi untuk koneksi ke Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Terhubung!");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Callback untuk menerima pesan MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Pesan diterima di topik: ");
  Serial.println(topic);
  Serial.print("Isi pesan: ");
  Serial.println(message);

  // Bandingkan topik dan nyalakan/matikan switch
  for (int i = 0; i < 6; i++) {
    if (String(topic) == switch_topics[i]) {
      if (message == "{\"switch\":\"" + String(i + 1) + "\",\"state\":\"ON\"}") {
        digitalWrite(switch_pins[i], HIGH); // Nyalakan
        Serial.println("Switch " + String(i + 1) + " ON");
      } else if (message == "{\"switch\":\"" + String(i + 1) + "\",\"state\":\"OFF\"}") {
        digitalWrite(switch_pins[i], LOW); // Matikan
        Serial.println("Switch " + String(i + 1) + " OFF");
      }
    }
  }
}

// Koneksi ulang ke broker jika terputus
void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi ke MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Terhubung!");
      // Subscribe ke semua topik switch
      for (int i = 0; i < 6; i++) {
        client.subscribe(switch_topics[i]);
        Serial.println("Subscribed ke topik: " + String(switch_topics[i]));
      }
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" Coba lagi dalam 5 detik...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Set pin sebagai output
  for (int i = 0; i < 6; i++) {
    pinMode(switch_pins[i], OUTPUT);
    digitalWrite(switch_pins[i], LOW);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
