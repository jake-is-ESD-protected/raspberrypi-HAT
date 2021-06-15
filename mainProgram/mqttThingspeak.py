import thingspeak
import RPi.GPIO as GPIO
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


# pin numbers to check whick HAT is connected to the Raspberry Pi
thermoButtonPin = 23
audioButtonPin = 4
thermo = 1
audio = 2

# check if BCM (call with GPIO numbers) or call with board numbers
GPIO.setmode(GPIO.BCM)
GPIO.setup(thermoButtonPin, GPIO.IN)
GPIO.setup(audioButtonPin, GPIO.IN)


if GPIO.input(thermoButtonPin) == True:
    print("Thermo-HAT registered. Starting thermo-branch in python.")
    hatType = thermo
elif GPIO.input(audioButtonPin) == True:
    print("Audio-HAT registered. Starting audio-branch in python.")
    hatType = audio
else:
    print("No HAT detected!")


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

 
# only send data to thingspeak if the temperature HAT is connected
if __name__ == "__main__":
    if hatType == thermo:
        while True:
            measure(channel, msg)
            # free thingspeak account has an api limit of 15sec
            time.sleep(20)
    else:
        print("Wrong or no HAT connected. Useless to open thingspeak client")