import os
import json
import requests
import time


# Definiendo sistema operativo para la variable clear que sirve para limpiar la pantalla
sos = os.name
clear = "indefinido"
if sos == 'posix':
    clear = 'clear'
elif sos == 'nt':
    clear = 'cls'
    
# Variables

tiempo = 5 # Tiempo para enviar y recibir datos


# Declaracion de estructuras utiles

headers = {
    'appKey': "d13445f2-6c11-46f9-9352-efdedf394208",
    'Accept': "application/json",
    'Content-Type': "application/json",
    'Cache-Control': "no-cache"
    }
url = "http://iot.dis.eafit.edu.co/Thingworx/Things"

# Valores que se obtienen
objetos = {"central" : { }, "planta1": { }, "planta2": { }}

claves = {
    "Plant_sise": ["s_luz_uv_sise", "s_luz_infrarroja_sise", "a_humedad_suelo_sise", "s_luz_blanca_sise", "s_humedad_suelo_sise", "s_ph_sise", "a_ph_sise", "a_luz_infrarroja_sise", "a_luz_uv_sise", "a_luz_blanca_sise"],
    "Central_sise": ["a_temperatura_tanque_sise",  "s_temperatura_tanque_sise",  "a_humedad_sise",  "a_nivel_agua_sise",  "a_ventilador_entrada_sise",  "s_co2_sise",  "s_humedad_sise",  "s_nivel_agua_sise",  "a_ventilador_salida_sise",  "a_co2_sise",  "a_temperatura_sise",  "s_temperatura_sise"]
}

# Valores que se enviaran
valores = {
    "planta1" : {
        "s_luz_uv_sise" : "0",
        "s_luz_infrarroja_sise" : "0",
        "a_humedad_suelo_sise" : "0",
        "s_luz_blanca_sise" : "0",
        "s_humedad_suelo_sise" : "0",
        "s_ph_sise" : "0",
        "a_ph_sise": "0",
        "a_luz_infrarroja_sise" : "0",
        "a_luz_uv_sise": "0",
        "a_luz_blanca_sise" : "0" },
    "planta2" : {
        "s_luz_uv_sise" : "0",
        "s_luz_infrarroja_sise" : "0",
        "a_humedad_suelo_sise" : "0",
        "s_luz_blanca_sise" : "0",
        "s_humedad_suelo_sise" : "0",
        "s_ph_sise" : "0",
        "a_ph_sise": "0",
        "a_luz_infrarroja_sise" : "0",
        "a_luz_uv_sise": "0",
        "a_luz_blanca_sise" : "0" },
    "central" : {
        "a_temperatura_tanque_sise" : "0.0",
        "s_temperatura_tanque_sise" : "0.0",
        "a_humedad_sise" : "0.0",
        "a_nivel_agua_sise" : "0.0",
        "a_ventilador_entrada_sise" : "False",
        "s_co2_sise" : "0.0",
        "s_humedad_sise" : "0.0",
        "s_nivel_agua_sise" : "0.0",
        "a_ventilador_salida_sise" : "False",
        "a_co2_sise" : "0.0",
        "a_temperatura_sise" : "0.0",
        "s_temperatura_sise" : "0.0" }
    
}

# Declaracion de metodos

# Envia todos los datos a la plataforma
def setAllServerData():
    for valor in valores:
        thingTemplate = objetos[valor]["thingTemplate"]
        setServerData(claves[thingTemplate], valores[valor], valor)
    return objetos
    
# Envia datos a un determinado objeto. Usamos claves y valores porque de esta forma podriamos enviar datos por separado y no
# necesariamente enviar todos los datos de una vez
def setServerData(claves, valores, objeto):
    for k in claves:
        tempurl = url + "/"+objeto+"_sise_v1/Properties/"+k
        payload = "{\n\t\""+k+"\":\""+valores[k]+"\"\n}"
        response = requests.request("PUT", tempurl, data=payload, headers=headers)

# Impresor solo valido para este contexto, ya que debe hacer .items a los contenores que recibe
def printer(container, objet):
    container = container.items()
    for key, value in container:
        if objet:
            if (key != 'name' and key != 'tags' and key != 'description' and key != 'thingTemplate'):
                print key,":",value
        else:
            print key,":",value

            
# Este metodo imprime los elementos de un diccionario; en este codigo son los datos de un objeto.
# p.e:  printData(valores["planta1"].items())
def printData(container):
    printer(container, True)

# Este metodo imprime los datos de todos los objetos
def printAllData():
    print("Mostrando datos de todos los objetos...\n")
    for objeto in objetos:
        print(objeto+":")
        printer(objetos[objeto], False)
        print

def getAllServerData():
    for objeto in objetos:
        objetos[objeto] = getServerData(objeto)
    return objetos
           
# Obtiene todas las variables y datos de un determinado objeto en thingworx y los retorna en un diccionario.
# Devuelve una estructura que aun no conocemos muy bien, pero se debe utilizar el metodo items() para leeral
def getServerData(objeto):
    tempurl = url + "/"+objeto+"_sise_v1/Services/GetPropertyValues"
    
    response = requests.request("POST", tempurl, headers=headers)

    result = json.loads(response.text)
    
    return result['rows'][0]

def main():
    getAllServerData()
    setAllServerData()
    printAllData()
    print("obtenido")
anterior = 0
while(True):
    actual = time.time()
    if actual - anterior >= tiempo:
        anterior = actual
        print(actual)
        os.system(clear)
        main()
