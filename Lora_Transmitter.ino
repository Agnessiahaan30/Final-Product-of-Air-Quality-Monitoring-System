#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <LoRa.h>

const int loraCsPin = 10;    // Pin Chip Select untuk LoRa
const int sensorPin = A0;    // Pin analog untuk sensor suhu LM35
const int mq7Pin = A1;       // Pin analog untuk sensor MQ-7
const int mq135Pin = A2;     // Pin analog untuk sensor MQ-135
const int dustPin = A3;      // Pin analog untuk sensor debu

// Inisialisasi LCD dengan I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Alamat I2C LCD: 0x27, Ukuran: 20x4

float temperature;           // Variabel untuk menyimpan suhu
int mq7Value;                // Nilai pembacaan sensor MQ-7
int mq135Value;              // Nilai pembacaan sensor MQ-135
int dustValue;               // Nilai pembacaan sensor debu
float COppm;                 // Konsentrasi CO dalam PPM
float NH3ppm;                // Konsentrasi NH3 dalam PPM

// Konstanta untuk kalibrasi MQ-7
const float RL = 10.0;       // Nilai beban resistansi (RL) dalam kOhm
const float R0_MQ7 = 10.0;  // Nilai resistansi sensor MQ-7 di udara bersih (perlu kalibrasi)
const float b_CO = -0.77;    // Konstanta dari kurva kalibrasi MQ-7
const float m_CO = -0.29;    // Konstanta dari kurva kalibrasi MQ-7

// Konstanta untuk kalibrasi MQ-135 (NH3)
const float RL_MQ135 = 10.0;       // Nilai beban resistansi (RL) dalam kOhm
const float R0_MQ135 = 10.0;       // Nilai resistansi sensor MQ-135 di udara bersih (perlu kalibrasi)
const float b_NH3 = -0.42;         // Konstanta dari kurva kalibrasi MQ-135 untuk NH3
const float m_NH3 = -0.38;         // Konstanta dari kurva kalibrasi MQ-135 untuk NH3

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Inisialisasi LoRa
  LoRa.setPins(loraCsPin, 9, 2);  // CS, RST, IRQ
  if (!LoRa.begin(915E6)) {
    Serial.println("Gagal mulai LoRa!");
    while (1);
  }
  Serial.println("LoRa mulai...");

  // Pengaturan parameter LoRa
  LoRa.setSpreadingFactor(7); // Spreading factor: 7
  LoRa.setSignalBandwidth(125E3); // Bandwidth: 125 kHz
  LoRa.setCodingRate4(5); // Coding rate: 4/5

  // Inisialisasi LCD dengan I2C
  lcd.begin(20, 4);                      // Inisialisasi LCD
  lcd.backlight();                 // Nyalakan backlight LCD
  lcd.clear();                     // Hapus tampilan LCD
  lcd.setCursor(0, 0);
  lcd.print("Suhu: ");

  // Tambahkan waktu pemanasan sensor MQ
  delay(30000); // Tunggu 30 detik untuk pemanasan sensor MQ
}

void loop() {
  // Baca nilai analog dari sensor LM35
  int sensorValue = analogRead(sensorPin);
  
  // Konversi nilai analog ke tegangan (0 - 5V)
  float voltage = sensorValue * (2.3 / 1023.0);
  
  // Konversi tegangan ke suhu dalam derajat Celsius
  temperature = voltage * 10.0;  // LM35 memberikan 10mV/°C

  // Baca nilai dari sensor MQ-7, MQ-135, dan sensor debu
  mq7Value = analogRead(mq7Pin);
  mq135Value = analogRead(mq135Pin);
  dustValue = analogRead(dustPin);

  // Konversi nilai analog MQ-7 ke konsentrasi CO dalam PPM
  float RS_CO = ((1023.0 / (float)mq7Value) - 1.0) * RL;  // Menghitung resistansi sensor (RS)
  COppm = pow(10, (log10(RS_CO / R0_MQ7) - b_CO) / m_CO);  // Menghitung konsentrasi CO dalam PPM

  // Konversi nilai analog MQ-135 ke konsentrasi NH3 dalam PPM
  float RS_NH3 = ((1023.0 / (float)mq135Value) - 1.0) * RL_MQ135;  // Menghitung resistansi sensor (RS)
  NH3ppm = pow(10, (log10(RS_NH3 / R0_MQ135) - b_NH3) / m_NH3);  // Menghitung konsentrasi NH3 dalam PPM

  // Kirim data suhu, nilai MQ-7, MQ-135, dan nilai sensor debu melalui LoRa
  LoRa.beginPacket();
  LoRa.print("Suhu: ");
  LoRa.print(temperature);
  LoRa.print(" C, CO: ");
  LoRa.print(COppm);
  LoRa.print(" PPM, NH3: ");
  LoRa.print(NH3ppm);
  LoRa.print(" PPM, Debu: ");
  LoRa.print(dustValue);
  LoRa.endPacket();

  Serial.print("Mengirim data: Suhu: ");
  Serial.print(temperature);
  Serial.print(" C, CO: ");
  Serial.print(COppm);
  Serial.print(" PPM, NH3: ");
  Serial.print(NH3ppm);
  Serial.print(" PPM, Debu: ");
  Serial.println(dustValue);

  // Tampilkan data pada LCD
  lcd.clear();  // Hapus tampilan sebelum menampilkan data baru

  lcd.setCursor(0, 0);
  lcd.print("Suhu: ");
  lcd.print(temperature, 1); // Menampilkan satu angka desimal untuk suhu
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("CO : ");
  lcd.print(COppm, 2); // Menampilkan dua angka desimal untuk PPM
  lcd.print(" ppm");

  lcd.setCursor(0, 2);
  lcd.print("NH3 : ");
  lcd.print(NH3ppm, 2); // Menampilkan dua angka desimal untuk PPM
  lcd.print(" ppm");

  lcd.setCursor(0, 3);
  lcd.print("Debu : ");
  lcd.print(dustValue);
  lcd.print(" ppm");

  delay(5000);
  
  lcd.clear();

  // Tampilkan kualitas udara berdasarkan nilai PPM CO dan NH3
  int airQualityScore = 0;

  // Skor untuk suhu (nilai suhu yang lebih tinggi dapat diubah sesuai kebutuhan)
  if (temperature < 0 || temperature > 40) {
    airQualityScore += 2; // Suhu yang ekstrem mendapatkan skor lebih tinggi (buruk)
  } else if (temperature < 10 || temperature > 30) {
    airQualityScore += 1; // Suhu yang agak ekstrem mendapatkan skor menengah
  }

  // Skor untuk CO
  if (COppm < 10) {
    airQualityScore += 0; // CO baik
  } else if (COppm < 50) {
    airQualityScore += 1; // CO sedang
  } else {
    airQualityScore += 2; // CO buruk
  }

  // Skor untuk NH3
  if (NH3ppm < 25) {
    airQualityScore += 0; // NH3 baik
  } else if (NH3ppm < 50) {
    airQualityScore += 1; // NH3 sedang
  } else {
    airQualityScore += 2; // NH3 buruk
  }

  // Skor untuk debu (nilai debu yang lebih tinggi dapat diubah sesuai kebutuhan)
  if (dustValue < 50) {
    airQualityScore += 0; // Debu baik
  } else if (dustValue < 150) {
    airQualityScore += 1; // Debu sedang
  } else {
    airQualityScore += 2; // Debu buruk
  }

  // Tentukan kualitas udara berdasarkan skor total
  lcd.setCursor(0, 1);
  lcd.print("Kualitas Udara: ");
  if (airQualityScore <= 2) {
    lcd.setCursor(0, 2);
    lcd.print("Baik");
  } else if (airQualityScore <= 4) {
    lcd.setCursor(0, 2);
    lcd.print("Sedang");
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Buruk");
  }

  delay(5000);  // Kirim data setiap 5 detik
}