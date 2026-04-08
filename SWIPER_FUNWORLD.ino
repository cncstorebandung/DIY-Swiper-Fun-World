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

int hargaGame = 15;

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
  
  tampilanStandby();
}

void loop() {
  // Reset jika komunikasi macet
  mfrc522.PCD_Init(); 

  // Cek kartu baru
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Autentikasi
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK) {
    stopKomunikasi();
    return;
  }

  // Baca Saldo
  byte buffer[18];
  byte size = sizeof(buffer);
  if (mfrc522.MIFARE_Read(4, buffer, &size) != MFRC522::STATUS_OK) {
    stopKomunikasi();
    return;
  }

  int saldoKartu = buffer[0];
  lcd.clear();

  if (saldoKartu >= hargaGame) {
    saldoKartu -= hargaGame;
    buffer[0] = (byte)saldoKartu;
    mfrc522.MIFARE_Write(4, buffer, 16);

    lcd.print("SUCCESS! PLAY!");
    lcd.setCursor(0, 1);
    lcd.print("Sisa: "); lcd.print(saldoKartu); lcd.print(" Tizo");

    nadaMarioKedapKedip(); // LED Hijau ikut irama
  } 
  else {
    lcd.print("SALDO KURANG!");
    lcd.setCursor(0, 1);
    lcd.print("Sisa: "); lcd.print(saldoKartu); lcd.print(" Tizo");

    bipPanjangKedapKedip(); // LED Merah ikut irama
  }

  stopKomunikasi();
  delay(500); // Jeda sebentar sebelum bisa tap lagi
  tampilanStandby();
}

void stopKomunikasi() {
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// --- FUNGSI NADA SINKRON LED ---

void nadaMarioKedapKedip() {
  int melodi[] = {659, 659, 659, 523, 659, 784};
  int durasi[] = {150, 300, 300, 100, 300, 550};

  for (int i = 0; i < 6; i++) {
    digitalWrite(LED_HIJAU, HIGH); // LED ON
    tone(BUZZER, melodi[i], durasi[i] * 0.8);
    delay(durasi[i]);
    digitalWrite(LED_HIJAU, LOW);  // LED OFF
    delay(50); 
  }
}

void bipPanjangKedapKedip() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_MERAH, HIGH); // LED ON
    tone(BUZZER, 200, 600);
    delay(700);
    digitalWrite(LED_MERAH, LOW);  // LED OFF
    delay(200);
  }
}

void tampilanStandby() {
  lcd.clear();
  lcd.print(" SWIPE TO PLAY ");
  lcd.setCursor(0, 1);
  lcd.print("Cost: 15 Tizo");
}