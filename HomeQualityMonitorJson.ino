// Home monitoring system. 
// Temperature, Humidity, Gas level and Dust level. Provides data on LCD screen.
// Written by Ilkhom Karimov

#include "DHT.h"
#include <LiquidCrystal.h>
#include <Process.h>

#define DHTpin 2       // DIgital Pin for Temperature and Humidity sensor
#define MQ2pin A0         // Analog Pin for MQ2 Gas sensor
#define dustMPin A5  // Analog Pin for Sharp GP2Y1010AU0F dust measure sensor
#define ledPower 4     // Digital Pin to power up Sharp GP2Y1010AU0F dust sensor

#define DHTtype DHT22   // DHT 22

DHT dht(DHTpin, DHTtype); //DHT 22 class initialization with pin and type

// Dust sensor timing
int samplingTime = 280; 
int deltaTime = 40;
int sleepTime = 9680;
// Measure Variables for dust sensor
float voMeasured = 0;
float calcVoltage = 0;
int dustDensity = 0;
// Curl counter and process
int c = 0;
Process p;

// initialize the LCD (LCD1602 HD44780 Controller) screen library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Set up DHT sensor
  dht.begin();
  // Bridge begin for curl
  Bridge.begin();
  
  // Setup dust sensor
  pinMode(ledPower,OUTPUT);
}

void loop() {
  // Wait a second between measurements.
  delay(1000);
  // Reading Gas Level on MQ2
  int Gaslevel = analogRead(MQ2pin);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old'
  float humidity = dht.readHumidity();
  
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();
  
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float farenheit = dht.readTemperature(true);
  
  // Reading the Dust Level from Sharp Dust sensor 
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(dustMPin); // read the dust value
  
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower, HIGH); // turn the LED off
  delayMicroseconds(sleepTime);

  // 0 - 5.0V mapped to 0 - 1023 integer values 
  calcVoltage = voMeasured * (5.0 / 1024.0); 
  
  // Dust density linear equation taken from http://www.howmuchsnow.com/arduino/airquality/
  dustDensity = (0.17 * calcVoltage - 0.1)*1000;
  
  
  // Check if any reads failed and exit early (to try again).
  if (humidity < 20 || isnan(temperature) || Gaslevel == 0 || dustDensity < 0)
  {
    Serial.println("Failed to read from one of the sensors!");
    //Print to LCD
    lcd.clear();
    lcd.print("Failed to read");
    lcd.setCursor(0, 1); // Set cursor to second line
    lcd.print("the sensor!");
    return;
  }
  // Print to serial interface sensor data
  Serial.print("Humidity:");
  Serial.print(humidity);
  Serial.print(" %  ");
  Serial.print("Temp1: ");
  Serial.print(farenheit);
  Serial.print(" *F ");
  Serial.print("Gas Level:");
  Serial.print(Gaslevel);
  Serial.println(";");
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);
  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);
  Serial.print(" - Dust Density [ug/m3]: ");
  Serial.println(dustDensity);
  
  // Clear screen every time for a new value
  lcd.clear();
  // Print Sensor results
  lcd.print("h="); lcd.print(humidity);lcd.print(" t="); lcd.print(farenheit); lcd.print("F");
  lcd.setCursor(0, 1);
  lcd.print("Gas="); lcd.print(Gaslevel); lcd.print(" Dust="); lcd.print(dustDensity);
  c=c+1;
  if (c == 4)
  {
    //p.begin("curl");  // Process that launch the "curl" command
    String jcurl  = "curl -H \"Content-Type: application/json\" -X POST -d '{\"title\":\"arduino\", \"temp\":\"";
    String jcurl2 = String(farenheit);
    String jcurl3 = "\", \"humidity\":\"";
    String jcurl4 = String(humidity);
    String jcurl5 = "\", \"gas\":\"";
    String jcurl6 = String(Gaslevel);
    String jcurl7 = "\"}' http://192.168.0.3:3000/temperatures";
    String totcurl = jcurl+jcurl2+jcurl3+jcurl4+jcurl5+jcurl6+jcurl7;
    Serial.println(totcurl);
    Serial.println("curl -H \"Content-Type: application/json\" -X POST -d '{\"title\":\"A2\", \"temp\":\"33\", \"humidity\":\"33\",\"gas\":\"11\"}' http://192.168.0.3:3000/temperatures");
    p.runShellCommand(jcurl+jcurl2+jcurl3+jcurl4+jcurl5+jcurl6+jcurl7);
    //p.runShellCommand("curl -H \"Content-Type: application/json\" -X POST -d '{\"name\":\"arduino\",\"value\":\"15\"}' http://localhost:1234/temp");
    //p.runShellCommand("curl -H \"Content-Type: application/json\" -X POST -d '{\"title\":\"A2\", \"temp\":\"33\", \"humidity\":\"33\",\"gas\":\"11\"}' http://192.168.0.3:3000/temperatures");
    //p.addParameter('"Content-Type: application/json" -X POST -d \'{"title":"newtemp", "temp":"22112","humidity":"99", "gas":"1"}' http://localhost:3000/temperatures\'); // Add the URL parameter to "curl"
    c=0;
  }
  }
