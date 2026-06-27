# ==========================================
# database.py
# Koneksi Database MySQL
# ==========================================

import mysql.connector
from mysql.connector import Error

class Database:

    def __init__(self):
        self.host = "localhost"
        self.user = "root"
        self.password = ""
        self.database = "data"

    def connect(self):
        try:
            connection = mysql.connector.connect(
                host=self.host,
                user=self.user,
                password=self.password,
                database=self.database
            )

            if connection.is_connected():
                print("✅ Berhasil terhubung ke MySQL")
                return connection

        except Error as e:
            print("❌ Gagal koneksi database :", e)
            return None


# Membuat objek database
db = Database()