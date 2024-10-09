#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TRIG_PIN 3  
#define ECHO_PIN 4
#define RELAY_PIN 9

#define TANK_FULL 30  // JSN-SR04T can go as low as 17cm (30cm from spec) check your sensor minimum distance
#define TANK_EMPTY 140
const int motor_off_at = 95;

const int thermistorPin = A1;
const int seriesResistor = 10000; // 10k resistor
const int thermistorNominal = 10000; // 10k thermistor
const int temperatureNominal = 25; // 25 degrees C
const int bCoefficient = 3950; // Beta coefficient of the thermistor
const int samples = 5; // Number of samples to take


LiquidCrystal_I2C lcd(0x27, 16, 2);

int d;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay off initially (active low)

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(15);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(30);
  digitalWrite(TRIG_PIN, LOW);

  int duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;

  if(distance>0 && distance<200)
    d=distance;

  Serial.print("Distance:");
  Serial.println(distance);
  delay(100);

  int waterLevel = map(d, TANK_EMPTY, TANK_FULL, 0, 100);
  waterLevel = constrain(waterLevel, 0, 100);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Level: ");
  lcd.print(waterLevel);
  lcd.print("%");

  if (waterLevel <= 40) {
    digitalWrite(RELAY_PIN, LOW); // Turn on motor
    lcd.setCursor(0, 1);
    lcd.print("M:ON");
  } 
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("M:ON");
  }
  
  if (waterLevel >=motor_off_at)
  {
    digitalWrite(RELAY_PIN, HIGH); // Turn off motor
    lcd.setCursor(0, 1);
    lcd.print("M:OFF");
  }

  float average = 0;

  // Take multiple samples to get an average
  for (int i = 0; i < samples; i++) {
    average += analogRead(thermistorPin);
    delay(10);
  }
  average /= samples;

  // Convert the value to resistance
  average = 1023 / average - 1;
  average = seriesResistor / average;

  // Calculate temperature in Celsius
  float steinhart;
  steinhart = average / thermistorNominal; // (R/Ro)
  steinhart = log(steinhart); // ln(R/Ro)
  steinhart /= bCoefficient; // 1/B * ln(R/Ro)
  steinhart += 1.0 / (temperatureNominal + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart; // Invert
  steinhart -= 273.15; // Convert to Celsius

  lcd.setCursor(7,1);
  lcd.print("T:");
  // Display temperature on LCD
  lcd.setCursor(9,1);
  lcd.print(steinhart);
  lcd.print((char)223); // Degree symbol
  lcd.print("C");

  delay(1000);

}
