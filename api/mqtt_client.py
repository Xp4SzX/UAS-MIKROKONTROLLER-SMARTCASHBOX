# ==========================================
# mqtt_client.py
# MQTT Subscriber Smart Tabungan IoT
# ==========================================

import json
import paho.mqtt.client as mqtt

from models import TransactionModel

# ==========================================
# KONFIGURASI MQTT
# ==========================================

MQTT_BROKER = "192.168.100.29"      # Ganti jika broker berada di komputer lain
MQTT_PORT = 1883
MQTT_TOPIC = "smarttabungan/data"

# ==========================================
# SAAT BERHASIL TERHUBUNG
# ==========================================

def on_connect(client, userdata, flags, rc):

    if rc == 0:
        print("===================================")
        print(" MQTT Connected")
        print("===================================")

        client.subscribe(MQTT_TOPIC)

        print(f"Subscribe Topic : {MQTT_TOPIC}")

    else:
        print("MQTT Connection Failed")


# ==========================================
# SAAT MENERIMA DATA
# ==========================================

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode()
        print("================================")
        print("MQTT PAYLOAD :", payload)

        data = json.loads(payload)

        nominal = data["nominal"]
        uv = data["uv"]
        saldo = data["saldo"]
        status = data["status"]

        print("Nominal :", nominal)
        print("UV :", uv)
        print("Saldo :", saldo)
        print("Status :", status)

        hasil = TransactionModel.insert_transaksi(
            nominal,
            uv,
            saldo,
            status,
            "Masuk",
            "ESP32"
        )

        print("HASIL INSERT =", hasil)

    except Exception as e:
        import traceback
        traceback.print_exc()


# ==========================================
# MENJALANKAN MQTT
# ==========================================

def start_mqtt():

    client = mqtt.Client()

    client.on_connect = on_connect

    client.on_message = on_message

    client.connect(
        MQTT_BROKER,
        MQTT_PORT,
        60
    )

    client.loop_start()

    return client