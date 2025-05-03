#include <TinyGPS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

// OLED setup
#define OLED_RESET -1
Adafruit_SH1106 display(OLED_RESET);

// GPS setup (using Serial1 for Mega)
TinyGPS gps;

// Button pins
const int buttonDownPin = 7;
const int buttonUpPin = 8;

int screenNumber = 0;
unsigned long previousMillis = 0;
const long screenDelay = 1000;

void setup() {
  Serial.begin(9600);      // for debugging
  Serial1.begin(9600);     // GPS module on Serial1 (TX1/RX1 → Pins 18/19)

  pinMode(buttonDownPin, INPUT_PULLUP);
  pinMode(buttonUpPin, INPUT_PULLUP);

  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Initializing GPS...");
  display.display();
}

void loop() {
  while (Serial1.available()) {
    gps.encode(Serial1.read());
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= screenDelay) {
    previousMillis = currentMillis;
    displayScreen();
  }

  checkButtons();
}

void checkButtons() {
  static bool lastDown = HIGH, lastUp = HIGH;
  bool down = digitalRead(buttonDownPin);
  bool up = digitalRead(buttonUpPin);

  if (down == LOW && lastDown == HIGH) {
    screenNumber = (screenNumber + 1) % 4;
    delay(200);
  }
  if (up == LOW && lastUp == HIGH) {
    screenNumber = (screenNumber == 0) ? 3 : screenNumber - 1;
    delay(200);
  }

  lastDown = down;
  lastUp = up;
}

void displayScreen() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  float lat, lon;
  unsigned long age;
  gps.f_get_position(&lat, &lon, &age);

  if (lat == TinyGPS::GPS_INVALID_F_ANGLE || lon == TinyGPS::GPS_INVALID_F_ANGLE || age > 5000) {
    display.setTextSize(1);
    display.setCursor(10, 20);
    display.println("NO GPS FIX...");
    display.setCursor(10, 40);
    display.println("Waiting for signal");
    display.display();
    return;
  }

  switch (screenNumber) {
    case 0: showTime(); break;
    case 1: showLatLon(lat, lon); break;
    case 2: showAltitudeHeading(); break;
    case 3: showSatellitesSpeed(); break;
  }

  display.display();
}

void showTime() {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;

  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("UTC Time:");

  display.setTextSize(2);
  display.setCursor(10, 25);
  if (hour < 10) display.print("0");
  display.print(hour); display.print(":");
  if (minute < 10) display.print("0");
  display.print(minute); display.print(":");
  if (second < 10) display.print("0");
  display.print(second);
}

void showLatLon(float lat, float lon) {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Latitude:");
  display.setTextSize(1.5);
  display.setCursor(0, 16);
  display.print(lat, 4);

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println("Longitude:");
  display.setTextSize(1.5);
  display.setCursor(0, 56);
  display.print(lon, 4);
}

void showAltitudeHeading() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Altitude (m):");
  display.setTextSize(1.5);
  display.setCursor(0, 16);
  float alt = gps.f_altitude();
  if (alt == TinyGPS::GPS_INVALID_F_ALTITUDE)
    display.print("Invalid");
  else
    display.print(alt, 1);

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println("Heading (deg):");
  display.setTextSize(1.5);
  display.setCursor(0, 56);
  float head = gps.f_course();
  if (head == TinyGPS::GPS_INVALID_F_ANGLE)
    display.print("Invalid");
  else
    display.print(head, 1);
}

void showSatellitesSpeed() {
  int sats = gps.satellites();
  float spd = gps.f_speed_kmph();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Satellites:");
  display.setTextSize(1.5);
  display.setCursor(0, 16);
  if (sats == TinyGPS::GPS_INVALID_SATELLITES)
    display.print("N/A");
  else
    display.print(sats);

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println("Speed (km/h):");
  display.setTextSize(1.5);
  display.setCursor(0, 56);
  if (spd == TinyGPS::GPS_INVALID_F_SPEED)
    display.print("N/A");
  else
    display.print(spd, 1);
}
