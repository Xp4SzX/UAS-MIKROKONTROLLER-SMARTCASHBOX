# ==========================================
# telegram_bot.py
# Utilitas Notifikasi Telegram
# ==========================================

import requests

# ✅ Token sudah dimasukkan sesuai dari BotFather
TELEGRAM_TOKEN = "8554484076:AAEPwQJjin3drjvTIw9qpvVIRq58M3ZAUn8"

# ⚠️ MASUKKAN CHAT ID ANDA DI SINI (Hanya berupa angka, contoh: "123456789")
CHAT_ID = "8611891086"

def kirim_notif_telegram(nominal, saldo, status, sumber="Celengan ESP32"):
    nominal_format = f"{nominal:,}".replace(",", ".")
    saldo_format = f"{saldo:,}".replace(",", ".")
    
    pesan = (
        "🤖 *LAPORAN SMART TABUNGAN*\n"
        "=============================\n"
        f"📥 *Sumber:* {sumber}\n"
        f"💵 *Nominal:* Rp {nominal_format}\n"
        f"🔍 *Status:* {status}\n"
        "=============================\n"
        f"💰 *Total Saldo Terbaru:* Rp {saldo_format}\n"
    )
    
    url = f"https://api.telegram.org/bot{TELEGRAM_TOKEN}/sendMessage"
    payload = {
        "chat_id": CHAT_ID,
        "text": pesan,
        "parse_mode": "Markdown"
    }
    
    try:
        response = requests.post(url, json=payload, timeout=5)
        if response.status_code == 200:
            print("[Telegram] Notifikasi berhasil dikirim!")
        else:
            print(f"[Telegram] Gagal, kode error: {response.status_code}")
    except Exception as e:
        print("[Telegram] Terjadi kesalahan koneksi:", e)