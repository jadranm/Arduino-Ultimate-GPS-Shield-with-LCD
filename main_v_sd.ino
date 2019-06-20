#include <SD.h>   //za SD
#include <SPI.h>  //za serisku komunikaciju
#include <Wire.h>
#include <avr/sleep.h>
#include <Adafruit_GPS.h>   
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>


SoftwareSerial mySerial(8, 7);

Adafruit_GPS GPS(&mySerial); //inicijalizacija seriskog prijenosa

LiquidCrystal_I2C lcd(0x3F, 16, 2); //lcd inicijalizacija

#define GPSECHO  false  //iskljucujemo ispis NMEA recenica
#define LOG_FIXONLY true

boolean usingInterrupt = false;
void useInterrupt(boolean);

#define chipSelect 10
#define ledPin 13

File logfile;

void setup() {
  while (!Serial);  //seriska komunikacija
  Serial.begin(115200);
  delay(5000);

  pinMode(ledPin, OUTPUT);
  pinMode(10, OUTPUT);
  lcd.begin();           // inicijaliziranje LCD‐a
  lcd.backlight();      // uključivanje pozadinskog osvjetljenja LCD‐a
  lcd.setCursor(0, 0); // postavljanje kursora na lokaciju 0,0
  //lcd.print("inicijalizacija");
  delay(1500);
  lcd.clear();

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  delay(1000);
  mySerial.println(PMTK_Q_RELEASE); //provjera verzije

}

uint32_t timer = millis();
void loop() {
  char c = GPS.read();
  if ((c) && (GPSECHO))
    Serial.write(c);

  if (GPS.newNMEAreceived()) {

    if (!GPS.parse(GPS.lastNMEA()))
      return;
  }
  if (timer > millis())  timer = millis();

  if (millis() - timer > 2000) { //parsing
    timer = millis();

    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);

      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);

      if (logfile) {
        Serial.print("zapisivanje na SD");
        logfile.println(GPS.longitude); logfile.print(","); logfile.print(GPS.latitude);  //zapisivanje na SD
        logfile.close();

      } else {
        Serial.println("greska");
      }
    }

    lcd.clear();//ne dirati
    lcd.print("no fix");
    delay(1000);
    /*
      lcd.print(GPS.hour); lcd.print(":");
      lcd.print(GPS.minute); lcd.print(":");
      lcd.print(GPS.seconds);
    */
    if (GPS.fix) {  //provjera dostupnosti signala
      lcd.setCursor(0, 0);
      if (int(GPS.speed) < 1) {
        lcd.print("0"); lcd.print(" km/h*");
        lcd.setCursor(0, 1);
        lcd.print("sateliti: "); lcd.print((int)GPS.satellites);
      }
      else
      {
        lcd.print(float(GPS.speed) * 1.852); lcd.print(" km/h");
        lcd.setCursor(0, 1);
        lcd.print("sateliti: "); lcd.print((int)GPS.satellites);
      }
      delay(2000);

      lcd.clear();  
      lcd.setCursor(0, 0);
      lcd.print(GPS.latitude);lcd.print(" N");
      lcd.setCursor(0, 1);
      lcd.print(GPS.longitude);lcd.print(" S");
      delay(2000);
      lcd.clear();
    }
  }
}
