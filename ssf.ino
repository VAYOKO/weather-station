#include <DS1302.h>
#include <SPI.h>
#include <SD.h>
File myFile; // สร้างออฟเจก File สำหรับจัดการข้อมูล
const int chipSelect = 10;
// Init the DS1302#include "DHT.h"#include "DHT.h"
#include "DHT.h"
#define DHTPIN 2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

DS1302 rtc(8, 7, 6);
const int led = 9;
// Init a Time-data structure
Time tim; 
int ledState = LOW;
unsigned long pre1 = 0;
unsigned long pre2 = 0;
unsigned long pre3 = 0;
int measurePin = 0; //Connect dust sensor to Arduino A0 pin
int ledPower = 3;   //Connect 3 led driver pins of dust sensor to Arduino D2

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
void setup()
{
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
  pinMode(ledPower,OUTPUT);
  // Setup Serial connection
  Serial.begin(9600); Serial.println(F("DHTxx test!"));

  dht.begin();
  pinMode(led, OUTPUT);
  // The following lines can be commented out to use the values already stored in the DS1302
  // rtc.setDOW(SATURDAY); // Set Day-of-Week to FRIDAY
  //rtc.setTime(10, 50, 30); // Set the time to 12:00:00 (24hr format)
  // rtc.setDate(10,7, 2021); // Set the date to August 6th, 2010while (!Serial) {
  // รอจนกระทั่งเชื่อมต่อกับ Serial port แล้ว สำหรับ Arduino Leonardo เท่านั้น
  pinMode(LED_BUILTIN, OUTPUT);

  while (!Serial) {
    ; // รอจนกระทั่งเชื่อมต่อกับ Serial port แล้ว สำหรับ Arduino Leonardo เท่านั้น
  }
  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

}

void loop()
{
   
  unsigned long cur2 = millis();

  
  if (cur2 - pre2 >= 1000) {
pre2= cur2;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
      
    } else {
      ledState = LOW;
     
    }

    // set the LED with the ledState of the variable:
    digitalWrite(led, ledState);
  }
  // Get data from the DS1302

   
  unsigned long cur1 = millis();
  if (cur1 - pre1 >= 1000) {
pre1= cur1;
  


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float tem = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(tem, h, false);
 digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(measurePin); // read the dust value

  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);

  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0 / 1024.0);

  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = 170 * calcVoltage - 0.1;

  
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(tem) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(tem);
  Serial.print(F(" C "));
  Serial.print(f);
  Serial.print(F(" F  Heat index: "));
  Serial.print(hic);
  Serial.print(F(" C "));
  Serial.print(hif);
  Serial.println(F(" F"));


  tim = rtc.getTime();

  
  // Send date over serial connection
  Serial.print(tim.date, DEC);
  Serial.print(" / ");
  Serial.print(rtc.getMonthStr());
  Serial.print(" / ");
  Serial.print(tim.year, DEC);
  Serial.print("  ");
  
  // Send Day-of-Week and time
  Serial.print(tim.hour, DEC);
  Serial.print(" : ");
  Serial.print(tim.min, DEC);
  Serial.print(" : ");
  Serial.print(tim.sec, DEC);
 Serial.print("  ");
  Serial.println(dustDensity); // unit: ug/m3
 
  // Send a divider for readability
   myFile = SD.open("test.txt", FILE_WRITE); // เปิดไฟล์ที่ชื่อ test.txt เพื่อเขียนข้อมูล โหมด FILE_WRITE
 
  if (myFile) {

 

    
  myFile.print(tim.date, DEC);
  myFile.print(" / ");
  myFile.print(rtc.getMonthStr());
  myFile.print(" / ");
  myFile.print(tim.year, DEC);
  myFile.print("  ");
  
  // Send Day-of-Week and time
  myFile.print(tim.hour, DEC);
  myFile.print(" : ");
  myFile.print(tim.min, DEC);
  myFile.print(" : ");
  myFile.print(tim.sec, DEC);
  myFile.print("   ");
  myFile.print(hic);
  myFile.print("   c");
  myFile.print("   ");
  myFile.print(tem);
  myFile.print("   humi");
  
  myFile.print("   ");
  myFile.print(dustDensity); // unit: ug/m3
  
  myFile.println("   ug/m3");
  
  myFile.close(); // ปิดไฟล์
    } else {
// ถ้าเปิดไฟลืไม่สำเร็จ ให้แสดง error
Serial.println("error opening test.txt");
}
  // Wait one second before repeating :)
}
}
