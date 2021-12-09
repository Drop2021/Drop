//refrence https://diyi0t.com/microcontroller-to-raspberry-pi-wifi-mqtt-communication/
#include "PubSubClient.h" // Connect and publish to the MQTT broker
#include "ESP8266WiFi.h"
// WiFi
const char* ssid = "Munirah iPhone";                 // Your personal network SSID
const char* wifi_password = "Mn1234554345"; // Your personal network password
 
// MQTT
const char* mqtt_server = "172.20.10.4";  // IP of the MQTT broker
const char* USDis_topic = "home/tank/";
const char* mqtt_username = "drop2022"; // MQTT username
const char* mqtt_password = "drop2022#"; // MQTT password
const char* clientID = "tank"; // MQTT client ID
 
// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
//1883 is the listener port for the Broker
PubSubClient client(mqtt_server, 1883, wifiClient);
 
 
#include "DHT.h"
#define DHTPIN D2
#define DHTTYPE DHT11 //DHT11
DHT dht(DHTPIN, DHTTYPE);
////
#include <HCSR04.h>
#define trigPin1 D6
#define echoPin1 D5
#define trigPin2 D7
#define echoPin2 D8
//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
long duration1;
float distanceCm1;
long duration2;
float distanceCm2;
//end distance
//.......
//Robo India tutorial on Magnetic Float Sensor
//https://www.roboindia.com/tutorials   
boolean buttonStateH;
boolean buttonStateQ;//reads pushbutton status
#define FloatSensorH D3
#define FloatSensorQ D4
//....end levelsensors....

//.......quality sensor...........
#define TdsSensorPin A0
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;
 

//......water consumption sensor.....
 
#define SENSOR  D1
long currentMillis = 0;
long previousMillis = 0;
long count = 0;
int interval = 1000;
float calibrationFactor = 9;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long presentMillis = millis();
//.....end water consumption....
void connect_MQTT(){
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
//Connect to the WiFi
  WiFi.begin(ssid, wifi_password);
 
//Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
// Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
// Connect to MQTT Broker
// client.connect returns a boolean value to let us know if the connection was successful.
// If the connection is failing, make sure you are using the correct MQTT Username and Password (Setup Earlier in the Instructable)
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}//end connect_MQTT
 
//water consumption
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}//end method
 
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(trigPin1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin1, INPUT); // Sets the echoPin as an Input
  pinMode(trigPin2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin2, INPUT); // Sets the echoPin as an Input
  pinMode(FloatSensorH, INPUT_PULLUP); //Arduino Internal Resistor 10K
  pinMode(FloatSensorQ, INPUT_PULLUP); //Arduino Internal Resistor 10K
  pinMode(TdsSensorPin, INPUT);
  pinMode(SENSOR, INPUT_PULLUP);//water consumption
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}
 
void loop() {
 
  connect_MQTT();
  // Clears the trigPin
    float c = dht.readTemperature();// Read temperature as Celsius (the default)
    float h = dht.readHumidity();// Reading humidity

    String Tem=String((float)c)+",";

    String Hum=String((float)h)+",";

  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration1 = pulseIn(echoPin1, HIGH);
  // Calculate the distance
  distanceCm1 = duration1 * SOUND_VELOCITY/2;
  // Prints the distance on the Serial Monitor
  Serial.print(distanceCm1);
  Serial.println(" CM ULT-First");

String USDis1=String((float)distanceCm1)+",";
// 
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration2 = pulseIn(echoPin2, HIGH);
  // Calculate the distance
  distanceCm2 = duration2 * SOUND_VELOCITY/2;
  // Prints the distance on the Serial Monitor
  Serial.print(distanceCm2);
  Serial.println(" CM ULT-Second");

  String USDis2=String((float)distanceCm2)+",";
 
////.....
buttonStateH = digitalRead(FloatSensorH);
buttonStateQ = digitalRead(FloatSensorQ);
int levelInt=0;
  if (buttonStateH == 0)
 {
    Serial.println( "WATER LEVEL - MORE THAN HALF");
     levelInt=3;
  }
  else
  {
    if(buttonStateQ == 0)
   {
    Serial.println( "WATER LEVEL IS BETWEEN HALF AND QUARTER");
     levelInt=2;
   }
   else {
   Serial.println( "WATER LEVEL IS LESS THAN QUARTER");
    levelInt=1;
   }
  }
//quality
 static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)  //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
    Serial.print("voltage:");
    Serial.print(averageVoltage,2);
    Serial.print("V   ");
    Serial.print("TDS----Value:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");
  }
 String tdsvalue=String((float)tdsValue)+",";
//end quality
  
   presentMillis = millis();
    while(millis() - presentMillis != 30000)//3600000UL)
    {
      currentMillis = millis();
      if (currentMillis - previousMillis > interval) {
   
        pulse1Sec = pulseCount;
        pulseCount = 0;
        flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
        previousMillis = millis();
        flowMilliLitres = (flowRate / 60) * 1000;//the flow rate in ml/sec

        totalMilliLitres += flowMilliLitres;
      // Print the flow rate for this second in litres / minute
       Serial.print("Flow rate: ");
       Serial.print(int(flowRate));  // Print the integer part of the variable
       Serial.print("L/min");
       Serial.print("\t");       // Print tab space  
       Serial.print("\n");
   
     }//end if
  }//stop the while loop after 1 hour

  String HourlyCons=String(((float)totalMilliLitres)/1000)+"";
  String levelInt1=String((float)levelInt)+",";
  Serial.println(HourlyCons);
  String  sensorInfo=Tem+Hum+USDis1+USDis2+levelInt1+tdsvalue+HourlyCons;
     //send to RaspberryPi before reset the total
  // PUBLISH to the MQTT Broker (topic = Distance, defined at the beginning)
  if (client.publish(USDis_topic,String(sensorInfo).c_str()) ) {
    Serial.println("info sent!");
     totalMilliLitres=0;//reset the total
  }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    //client.publish(USDis_topic, String(USDis).c_str());
    client.publish(USDis_topic,String(sensorInfo).c_str()) ;
     totalMilliLitres=0;//reset the total
  }
client.disconnect();  // disconnect from the MQTT broker

 

 
}
