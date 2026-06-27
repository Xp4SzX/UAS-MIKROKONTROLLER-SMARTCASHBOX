// ==========================================
// mqtt.js
// Smart Tabungan IoT
// ==========================================

// Broker MQTT melalui WebSocket
const MQTT_BROKER = "ws://localhost:9001";

// Topic yang digunakan ESP32
const MQTT_TOPIC = "smarttabungan/data";

// ==========================================
// CONNECT MQTT
// ==========================================

const client = mqtt.connect(MQTT_BROKER);

client.on("connect", () => {
  console.log("MQTT Connected");

  document.getElementById("mqttStatus").innerHTML = "🟢 Connected";

  client.subscribe(MQTT_TOPIC);
});

client.on("error", (err) => {
  console.log(err);

  document.getElementById("mqttStatus").innerHTML = "🔴 Error";
});

client.on("offline", () => {
  document.getElementById("mqttStatus").innerHTML = "🟠 Offline";
});

client.on("reconnect", () => {
  document.getElementById("mqttStatus").innerHTML = "🟡 Reconnecting...";
});

// ==========================================
// MENERIMA DATA DARI ESP32
// ==========================================

client.on("message", (topic, message) => {
  if (topic !== MQTT_TOPIC) return;

  try {
    const data = JSON.parse(message.toString());

    console.log(data);

    // Update Card Dashboard

    document.getElementById("nominal").innerHTML =
      "Rp " + Number(data.nominal).toLocaleString("id-ID");

    document.getElementById("status").innerHTML = data.status;

    document.getElementById("uv").innerHTML = data.uv;

    document.getElementById("saldo").innerHTML =
      "Rp " + Number(data.saldo).toLocaleString("id-ID");

    // Refresh tabel dari database
    loadDashboard();
  } catch (err) {
    console.log("MQTT Error");

    console.log(err);
  }
});


// ==========================================
// MENGIRIM PERINTAH KE ESP32
// ==========================================
function kirimPerintahESP(saldoBaru) {
  if (client.connected) {
    // Mengirim (publish) saldo terbaru ke ESP32 dalam bentuk teks (String)
    client.publish("smarttabungan/perintah", saldoBaru.toString());
    console.log("Kirim ke ESP32 sukses: Rp " + saldoBaru);
  } else {
    console.log("Gagal kirim ke ESP32: MQTT Web Offline");
  }
}