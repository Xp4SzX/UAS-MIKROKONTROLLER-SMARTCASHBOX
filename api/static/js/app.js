// ==========================================
// app.js
// Smart Tabungan IoT
// ==========================================

const API_URL = "/api";

// ==========================================
// TOAST NOTIFICATION SYSTEM
// ==========================================

/**
 * showToast(options)
 * @param {string} options.type    - 'success' | 'error' | 'warning' | 'info'
 * @param {string} options.title   - Judul singkat
 * @param {string} options.message - Pesan detail
 * @param {number} options.duration - Durasi ms (default 3500)
 */
function showToast({ type = "info", title = "", message = "", duration = 3500 } = {}) {
  const container = document.getElementById("toast-container");

  const icons = {
    success: "fas fa-check-circle",
    error:   "fas fa-times-circle",
    warning: "fas fa-exclamation-triangle",
    info:    "fas fa-info-circle",
  };

  const toast = document.createElement("div");
  toast.className = `toast toast-${type}`;
  toast.style.setProperty("--toast-duration", `${duration}ms`);

  toast.innerHTML = `
    <div class="toast-icon"><i class="${icons[type] || icons.info}"></i></div>
    <div class="toast-body">
      ${title   ? `<div class="toast-title">${title}</div>` : ""}
      ${message ? `<div class="toast-msg">${message}</div>` : ""}
    </div>
    <button class="toast-close" aria-label="Tutup"><i class="fas fa-times"></i></button>
  `;

  container.appendChild(toast);

  // Dismiss manual
  toast.querySelector(".toast-close").addEventListener("click", () => dismissToast(toast));

  // Auto dismiss
  const timer = setTimeout(() => dismissToast(toast), duration);

  // Pause progress on hover
  toast.addEventListener("mouseenter", () => {
    clearTimeout(timer);
    toast.style.animationPlayState = "paused";
    toast.style.setProperty("--toast-duration", "9999s");
  });
  toast.addEventListener("mouseleave", () => {
    toast.style.setProperty("--toast-duration", "1.2s");
    setTimeout(() => dismissToast(toast), 1200);
  });
}

function dismissToast(toast) {
  if (!toast || toast.classList.contains("toast-exit")) return;
  toast.classList.add("toast-exit");
  toast.addEventListener("animationend", () => toast.remove(), { once: true });
}

// Shorthand helpers
const toast = {
  success: (title, message, duration) => showToast({ type: "success", title, message, duration }),
  error:   (title, message, duration) => showToast({ type: "error",   title, message, duration }),
  warning: (title, message, duration) => showToast({ type: "warning", title, message, duration }),
  info:    (title, message, duration) => showToast({ type: "info",    title, message, duration }),
};

// ==========================================
// CUSTOM CONFIRM DIALOG
// ==========================================

/**
 * showConfirm(options) → Promise<boolean>
 * @param {string} options.title     - Judul dialog
 * @param {string} options.message   - Pesan konfirmasi
 * @param {string} options.type      - 'danger' | 'warning' | 'info'
 * @param {string} options.okText    - Teks tombol konfirmasi (default 'Ya')
 * @param {string} options.cancelText- Teks tombol batal (default 'Batal')
 */
function showConfirm({
  title      = "Konfirmasi",
  message    = "Apakah kamu yakin?",
  type       = "danger",
  okText     = "Ya, Lanjutkan",
  cancelText = "Batal",
} = {}) {
  return new Promise((resolve) => {
    const overlay  = document.getElementById("confirm-overlay");
    const iconWrap = document.getElementById("confirm-icon-wrap");
    const iconEl   = document.getElementById("confirm-icon");
    const titleEl  = document.getElementById("confirm-title");
    const msgEl    = document.getElementById("confirm-message");
    const okBtn    = document.getElementById("confirm-ok");
    const cancelBtn= document.getElementById("confirm-cancel");

    const icons = {
      danger:  "fas fa-trash-alt",
      warning: "fas fa-exclamation-triangle",
      info:    "fas fa-question-circle",
    };

    iconWrap.className = `confirm-icon-wrap ${type}`;
    iconEl.className   = icons[type] || icons.info;
    titleEl.textContent = title;
    msgEl.textContent   = message;
    okBtn.textContent   = okText;
    cancelBtn.textContent = cancelText;

    // Style OK button variant
    okBtn.className = type === "info" ? "ok-green" : "";

    overlay.classList.add("show");

    function close(result) {
      overlay.classList.remove("show");
      okBtn.removeEventListener("click", onOk);
      cancelBtn.removeEventListener("click", onCancel);
      overlay.removeEventListener("click", onOverlay);
      resolve(result);
    }

    const onOk      = () => close(true);
    const onCancel  = () => close(false);
    const onOverlay = (e) => { if (e.target === overlay) close(false); };

    okBtn.addEventListener("click", onOk);
    cancelBtn.addEventListener("click", onCancel);
    overlay.addEventListener("click", onOverlay);
  });
}

// ==========================================
// LOAD DASHBOARD
// ==========================================
async function loadDashboard() {
  try {
    const response = await fetch(`${API_URL}/dashboard`);
    const result   = await response.json();

    if (!result.success) return null;

    document.getElementById("saldo").innerHTML =
      "Rp " + result.saldo.toLocaleString("id-ID");

    if (result.last) {
      document.getElementById("nominal").innerHTML =
        "Rp " + Number(result.last.nominal).toLocaleString("id-ID");
      document.getElementById("status").innerHTML = result.last.status;
      document.getElementById("uv").innerHTML     = result.last.uv;
    }

    loadTable(result.transaksi);
    return result.saldo;
  } catch (err) {
    console.error(err);
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
    toast.warning("Input Tidak Valid", "Masukkan nominal yang benar terlebih dahulu.");
    return;
  }

  try {
    const response = await fetch(`${API_URL}/tambah`, {
      method:  "POST",
      headers: { "Content-Type": "application/json" },
      body:    JSON.stringify({ jumlah }),
    });

    const result = await response.json();

    document.getElementById("modalTambah").style.display = "none";
    document.getElementById("jumlahTambah").value = "";

    if (result.success !== false) {
      toast.success(
        "Saldo Ditambahkan",
        `Rp ${jumlah.toLocaleString("id-ID")} berhasil ditambahkan ke celengan.`
      );
    } else {
      toast.error("Gagal", result.message || "Terjadi kesalahan saat menambah saldo.");
    }

    const saldoBaru = await loadDashboard();
    if (saldoBaru !== null && typeof kirimPerintahESP === "function") {
      kirimPerintahESP(saldoBaru);
    }
  } catch (err) {
    console.error(err);
    toast.error("Koneksi Error", "Tidak dapat terhubung ke server. Coba lagi.");
  }
}

// ==========================================
// AMBIL UANG
// ==========================================
async function ambilUang() {
  const jumlah = Number(document.getElementById("jumlahAmbil").value);

  if (jumlah <= 0) {
    toast.warning("Input Tidak Valid", "Masukkan nominal yang benar terlebih dahulu.");
    return;
  }

  try {
    const response = await fetch(`${API_URL}/ambil`, {
      method:  "POST",
      headers: { "Content-Type": "application/json" },
      body:    JSON.stringify({ jumlah }),
    });

    const result = await response.json();

    document.getElementById("modalAmbil").style.display = "none";
    document.getElementById("jumlahAmbil").value = "";

    if (result.success !== false) {
      toast.success(
        "Uang Diambil",
        `Rp ${jumlah.toLocaleString("id-ID")} berhasil diambil dari celengan.`
      );
    } else {
      toast.error("Gagal", result.message || "Terjadi kesalahan. Mungkin saldo tidak cukup.");
    }

    const saldoBaru = await loadDashboard();
    if (saldoBaru !== null && typeof kirimPerintahESP === "function") {
      kirimPerintahESP(saldoBaru);
    }
  } catch (err) {
    console.error(err);
    toast.error("Koneksi Error", "Tidak dapat terhubung ke server. Coba lagi.");
  }
}

// ==========================================
// RESET
// ==========================================
async function resetRiwayat() {
  const konfirmasi = await showConfirm({
    title:      "Hapus Semua Riwayat?",
    message:    "Seluruh transaksi akan dihapus permanen dan saldo akan direset ke nol. Tindakan ini tidak dapat dibatalkan.",
    type:       "danger",
    okText:     "Ya, Hapus Semua",
    cancelText: "Batal",
  });

  if (!konfirmasi) return;

  try {
    const response = await fetch(`${API_URL}/reset`, { method: "DELETE" });
    const result   = await response.json();

    if (result.success !== false) {
      toast.success("Riwayat Dihapus", "Semua transaksi telah dihapus dan saldo direset.");
    } else {
      toast.error("Gagal", result.message || "Terjadi kesalahan saat mereset data.");
    }

    const saldoBaru = await loadDashboard();
    if (saldoBaru !== null && typeof kirimPerintahESP === "function") {
      kirimPerintahESP(saldoBaru);
    }
  } catch (err) {
    console.error(err);
    toast.error("Koneksi Error", "Tidak dapat terhubung ke server. Coba lagi.");
  }
}

// ==========================================
// BUTTON EVENTS
// ==========================================

document.getElementById("btnTambah").onclick = () => {
  document.getElementById("modalTambah").style.display = "flex";
};

document.getElementById("btnAmbil").onclick = () => {
  document.getElementById("modalAmbil").style.display = "flex";
};

document.getElementById("btnReset").onclick = () => {
  resetRiwayat();
};

document.getElementById("simpanTambah").onclick = () => tambahSaldo();
document.getElementById("simpanAmbil").onclick  = () => ambilUang();

document.getElementById("batalTambah").onclick = () => {
  document.getElementById("modalTambah").style.display = "none";
};

document.getElementById("batalAmbil").onclick = () => {
  document.getElementById("modalAmbil").style.display = "none";
};

// Tutup modal ketika klik di luar modal-content
document.querySelectorAll(".modal").forEach((modal) => {
  modal.addEventListener("click", (e) => {
    if (e.target === modal) modal.style.display = "none";
  });
});

// ==========================================
// LOAD AWAL
// ==========================================
loadDashboard();
setInterval(loadDashboard, 5000);