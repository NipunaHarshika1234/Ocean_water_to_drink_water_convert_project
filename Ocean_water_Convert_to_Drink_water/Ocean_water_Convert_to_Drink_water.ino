#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ===================== DS18B20 =====================
#define ONE_WIRE_BUS 3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ===================== DHT22 =====================
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===================== Ultrasonic Sensors =====================
#define TRIG_PIN1 4
#define ECHO_PIN1 5

#define TRIG_PIN2 6
#define ECHO_PIN2 7

// ===================== Tank Control =====================
#define RELAY_PIN       9
#define RELAY_FEEDBACK 10
#define LOWER_LEVEL    11
#define UPPER_LEVEL    12
#define ERROR_LED       8
#define MOTOR_LED      14

// ===================== Variables =====================
float hum = 0;
float temp = 0;
float Celsius = 0;
float Fahrenheit = 0;

float duration1, distance1;
float duration2, distance2;

double percentage1 = 0;
double percentage2 = 0;

// Tank depth in cm
const float TANK_HEIGHT = 200.0;

// =====================================================
// Ultrasonic Sensor Reading
// =====================================================
void ultrasonic()
{
  // Sensor 1
  digitalWrite(TRIG_PIN1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN1, LOW);

  duration1 = pulseIn(ECHO_PIN1, HIGH, 30000);

  if (duration1 > 0)
  {
    distance1 = (duration1 * 0.0343) / 2;

    percentage1 =
      ((TANK_HEIGHT - distance1) / TANK_HEIGHT) * 100.0;

    percentage1 = constrain(percentage1, 0, 100);

    Serial.print("Tank 1 Distance: ");
    Serial.print(distance1);
    Serial.println(" cm");

    Serial.print("Tank 1 Level: ");
    Serial.print(percentage1);
    Serial.println("%");
  }
  else
  {
    Serial.println("Tank 1 Sensor Timeout");
  }

  // Sensor 2
  digitalWrite(TRIG_PIN2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN2, LOW);

  duration2 = pulseIn(ECHO_PIN2, HIGH, 30000);

  if (duration2 > 0)
  {
    distance2 = (duration2 * 0.0343) / 2;

    percentage2 =
      ((TANK_HEIGHT - distance2) / TANK_HEIGHT) * 100.0;

    percentage2 = constrain(percentage2, 0, 100);

    Serial.print("Tank 2 Distance: ");
    Serial.print(distance2);
    Serial.println(" cm");

    Serial.print("Tank 2 Level: ");
    Serial.print(percentage2);
    Serial.println("%");
  }
  else
  {
    Serial.println("Tank 2 Sensor Timeout");
  }
}

// =====================================================
// DHT22
// =====================================================
void humiditySensor()
{
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  if (isnan(hum) || isnan(temp))
  {
    Serial.println("Failed to read DHT22!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.print("Temperature (DHT22): ");
  Serial.print(temp);
  Serial.println(" °C");
}

// =====================================================
// DS18B20
// =====================================================
void tempSensor()
{
  sensors.requestTemperatures();

  Celsius = sensors.getTempCByIndex(0);
  Fahrenheit = sensors.toFahrenheit(Celsius);

  Serial.print("DS18B20 Temperature: ");
  Serial.print(Celsius);
  Serial.println(" °C");

  Serial.print("DS18B20 Temperature: ");
  Serial.print(Fahrenheit);
  Serial.println(" °F");
}

// =====================================================
// Setup
// =====================================================
void setup()
{
  Serial.begin(9600);

  dht.begin();
  sensors.begin();

  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);

  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ERROR_LED, OUTPUT);
  pinMode(MOTOR_LED, OUTPUT);

  pinMode(LOWER_LEVEL, INPUT);
  pinMode(UPPER_LEVEL, INPUT);
  pinMode(RELAY_FEEDBACK, INPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(ERROR_LED, LOW);
  digitalWrite(MOTOR_LED, LOW);

  Serial.println("System Started");
}

// =====================================================
// Main Loop
// =====================================================
void loop()
{
  ultrasonic();
  humiditySensor();
  tempSensor();

  int upperLevel = digitalRead(UPPER_LEVEL);
  int lowerLevel = digitalRead(LOWER_LEVEL);
  int relayFeedback = digitalRead(RELAY_FEEDBACK);

  // Motor status indication
  if (digitalRead(RELAY_PIN) == HIGH)
  {
    Serial.println("Motor is ON");
    digitalWrite(MOTOR_LED, HIGH);
  }
  else
  {
    digitalWrite(MOTOR_LED, LOW);
  }

  // ===================================================
  // Tank Control Logic
  // ===================================================

  // Tank empty -> Start motor
  if (relayFeedback == LOW &&
      upperLevel == LOW &&
      lowerLevel == LOW)
  {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(ERROR_LED, LOW);

    Serial.println("Motor ON");
    Serial.println("Loop1");
  }

  // Fault condition
  else if (upperLevel == HIGH &&
           lowerLevel == LOW)
  {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(ERROR_LED, HIGH);

    Serial.println("Motor OFF");
    Serial.println("Loop4");
    Serial.println("ERROR: Tank sensor fault!");
  }

  // Tank full -> Stop motor
  else if (relayFeedback == HIGH &&
           upperLevel == HIGH)
  {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(ERROR_LED, LOW);

    Serial.println("Motor OFF");
    Serial.println("Loop2");
  }

  // Tank below lower level -> Start motor
  else if (relayFeedback == LOW &&
           lowerLevel == LOW)
  {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(ERROR_LED, LOW);

    Serial.println("Motor ON");
    Serial.println("Loop3");
  }

  Serial.print("Upper Level = ");
  Serial.print(upperLevel);

  Serial.print(" | Lower Level = ");
  Serial.println(lowerLevel);

  Serial.println("*************************");

  delay(1000);
}