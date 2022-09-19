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
int delayLoop = 8000;

// Variable Declaration
float temperature = 0, ambient = 0;
float volt = 0, ampere = 0, power = 0;
float x = 0, y = 0, z = 0;
int response;
float sensor2_warning = 0, sensor2_danger = 0;
float sensor3_warning = 0, sensor3_danger = 0;

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
const char *ssid = "KP-Unilever";
const char *pass = "smartfren";

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
  digitalWrite(LED1_GREEN, LOW);
  digitalWrite(LED1_YELLOW, LOW);
  digitalWrite(LED1_RED, LOW);
  digitalWrite(LED2_GREEN, LOW);
  digitalWrite(LED2_YELLOW, LOW);
  digitalWrite(LED2_RED, LOW);
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

String getPlantResponse()
{
  http.begin(baseUrl + "/" + plant + "/setpoint");
  http.GET();
  String response = http.getString();

  return response;
}

int postData(float T, float A, float x, float y, float z, float volt, float ampere, float power)
{
  http.begin(baseUrl + "/" + plant);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> buff;
  String jsonParams;

  buff["temperature"] = T;
  buff["ambient"] = A;
  buff["x_axis"] = x;
  buff["y_axis"] = y;
  buff["z_axis"] = z;
  buff["volt"] = volt;
  buff["ampere"] = ampere;
  buff["power"] = power;
  serializeJson(buff, jsonParams);

  int code = http.POST(jsonParams);

  return code;
}

void mlxReader()
{
  temperature = mlx.readObjectTempC();
  ambient = mlx.readAmbientTempC();

  Serial.println("--------- MLX Reader ---------");
  Serial.print("Warning : ");
  Serial.print(sensor2_warning);
  Serial.print("  Danger : ");
  Serial.println(sensor2_danger);

  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.print("  Ambient : ");
  Serial.println(ambient);
}

void pzemReader()
{
  volt = pzem_r.voltage();
  ampere = pzem_r.current();
  power = pzem_r.power();

  Serial.println("--------- PZEM Reader ---------");
  Serial.print("Warning : ");
  Serial.print(sensor3_warning);
  Serial.print("  Danger : ");
  Serial.println(sensor3_danger);

  Serial.print("Volt : ");
  Serial.print(volt);
  Serial.print("  Ampere : ");
  Serial.print(ampere);
  Serial.print("  Power : ");
  Serial.println(power);

  if (isnan(volt) || isnan(ampere))
  {
    digitalWrite(LED2_RED, HIGH);
    digitalWrite(LED2_YELLOW, LOW);
    digitalWrite(LED2_GREEN, LOW);
  }
}

void adxlReader()
{
  sensors_event_t event;
  accel.getEvent(&event);

  x = event.acceleration.x;
  y = event.acceleration.y;
  z = event.acceleration.z;

  Serial.println("--------- ADXL Reader ---------");
  Serial.print("X : ");
  Serial.print(x);
  Serial.print("  Y : ");
  Serial.print(y);
  Serial.print("  Z : ");
  Serial.println(z);
}

void mlxChecker()
{
  Serial.print("MLX Result : ");
  if (temperature < sensor2_warning)
  {
    Serial.println("Safe");
    digitalWrite(LED1_GREEN, HIGH);
    digitalWrite(LED1_YELLOW, LOW);
    digitalWrite(LED1_RED, LOW);
  }
  else if (temperature >= sensor2_danger)
  {
    Serial.println("DANGER!!");
    digitalWrite(LED1_GREEN, LOW);
    digitalWrite(LED1_YELLOW, LOW);
    digitalWrite(LED1_RED, HIGH);
  }
  else if (temperature >= sensor2_warning && temperature < sensor2_danger)
  {
    Serial.println("WARNING");
    digitalWrite(LED1_GREEN, LOW);
    digitalWrite(LED1_YELLOW, HIGH);
    digitalWrite(LED1_RED, LOW);
  }
}

void pzemChecker()
{
  Serial.print("PZEM Result : ");
  if (ampere < sensor3_warning)
  {
    Serial.println("Safe");
    digitalWrite(LED2_GREEN, HIGH);
    digitalWrite(LED2_YELLOW, LOW);
    digitalWrite(LED2_RED, LOW);
  }
  else if (ampere >= sensor3_danger)
  {
    Serial.println("DANGER!!");
    digitalWrite(LED2_GREEN, LOW);
    digitalWrite(LED2_YELLOW, LOW);
    digitalWrite(LED2_RED, HIGH);
  }
  else if (ampere >= sensor3_warning && ampere < sensor3_danger)
  {
    Serial.println("WARNING");
    digitalWrite(LED2_GREEN, LOW);
    digitalWrite(LED2_YELLOW, HIGH);
    digitalWrite(LED2_RED, LOW);
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

  if (getPlantResponse())
  {
    deserializeJson(vibrationData, getPlantResponse());
    JsonObject vibrationObj = vibrationData.as<JsonObject>();

    deserializeJson(temperatureData, getPlantResponse());
    JsonObject temperatureObj = temperatureData.as<JsonObject>();

    sensor2_warning = temperatureObj[String("data")][1][String("set_point")][String("warning")];
    sensor2_danger = temperatureObj[String("data")][1][String("set_point")][String("danger")];

    deserializeJson(currentData, getPlantResponse());
    JsonObject currentObj = currentData.as<JsonObject>();

    sensor3_warning = currentObj[String("data")][2][String("set_point")][String("warning")];
    sensor3_danger = currentObj[String("data")][2][String("set_point")][String("danger")];
  }
  adxlReader();
  mlxReader();
  pzemReader();

  response = postData(temperature, ambient, x, y, z, volt, ampere, power);
  Serial.print("Code : ");
  Serial.println(response);
  Serial.println();

  if (response == 201)
  {
    mlxChecker();
    pzemChecker();
  }
  else
  {
    Serial.println("NO RESULT");
    ledBlinking();
  }

  delay(delayLoop);
}