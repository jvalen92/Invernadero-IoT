import serial

arduino = serial.Serial('/dev/ttyACM0', 9600)

print("starting!")

while True:
    comando = raw_input('Introduce un comando: ')
    arduino.write(comando)
    if comando == 'H':
        print('LED ENCENDIDO')
    elif comando == 'L':
        print('LED APAGADO')

arduino.close()
