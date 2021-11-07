#include "PubSubClient.h" // Connect and publish to the MQTT broker
#include <ESP8266WiFi.h>
// WiFi
const char* ssid = "iPhone";                 // Your personal network SSID
const char* wifi_password = "11221122"; // Your personal network password

// MQTT
const char* mqtt_server = "172.20.10.2";  // IP of the MQTT broker
const char* USDis_topic = "home/tank/";
const char* mqtt_username = "drop2022"; // MQTT username
const char* mqtt_password = "drop2022#"; // MQTT password
const char* clientID = "client_tank"; // MQTT client ID

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




void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(trigPin1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin1, INPUT); // Sets the echoPin as an Input
  pinMode(trigPin2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin2, INPUT); // Sets the echoPin as an Input
  pinMode(FloatSensorH, INPUT_PULLUP); //Arduino Internal Resistor 10K
  pinMode(FloatSensorQ, INPUT_PULLUP); //Arduino Internal Resistor 10K
  
}

void loop() {

  connect_MQTT();
  // Clears the trigPin
    float c = dht.readTemperature();// Read temperature as Celsius (the default)
    float h = dht.readHumidity();// Reading humidity 
//    String Tem="Temperature:"+String((float)c)+",";
    String Tem=String((float)c)+",";
//    String Hum="Humedity:"+String((float)h)+",";
    String Hum=String((float)h)+",";
//    Serial.print("Temperature C:");
//    Serial.println(c);
//    Serial.print("Humidity:");
//    Serial.println(h);
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
//  String USDis1="Distance1:"+String((float)distanceCm1)+",";
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
//  String USDis2="Distance2:"+String((float)distanceCm2)+",";
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
   else 
   Serial.println( "WATER LEVEL IS LESS THAN QUARTER"); 
    levelInt=1;
  } 
String  sensorInfo=Tem+Hum+USDis1+USDis2+levelInt;

 Serial.println(sensorInfo);
  
  // PUBLISH to the MQTT Broker (topic = Distance, defined at the beginning)
  if (client.publish(USDis_topic,String(sensorInfo).c_str()) ) {
    Serial.println("info sent!");
  }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Distance failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    //client.publish(USDis_topic, String(USDis).c_str());
    client.publish(USDis_topic,String(sensorInfo).c_str()) ;
  }
 client.disconnect();  // disconnect from the MQTT broker
  delay(5000);  

//.................

}
