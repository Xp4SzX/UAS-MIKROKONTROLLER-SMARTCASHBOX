# ==========================================
# models.py
# Smart Tabungan IoT
# ==========================================

from database import db
import mysql.connector


class TransactionModel:

    # ======================================
    # INSERT TRANSAKSI
    # ======================================
    @staticmethod
    def insert_transaksi(nominal, uv, saldo, status, jenis, sumber):

        conn = db.connect()

        if conn is None:
            print("❌ Database tidak terhubung")
            return False

        cursor = None

        try:
            cursor = conn.cursor()

            sql = """
            INSERT INTO transaksi
            (nominal, uv, saldo, status, jenis, sumber)
            VALUES (%s, %s, %s, %s, %s, %s)
            """

            value = (
                nominal,
                uv,
                saldo,
                status,
                jenis,
                sumber
            )

            cursor.execute(sql, value)

            conn.commit()

            print("✅ Data berhasil disimpan")

            return True

        except mysql.connector.Error as err:

            conn.rollback()

            print("❌ Gagal INSERT :", err)

            return False

        finally:

            if cursor:
                cursor.close()

            conn.close()

    # ======================================
    # AMBIL SEMUA TRANSAKSI
    # ======================================
    @staticmethod
    def get_all_transaksi():

        conn = db.connect()

        if conn is None:
            return []

        cursor = None

        try:

            cursor = conn.cursor(dictionary=True)

            cursor.execute("""
                SELECT *
                FROM transaksi
                ORDER BY id DESC
            """)

            return cursor.fetchall()

        except mysql.connector.Error as err:

            print("❌ Error get_all_transaksi :", err)

            return []

        finally:

            if cursor:
                cursor.close()

            conn.close()

    # ======================================
    # TRANSAKSI TERAKHIR
    # ======================================
    @staticmethod
    def get_last_transaksi():

        conn = db.connect()

        if conn is None:
            return None

        cursor = None

        try:

            cursor = conn.cursor(dictionary=True)

            cursor.execute("""
                SELECT *
                FROM transaksi
                ORDER BY id DESC
                LIMIT 1
            """)

            return cursor.fetchone()

        except mysql.connector.Error as err:

            print("❌ Error get_last_transaksi :", err)

            return None

        finally:

            if cursor:
                cursor.close()

            conn.close()

    # ======================================
    # SALDO TERAKHIR
    # ======================================
    @staticmethod
    def get_last_saldo():

        conn = db.connect()

        if conn is None:
            return 0

        cursor = None

        try:

            cursor = conn.cursor(dictionary=True)

            cursor.execute("""
                SELECT saldo
                FROM transaksi
                ORDER BY id DESC
                LIMIT 1
            """)

            result = cursor.fetchone()

            if result:
                return result["saldo"]

            return 0

        except mysql.connector.Error as err:

            print("❌ Error get_last_saldo :", err)

            return 0

        finally:

            if cursor:
                cursor.close()

            conn.close()

    # ======================================
    # TAMBAH SALDO MANUAL
    # ======================================
    @staticmethod
    def tambah_manual(jumlah):

        saldo = TransactionModel.get_last_saldo()

        saldo_baru = saldo + jumlah

        return TransactionModel.insert_transaksi(
            nominal=jumlah,
            uv=0,
            saldo=saldo_baru,
            status="Tambah Manual",
            jenis="Masuk",
            sumber="Website"
        )

    # ======================================
    # AMBIL UANG
    # ======================================
    @staticmethod
    def ambil_uang(jumlah):

        saldo = TransactionModel.get_last_saldo()

        if jumlah > saldo:

            return False

        saldo_baru = saldo - jumlah

        return TransactionModel.insert_transaksi(
            nominal=jumlah,
            uv=0,
            saldo=saldo_baru,
            status="Uang Diambil",
            jenis="Keluar",
            sumber="Website"
        )

    # ======================================
    # RESET TRANSAKSI
    # ======================================
    @staticmethod
    def reset_transaksi():

        conn = db.connect()

        if conn is None:
            return False

        cursor = None

        try:

            cursor = conn.cursor()

            cursor.execute("TRUNCATE TABLE transaksi")

            conn.commit()

            print("✅ Riwayat berhasil dihapus")

            return True

        except mysql.connector.Error as err:

            conn.rollback()

            print("❌ Error reset :", err)

            return False

        finally:

            if cursor:
                cursor.close()

            conn.close()