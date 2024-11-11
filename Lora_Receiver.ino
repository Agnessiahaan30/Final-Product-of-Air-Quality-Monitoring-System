#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>

const int loraCsPin = 10; // Pin Chip Select untuk LoRa
const int resetPin = 9;   // Pin Reset untuk LoRa

SoftwareSerial espSerial(2, 3); // RX, TX pins for ESP32 communication

void setup() {
    Serial.begin(115200);
    espSerial.begin(115200);      // Begin software serial for ESP32
    while (!Serial)
        ; // Inisialisasi LoRa
    LoRa.setPins(loraCsPin, resetPin, -1); // CS, RST, IRQ (-1: tidak dipakai)
    if (!LoRa.begin(915E6)) {
        Serial.println("Gagal mulai LoRa!");
        while (1)
            ;
    }
    Serial.println("LoRa mulai...");
}

void loop() {
    // Cek jika ada paket yang diterima
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // Jika ada paket, baca data
        String dataReceived = "";
        while (LoRa.available()) {
            dataReceived += (char)LoRa.read(); // Baca data LoRa dan simpan dalam string
        }
        Serial.println(dataReceived); // Kirim data ke serial

        // Hitung dan tampilkan RSSI
        int rssi = LoRa.packetRssi();
        Serial.print(", RSSI: ");
        Serial.println(rssi);

        // Hitung dan tampilkan SNR
        float snr = LoRa.packetSnr();
        Serial.print(", SNR: ");
        Serial.println(snr);

        // Tentukan  kualitas sinyal
        String signalQuality;
        if (rssi >= -70 && snr >= 7) {
            signalQuality = "Baik";
        } else if (rssi >= -90 && snr >= 3) {
            signalQuality = "Sedang";     
        } else {
            signalQuality = "Buruk";
        }

        Serial.print("Kualitas Udara: ");
        Serial.println(signalQuality);

        // Kirim data ke ESP32 melalui komunikasi serial
        espSerial.print(dataReceived);
        espSerial.print(", RSSI: ");
        espSerial.print(rssi);
        espSerial.print(", SNR: ");
        espSerial.print(snr);
        espSerial.print(", Kualitas Udara: ");
        espSerial.println(signalQuality);
    }
}