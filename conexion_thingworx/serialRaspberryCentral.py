# getArduino():
# Recibe los datos de los sensores por serial en 1 string separado por comas
# pone estos datos en un diccionario y retorna el diccionario con los valores
# y con su respectivo nombre.
#
#
# SendArduino():
# Recibe un diccionario y saca los valores y los junta en un string delimitandolos
# por ',' Luego envia este string al arduino.
#
# Por corregir:
# Todavia hay un error en getArduino() y sendArduino() porque a veces el primer valor
# que recibe o envia es vacio o no tiene todos los valores, lo que causa que a veces
# no haga lo esperado.


import serial
import time
import os

# Definir arduino segun sistema operativo
sos = os.name
arduino = 'indefinido'

if sos == 'posix':
    # linux
    arduino = '/dev/ttyACM0'
elif sos == 'nt':
    # windowns o mac
    arduino = 'COM6'

cantidadDatosSensores = 5
# Se establece la conexion
ser = serial.Serial(arduino, 9600, timeout=1)
time.sleep(1)


def getArduino():

    data = ser.readline().decode('ascii')
    time.sleep(1)
    data = data.strip()
    data = data.split(',')
    print "recibiendo de arduino: "
    # Orden de sensores
    # luzBlanca, ph, luzInfrarroja, humedad_suelo, luz ultravioleta, temperatura_suelo
    print data
    if len(data) == cantidadDatosSensores:
        print "verdadero"
        datosDic = {"s_co2_sise": data[0], "s_temperatura_tanque_sise": data[1], "s_humedad_sise": data[2],
                    "s_nivel_agua_sise": data[3], "s_temperatura_sise": data[4]}
    else:
        print "falso"
        datosDic = None
        # return datosDic

    return datosDic


def sendArduino(valores_recibir):
    aList = list(valores_recibir.values())
    aString = ','.join(map(str, aList))
    aString = aString + ','
    ser.write(aString.encode('ascii'))
    print "enviando a arduino: "
    # Orden de actuadores
    # entrada_aire, temperatura_agua, modo_manual, salida_aire, nivel_co2
    print aString
    time.sleep(1)
