# ==========================================
# app.py
# Main Flask Application
# Smart Tabungan IoT
# ==========================================

from flask import Flask, jsonify
from flask_cors import CORS

# Import Blueprint API
from routes import api

# Import MQTT Client
from mqtt_client import start_mqtt

# ==========================================
# MEMBUAT APLIKASI FLASK
# ==========================================

app = Flask(__name__)

# Mengizinkan akses dari Frontend
CORS(app)

# ==========================================
# REGISTER ROUTES
# ==========================================

app.register_blueprint(api, url_prefix="/api")

# ==========================================
# HALAMAN UTAMA
# ==========================================
from flask import render_template # Jangan lupa tambahkan ini di paling atas bersama Flask

@app.route("/")
def home():
    return render_template("index.html")


# ==========================================
# CEK STATUS SERVER
# ==========================================

@app.route("/health")
def health():

    return jsonify({
        "status": "OK",
        "server": "Flask Running",
        "mqtt": "Connected",
        "database": "MySQL Connected"
    })


# ==========================================
# HANDLE ERROR 404
# ==========================================

@app.errorhandler(404)
def not_found(error):

    return jsonify({
        "success": False,
        "message": "Endpoint tidak ditemukan."
    }), 404


# ==========================================
# HANDLE ERROR 500
# ==========================================

@app.errorhandler(500)
def server_error(error):

    return jsonify({
        "success": False,
        "message": "Terjadi kesalahan pada server."
    }), 500


# ==========================================
# MAIN PROGRAM
# ==========================================

if __name__ == "__main__":

    print("=" * 50)
    print("      SMART TABUNGAN IoT")
    print("=" * 50)

    print("Menjalankan MQTT Client...")

    mqtt_client = start_mqtt()

    print("MQTT Client Berhasil Dijalankan")

    print("Menjalankan Flask Server...")

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )