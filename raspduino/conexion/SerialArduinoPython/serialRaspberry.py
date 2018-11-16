import serial
import time

# Se establece la conexion
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(1)

# Recibe los datos de los sensores por serial en 1 string separado por comas
# pone estos datos en un diccionario y retorna el diccionario con los valores
# y con su respectivo nombre


def getSerial():
    data = ser.readline()
    data = data.strip()
    data = data.decode().strip('b')
    data = data.split(',')
    if len(data) == 7:
        datosDic = {"s_luz_blanca_sise": data[0], "s_ph_sise": data[1], "s_luz_infrarroja_sise": data[2],
                    "s_humedad_suelo_sise": data[3], "s_luz_uv_sise": data[4], "s_temperatura_suelo_sise": data[5],
                    "s_valvula_sise": data[6]}
        return datosDic
    elif len(data) < 6:
        print("no se ha llenado la lista")


while True:
    getSerial()
    time.sleep(3)
