import os
import time
import sys
import Adafruit_DHT as dht
import paho.mqtt.client as mqtt
import json

THINGSBOARD_HOST = 'demo.thingsboard.io'
ACCESS_TOKEN = '4pQOKfbhs43Ndzi9wAFJ'

# Data capture and upload interval in seconds. Less interval will eventually hang the DHT22.
INTERVAL=2

sensor_data = {'temperature': 0, 'humidity': 0, 'sensor_mq': 0, 'humedad_suelo':0, 'intensidad luminica' :0}

next_reading = time.time()

client = mqtt.Client()

# Set access token
client.username_pw_set(ACCESS_TOKEN)

# Connect to ThingsBoard using default MQTT port and 60 seconds keepalive interval
client.connect(THINGSBOARD_HOST, 1883, 60)

client.loop_start()

try:
        while True:
                humidity = 50
                temperature = 26
                #humidity,temperature = dht.read_retry( 11, 4)
                #humidity = round(humidity, 2)
                #temperature = round(temperature, 2)
                sensor_mq = 456
                humedad_suelo = 45 
                luz  =50 
                    
            #print(u"Temperature: {:g}\u00b0C, Humidity: {:g}%".format(temperature, humidity))
                #print temperature
                #print humidity
                sensor_data['temperature'] = temperature
                sensor_data['humidity'] = humidity
                sensor_data['sensor_mq']= sensor_mq
                sensor_data['humedad_suelo'] = humedad_suelo
                sensor_data['intensidad luminica'] = luz

                print sensor_data
            # Sending humidity and temperature data to ThingsBoard
                client.publish('v1/devices/me/telemetry', json.dumps(sensor_data), 1)

        next_reading += INTERVAL
        sleep_time = next_reading-time.time()
        if sleep_time > 0:
                time.sleep(sleep_time)
                    
except KeyboardInterrupt:
        pass
client.loop_stop()
client.disconnect()
