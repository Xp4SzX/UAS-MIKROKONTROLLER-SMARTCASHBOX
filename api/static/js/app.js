// ==========================================
// app.js
// Smart Tabungan IoT
// ==========================================

// URL Backend Flask
const API_URL = "http://127.0.0.1:5000/api";

// ==========================================
// LOAD DASHBOARD
// ==========================================
async function loadDashboard() {
  try {
    const response = await fetch(`${API_URL}/dashboard`);
    const result = await response.json();

    if (!result.success) return null;

    // Card Saldo
    document.getElementById("saldo").innerHTML =
      "Rp " + result.saldo.toLocaleString("id-ID");

    // Card Transaksi Terakhir
    if (result.last) {
      document.getElementById("nominal").innerHTML =
        "Rp " + Number(result.last.nominal).toLocaleString("id-ID");
      document.getElementById("status").innerHTML = result.last.status;
      document.getElementById("uv").innerHTML = result.last.uv;
    }

    // Isi Tabel
    loadTable(result.transaksi);

    // KEMBALIKAN NILAI SALDO TERBARU
    return result.saldo;
  } catch (err) {
    console.log(err);
    return null;
  }
}

// ==========================================
// LOAD TABLE
// ==========================================
function loadTable(data) {
  const tbody = document.getElementById("historyTable");
  tbody.innerHTML = "";

  data.forEach((item, index) => {
    tbody.innerHTML += `
        <tr>
            <td>${index + 1}</td>
            <td>Rp ${Number(item.nominal).toLocaleString("id-ID")}</td>
            <td>${item.jenis}</td>
            <td>${item.status}</td>
            <td>${item.sumber}</td>
            <td>${item.uv}</td>
            <td>Rp ${Number(item.saldo).toLocaleString("id-ID")}</td>
            <td>${item.waktu}</td>
        </tr>
        `;
  });
}

// ==========================================
// TAMBAH SALDO
// ==========================================
async function tambahSaldo() {
  const jumlah = Number(document.getElementById("jumlahTambah").value);

  if (jumlah <= 0) {
    alert("Masukkan nominal yang benar!");
    return;
  }

  const response = await fetch(`${API_URL}/tambah`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      jumlah: jumlah,
    }),
  });

  const result = await response.json();
  alert(result.message);

  document.getElementById("modalTambah").style.display = "none";
  document.getElementById("jumlahTambah").value = "";

  // Refresh web & kirim saldo ke ESP32
  const saldoBaru = await loadDashboard();
  if (saldoBaru !== null && typeof kirimPerintahESP === "function") {
    kirimPerintahESP(saldoBaru);
  }
}

// ==========================================
// AMBIL UANG
// ==========================================
async function ambilUang() {
  const jumlah = Number(document.getElementById("jumlahAmbil").value);

  if (jumlah <= 0) {
    alert("Masukkan nominal yang benar!");
    return;
  }

  const response = await fetch(`${API_URL}/ambil`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      jumlah: jumlah,
    }),
  });

  const result = await response.json();
  alert(result.message);

  document.getElementById("modalAmbil").style.display = "none";
  document.getElementById("jumlahAmbil").value = "";

  // Refresh web & kirim saldo ke ESP32
  const saldoBaru = await loadDashboard();
  if (saldoBaru !== null && typeof kirimPerintahESP === "function") {
    kirimPerintahESP(saldoBaru);
  }
}

// ==========================================
// RESET
// ==========================================
async function resetRiwayat() {
  const konfirmasi = confirm("Yakin ingin menghapus seluruh transaksi?");

  if (!konfirmasi) return;

  const response = await fetch(`${API_URL}/reset`, {
    method: "DELETE",
  });

  const result = await response.json();
  alert(result.message);

  // Refresh web & kirim angka 0 ke ESP32 karena di-reset
  const saldoBaru = await loadDashboard();
  if (saldoBaru !== null && typeof kirimPerintahESP === "function") {
    kirimPerintahESP(saldoBaru);
  }
}

// ==========================================
// BUTTON
// ==========================================

// Tambah Saldo
document.getElementById("btnTambah").onclick = () => {
  document.getElementById("modalTambah").style.display = "flex";
};

// Ambil Uang
document.getElementById("btnAmbil").onclick = () => {
  document.getElementById("modalAmbil").style.display = "flex";
};

// Reset
document.getElementById("btnReset").onclick = () => {
  resetRiwayat();
};

// Simpan Tambah
document.getElementById("simpanTambah").onclick = () => {
  tambahSaldo();
};

// Simpan Ambil
document.getElementById("simpanAmbil").onclick = () => {
  ambilUang();
};

// Batal Tambah
document.getElementById("batalTambah").onclick = () => {
  document.getElementById("modalTambah").style.display = "none";
};

// Batal Ambil
document.getElementById("batalAmbil").onclick = () => {
  document.getElementById("modalAmbil").style.display = "none";
};

// ==========================================
// LOAD AWAL
// ==========================================
loadDashboard();

// Refresh dashboard setiap 5 detik sebagai cadangan.
setInterval(loadDashboard, 5000);
