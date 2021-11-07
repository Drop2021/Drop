from datetime import datetime
import paho.mqtt.client as mqtt
import parser
import time, random
MQTT_ADDRESS  ='172.20.10.2'
MQTT_USER ='drop2022'
MQTT_PASSWORD='drop2022#'
MQTT_TOPIC ='home/tank/+'
value=None

# client, user and device details
serverUrl   = "mqtt.cumulocity.com"
clientId    = "test"
device_name = "d1"
tenant      = "iotccis21"
username    = "439200704@student.ksu.edu.sa"
password    = "Deba5590#"
client = mqtt.Client(clientId)
client.username_pw_set(tenant + "/" + username, password)

receivedMessages = []


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' ' + str(msg.payload))
    Dis1,Dis2,Temp,Hum,Level=str(msg.payload).split(",")
    publish("s/us", "200,c8y_Distance1,D1," +Dis1+",Cm")
    publish("s/us", "200,c8y_Distance2,D2," +Dis2+",Cm")
    publish("s/us", "200,c8y_Temperature,T," +Temp+",C")
    publish("s/us", "200,c8y_Humedity,H," +Hum+",")
    publish("s/us", "200,c8y_level,L," +Level+",")






# publish a message
def publish(topic, message, waitForAck = False):
    mid = client.publish(topic, message, 2)[1]
    if (waitForAck):
        while mid not in receivedMessages:
            time.sleep(0.25)

def on_publish(client, userdata, mid):
    receivedMessages.append(mid)



def main():
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    client.on_message = on_message
    client.on_publish = on_publish



  #

    client.connect(serverUrl)
    client.loop_start()
    publish("s/us", "100," + device_name + ",c8y_MQTTDevice", True)
    publish("s/us", "110,D2022,Drop,Rev0.1")
    publish("s/us", "114,c8y_Restart")
    print("Device registered successfully!")
    client.subscribe("s/ds")
    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_forever()






if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()
