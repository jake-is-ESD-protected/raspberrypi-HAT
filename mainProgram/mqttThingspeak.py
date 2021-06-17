import thingspeak
import RPi.GPIO as GPIO
import time
import paho.mqtt.client as mqtt
 
channel_id_thermo = 1413297                # channel ID
write_key_thermo  = '7F9TGV6JCRCN2OHN'     # write-key
read_key_thermo   = 'EZ3CRJF9II408YNP'     # read-key
channel_id_audio = 1234567                 # not a real value - added when the audio-HAT runs 
write_key_audio = ''                       # not a real value - added when the audio-HAT runs 
read_key_audio = ''                        # not a real value - added when the audio-HAT runs 


broker = "localhost"                # broker, in this case the raspi itself
mqttHost = "mqtt.thingspeak.com"    # subscriber on thingsspeak which displays data
tPort = 1883                        # port for thingspeak
qos = 0                             # quality of service


# pin numbers to check whick HAT is connected to the Raspberry Pi
thermoButtonPin = 23
audioButtonPin = 4

# check if BCM (call with GPIO numbers) or call with board numbers
GPIO.setmode(GPIO.BCM)
GPIO.setup(thermoButtonPin, GPIO.IN)
GPIO.setup(audioButtonPin, GPIO.IN)



# check which HAT is connected to the Raspberry Pi
if GPIO.input(thermoButtonPin) == True:
    # variable to communicate with the thingspeak channel using the thingspeak channel IDs
    channel = thingspeak.Channel(id=channel_id_thermo, write_key=write_key_thermo, api_key=read_key_thermo)
    topic = "thermo"
    print("[Python] Thermo-HAT registered. Starting thermo-branch.")

elif GPIO.input(audioButtonPin) == True:
    channel = thingspeak.Channel(id=channel_id_audio, write_key=write_key_audio, api_key=read_key_audio)
    topic = "audio"
    print("[Python] Audio-HAT registered. Starting audio-branch.")

else:
    print("No HAT detected!")



# function to get the temperature or audio data from mosquitto
def measure(channel, msg):
    try:
        # write
        response = channel.update({'field1': msg.payload})
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
    print(topic+" = "+str(msg.payload))



# variables for the mosquitto client   
client_thermo = mqtt.Client()
client_thermo.on_connect = on_connect
client_thermo.on_message = on_message
client_thermo.connect(broker, tPort, 60)
client_thermo.loop_forever()



if __name__ == "__main__":
    while True:
        measure(channel, msg)
        # free thingspeak account has an api limit of 15sec
        time.sleep(20)
