import json
import requests

# Declaracion de estructuras utiles

headers = {
    'appKey': "d13445f2-6c11-46f9-9352-efdedf394208",
    'Accept': "application/json",
    'Content-Type': "application/json",
    'Cache-Control': "no-cache"
    }
url = "http://iot.dis.eafit.edu.co/Thingworx/Things"
        
claves = {
    "planta": ["s_luz_uv_sise", "s_luz_infrarroja_sise", "a_humedad_suelo_sise", "s_luz_blanca_sise", "s_humedad_suelo_sise", "s_ph_sise", "a_ph_sise", "a_luz_infrarroja_sise", "a_luz_uv_sise", "a_luz_blanca_sise"],
    "central": ["a_temperatura_tanque_sise",  "s_temperatura_tanque_sise",  "a_humedad_sise",  "a_nivel_agua_sise",  "a_ventilador_entrada_sise",  "s_co2_sise",  "s_humedad_sise",  "s_nivel_agua_sise",  "a_ventilador_salida_sise",  "a_co2_sise",  "a_temperatura_sise",  "s_temperatura_sise"]
}

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

# Envia datos a un determinado objeto
def setServerData(claves, valores, objeto):
    for k in claves:
        tempurl = url + "/"+objeto+"_sise_v1/Properties/"+k
        payload = "{\n\t\""+k+"\":\""+valores[k]+"\"\n}"
        response = requests.request("PUT", tempurl, data=payload, headers=headers)

# Este metodo imprime los elementos de un diccionario; en este codigo son los datos de un objeto
def printData(container):
    for key, value in container:
        if (key != 'name' and key != 'tags' and key != 'description' and key != 'thingTemplate'):
           print key,":",value

# Obtiene todas las variables y datos de un determinado objeto en thingworx y los retorna en un diccionario
def getServerData(objeto):
    tempurl = url + "/"+objeto+"_sise_v1/Services/GetPropertyValues"
    
    response = requests.request("POST", tempurl, headers=headers)

    result = json.loads(response.text)

    return result['rows'][0].items()
    

setServerData(claves["central"], valores["central"], "central")
printData(getServerData("central"))
