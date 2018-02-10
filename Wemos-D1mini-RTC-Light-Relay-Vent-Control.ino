#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <SimpleTimer.h>
//
#include "RTClib.h"       // RealTimeClock Library for DS1307 and DS3231
//
#define lightA 22             //Relay 1/a  -  Relays[0]
#define lightB 23             //Relay 2/b  -  Relays[1]
#define pumpA 24              //Relay 3/c  -  Relays[2]
#define pumpB 25              //Relay 4/d  -  Relays[3]
#define ROpump 26             //Relay 5/e  -  Relays[4]
#define scrubberFan 27        //Relay 6/f  -  Relays[5]
#define heatVentA 28          //Relay 7/g  -  Relays[6]
#define heatVentB 29          //Relay 8/h  -  Relays[7]
int Relays[8] {lightA, lightB, pumpA, pumpB, ROpump, scrubberFan, heatVentA, heatVentB};
//Digital Pin Relay Assignments - 30 Series Pins - 120VAC~ switching
//#define 30                  //2nd bank of 8 relays
//#define 31
//#define 32
//#define 33
//#define 34
//#define 35
//#define 36
//#define 37
//int Relays2[8] {30, 31, 32, 33, 34, 35, 36, 37};
#define TURN_ON LOW // TURN_ON and TURN_OFF are defined to account for Active High relays
#define TURN_OFF HIGH // Used to switch relay states for on/off of AC devices   

#include <Phant.h>
#include <DHT.h>
#define DHTTYPE DHT22
#define DHTPIN  D4
DHT dht(DHTPIN, DHTTYPE, D4);
//  DHT Reference Values - CHANGE THESE TO MANAGE YOUR ROOM'S CLIMATE - //
byte hiMaxTemp = 90;   // temp that triggers heat removal fan on
byte lowMaxTemp = 75;  // temp that triggers heat removal fan off
byte hiMinTemp = 70;   // temp that triggers heater on
byte lowMinTemp = 60;  // temp that triggers heater off
byte hiHum = 75;       // High humidity value that triggers dehumidifier on
byte lowHum = 70;      // Low humidity value that triggers dehumidifier off

RTC_DS1307 RTC;
float UTCOffset = -5.0;    // Your timezone relative to UTC (http://en.wikipedia.org/wiki/UTC_offset)

#define BLYNK_PRINT Serial
char auth[] = "Your Auth Token Here";
WidgetLCD lcdA(V2);  //Set LCD widget to Advanced Mode - Widget to display project clock


float humidity, temp;  // Values read from sensor
unsigned long previousMillis = 0;
const long interval = 15000;          // interval at which to read sensor / Update values

// Auth Token App via Mail of printscreen
char auth[] = "1847ec45d79f4d4495d67d59101d3f77";

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, "G.E.C.K", "ewRiOrgRSqCE");
  dht.begin();}
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__));
void gettemperature() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor
    previousMillis = currentMillis;

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp = dht.readTemperature(false);     // Read temperature as Fahrenheit


    // Serial Monitor
    Serial.print("Humidity ");
    Serial.print(humidity);
    Serial.println(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" *C ");

    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, humidity);

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}

bool isFirstConnect = true;
BLYNK_CONNECTED() {
  if (isFirstConnect)
  {
    Blynk.syncAll();
    isFirstConnect = false;
  }
}

void loop()
{
  Blynk.run();
  gettemperature();
}
void climateRoutine()
{
  byte h1 = dhtA.readHumidity();          // f1 and h1 are fahrenheit and humidity readings
  byte f1 = dhtA.readTemperature(true);   // from DHT/A
  byte h2 = dhtB.readHumidity();          // f2 and h2 are fahrenheit and humidity readings 
  byte f2 = dhtB.readTemperature(true);   // from DHT/B
Blynk.virtualWrite(V0, f1);      //  Set Virtual Pin 0 frequency to PUSH in Blynk app
  Blynk.virtualWrite(V1, h1);      //  Set Virtual Pin 1 frequency to PUSH in Blynk app
  Blynk.virtualWrite(V3, f2);      //  Set Virtual Pin 2 frequency to PUSH in Blynk app
  Blynk.virtualWrite(V4, h2);      //  Set Virtual Pin 3 frequency to PUSH in Blynk app
  Serial.println(f1);
  Serial.println(h1);
  Serial.println(f2);
  Serial.println(h2);
  
  //   Tests temp & humidity to see if preset values are exceed.
  //   If exceeded, a relay is trigger high to power an exhaust fan or heater.
  
  //**************************** REMOVING HEAT *************************************//
  if (f1 >= hiMaxTemp)  //if "f1" is greater than or equal to hiMaxTemp,
  {
    digitalWrite(heatVentA, TURN_ON);  // TURN_ON heatVentA (fan).
    Serial.print("\t");
    Serial.println(F("Exhausting the heat from A")); //  Text printed to serial monitor
    Serial.print("\t");
    Serial.println();
  }
  else if (f1 <= lowMaxTemp)  //  or else if "f1" is less than or equal to lowMaxTemp
  {
    digitalWrite(heatVentA, TURN_OFF); //  TURN_OFF relay E.
  }
}
void timeRoutine()
{
  DateTime now = RTC.now();  // reads time at beginning of loop
byte twelveHour = now.hour() - 12; // Variable used to display 13+ hours in 12 hour format
  byte zeroHour = 12;                // Variable use to convert "0" zero hour to display it as 12:00+
  byte displayHour;
  byte MIN = now.minute();
  byte SEC = now.second();
  char* meridian;
if (now.hour() == 0)  // First we test if the hour reads "0"
  {
    displayHour = zeroHour;
    meridian = "AM";
  }
  else if (now.hour() >= 13)  // if no, Second we test if the hour reads "13 or more"
  {
    displayHour = twelveHour;
    meridian = "PM";
  }
  else
  {
    displayHour = now.hour();
    meridian = "AM";
  }
char timeStamp[11];
  char dateStamp[11];
  sprintf(timeStamp, "%02d:%02d:%02d-%02s", displayHour, MIN, SEC, meridian);
  sprintf(dateStamp, "%02d/%02d/%04d", now.month(), now.day(), now.year());
  String ts;
  String ds;
  ts = timeStamp;
  ds = dateStamp;
lcdA.clear();            //Advanced Mode
  lcdA.print(3, 0, ts);    //Advanced Mode
  lcdA.print(3, 1, ds);    //Advanced Mode
  Serial.println(ts);
  Serial.print(ds);
//*************** 12/12 BloomA Light - 6AM-6PM**********************************//
  //***** Adjust hours and minutes in accordance with 24 hour time format. *******//
  //***** Create tests where true is ON time and false is OFF time. *************//
  boolean lightAstate = false;
  if (now.hour() >= 6 && now.hour() <= 17) lightAstate = true;
  if (lightAstate == true)
  {
    digitalWrite(lightA, TURN_ON);
    Serial.print("\t");
    Serial.print(F("Bloom A Light On"));  //  Text printed to serial monitor
    Serial.print("\t");
    Serial.println();
  }
  else
  {
    digitalWrite(lightA, TURN_OFF);
    Serial.print("\t");
    Serial.println(F("Bloom A Light OFf"));  //  Text printed to serial monitor
    Serial.print("\t");
    Serial.println();
  }
  //*************** 12/12 BloomB Light - 6PM-6AM**********************************//
  //***** Adjust hours and minutes in accordance with 24 hour time format. *******//
  //***** lightB is lit during the opposing 12 hours to lightA. *****************//
  boolean lightBstate = false;
  if (lightAstate == false) lightBstate = true;
  if (lightBstate == true)
  {
    digitalWrite(lightB, TURN_ON);
    Serial.print("\t");
    Serial.print(F("Bloom B Light On"));  //  Text printed to serial monitor
    Serial.print("\t");
    Serial.println();
  }
  else
  {
    digitalWrite(lightB, TURN_OFF);
    Serial.print("\t");
    Serial.println(F("Bloom B Light OFf"));  //  Text printed to serial monitor
    Serial.print("\t");
    Serial.println();
  }

