const express = require("express");
const mqtt = require("mqtt");
const app = express();
const port = 3000;

// Koneksi ke broker MQTT
const mqttClient = mqtt.connect("mqtt://127.0.0.1");

// Event handler ketika terhubung ke MQTT broker
mqttClient.on("connect", () => {
  console.log("Terhubung ke broker MQTT");
});

// Middleware untuk serving static files (halaman web)
app.use(express.static("public"));

// Endpoint untuk publish state ke broker MQTT
app.get("/publish/:switchId/:state", (req, res) => {
  const { switchId, state } = req.params;

  // Format topik dan payload
  const topic = `switch/${switchId}`;
  const message = JSON.stringify({ switch: switchId, state: state.toUpperCase() });

  // Publish ke broker
  mqttClient.publish(topic, message, () => {
    console.log(`Pesan terkirim ke topik "${topic}": ${message}`);
    res.send(`Switch ${switchId} diatur ke ${state.toUpperCase()}`);
  });
});

// Jalankan server
app.listen(port, () => {
  console.log(`Server berjalan di http://localhost:${port}`);
});
