import thingspeak
import time
import paho.mqtt.client as mqtt
 
channel_id = 1413297                # channel ID
write_key  = '7F9TGV6JCRCN2OHN'     # write-key
read_key   = 'EZ3CRJF9II408YNP'     # read-key

broker = "localhost"                # broker, in this case the raspi itself
mqttHost = "mqtt.thingspeak.com"    # subscriber on thingsspeak which displays data
topic = "thermo"                    # topic to which the client subscribe and gets data
tPort = 1883                        # port for thingspeak
qos = 0

# variable to communicate with the thingspeak channel using the thingspeak channel IDs
channel = thingspeak.Channel(id=channel_id, write_key=write_key, api_key=read_key)

# function to get the temperature from mosquitto
def measure(channel, msg):
    try:
        temperature = msg.payload
        # write
        response = channel.update({'field1': temperature})
        
        # read
        read = channel.get({})
        print("Read:", read)
 
    except:
        print("connection failed")


# The callback for when the client receives a CONNACK response from mosquitto.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(topic, qos)

# The callback for when a PUBLISH message is received from mosquitto.
def on_message(client, userdata, msg):
    measure(channel, msg)
    print(msg.topic+" "+str(msg.payload))


# variables for the mosquitto client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(broker, tPort, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()

 
if __name__ == "__main__":
    while True:
        measure(channel, msg)
        # free thingspeak account has an api limit of 15sec
        time.sleep(15)