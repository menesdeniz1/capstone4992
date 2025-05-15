#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>

// LCD at I2C address 0x27, 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial BTSerial(2, 3);  // RX, TX for Bluetooth module
Servo directionServo;  // Servo for positioning
Servo dropServo;       // Servo for dropping mechanism

int dropClosed = 80;   // Closed position angle for drop servo
int dropOpen = 30;     // Open position angle for drop servo

String lastCommand = "";

// Map color code character to servo angle
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

  directionServo.attach(11);  // Attach direction servo to pin 11
  dropServo.attach(8);        // Attach drop servo to pin 8

  directionServo.write(0);         // Initial position
  dropServo.write(dropClosed);     // Keep drop closed
}

void loop() {
  if (BTSerial.available()) {
    String incomingStr = BTSerial.readStringUntil('\n');
    incomingStr.trim();

    // Validate ASCII character (printable range: 32–126)
    if (incomingStr.length() != 1 || incomingStr.charAt(0) < 32 || incomingStr.charAt(0) > 126) {
      Serial.print("Invalid ASCII character received: [");
      Serial.print(incomingStr);
      Serial.print("] ASCII code: ");
      Serial.println((int)incomingStr.charAt(0));

      while (BTSerial.available()) BTSerial.read();  // Clear buffer
      return;
    }

    if (incomingStr.length() == 0) {
      Serial.println("Empty data received.");
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

      // Map color code to full name
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

      // Update LCD
      lcd.setCursor(0, 0);
      lcd.print("Color:            ");  // Clear line
      lcd.setCursor(0, 0);
      lcd.print("Color: " + colorName);

      lcd.setCursor(0, 1);
      lcd.print("Code: "); lcd.print(c);
      lcd.print("  Deg: "); lcd.print(targetAngle);

      // Perform servo movements
      if (targetAngle >= 0) {
        directionServo.write(0);
        delay(500);
        directionServo.write(targetAngle);
        delay(500);

        dropServo.write(dropOpen);
        delay(1000);
        dropServo.write(dropClosed);
      }
    }
  }
}
