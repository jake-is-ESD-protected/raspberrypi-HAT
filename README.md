# raspberrypi-temperatureHAT
A repo for the FHV-made temperature and audio-HATs by JT and SD on the RaspberryPi.

# Dual-HAT model
The program is designed to auto-detect which HAT is attached. This method can be expanded on with multiple HATs, all derived from a basic HAT-class which holds standard functions and error-catching.

# HAT-states
Each HAT has similar or unique states which can be cycled through by hardware control. These states act outward and show their activity on the console. The states (right now) are:
  -Standby (only the poll-thread is active and awaits new orders)
  -Blinky (shows a short LED demo in all possible colors)
  -Print (prints sensor values to console)
  -bot (opens a chat with a telegram bot. Name is displayed in console. Activated by /start. Receive values by asking /thermo or /sound, depending on the HAT)
  -MQTT (publishes data to a python bridge server running in the background. This server sends the data to ThingSpeak to get displayed)
 
 #Misc
 Check the "old" folder for basic functionalities like an audio-sampler with alsa, communication with ADT7310 or the ublox SARA-module. 
![image](https://user-images.githubusercontent.com/78532991/122428439-2f429680-cf92-11eb-9cb8-06ff6c601d96.png)
