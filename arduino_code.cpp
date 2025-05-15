#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial BTSerial(2, 3);  // RX, TX
Servo myServo;  // Yönlendirme servosu
Servo Drop;     // Düşürme mekanizması servosu

int dropClosed = 80;
int dropOpen = 30;

String lastCommand = "";

int getServoAngle(char colorCode) {
  switch (colorCode) {
    case 'R': return 0;
    case 'G': return 25;
    case 'B': return 50;
    case 'Y': return 80;
    case 'O': return 115;
    case 'N': return 140;
    case 'M': return 195;
    default:  return -1;
  }
}

void setup() {
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  BTSerial.begin(9600);

  myServo.attach(11);  // Yönlendirme servosu
  Drop.attach(8);      // Bırakma mekanizması servosu

  myServo.write(0);
  Drop.write(dropClosed);
}

void loop() {
  if (BTSerial.available()) {
    String incomingStr = BTSerial.readStringUntil('\n');
    incomingStr.trim();

// ASCII karakter kontrolü (32-126 arası geçerli karakterler)
  if (incomingStr.length() != 1 || incomingStr.charAt(0) < 32 || incomingStr.charAt(0) > 126) {
    Serial.print("Geçersiz ASCII karakter alındı: [");
    Serial.print(incomingStr);
    Serial.print("] ASCII kodu: ");
    Serial.println((int)incomingStr.charAt(0));

    while (BTSerial.available()) BTSerial.read();  // buffer temizle
    return;
  }

  if (incomingStr.length() == 0) {
    Serial.println("Boş veri alındı.");
    while (BTSerial.available()) BTSerial.read();
    return;
  }

    char c = incomingStr.charAt(0);
    Serial.print("Received: ");
    Serial.println(c);

    if (incomingStr != lastCommand) {
      lastCommand = incomingStr;

      int targetAngle = getServoAngle(c);
      String colorName = "";

      // Renk karakterinden isme dönüştür
      switch (c) {
        case 'R': colorName = "Red"; break;
        case 'G': colorName = "Green"; break;
        case 'B': colorName = "Blue"; break;
        case 'Y': colorName = "Yellow"; break;
        case 'O': colorName = "Orange"; break;
        case 'N': colorName = "Brown"; break;
        case 'M': colorName = "Non-MM"; break;
        default:  colorName = "Unknown"; break;
      }

      // LCD Güncellemesi
      lcd.setCursor(0, 0);
      lcd.print("Color:            ");  // önce sil
      lcd.setCursor(0, 0);
      lcd.print("Color: " + colorName);

      lcd.setCursor(0, 1);
      lcd.print("Code: "); lcd.print(c);
      lcd.print("  Deg: "); lcd.print(targetAngle);

      // Servo hareketleri
      if (targetAngle >= 0) {
        myServo.write(0);
        delay(500);
        myServo.write(targetAngle);
        delay(500);

        Drop.write(dropOpen);
        delay(1000);
        Drop.write(dropClosed);
      }
    }
  }

}
