#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ==========================================
// WIFI
// ==========================================
const char* ssid = "Yank88";
const char* password = "Sulastian88";

// ==========================================
// MQTT
// ==========================================
const char* mqtt_server = "192.168.100.29";
const int mqtt_port = 1883;
const char* mqtt_topic_kirim  = "smarttabungan/data";
const char* mqtt_topic_terima = "smarttabungan/perintah";

// ==========================================
// LCD
// ==========================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================================
// PIN SENSOR
// ==========================================
// ==========================================
// PIN SENSOR
// ==========================================
#define TRIG_PIN 5
#define ECHO_PIN 34

#define S2_PIN 26
#define S3_PIN 13
#define OUT_PIN 36       // tetap 36, tapi kita atasi dengan cara lain
#define LED_TCS 14

#define LED_UV 18
#define UV_SENSOR 35     // ✅ GANTI dari 2 ke 35 — GPIO 2 tidak reliable untuk ADC

// ==========================================
// VARIABEL
// ==========================================
WiFiClient espClient;
PubSubClient client(espClient);

int totalSaldo = 0;
int jarakTrigger = 6;
int nilaiAmbangUV = 100;

unsigned long lastReconnect = 0;
unsigned long lastScan = 0;
bool sudahScan = false;

// ==========================================
// PROTOTYPE
// ==========================================
void reconnectMQTT();
void callback(char*, byte*, unsigned int);
long ukurJarak();
int cekNominalWarna();
int cekKeaslianUV();
void kirimDataMQTT(int nominal, int uv, int total);
void tampilLCD();

// ==========================================
// SETUP
// ==========================================
void setup()
{
  Serial.begin(115200);
  Wire.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);

  // ✅ PERBAIKAN: Gunakan INPUT_PULLUP untuk stabilitas pulseIn
  pinMode(OUT_PIN, INPUT_PULLUP);

  pinMode(LED_TCS, OUTPUT);
  pinMode(LED_UV, OUTPUT);

  digitalWrite(LED_TCS, LOW);
  digitalWrite(LED_UV, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Tabungan");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  // WIFI
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  // MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnectMQTT();

  lcd.clear();
  tampilLCD();
}

// ==========================================
// LOOP
// ==========================================
void loop()
{
  if (!client.connected())
  {
    reconnectMQTT();
  }
  client.loop();

  long jarak = ukurJarak();

  if (jarak > 0 && jarak < jarakTrigger)
  {
    if (!sudahScan)
    {
      sudahScan = true;
      lastScan = millis();

      Serial.println();
      Serial.println("==============================");
      Serial.println("Uang Terdeteksi");
      Serial.println("==============================");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Memproses...");

      int nominal = cekNominalWarna();

      delay(2000);

      int uv = cekKeaslianUV();

      bool uangAsli = (nominal > 0 && uv >= nilaiAmbangUV);

      if (uangAsli)
      {
        totalSaldo += nominal;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Masuk:");
        lcd.print(nominal);
        lcd.setCursor(0, 1);
        lcd.print("Saldo:");
        lcd.print(totalSaldo);

        Serial.println("HASIL : UANG ASLI");
        Serial.print("Nominal : ");
        Serial.println(nominal);
        Serial.print("UV : ");
        Serial.println(uv);
        Serial.print("Saldo : ");
        Serial.println(totalSaldo);

        kirimDataMQTT(nominal, uv, totalSaldo);
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Uang Ditolak");
        lcd.setCursor(0, 1);
        lcd.print("Tidak Valid");

        Serial.println("HASIL : DITOLAK");
        kirimDataMQTT(0, uv, totalSaldo);
      }
    }
  }
  else
  {
    if (sudahScan)
    {
      if (millis() - lastScan > 1500)
      {
        sudahScan = false;
        tampilLCD();
      }
    }
  }
  delay(50);
}

// ==========================================
// MQTT RECONNECT
// ==========================================
void reconnectMQTT()
{
  while (!client.connected())
  {
    Serial.print("Menghubungkan MQTT...");
    String clientId = "ESP32-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str()))
    {
      Serial.println(" BERHASIL");
      client.subscribe(mqtt_topic_terima);
      Serial.print("Subscribe : ");
      Serial.println(mqtt_topic_terima);
    }
    else
    {
      Serial.print(" GAGAL rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

// ==========================================
// CALLBACK MQTT
// ==========================================
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println();
  Serial.println("========== MQTT MASUK ==========");

  String pesan = "";
  for (int i = 0; i < length; i++)
  {
    pesan += (char)payload[i];
  }

  Serial.print("Topic : ");
  Serial.println(topic);
  Serial.print("Isi   : ");
  Serial.println(pesan);
  totalSaldo = pesan.toInt();
  Serial.print("Saldo Baru : ");
  Serial.println(totalSaldo);
  tampilLCD();
}

// ==========================================
// TAMPILKAN LCD
// ==========================================
void tampilLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Celengan Siap");
  lcd.setCursor(0, 1);
  lcd.print("Saldo:");
  lcd.print(totalSaldo);
}

// ==========================================
// UKUR JARAK (HC-SR04)
// ==========================================
long ukurJarak()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long durasi = pulseIn(ECHO_PIN, HIGH, 30000);
  long jarak = durasi * 0.034 / 2;
  return jarak;
}

// ==========================================
// CEK NOMINAL BERDASARKAN WARNA (TCS3200)
// ==========================================
int cekNominalWarna()
{
  digitalWrite(LED_TCS, HIGH);
  delay(500);

  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
  delay(200);
  int R = pulseIn(OUT_PIN, LOW, 200000);

  digitalWrite(S2_PIN, HIGH);
  digitalWrite(S3_PIN, HIGH);
  delay(200);
  int G = pulseIn(OUT_PIN, LOW, 200000);

  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, HIGH);
  delay(200);
  int B = pulseIn(OUT_PIN, LOW, 200000);

  digitalWrite(LED_TCS, LOW);

  Serial.println();
  Serial.println("========== RGB ==========");
  Serial.print("R : "); Serial.println(R);
  Serial.print("G : "); Serial.println(G);
  Serial.print("B : "); Serial.println(B);
  Serial.println("=========================");

  if (R == 0 && G == 0 && B == 0)
  {
    Serial.println("PERINGATAN: TCS tidak merespon!");
    return 0;
  }
int nominal = 0;
  
  // Menghitung total RGB untuk memastikan sensor tidak sedang membaca ruang gelap/kosong
  int totalRGB = R + G + B;

  // ============================================================
  // LOGIKA DETEKSI UANG MENGGUNAKAN MATRIKS PERBANDINGAN
  // (Mengabaikan Rp2.000 untuk meminimalisir error/pengacau)
  // ============================================================

  // Proteksi: Jika tidak ada benda atau terlalu gelap
  if (totalRGB < 150) 
  {
    nominal = 0; 
  }
  // Rp100.000 — R < B < G
  else if (R < B && B < G) 
  {
    nominal = 100000;
  }
  // Rp50.000 — B < G < R
  else if (B < G && G < R) 
  {
    nominal = 50000;
  }
  // Rp10.000 — B < R < G
  else if (B < R && R < G) 
  {
    nominal = 10000;
  }
  // Rp20.000 — G Paling Kecil (mencakup G < R < B dan G < B < R)
  else if (G < R && G < B) 
  {
    nominal = 20000;
  }
  // Rp5.000 — R < G < B
  else if (R < G && G < B) 
  {
    nominal = 5000;
  }

  // ============================================================
  // OUTPUT KE SERIAL MONITOR
  // ============================================================
  if (nominal == 0)
    Serial.println("Nominal : Tidak Dikenali");
  else
  {
    Serial.print("Nominal : Rp");
    Serial.println(nominal);
  }

  return nominal;
}

// ==========================================
// CEK KEASLIAN UV
// ==========================================
int cekKeaslianUV()
{
  digitalWrite(LED_UV, HIGH);
  delay(1000); // ✅ Naikkan warmup ke 1 detik agar LED stabil

  // Buang 3 pembacaan pertama (sering tidak stabil)
  for (int i = 0; i < 3; i++)
  {
    analogRead(UV_SENSOR);
    delay(20);
  }

  // Ambil rata-rata 20 pembacaan
  long total = 0;
  for (int i = 0; i < 20; i++)
  {
    total += analogRead(UV_SENSOR);
    delay(15);
  }
  int uv = total / 20;

  digitalWrite(LED_UV, LOW);

  Serial.print("Nilai UV : ");
  Serial.println(uv);

  if (uv >= nilaiAmbangUV)
    Serial.println("Status UV : ASLI");
  else
    Serial.println("Status UV : PALSU");

  return uv;
}

// ==========================================
// KIRIM DATA MQTT
// ==========================================
void kirimDataMQTT(int nominal, int uv, int total)
{
  // Tentukan status berdasarkan nominal
  String statusUang = (nominal > 0) ? "Asli" : "Palsu/Ditolak";

  // Sesuaikan nama variabel dengan mqtt.js (gunakan "saldo" dan tambahkan "status")
  String payload = "{\"nominal\":" + String(nominal) +
                   ",\"uv\":" + String(uv) +
                   ",\"saldo\":" + String(total) + 
                   ",\"status\":\"" + statusUang + "\"}";

  if (client.publish(mqtt_topic_kirim, payload.c_str()))
  {
    Serial.println("MQTT: Data berhasil dikirim!");
    Serial.println("Payload: " + payload);
  }
  else
  {
    Serial.println("MQTT: Gagal mengirim data!");
  }
}