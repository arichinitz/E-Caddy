#include <TinyGPS++.h>               // Library for GPS module parsing
#include <Wire.h>                    // I2C communication library
#include <LiquidCrystal_I2C.h>       // Library for LCD display
#include <SoftwareSerial.h>

//Data from website will be inputted into this program as the golfers clubs distance ranges
//Example of ranges is Jacks distances

int minDriverDist = 250;

int maxIronDist = 249;
int minIronDist = 30;

//initializing gps stuff
static const int RXPin = 10, TXPin = 11;

// GPS communication settings
const int GPSBaud = 9600;            // Baud rate for GPS communication

// GPS parsing object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);



// LCD settings
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD at address 0x27, size 16x2

// Actuator control pins
const int dirPin1 = 29;               // Actuator extend direction
const int dirPin2 = 31;               // Actuator retract direction
const int enablePin = 33;             // Actuator enable pin

const int ac2dirPin1 = 37; 
const int ac2dirPin2 = 39;
const int ac2enablePin = 35;

const int buttonPinNewHole = 5;              // Button input pin for the button that says the golf ball went in and its time to move to the next hole

const int buttonPinSameHole = 7;              // Button input pin for the button that says the golf ball went in and its time to move to the next hole


// Target hole coordinates (latitude, longitude)
const double holeCoordinates[][2] = {
  {41.035150, -73.796459}, // Hole 1.  
  {41.037637, -73.797480}, // Hole 2
  {41.035010, -73.799240}, // Hole 3
  {41.036135, -73.798787}, // Hole 4
  {41.034773, -73.801997}, // Hole 5
  {41.034004, -73.806028}, // Hole 6
  {41.037278, -73.806424}, // Hole 7
  {41.035155, -73.804425}, // Hole 8
  {41.034720, -73.802448}, // Hole 9
  {41.037515, -73.805384}, // Hole 10
  {41.038685, -73.798049}, // Hole 11
  {41.039313, -73.793242}, // Hole 12
  {41.037799, -73.792953}, // Hole 13
  {41.038005, -73.797490}, // Hole 14
  {41.037279, -73.798785}, // Hole 15
  {41.037448, -73.803620}, // Hole 16
  {41.037257, -73.799535}, // Hole 17
  {41.035583, -73.802854}  // Hole 18
};

int holeNumber = 1;            // Selected hole (1-based index)


void setup() {
  // Initialize serial monitor and GPS module
  Serial.begin(115200);         // Serial0 for debugging (USB)
  ss.begin(GPSBaud);     // Serial1 for GPS (pins 18 TX1, 19 RX1)

  // Initialize actuator pins and button
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(enablePin, OUTPUT);
  
  // Initialize second actuator
  pinMode(ac2dirPin1, OUTPUT);
  pinMode(ac2dirPin2, OUTPUT);
  pinMode(ac2enablePin, OUTPUT);

  pinMode(buttonPinNewHole, INPUT_PULLUP); // Button with internal pull-up resistor

  pinMode(buttonPinSameHole, INPUT_PULLUP); // Button with internal pull-up resistor


  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  Serial.println("GPS Golf Distance Tracker Initializing...");
  lcd.setCursor(0, 0);
  lcd.print("GPS Initializing");
}

void loop() {
  double targetLat = holeCoordinates[holeNumber - 1][0];
  double targetLng = holeCoordinates[holeNumber - 1][1];



  // Process GPS data using hardware Serial1
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }



//FOR NEXT CLASS WEDNESDAY 3-26
//COPY IF STATEMENT BELOW AND CHANGE buttonPinNewHole to buttonPinSameHole (MAKE THE VARIABLE) and update code as necesary


if (digitalRead(buttonPinSameHole) == LOW) {
    // Display initial message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hole ");
    lcd.print(holeNumber);
    delay(2000);
    
    Serial.println("Button pressed - calculating distance");
    
    // Update GPS data if available and calculate distance
    double distanceYards = 0.0;
    if (gps.location.isUpdated()) {
      double currentLat = gps.location.lat();
      double currentLng = gps.location.lng();

      // Calculate distance
      const double R = 6371000; // Earth's radius in meters
      double latRad1 = radians(currentLat);
      double latRad2 = radians(targetLat);
      double deltaLat = radians(targetLat - currentLat);
      double deltaLon = radians(targetLng - currentLng);
      double a = sin(deltaLat / 2) * sin(deltaLat / 2) +
                 cos(latRad1) * cos(latRad2) *
                 sin(deltaLon / 2) * sin(deltaLon / 2);
      double c = 2 * atan2(sqrt(a), sqrt(1 - a));
      double distanceMeters = R * c;
      distanceYards = distanceMeters * 1.09361;


      // Display distance on LCD
      lcd.setCursor(0, 1);
      lcd.print("Dist: ");
      lcd.print(distanceYards, 1); // Show 1 decimal place
      lcd.print(" yd");
      Serial.print("Distance to hole: ");
      Serial.print(distanceYards);
      Serial.println(" yards");
    }

    // Check for GPS signal timeout
    if (millis() > 7000 && gps.charsProcessed() < 10) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No GPS signal");
      Serial.println("No GPS signal detected.");
      delay(2000); // Pause before retry
    }

    // Perform actuator movement based on distance
    if (distanceYards > minDriverDist) {
      // Activate small actuator - push up and come back
      Serial.println("Activating DRIVER actuator");
      
      // Step 1: Extend actuator
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
      digitalWrite(enablePin, HIGH);
      delay(5000);

      // Step 2: Pause actuator
      digitalWrite(enablePin, LOW);
      delay(1000);

      // Step 3: Retract actuator
      digitalWrite(dirPin1, LOW);
      digitalWrite(dirPin2, HIGH);
      digitalWrite(enablePin, HIGH);
      delay(5000);

      // Step 4: Pause before resetting
      digitalWrite(enablePin, LOW);
      delay(1000);
    } else if (distanceYards <= maxIronDist && distanceYards > minIronDist) {
      // Activating long actuator - pushing up and coming back
      Serial.println("Activating iron actuator");
      
      // Step 1: Extend actuator
      digitalWrite(ac2dirPin1, HIGH);
      digitalWrite(ac2dirPin2, LOW);
      digitalWrite(ac2enablePin, HIGH);
      delay(6000);

      // Step 2: Pause actuator
      digitalWrite(ac2enablePin, LOW);
      delay(1000);

      // Step 3: Retract actuator
      digitalWrite(ac2dirPin1, LOW);
      digitalWrite(ac2dirPin2, HIGH);
      digitalWrite(ac2enablePin, HIGH);
      delay(6000);

      // Step 4: Pause before resetting
      digitalWrite(ac2enablePin, LOW);
      delay(1000);
    }

    delay(1000);
    lcd.clear();
    }








//the button underneath this line is the button that means the hole is done - the golfer is finished on this hole (int holeNumber) - the golf ball went in and its time to move to the next hole
  if (digitalRead(buttonPinNewHole) == LOW) {
   
   

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Switching holes");
    lcd.setCursor(0, 1);
    lcd.print("to hole #");
    holeNumber++;
    lcd.print(holeNumber);

    delay(4000);
    lcd.clear();
    
    
    if (holeNumber == 19) {
      lcd.setCursor(0, 0);
      lcd.print("Go to the bar");
      lcd.setCursor(0, 1);
      lcd.print("and get a drink ");
      delay(5000);
      holeNumber = 1; // Reset to hole 1 instead of halting
    }
  }


}