# ==========================================
# mqtt_client.py
# MQTT Subscriber Smart Tabungan IoT
# ==========================================

import json
import paho.mqtt.client as mqtt

from models import TransactionModel
from telegram_bot import kirim_notif_telegram  # 🟢 1. IMPORT FUNGSI TELEGRAM BOT

# ==========================================
# KONFIGURASI MQTT
# ==========================================

MQTT_BROKER = "webesp32smartcashbox.cloud.shiftr.io"      
MQTT_PORT = 1883
MQTT_TOPIC = "smarttabungan/data"

# ==========================================
# SAAT BERHASIL TERHUBUNG
# ==========================================

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("===================================")
        print(" MQTT Connected to Shiftr.io")
        print("===================================")

        client.subscribe(MQTT_TOPIC)
        print(f"Subscribe Topic : {MQTT_TOPIC}")
    else:
        print(f"MQTT Connection Failed! Kode Error (rc): {rc}")


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

        # Menyimpan ke Database MySQL
        hasil = TransactionModel.insert_transaksi(
            nominal,
            uv,
            saldo,
            status,
            "Masuk",
            "ESP32"
        )
        print("HASIL INSERT =", hasil)

        # 🟢 2. KIRIM LAPORAN KE TELEGRAM SETELAH BERHASIL INSERT DATABASE
        kirim_notif_telegram(
            nominal=nominal,
            saldo=saldo,
            status=status,
            sumber="Celengan ESP32"
        )

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

    # Kredensial Shiftr.io Cloud Anda
    client.username_pw_set("webesp32smartcashbox", "FUQ8wyi7ZYrywNLF")

    client.connect(
        MQTT_BROKER,
        MQTT_PORT,
        60
    )

    client.loop_start()

    return client