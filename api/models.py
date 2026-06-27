# ==========================================
# models.py
# Smart Tabungan IoT
# ==========================================

from database import db


class TransactionModel:

    # ======================================
    # INSERT TRANSAKSI (ESP32 / Website)
    # ======================================
    @staticmethod
    def insert_transaksi(nominal, uv, saldo, status, jenis, sumber):

        conn = db.connect()

        if conn is None:
            return False

        cursor = conn.cursor()

        sql = """
        INSERT INTO transaksi
        (nominal, uv, saldo, status, jenis, sumber)
        VALUES (%s,%s,%s,%s,%s,%s)
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

        cursor.close()
        conn.close()

        return True


    # ======================================
    # AMBIL SEMUA TRANSAKSI
    # ======================================
    @staticmethod
    def get_all_transaksi():

        conn = db.connect()

        if conn is None:
            return []

        cursor = conn.cursor(dictionary=True)

        cursor.execute("""
            SELECT *
            FROM transaksi
            ORDER BY id DESC
        """)

        data = cursor.fetchall()

        cursor.close()
        conn.close()

        return data


    # ======================================
    # TRANSAKSI TERAKHIR
    # ======================================
    @staticmethod
    def get_last_transaksi():

        conn = db.connect()

        if conn is None:
            return None

        cursor = conn.cursor(dictionary=True)

        cursor.execute("""
            SELECT *
            FROM transaksi
            ORDER BY id DESC
            LIMIT 1
        """)

        data = cursor.fetchone()

        cursor.close()
        conn.close()

        return data


    # ======================================
    # SALDO TERAKHIR
    # ======================================
    @staticmethod
    def get_last_saldo():

        conn = db.connect()

        if conn is None:
            return 0

        cursor = conn.cursor(dictionary=True)

        cursor.execute("""
            SELECT saldo
            FROM transaksi
            ORDER BY id DESC
            LIMIT 1
        """)

        result = cursor.fetchone()

        cursor.close()
        conn.close()

        if result:
            return result["saldo"]

        return 0


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
    # RESET RIWAYAT
    # ======================================
    @staticmethod
    def reset_transaksi():

        conn = db.connect()

        if conn is None:
            return False

        cursor = conn.cursor()

        cursor.execute("TRUNCATE TABLE transaksi")

        conn.commit()

        cursor.close()
        conn.close()

        return True