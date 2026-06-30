# 💰 Smart Cash Box IoT

Sistem **Smart Cash Box berbasis Internet of Things (IoT)** yang menggunakan **ESP32**, **MQTT**, **Flask**, **MySQL**, dan **Telegram Bot** untuk melakukan monitoring tabungan secara real-time melalui website maupun Telegram.

---

# 📖 Deskripsi

Smart Cash Box merupakan sebuah celengan pintar yang mampu menghitung saldo secara otomatis berdasarkan nominal uang yang dimasukkan. Seluruh data dikirim menggunakan protokol **MQTT** ke server, kemudian ditampilkan pada **Web Dashboard** serta dikirimkan sebagai notifikasi ke **Telegram Bot**.

Proyek ini dibuat sebagai tugas **Ujian Akhir Semester Mata Kuliah Mikrokontroler**.

---

# ✨ Fitur

* Monitoring saldo secara real-time
* Dashboard berbasis web
* Komunikasi menggunakan MQTT
* Penyimpanan data ke database
* Riwayat transaksi
* Notifikasi Telegram
* Mendeteksi uang berdasarkan warna

---

# 🛠 Teknologi yang Digunakan

## Hardware

* ESP32
* LCD I2C 16x2
* Sensor warna (TCS3200)
* Sensor UV Guva
* Breadboard
* Resistor 1k, 100
* Kabel jumper male to male, female to male
* LED UV
* LED RGB
* Buzzer

## Software

* ESP32 
* Python
* Flask
* HTML
* CSS
* JavaScript
* MQTT (Mosquitto)
* MySQL
* NgRok
* shiftr.io cloude

---

# 📁 Struktur Project

```text
UAS-MIKROKONTROLLER-SMARTCASHBOX-main/
│
├── api/
│   ├── app.py
│   ├── routes.py
│   ├── database.py
│   ├── mqtt_client.py
│   ├── telegram_bot.py
│   ├── models.py
│   ├── templates/
│   └── static/
│
├── ESPCODE.ino
├── ESPCODE_RTOS.ino
├── requirements.txt
└── README.md
```

---

# 🔄 Alur Sistem

```text
ESP32
   │
   ▼
MQTT Broker
   │
   ▼
Flask Backend
   │
   ├────────► Database
   │
   ├────────► Web Dashboard
   │
   └────────► Telegram Bot
```

---

# 📦 Dependency

```
Flask==3.0.3
Flask-Cors==5.0.0
paho-mqtt==2.1.0
mysql-connector-python==9.0.0
```

---

# 👨‍💻 Anggota Kelompok

| Nama                   | NIM         |
| ---------------------- | ----------- |
| Arrafi Hilmi           | 23552011388 |
| Naufal Annafi Alistair | 23552011308 |
| Sulastian Setiadi      | 23552011342 |


