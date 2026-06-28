// ==========================================
// SMART TABUNGAN — ESP32 dengan FreeRTOS
// UAS Mikrokontroler · RP 23 CID B
// ==========================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// ==========================================
// WIFI
// ==========================================
const char* ssid     = "Yank88";
const char* password = "Sulastian88";

// ==========================================
// MQTT
// ==========================================
const char* mqtt_server       = "192.168.100.29";
const int   mqtt_port         = 1883;
const char* mqtt_topic_kirim  = "smarttabungan/data";
const char* mqtt_topic_terima = "smarttabungan/perintah";

// ==========================================
// LCD
// ==========================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================================
// PIN SENSOR
// ==========================================
#define TRIG_PIN  5
#define ECHO_PIN  34

#define S2_PIN    26
#define S3_PIN    13
#define OUT_PIN   36
#define LED_TCS   14

#define LED_UV    18
#define UV_SENSOR 35

// ==========================================
// KONSTANTA
// ==========================================
#define JARAK_TRIGGER   6
#define AMBANG_UV       100
#define STACK_MQTT      4096
#define STACK_JARAK     2048
#define STACK_DETEKSI   4096
#define STACK_LCD       2048
#define STACK_WATCHDOG  2048

// ==========================================
// STRUCT DATA COIN
// ==========================================
typedef struct {
  int nominal;
  int uv;
  int saldo;
  bool asli;
} CoinData_t;

typedef struct {
  char baris0[17];  // maks 16 char + null
  char baris1[17];
} LCDMsg_t;

// ==========================================
// VARIABEL GLOBAL
// ==========================================
WiFiClient    espClient;
PubSubClient  client(espClient);

volatile int  totalSaldo   = 0;
volatile bool sedangProses = false;

// ==========================================
// HANDLE IPC (FreeRTOS)
// ==========================================
QueueHandle_t     xQueueCoinData;
QueueHandle_t     xQueueLCDMsg;
SemaphoreHandle_t xSemCoinDetected;
SemaphoreHandle_t xMutexSaldo;
SemaphoreHandle_t xMutexLCD;
SemaphoreHandle_t xMutexMQTT;

// ==========================================
// PROTOTYPE FUNGSI SENSOR
// ==========================================
long ukurJarak();
int  cekNominalWarna();
int  cekKeaslianUV();

// ==========================================
// PROTOTYPE TASK
// ==========================================
void taskMQTT(void* pvParam);
void taskJarak(void* pvParam);
void taskDeteksiUang(void* pvParam);
void taskLCD(void* pvParam);
void taskWifiWatchdog(void* pvParam);

// ==========================================
// MQTT CALLBACK (dipanggil dari taskMQTT)
// ==========================================
void callback(char* topic, byte* payload, unsigned int length)
{
  String pesan = "";
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  Serial.println();
  Serial.println("========== MQTT MASUK ==========");
  Serial.print("Topic : "); Serial.println(topic);
  Serial.print("Isi   : "); Serial.println(pesan);

  int saldoBaru = pesan.toInt();

  // Update totalSaldo dengan mutex
  xSemaphoreTake(xMutexSaldo, portMAX_DELAY);
  totalSaldo = saldoBaru;
  xSemaphoreGive(xMutexSaldo);

  Serial.print("Saldo Baru : "); Serial.println(saldoBaru);

  // Kirim perintah update LCD
  LCDMsg_t msg;
  snprintf(msg.baris0, 17, "Celengan Siap");
  snprintf(msg.baris1, 17, "Saldo:%d", saldoBaru);
  xQueueSend(xQueueLCDMsg, &msg, 0);
}

// ==========================================
// SETUP
// ==========================================
void setup()
{
  Serial.begin(115200);
  Wire.begin();

  // Pin sensor
  pinMode(TRIG_PIN,  OUTPUT);
  pinMode(ECHO_PIN,  INPUT);
  pinMode(S2_PIN,    OUTPUT);
  pinMode(S3_PIN,    OUTPUT);
  pinMode(OUT_PIN,   INPUT_PULLUP);
  pinMode(LED_TCS,   OUTPUT);
  pinMode(LED_UV,    OUTPUT);

  digitalWrite(LED_TCS, LOW);
  digitalWrite(LED_UV,  LOW);

  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Smart Tabungan");
  lcd.setCursor(0, 1); lcd.print("Starting...");

  // Sambung WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected : " + WiFi.localIP().toString());

  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Buat semua IPC object
  xQueueCoinData   = xQueueCreate(5, sizeof(CoinData_t));
  xQueueLCDMsg     = xQueueCreate(5, sizeof(LCDMsg_t));
  xSemCoinDetected = xSemaphoreCreateBinary();
  xMutexSaldo      = xSemaphoreCreateMutex();
  xMutexLCD        = xSemaphoreCreateMutex();
  xMutexMQTT       = xSemaphoreCreateMutex();

  // Buat semua task
  // Core 0: MQTT + WiFi Watchdog
  // Core 1: Sensor (Jarak + Deteksi) + LCD
  xTaskCreatePinnedToCore(taskMQTT,        "MQTT",      STACK_MQTT,     NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(taskWifiWatchdog,"Watchdog",  STACK_WATCHDOG, NULL, 4, NULL, 0);
  xTaskCreatePinnedToCore(taskJarak,       "Jarak",     STACK_JARAK,    NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(taskDeteksiUang, "Deteksi",   STACK_DETEKSI,  NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskLCD,         "LCD",       STACK_LCD,      NULL, 1, NULL, 1);

  Serial.println("Semua task RTOS dimulai.");
}

// ==========================================
// LOOP — kosong, semua dihandle FreeRTOS
// ==========================================
void loop()
{
  vTaskDelay(portMAX_DELAY);
}

// ==========================================
// TASK 1: MQTT (Core 0, Prioritas 5)
// Tugas: jaga koneksi + client.loop() terus
// ==========================================
void taskMQTT(void* pvParam)
{
  // Koneksi awal
  while (!client.connected()) {
    Serial.print("Menghubungkan MQTT...");
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("BERHASIL");
      client.subscribe(mqtt_topic_terima);
    } else {
      Serial.print("GAGAL rc=");
      Serial.println(client.state());
      vTaskDelay(pdMS_TO_TICKS(3000));
    }
  }

  // Tampil LCD awal
  LCDMsg_t msgAwal;
  snprintf(msgAwal.baris0, 17, "Celengan Siap");
  xSemaphoreTake(xMutexSaldo, portMAX_DELAY);
  snprintf(msgAwal.baris1, 17, "Saldo:%d", totalSaldo);
  xSemaphoreGive(xMutexSaldo);
  xQueueSend(xQueueLCDMsg, &msgAwal, 0);

  for (;;) {
    // Reconnect kalau putus
    if (!client.connected()) {
      Serial.print("[MQTT] Reconnecting...");
      String clientId = "ESP32-" + String(random(0xffff), HEX);
      if (client.connect(clientId.c_str())) {
        Serial.println("OK");
        client.subscribe(mqtt_topic_terima);
      } else {
        vTaskDelay(pdMS_TO_TICKS(3000));
        continue;
      }
    }

    // Proses incoming MQTT (callback dipanggil di sini)
    xSemaphoreTake(xMutexMQTT, portMAX_DELAY);
    client.loop();
    xSemaphoreGive(xMutexMQTT);

    // Ambil data dari queue lalu publish
    CoinData_t data;
    if (xQueueReceive(xQueueCoinData, &data, 0) == pdTRUE) {
      String statusUang = data.asli ? "Asli" : "Palsu/Ditolak";
      String payload =
        "{\"nominal\":" + String(data.nominal) +
        ",\"uv\":"      + String(data.uv) +
        ",\"saldo\":"   + String(data.saldo) +
        ",\"status\":\"" + statusUang + "\"}";

      xSemaphoreTake(xMutexMQTT, portMAX_DELAY);
      bool ok = client.publish(mqtt_topic_kirim, payload.c_str());
      xSemaphoreGive(xMutexMQTT);

      if (ok) {
        Serial.println("[MQTT] Data terkirim: " + payload);
      } else {
        Serial.println("[MQTT] Gagal kirim!");
        // Kembalikan ke queue untuk retry
        xQueueSendToFront(xQueueCoinData, &data, 0);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ==========================================
// TASK 2: WiFi Watchdog (Core 0, Prioritas 4)
// Tugas: pantau koneksi WiFi tiap 5 detik
// ==========================================
void taskWifiWatchdog(void* pvParam)
{
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(5000));

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[Watchdog] WiFi putus! Reconnecting...");

      LCDMsg_t msg;
      snprintf(msg.baris0, 17, "WiFi Terputus!");
      snprintf(msg.baris1, 17, "Reconnecting...");
      xQueueSend(xQueueLCDMsg, &msg, 0);

      WiFi.disconnect();
      WiFi.begin(ssid, password);

      int coba = 0;
      while (WiFi.status() != WL_CONNECTED && coba < 20) {
        vTaskDelay(pdMS_TO_TICKS(500));
        coba++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[Watchdog] WiFi kembali terhubung.");
      } else {
        Serial.println("[Watchdog] Gagal reconnect WiFi.");
      }
    }
  }
}

// ==========================================
// TASK 3: Jarak (Core 1, Prioritas 3)
// Tugas: polling HC-SR04, trigger deteksi
// ==========================================
void taskJarak(void* pvParam)
{
  for (;;) {
    long jarak = ukurJarak();

    if (jarak > 0 && jarak < JARAK_TRIGGER) {
      // Hanya trigger kalau tidak sedang proses
      if (!sedangProses) {
        xSemaphoreGive(xSemCoinDetected);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ==========================================
// TASK 4: Deteksi Uang (Core 1, Prioritas 2)
// Tugas: baca warna + UV, update saldo
// ==========================================
void taskDeteksiUang(void* pvParam)
{
  for (;;) {
    // Tunggu sinyal dari Task Jarak
    if (xSemaphoreTake(xSemCoinDetected, portMAX_DELAY) == pdTRUE) {
      sedangProses = true;

      Serial.println();
      Serial.println("==============================");
      Serial.println("Uang Terdeteksi");
      Serial.println("==============================");

      // LCD: memproses
      LCDMsg_t msgProses;
      snprintf(msgProses.baris0, 17, "Memproses...");
      snprintf(msgProses.baris1, 17, "");
      xQueueSend(xQueueLCDMsg, &msgProses, 0);

      // Baca warna
      int nominal = cekNominalWarna();

      // Baca UV (dengan vTaskDelay agar MQTT tetap jalan)
      vTaskDelay(pdMS_TO_TICKS(500));
      int uv = cekKeaslianUV();

      bool asli = (nominal > 0 && uv >= AMBANG_UV);

      // Update saldo dengan mutex
      xSemaphoreTake(xMutexSaldo, portMAX_DELAY);
      if (asli) totalSaldo += nominal;
      int saldoSekarang = totalSaldo;
      xSemaphoreGive(xMutexSaldo);

      // Log serial
      Serial.println(asli ? "HASIL : UANG ASLI" : "HASIL : DITOLAK");
      Serial.print("Nominal : "); Serial.println(nominal);
      Serial.print("UV      : "); Serial.println(uv);
      Serial.print("Saldo   : "); Serial.println(saldoSekarang);

      // Kirim ke queue MQTT
      CoinData_t coinData = {nominal, uv, saldoSekarang, asli};
      xQueueSend(xQueueCoinData, &coinData, pdMS_TO_TICKS(100));

      // Kirim ke queue LCD
      LCDMsg_t msgHasil;
      if (asli) {
        snprintf(msgHasil.baris0, 17, "Masuk: Rp%d", nominal);
        snprintf(msgHasil.baris1, 17, "Saldo:%d", saldoSekarang);
      } else {
        snprintf(msgHasil.baris0, 17, "Uang Ditolak");
        snprintf(msgHasil.baris1, 17, "Tidak Valid");
      }
      xQueueSend(xQueueLCDMsg, &msgHasil, 0);

      // Tahan sebentar supaya uang tidak discanning 2x
      vTaskDelay(pdMS_TO_TICKS(2000));

      // Kembali ke tampilan standby
      LCDMsg_t msgStandby;
      snprintf(msgStandby.baris0, 17, "Celengan Siap");
      snprintf(msgStandby.baris1, 17, "Saldo:%d", saldoSekarang);
      xQueueSend(xQueueLCDMsg, &msgStandby, 0);

      sedangProses = false;
    }
  }
}

// ==========================================
// TASK 5: LCD (Core 1, Prioritas 1)
// Tugas: update LCD dari queue
// ==========================================
void taskLCD(void* pvParam)
{
  for (;;) {
    LCDMsg_t msg;
    // Tunggu pesan, timeout 1 detik
    if (xQueueReceive(xQueueLCDMsg, &msg, pdMS_TO_TICKS(1000)) == pdTRUE) {
      xSemaphoreTake(xMutexLCD, portMAX_DELAY);
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print(msg.baris0);
      lcd.setCursor(0, 1); lcd.print(msg.baris1);
      xSemaphoreGive(xMutexLCD);
    }
  }
}

// ==========================================
// FUNGSI SENSOR: Ukur Jarak (HC-SR04)
// ==========================================
long ukurJarak()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long durasi = pulseIn(ECHO_PIN, HIGH, 30000);
  long jarak  = durasi * 0.034 / 2;
  return jarak;
}

// ==========================================
// FUNGSI SENSOR: Cek Nominal Warna (TCS3200)
// ==========================================
int cekNominalWarna()
{
  digitalWrite(LED_TCS, HIGH);
  vTaskDelay(pdMS_TO_TICKS(500));

  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
  vTaskDelay(pdMS_TO_TICKS(200));
  int R = pulseIn(OUT_PIN, LOW, 200000);

  digitalWrite(S2_PIN, HIGH);
  digitalWrite(S3_PIN, HIGH);
  vTaskDelay(pdMS_TO_TICKS(200));
  int G = pulseIn(OUT_PIN, LOW, 200000);

  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, HIGH);
  vTaskDelay(pdMS_TO_TICKS(200));
  int B = pulseIn(OUT_PIN, LOW, 200000);

  digitalWrite(LED_TCS, LOW);

  Serial.println();
  Serial.println("========== RGB ==========");
  Serial.print("R : "); Serial.println(R);
  Serial.print("G : "); Serial.println(G);
  Serial.print("B : "); Serial.println(B);
  Serial.println("=========================");

  if (R == 0 && G == 0 && B == 0) {
    Serial.println("PERINGATAN: TCS tidak merespon!");
    return 0;
  }

  int nominal   = 0;
  int totalRGB  = R + G + B;

  if      (totalRGB < 150)           nominal = 0;
  else if (R < B && B < G)           nominal = 100000;
  else if (B < G && G < R)           nominal = 50000;
  else if (B < R && R < G)           nominal = 10000;
  else if (G < R && G < B)           nominal = 20000;
  else if (R < G && G < B)           nominal = 5000;

  if (nominal == 0)
    Serial.println("Nominal : Tidak Dikenali");
  else {
    Serial.print("Nominal : Rp");
    Serial.println(nominal);
  }

  return nominal;
}

// ==========================================
// FUNGSI SENSOR: Cek Keaslian UV
// ==========================================
int cekKeaslianUV()
{
  digitalWrite(LED_UV, HIGH);
  vTaskDelay(pdMS_TO_TICKS(1000));  // warmup 1 detik

  // Buang 3 pembacaan pertama
  for (int i = 0; i < 3; i++) {
    analogRead(UV_SENSOR);
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  // Rata-rata 20 pembacaan
  long total = 0;
  for (int i = 0; i < 20; i++) {
    total += analogRead(UV_SENSOR);
    vTaskDelay(pdMS_TO_TICKS(15));
  }
  int uv = total / 20;

  digitalWrite(LED_UV, LOW);

  Serial.print("Nilai UV : "); Serial.println(uv);
  Serial.println(uv >= AMBANG_UV ? "Status UV : ASLI" : "Status UV : PALSU");

  return uv;
}
