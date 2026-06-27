# ==========================================
# routes.py
# API Smart Tabungan IoT
# ==========================================

from flask import Blueprint, jsonify, request
from models import TransactionModel

# ==========================================
# BLUEPRINT
# ==========================================

api = Blueprint("api", __name__)


# ==========================================
# GET SEMUA TRANSAKSI
# ==========================================

@api.route("/transaksi", methods=["GET"])
def get_transaksi():

    data = TransactionModel.get_all_transaksi()

    return jsonify({
        "success": True,
        "total": len(data),
        "data": data
    })


# ==========================================
# GET TRANSAKSI TERAKHIR
# ==========================================

@api.route("/last", methods=["GET"])
def get_last():

    data = TransactionModel.get_last_transaksi()

    if data:

        return jsonify({
            "success": True,
            "data": data
        })

    return jsonify({
        "success": False,
        "message": "Belum ada transaksi"
    })


# ==========================================
# GET SALDO TERAKHIR
# ==========================================

@api.route("/saldo", methods=["GET"])
def get_saldo():

    saldo = TransactionModel.get_last_saldo()

    return jsonify({
        "success": True,
        "saldo": saldo
    })


# ==========================================
# TAMBAH SALDO MANUAL
# ==========================================

@api.route("/tambah", methods=["POST"])
def tambah_saldo():

    data = request.get_json()

    if not data or "jumlah" not in data:

        return jsonify({
            "success": False,
            "message": "Jumlah tidak boleh kosong."
        }), 400

    jumlah = int(data["jumlah"])

    if jumlah <= 0:

        return jsonify({
            "success": False,
            "message": "Jumlah harus lebih dari 0."
        }), 400

    berhasil = TransactionModel.tambah_manual(jumlah)

    return jsonify({
        "success": berhasil,
        "message": "Saldo berhasil ditambahkan."
    })


# ==========================================
# AMBIL UANG
# ==========================================

@api.route("/ambil", methods=["POST"])
def ambil_uang():

    data = request.get_json()

    if not data or "jumlah" not in data:

        return jsonify({
            "success": False,
            "message": "Jumlah tidak boleh kosong."
        }), 400

    jumlah = int(data["jumlah"])

    if jumlah <= 0:

        return jsonify({
            "success": False,
            "message": "Jumlah harus lebih dari 0."
        }), 400

    berhasil = TransactionModel.ambil_uang(jumlah)

    if berhasil:

        return jsonify({
            "success": True,
            "message": "Uang berhasil diambil."
        })

    return jsonify({
        "success": False,
        "message": "Saldo tidak mencukupi."
    }), 400


# ==========================================
# RESET TRANSAKSI
# ==========================================

@api.route("/reset", methods=["DELETE"])
def reset():

    TransactionModel.reset_transaksi()

    return jsonify({
        "success": True,
        "message": "Riwayat transaksi berhasil dihapus."
    })


# ==========================================
# DASHBOARD
# ==========================================

@api.route("/dashboard", methods=["GET"])
def dashboard():

    saldo = TransactionModel.get_last_saldo()

    last = TransactionModel.get_last_transaksi()

    transaksi = TransactionModel.get_all_transaksi()

    return jsonify({

        "success": True,

        "saldo": saldo,

        "last": last,

        "jumlah_transaksi": len(transaksi),

        "transaksi": transaksi

    })