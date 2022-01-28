#include <Arduino.h>
#include <Arduino_LSM9DS1.h>

const int ledPin = LED_BUILTIN;
int blinkRate = 0;
int currentState = -1;

#define RED_PIN 22
#define GREEN_PIN 23
#define BLUE_PIN 24

#define INIT 1
#define START 2
#define STOP 3
#define ACC_DATA 100
#define ERROR_ACC_INT 400

void powerOffAllLEDs()
{
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
}

void toggle_LED(int pin, int value)
{
  digitalWrite(pin, value);
}

void turn_red_led()
{
  toggle_LED(RED_PIN, LOW);
  toggle_LED(GREEN_PIN, HIGH);
  toggle_LED(BLUE_PIN, HIGH);
}

void turn_green_led()
{
  toggle_LED(RED_PIN, HIGH);
  toggle_LED(GREEN_PIN, LOW);
  toggle_LED(BLUE_PIN, HIGH);
}

void turn_blue_led()
{
  toggle_LED(RED_PIN, HIGH);
  toggle_LED(GREEN_PIN, HIGH);
  toggle_LED(BLUE_PIN, LOW);
}

void start_recording()
{
  turn_red_led();
}

void stop_recording()
{
  turn_green_led();
}

void init_recording()
{
  turn_blue_led();
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup()
{
  // intitialize the LED Pins as an output
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  powerOffAllLEDs();
  Serial.begin(9600);
  while (!Serial.available())
    ; //wait until a byte was received

  if (!IMU.begin())
  {
    Serial.println("400:Failed to initialized IMU!");

    while (1)
      ;
  }
}

void loop()
{
  float ax, ay, az, gx, gy, gz;
  int accelAvail = IMU.accelerationAvailable();
  int gyroAvail = IMU.gyroscopeAvailable();
  if (accelAvail && currentState == START)
  {
    IMU.readAcceleration(ax, ay, az);
  }

  if (gyroAvail && currentState == START)
  {
    IMU.readGyroscope(gx, gy, gz);
  }

  if (accelAvail && gyroAvail && currentState == START)
  {
    String accData = "100:" + String(ax, 6) + ',' + String(ay, 6) + ',' + String(az, 6) + ',' + String(gx, 6) + ',' + String(gy, 6) + ',' + String(gz, 6) + '\n';
    Serial.write(accData.c_str());
    delay(20);
  }

  if (Serial.available() > 0)
  {
    // Data format defined as <action>:<payload> where action is set of integers
    // defined in actions.h and payload can be any sort of csv separated data
    String data = Serial.readStringUntil('\n');
    Serial.print("received by arduino " + data);

    if (data)
    {
      String action = getValue(data, ':', 0);
      String payload = getValue(data, ':', 1);
      switch (action.toInt())
      {
      case START:
        start_recording();
        currentState = START;
        break;
      case STOP:
        stop_recording();
        currentState = STOP;
        break;
      case INIT:
        init_recording();
        currentState = INIT;
        break;
      }
    }
  }
}