import serial
import time

# Se puso este diccionario mientras se soluciona lo de sacar el diccionario de planta 1
valores_recibir = {
    "a_humedad_suelo_sise": "85",
    "a_ph_sise": "7",
    "a_luz_infrarroja_sise": "300",
    "a_luz_uv_sise": "350",
    "a_luz_blanca_sise": "400",
    "a_temperatura_suelo_sise": "40",
    "a_valvula_sise": "1"}


# Se establece la conexion
ser = serial.Serial('COM5', 9600, timeout=1)
time.sleep(1)

# Recibe los datos de los sensores por serial en 1 string separado por comas
# pone estos datos en un diccionario y retorna el diccionario con los valores
# y con su respectivo nombre


def getArduino():
    data = ser.readline()  # Aqui lee la linea desde serial y la guarda en una variable
    # Se le quita todas esas cosas que deja al principio y al final del string
    data = data.strip()
    # No se porque no quito tambien esta b, entonces toco especificar que la quite (mejorar como hace este proceso)
    data = data.decode().strip('b')
    # Se separan los valores donde estan las ',' y se guarda en una lista
    data = data.split(',')

    # aca guarda los datos en un diccionario con su respectiva clave y valor
    if len(data) == 7:
        datosDic = {"s_luz_blanca_sise": data[0], "s_ph_sise": data[1], "s_luz_infrarroja_sise": data[2],
                    "s_humedad_suelo_sise": data[3], "s_luz_uv_sise": data[4], "s_temperatura_suelo_sise": data[5],
                    "s_valvula_sise": data[6]}
        # De prueba
        # return datosDic
        print(datosDic)
    # En algunos casos las primeras veces que es llamada la funcion getSerial() como que no coge todos los valores,
    # entonces para que no cree la lista si no llegaron los valores completos, aunque deberia pensar en una mejor
    # forma de hacer esa parte de verificacion (le estoy pensando)
    elif len(data) < 6:
        print("no se ha llenado la lista")


def sendArduino(valores_recibir):
    # Por el momento voy a asumir que ya tengo los valores en un diccionario, mientras resuelvo eso de como sacarlo
    # desde Planta1.py
    aList = list(valores_recibir.values())
    for value in aList:
        ser.write(value.encode())
        print(value)
    print("Se acabo el proceso de enviar")


sendArduino(valores_recibir)
time.sleep(5)

data1 = ser.readline()
while True:
    print(data1)

# para hacer pruebas
""" while True:
    getArduino()
    time.sleep(2) """
