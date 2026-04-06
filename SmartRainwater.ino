#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
int rainSensorPin = A0;
int servoPin = 9;
int trigPin = 7;
int echoPin = 8;

// Settings
int threshold = 700;
bool servoOpen = false;

// Tank height (CHANGE if needed)
const int tankHeight = 13;   // 13 cm empty, 1 cm full

// -------- Stable Ultrasonic Reading --------
long getDistance()
{
  long total = 0;

  for(int i = 0; i < 5; i++)
  {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);
    long distance = duration * 0.034 / 2;

    total += distance;
    delay(50);
  }

  return total / 5;   // average distance
}

void setup()
{
  myServo.attach(servoPin);
  myServo.write(0);

  pinMode(rainSensorPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
}

void loop()
{
  int sensorValue = analogRead(rainSensorPin);

  // -------- Rain Confirmation --------
  bool rainDetected = false;

  if(sensorValue < threshold)
  {
    delay(2000);
    if(analogRead(rainSensorPin) < threshold)
      rainDetected = true;
  }

  // -------- Get Averaged Distance --------
  long avgDistance = getDistance();

  // Limit valid range
  avgDistance = constrain(avgDistance, 1, tankHeight);

  // -------- Calculate Percentage using map() --------
  int waterLevel = map(avgDistance, tankHeight, 1, 0, 100);
  waterLevel = constrain(waterLevel, 0, 100);

  //  Full tank stabilization
  if(waterLevel >= 91)
  {
    waterLevel = 100;
  }

  // -------- Serial Output --------
  Serial.print("Distance: ");
  Serial.print(avgDistance);
  Serial.print(" cm | Water: ");
  Serial.print(waterLevel);
  Serial.println("%");

  // -------- Clean Serial Line for Python Dashboard --------
  Serial.println(waterLevel);   // <-- THIS is important
  delay(500);                   // small delay for stability

  // -------- Smart Servo Logic --------
  if(waterLevel >= 98)
  {
    if(servoOpen)
    {
      myServo.write(0);
      servoOpen = false;
    }
  }
  else if(rainDetected)
  {
    if(!servoOpen)
    {
      myServo.write(90);
      servoOpen = true;
    }
  }
  else
  {
    if(servoOpen)
    {
      myServo.write(0);
      servoOpen = false;
    }
  }

  // -------- LCD Display --------
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Water: ");
  lcd.print(waterLevel);
  lcd.print("%");

  lcd.setCursor(0,1);

  if(waterLevel >= 98)
  {
    lcd.print("Tank Full");
  }
  else if(rainDetected)
  {
    lcd.print("Filling...");
  }
  else
  {
    lcd.print("No Rain");
  }

  delay(1000);
}
