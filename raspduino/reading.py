import serial

arduino = serial.Serial('/dev/ttyACM0', 9600)

print("starting!")

while True:
    comando = arduino.read();
    print(comando)    
arduino.close()
