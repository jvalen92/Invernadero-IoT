import RPi.GPIO as GPIO
import time
import paho.mqtt.client as mqtt
import json

GPIO.setmode(GPIO.BCM)
GPIO.setup(18,GPIO.OUT)
GPIO.setwarnings(False)

iot_hub = "demo.thingsboard.io" 
port = 1883
username = "yh9LcFSL5bAeqfEoWWkh" #cambiar
password = ""
topic = "v1/devices/me/telemetry" #cambiar

client = mqtt.Client()
client.username_pw_set(username,password)
client.connect(iot_hub,port)
print("Conection success")

data = dict()
while True:
        data["Prueba"]="Activated"
        data_out = json.dumps(data)
	client.publish(topic,data_out,0)
	time.sleep(1)
	GPIO.output(18,GPIO.HIGH)
	print("Led is ON")
	data["GPIO-Status"] = "ON"
        data_out = json.dumps(data)
	client.publish(topic,data_out,0)
	time.sleep(3)

	GPIO.output(18,GPIO.LOW)
	print("Led is OFF")
	data["GPIO-Status"] = "OFF"
	data_out = json.dumps(data)
	client.publish(topic,data_out,0)
	time.sleep(3)
	
