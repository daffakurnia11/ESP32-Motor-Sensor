#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <HTTPClient.h>

String plant = "Motor1";
int delayLoop = 4000;

// Variable Declaration
float temperature = 0, ambient = 0;
float volt = 0, ampere = 0, power = 0;
float x = 0, y = 0, z = 0;
int response, sensor1_id = 0, sensor2_id = 0, sensor3_id = 0;
float sensor1_warning = 0, sensor1_danger = 0;
float sensor2_warning = 0, sensor2_danger = 0;

// PZEM Serial Declarations
#define RXD2 16
#define TXD2 17

// LED Pin Declaration
#define LED1_RED 32
#define LED1_YELLOW 33
#define LED1_GREEN 25
#define LED2_RED 27
#define LED2_YELLOW 14
#define LED2_GREEN 12

// SSID Declaration
const char *ssid = "Deltakilo";
const char *pass = "1sempak8";

// API Declaration
String baseUrl = "http://128.199.87.189/api";
HTTPClient http;
WiFiClient wifi;

// Sensor Declaration
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
PZEM004Tv30 pzem_r(Serial2, RXD2, TXD2);

void ledBlinking()
{
  digitalWrite(LED1_GREEN, LOW);
  digitalWrite(LED1_YELLOW, LOW);
  digitalWrite(LED1_RED, LOW);
  digitalWrite(LED2_GREEN, LOW);
  digitalWrite(LED2_YELLOW, LOW);
  digitalWrite(LED2_RED, LOW);
  delay(500);
  digitalWrite(LED1_GREEN, HIGH);
  digitalWrite(LED1_YELLOW, HIGH);
  digitalWrite(LED1_RED, HIGH);
  digitalWrite(LED2_GREEN, HIGH);
  digitalWrite(LED2_YELLOW, HIGH);
  digitalWrite(LED2_RED, HIGH);
  delay(500);
  digitalWrite(LED1_GREEN, LOW);
  digitalWrite(LED1_YELLOW, LOW);
  digitalWrite(LED1_RED, LOW);
  digitalWrite(LED2_GREEN, LOW);
  digitalWrite(LED2_YELLOW, LOW);
  digitalWrite(LED2_RED, LOW);
  delay(500);
  digitalWrite(LED1_GREEN, HIGH);
  digitalWrite(LED1_YELLOW, HIGH);
  digitalWrite(LED1_RED, HIGH);
  digitalWrite(LED2_GREEN, HIGH);
  digitalWrite(LED2_YELLOW, HIGH);
  digitalWrite(LED2_RED, HIGH);
  delay(500);
}

void mlxSetup()
{
  if (!mlx.begin())
  {
    Serial.println("Ooops, no MLX90614 detected ... Check your wiring!");
    while (1)
    {
      digitalWrite(LED1_RED, HIGH);
      digitalWrite(LED1_GREEN, LOW);
    }
  }
  else
  {
    digitalWrite(LED1_RED, LOW);
    digitalWrite(LED1_GREEN, HIGH);
  }
  delay(1000);
}

void adxlSetup()
{
  if (!accel.begin())
  {
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1)
    {
      digitalWrite(LED2_RED, HIGH);
      digitalWrite(LED2_GREEN, LOW);
    }
  }
  else
  {
    digitalWrite(LED2_RED, LOW);
    digitalWrite(LED2_GREEN, HIGH);
    accel.setRange(ADXL345_RANGE_16_G);
  }
  delay(1000);
}

void ledSetup()
{
  Serial.println("Setting up LED");
  pinMode(LED1_RED, OUTPUT);
  pinMode(LED1_YELLOW, OUTPUT);
  pinMode(LED1_GREEN, OUTPUT);

  pinMode(LED2_RED, OUTPUT);
  pinMode(LED2_YELLOW, OUTPUT);
  pinMode(LED2_GREEN, OUTPUT);

  digitalWrite(LED1_RED, HIGH);
  digitalWrite(LED2_RED, HIGH);
  digitalWrite(LED1_YELLOW, HIGH);
  digitalWrite(LED2_YELLOW, HIGH);
  digitalWrite(LED1_GREEN, HIGH);
  digitalWrite(LED2_GREEN, HIGH);
  delay(2000);
  Serial.println("LED Setup success!");

  digitalWrite(LED1_RED, LOW);
  digitalWrite(LED2_RED, LOW);
  digitalWrite(LED1_YELLOW, LOW);
  digitalWrite(LED2_YELLOW, LOW);
  digitalWrite(LED1_GREEN, LOW);
  digitalWrite(LED2_GREEN, LOW);
}

void wifiConnect()
{
  Serial.print("Connecting to Wifi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }
  Serial.println("\nWifi Connected!");
}

String getPlantResponse(String method)
{
  http.begin(baseUrl + "/motor_sensor/" + plant + "/" + method);
  http.GET();
  String response = http.getString();

  return response;
}

int postTemperature(int id, float T, float A)
{
  http.begin(baseUrl + "/motor_sensor/temperature");
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> buff;
  String jsonParams;

  buff["sensor_id"] = id;
  buff["temperature"] = T;
  buff["ambient"] = A;
  serializeJson(buff, jsonParams);

  int code = http.POST(jsonParams);

  return code;
}

int postVibration(int id, float x, float y, float z)
{
  http.begin(baseUrl + "/motor_sensor/vibration");
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> buff;
  String jsonParams;

  buff["sensor_id"] = id;
  buff["x_axis"] = x;
  buff["y_axis"] = y;
  buff["z_axis"] = z;
  serializeJson(buff, jsonParams);

  int code = http.POST(jsonParams);

  return code;
}

int postCurrent(int id, float volt, float ampere, float power)
{
  http.begin(baseUrl + "/motor_sensor/current");
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> buff;
  String jsonParams;

  buff["sensor_id"] = id;
  buff["volt"] = volt;
  buff["ampere"] = ampere;
  buff["power"] = power;
  serializeJson(buff, jsonParams);

  int code = http.POST(jsonParams);

  return code;
}

void mlxReader(int temp_id, float temp_warning, float temp_danger)
{
  temperature = mlx.readObjectTempC();
  ambient = mlx.readAmbientTempC();

  if (temp_id != 0 || temp_warning != 0 || temp_danger != 0)
  {
    if (sensor1_id != temp_id || sensor1_warning != temp_warning || sensor1_danger != temp_danger)
    {
      sensor1_id = temp_id;
      sensor1_warning = temp_warning;
      sensor1_danger = temp_danger;
    }

    Serial.println("--------- MLX Reader ---------");

    Serial.print("Sensor ID : ");
    Serial.print(sensor1_id);
    Serial.print("  Warning : ");
    Serial.print(sensor1_warning);
    Serial.print("  Danger : ");
    Serial.println(sensor1_danger);

    Serial.print("Temperature : ");
    Serial.print(temperature);
    Serial.print("  Ambient : ");
    Serial.println(ambient);

    response = postTemperature(sensor1_id, temperature, ambient);
    Serial.print("Code : ");
    Serial.println(response);

    if (response > 0)
    {
      Serial.print("Sensor Result : ");
      if (temperature < sensor1_warning)
      {
        Serial.println("Safe");
        digitalWrite(LED1_GREEN, HIGH);
        digitalWrite(LED1_YELLOW, LOW);
        digitalWrite(LED1_RED, LOW);
      }
      else if (temperature >= sensor1_danger)
      {
        Serial.println("DANGER!!");
        digitalWrite(LED1_GREEN, LOW);
        digitalWrite(LED1_YELLOW, LOW);
        digitalWrite(LED1_RED, HIGH);
      }
      else if (temperature >= sensor1_warning && temperature < sensor1_danger)
      {
        Serial.println("WARNING");
        digitalWrite(LED1_GREEN, LOW);
        digitalWrite(LED1_YELLOW, HIGH);
        digitalWrite(LED1_RED, LOW);
      }
      else
      {
        Serial.println("NO RESULT");
        digitalWrite(LED1_GREEN, LOW);
        digitalWrite(LED1_YELLOW, LOW);
        digitalWrite(LED1_RED, HIGH);
      }
    }
    else
    {
      ledBlinking();
    }
    Serial.println();
  }
}

void pzemReader(int temp_id, float temp_warning, float temp_danger)
{
  volt = pzem_r.voltage();
  ampere = pzem_r.current();
  power = pzem_r.power();

  if (temp_id != 0 || temp_warning != 0 || temp_danger != 0)
  {
    if (sensor2_id != temp_id || sensor2_warning != temp_warning || sensor2_danger != temp_danger)
    {
      sensor2_id = temp_id;
      sensor2_warning = temp_warning;
      sensor2_danger = temp_danger;
    }

    Serial.println("--------- PZEM Reader ---------");

    Serial.print("Sensor ID : ");
    Serial.print(sensor2_id);
    Serial.print("  Warning : ");
    Serial.print(sensor2_warning);
    Serial.print("  Danger : ");
    Serial.println(sensor2_danger);

    Serial.print("Volt : ");
    Serial.print(volt);
    Serial.print("  Ampere : ");
    Serial.print(ampere);
    Serial.print("  Power : ");
    Serial.println(power);

    response = postCurrent(sensor2_id, volt, ampere, power);
    Serial.print("Code : ");
    Serial.println(response);

    if (response > 0)
    {
      Serial.print("Sensor Result : ");
      if (ampere < sensor2_warning)
      {
        Serial.println("Safe");
        digitalWrite(LED2_GREEN, HIGH);
        digitalWrite(LED2_YELLOW, LOW);
        digitalWrite(LED2_RED, LOW);
      }
      else if (ampere >= sensor2_danger)
      {
        Serial.println("DANGER!!");
        digitalWrite(LED2_GREEN, LOW);
        digitalWrite(LED2_YELLOW, LOW);
        digitalWrite(LED2_RED, HIGH);
      }
      else if (ampere >= sensor2_warning && ampere < sensor2_danger)
      {
        Serial.println("WARNING");
        digitalWrite(LED2_GREEN, LOW);
        digitalWrite(LED2_YELLOW, HIGH);
        digitalWrite(LED2_RED, LOW);
      }
      else
      {
        Serial.println("NO RESULT");
        digitalWrite(LED2_GREEN, LOW);
        digitalWrite(LED2_YELLOW, LOW);
        digitalWrite(LED2_RED, HIGH);
      }
    }
    else
    {
      ledBlinking();
    }
    Serial.println();
  }
}

void adxlReader(int temp_id)
{
  sensors_event_t event;
  accel.getEvent(&event);

  x = event.acceleration.x;
  y = event.acceleration.y;
  z = event.acceleration.z;

  if (temp_id != 0)
  {
    if (sensor3_id != temp_id)
    {
      sensor3_id = temp_id;
    }

    Serial.println("--------- ADXL Reader ---------");

    Serial.print("Sensor ID : ");
    Serial.println(sensor3_id);

    Serial.print("X : ");
    Serial.print(x);
    Serial.print("  Y : ");
    Serial.print(y);
    Serial.print("  Z : ");
    Serial.println(z);

    response = postVibration(sensor3_id, x, y, z);
    Serial.print("Code : ");
    Serial.println(response);
    Serial.println();
  }
}

void setup()
{
  Serial.begin(9600);
  wifiConnect();
  ledSetup();
  mlxSetup();
  adxlSetup();
  ledBlinking();

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop()
{
  int temp_id = 0;
  float temp_warning = 0, temp_danger = 0;
  StaticJsonDocument<1024> temperatureData;
  StaticJsonDocument<1024> currentData;
  StaticJsonDocument<1024> vibrationData;

  if (getPlantResponse("temperature") && getPlantResponse("current") && getPlantResponse("vibration"))
  {
    deserializeJson(temperatureData, getPlantResponse("temperature"));
    JsonObject temperatureObj = temperatureData.as<JsonObject>();

    temp_id = temperatureObj[String("data")][0][String("id")];
    temp_warning = temperatureObj[String("data")][0][String("set_point")][String("warning")];
    temp_danger = temperatureObj[String("data")][0][String("set_point")][String("danger")];

    mlxReader(temp_id, temp_warning, temp_danger);

    deserializeJson(currentData, getPlantResponse("current"));
    JsonObject currentObj = currentData.as<JsonObject>();

    temp_id = currentObj[String("data")][0][String("id")];
    temp_warning = currentObj[String("data")][0][String("set_point")][String("warning")];
    temp_danger = currentObj[String("data")][0][String("set_point")][String("danger")];

    pzemReader(temp_id, temp_warning, temp_danger);

    deserializeJson(vibrationData, getPlantResponse("vibration"));
    JsonObject vibrationObj = vibrationData.as<JsonObject>();

    temp_id = vibrationObj[String("data")][0][String("id")];

    adxlReader(temp_id);
    delay(delayLoop);
  }
}