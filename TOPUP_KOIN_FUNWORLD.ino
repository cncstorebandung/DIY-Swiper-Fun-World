#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9
#define LED_HIJAU 7
#define LED_MERAH 6
#define BUZZER 8

MFRC522 mfrc522(SS_PIN, RST_PIN); 
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init(); 
  lcd.backlight();
  
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; 
  
  tampilanStandbyKasir();
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;

  lcd.clear(); 
  lcd.print("KARTU TERDETEKSI");
  lcd.setCursor(0, 1);
  lcd.print("INPUT DI SERIAL");

  Serial.println("\nMasukkan jumlah koin (10 - 100):");
  while (Serial.available() == 0);
  int inputKoin = Serial.parseInt();

  // --- LOGIKA VALIDASI & INDIKATOR ---
  if (inputKoin < 10 || inputKoin > 100) {
    // JIKA GAGAL (Rentang salah)
    Serial.println("GAGAL: Jumlah koin tidak sesuai aturan!");
    lcd.clear();
    lcd.print("INPUT SALAH!");
    lcd.setCursor(0, 1);
    lcd.print("10 - 100 KOIN");
    
    aksiGagal(); // Nyalakan LED Merah & Buzzer Bip Cepat
  } 
  else {
    // PROSES TULIS KE KARTU
    byte dataBlock[16] = {0};
    dataBlock[0] = (byte)inputKoin; 

    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)) == MFRC522::STATUS_OK) {
      mfrc522.MIFARE_Write(4, dataBlock, 16);
      
      Serial.print("BERHASIL! +"); Serial.println(inputKoin);

      lcd.clear();
      lcd.print("TOPUP BERHASIL!");
      lcd.setCursor(0, 1);
      lcd.print("+ "); lcd.print(inputKoin); lcd.print(" KOIN");
      
      aksiBerhasil(); // Nyalakan LED Hijau & Nada Berhasil
    } else {
      Serial.println("GAGAL: Kartu tidak terbaca dengan baik!");
      lcd.clear();
      lcd.print("AKSES GAGAL!");
      aksiGagal();
    }
  }

  delay(1000);
  tampilanStandbyKasir();
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// --- FUNGSI INDIKATOR ---

void aksiBerhasil() {
  digitalWrite(LED_HIJAU, HIGH);
  // Nada Berhasil (Dua nada naik: do - mi)
  tone(BUZZER, 523, 200); // Nada C5
  delay(200);
  tone(BUZZER, 659, 300); // Nada E5
  delay(2000); // LED menyala selama 2 detik
  digitalWrite(LED_HIJAU, LOW);
}

void aksiGagal() {
  digitalWrite(LED_MERAH, HIGH);
  // Nada Gagal (Bip bip bip cepat)
  for (int i = 0; i < 3; i++) {
    tone(BUZZER, 150, 150); // Nada rendah (150Hz)
    delay(200);
  }
  delay(1000); // LED merah tetap menyala sebentar
  digitalWrite(LED_MERAH, LOW);
}

void tampilanStandbyKasir() {
  lcd.clear();
  lcd.print(" KASIR TIMEZONE ");
  lcd.setCursor(0, 1);
  lcd.print("SIAP ISI KOIN...");
}