#include <Arduino.h>
#include <Arduino_LSM9DS1.h>
#include <MadgwickAHRS.h>

const int ledPin = LED_BUILTIN;
int blinkRate = 0;
int currentState = -1;
Madgwick madgwickFilter;
float sensorRate = 0.00;
unsigned long microsPerReading, microsPrevious;

#define RED_PIN 22
#define GREEN_PIN 23
#define BLUE_PIN 24

#define INIT 1
#define START 2
#define STOP 3
#define INFERENCE 4
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

void turn_white_led()
{
  toggle_LED(RED_PIN, LOW);
  toggle_LED(GREEN_PIN, LOW);
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

void start_inferencing()
{
  turn_white_led();
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
  Serial.begin(115200);
  while (!Serial.available())
    ; // wait until a byte was received
  if (!IMU.begin())
  {
    Serial.println("400:Failed to initialized IMU!");

    while (1)
      ;
  }
  sensorRate = IMU.accelerationSampleRate();
  madgwickFilter.begin(sensorRate);
}

float normalize_a(float ax)
{
  return ax * 4.0 / 8.0;
}

float normalize_g(float gx)
{
  return gx * 2000 / 4000;
}

void loop()
{
  float ax, ay, az, gx, gy, gz;
  float yaw, pitch, roll;
  float norm_ax, norm_ay, norm_az, norm_gx, norm_gy, norm_gz;

  int accelAvail = IMU.accelerationAvailable();
  int gyroAvail = IMU.gyroscopeAvailable();

  if (accelAvail && gyroAvail && (currentState == START || currentState == INFERENCE))
  {
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);
    // IMU.readMagneticField(mx, my, mz);
    norm_ax = normalize_a(ax);
    norm_ay = normalize_a(ay);
    norm_az = normalize_a(az);
    norm_gx = normalize_g(gx);
    norm_gy = normalize_g(gy);
    norm_gz = normalize_g(gz);
    String actionHeader = "";
    switch (currentState)
    {
    case START:
      actionHeader = "100:";
      break;
    case INFERENCE:
      actionHeader = "900:";
      break;
    }

    String accData = actionHeader +
                     String(norm_ax, 6) + ',' + String(norm_ay, 6) + ',' + String(norm_az, 6) + ',' +
                     String(norm_gx, 6) + ',' + String(norm_gy, 6) + ',' + String(norm_gz, 6) + ',';
    madgwickFilter.updateIMU(gx, gy, gz, ax, ay, az);

    yaw = madgwickFilter.getYawRadians();
    pitch = madgwickFilter.getPitchRadians();
    roll = madgwickFilter.getRollRadians();

    accData +=
        String(yaw, 6) + ',' + String(pitch, 6) + ',' + String(roll, 6) + '\n';
    Serial.write(accData.c_str());
  }

  if (Serial.available() > 0)
  {
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
      case INFERENCE:
        start_inferencing();
        currentState = INFERENCE;
        break;
      }
    }
  }
}
