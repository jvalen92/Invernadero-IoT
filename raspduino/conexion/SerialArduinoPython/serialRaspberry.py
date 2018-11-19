import serial
import time

# Se puso este diccionario mientras se soluciona lo de sacar el diccionario de planta 1
valores_recibir = {
    "a_humedad_suelo_sise": "85",
    "a_ph_sise": "7.4",
    "a_luz_infrarroja_sise": "300",
    "a_luz_uv_sise": "350",
    "a_luz_blanca_sise": "400",
    "a_temperatura_suelo_sise": "40.6",
    "a_valvula_sise": "1"}


# Se establece la conexion
ser = serial.Serial('COM5', 9600, timeout=1)
time.sleep(1)

# Recibe los datos de los sensores por serial en 1 string separado por comas
# pone estos datos en un diccionario y retorna el diccionario con los valores
# y con su respectivo nombre.
# ver que hacer que a veces cuando se resetea, no tira todos los valores, entonces
# buscar como manejar ese error.


def getArduino():
    print('entrando a getArduino')
    data = ser.readline().decode('ascii')
    data = data.strip()
    data = data.split(',')
    if len(data) == 7:
        datosDic = {"s_luz_blanca_sise": data[0], "s_ph_sise": data[1], "s_luz_infrarroja_sise": data[2],
                    "s_humedad_suelo_sise": data[3], "s_luz_uv_sise": data[4], "s_temperatura_suelo_sise": data[5],
                    "s_valvula_sise": data[6]}
        print(datosDic)
    else:
        data[len(data):] = '0'

    # return datosDic


def sendArduino(valores_recibir):
    # Por el momento voy a asumir que ya tengo los valores en un diccionario, mientras resuelvo eso de como sacarlo
    # desde Planta1.py
    aList = list(valores_recibir.values())
    aString = ','.join(map(str, aList))
    aString = '<' + aString + '>'
    ser.write(aString.encode())
    print(aString)
    print("Se acabo el proceso de enviar")


""" sendArduino(valores_recibir)
time.sleep(5)

data1 = ser.readline()
print(data1) """
sendArduino(valores_recibir)
# time.sleep(5)
while True:
    getArduino()
    time.sleep(5)
print('end')
# para hacer pruebas
""" while True:
    getArduino()
    time.sleep(2)
 """
