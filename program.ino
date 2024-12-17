#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wi-Fi and MQTT Configuration
class Config {
public:
    static const char* ssid;
    static const char* password;
    static const char* mqttServer;
    static const int mqttPort;
};

// Wi-Fi and MQTT Configuration Values
const char* Config::ssid = "NAMA_WIFI_ANDA";
const char* Config::password = "PASSWORD_WIFI";
const char* Config::mqttServer = "192.168.1.100";
const int Config::mqttPort = 1883;

// Switch Controller Class
class SwitchController {
private:
    const int* pins;
    const char** topics;
    int count;

public:
    // Konstruktor untuk SwitchController
    SwitchController(const int* pins, const char** topics, int count)
        : pins(pins), topics(topics), count(count) {}

    // Fungsi untuk menginisialisasi pin sebagai OUTPUT
    void initialize() {
        for (int i = 0; i < count; ++i) {
            pinMode(pins[i], OUTPUT);
            digitalWrite(pins[i], LOW);
        }
    }

    // Fungsi untuk menangani pesan yang diterima dari broker MQTT
    void handleMessage(const String& topic, const String& message) {
        for (int i = 0; i < count; ++i) {
            if (topic == String(topics[i])) {
                if (message == "{\"switch\":\"" + String(i + 1) + "\",\"state\":\"ON\"}") {
                    digitalWrite(pins[i], HIGH);
                    Serial.println("Switch " + String(i + 1) + " ON");
                } else if (message == "{\"switch\":\"" + String(i + 1) + "\",\"state\":\"OFF\"}") {
                    digitalWrite(pins[i], LOW);
                    Serial.println("Switch " + String(i + 1) + " OFF");
                }
            }
        }
    }
};

// Global Instances
const char* switchTopics[] = {"switch/1", "switch/2", "switch/3", "switch/4", "switch/5", "switch/6"};
const int switchPins[] = {5, 4, 14, 12, 13, 15};
SwitchController switchController(switchPins, switchTopics, 6);

WiFiClient espClient;
PubSubClient client(espClient);


/**
 * @brief Menghubungkan ke jaringan Wi-Fi menggunakan SSID dan kata sandi yang ditentukan dalam kelas Config.
 * 
 * Fungsi ini mencoba menghubungkan ke jaringan Wi-Fi dan mencetak status koneksi ke monitor Serial.
 * Fungsi ini terus memeriksa status koneksi dan mencetak titik (.) setiap 500 milidetik sampai koneksi berhasil.
 * Setelah terhubung, fungsi ini mencetak alamat IP yang diberikan ke perangkat.
 * 
 * @note Pastikan bahwa Config::ssid dan Config::password telah diatur dengan benar sebelum memanggil fungsi ini.
 */
void connectToWiFi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(Config::ssid, Config::password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected. IP Address: " + WiFi.localIP().toString());
}

/**
 * @brief Fungsi callback yang dipanggil ketika pesan diterima dari broker MQTT.
 * 
 * Fungsi ini mencetak topik dan pesan yang diterima ke monitor Serial.
 * Fungsi ini memanggil SwitchController::handleMessage dengan topik dan pesan yang diterima.
 * 
 * @param topic Topik pesan yang diterima.
 * @param payload Isi pesan yang diterima.
 * @param length Panjang pesan yang diterima.
 */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; ++i) {
        message += (char)payload[i];
    }
    Serial.println("Message received on topic: " + String(topic));
    Serial.println("Message: " + message);
    switchController.handleMessage(String(topic), message);
}

/**
 * @brief Menghubungkan kembali ke broker MQTT menggunakan klien yang telah dibuat.
 * 
 * Fungsi ini mencoba menghubungkan kembali ke broker MQTT dan mencetak status koneksi ke monitor Serial.
 * Fungsi ini terus mencoba menghubungkan kembali setiap 5 detik sampai berhasil terhubung.
 * Setelah terhubung, fungsi ini mendaftarkan kembali semua topik yang telah diatur.
 */
void reconnectMQTT() {
    while (!client.connected()) {
        Serial.println("Connecting to MQTT broker...");
        if (client.connect("ESP8266Client")) {
            Serial.println("Connected to MQTT broker.");
            for (int i = 0; i < 6; ++i) {
                client.subscribe(switchTopics[i]);
                Serial.println("Subscribed to topic: " + String(switchTopics[i]));
            }
        } else {
            Serial.println("Failed to connect. Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

/**
 * @brief Setup Arduino yang dipanggil sekali saat perangkat dinyalakan.
 * 
 * Fungsi ini memulai monitor Serial, menghubungkan ke jaringan Wi-Fi, dan menginisialisasi koneksi ke broker MQTT.
 * Fungsi ini juga menginisialisasi SwitchController dan mengatur pin sebagai OUTPUT.
 */
void setup() {
    Serial.begin(115200);
    connectToWiFi();
    client.setServer(Config::mqttServer, Config::mqttPort);
    client.setCallback(mqttCallback);
    switchController.initialize();
}

/**
 * @brief Loop Arduino yang dipanggil secara berulang.
 * 
 * Fungsi ini memeriksa koneksi ke broker MQTT dan memanggil fungsi reconnectMQTT jika koneksi terputus.
 * Fungsi ini juga memanggil client.loop untuk memproses pesan yang diterima dari broker MQTT.
 */
void loop() {
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();
}
