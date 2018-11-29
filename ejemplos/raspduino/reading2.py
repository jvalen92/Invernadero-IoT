import serial

#ruta del puerto serial que ocupa el arduino
arduino = serial.Serial('/dev/ttyACM0', 9600)

print("starting!")

#archivo de texto que guarda los datos leidos por la raspberry
f=open("data.txt",'w')
while True:

    comando = str( arduino.read())
    #debido a que el serial manda los datos caracter por caracter tendremos que filtrar el dato
    aux=comando[2]
    if aux not "\\":
        print(aux)
        f.writelines(aux)

f.close()    
arduino.close()
