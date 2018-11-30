import os
import json
import requests
import time
import datetime
import serialRaspberryPlanta as ras

# Definiendo sistema operativo para la variable clear que sirve para limpiar la pantalla
sos = os.name
clear = "indefinido"
if sos == 'posix':
    clear = 'clear'
elif sos == 'nt':
    clear = 'cls'

# Variables

iteration = 0
tiempo = 0  # Tiempo para enviar y recibir datos
dics = {}

# Declaracion de estructuras utiles

headers = {
    'appKey': "d13445f2-6c11-46f9-9352-efdedf394208",
    'Accept': "application/json",
    'Content-Type': "application/json",
    'Cache-Control': "no-cache"
}
url = "http://iot.dis.eafit.edu.co/Thingworx/Things"

# Valores que se obtienen
objetos = {"planta1": {}}


# Valores oficiales

claves_enviar = {
    "Plant_sise": ["s_luz_uv_sise", "s_luz_infrarroja_sise", "s_luz_blanca_sise", "s_humedad_suelo_sise", "s_ph_sise"]
}

claves_recibir = {
    "Plant_sise": ["a_humedad_suelo_sise", "a_luz_infrarroja_sise", "a_luz_uv_sise", "a_luz_blanca_sise", "a_electro_val_sise", "a_motobomba_sise"],
}

# Valores que se enviaran
valores_enviar = {
    "planta1": {
        "s_luz_uv_sise": "0",
        "s_luz_infrarroja_sise": "0",
        "s_luz_blanca_sise": "0",
        "s_humedad_suelo_sise": "0",
        "s_ph_sise": "0",

    }

}

valores_recibir = {
    "planta1": {
        "a_humedad_suelo_sise": "0",
        "a_luz_infrarroja_sise": "0",
        "a_luz_uv_sise": "0",
        "a_luz_blanca_sise": "0",
        "a_motobomba_sise": "0",
        "a_electro_val_sise": "0",

    }

}


# Declaracion de metodos


# Envia todos los datos a la plataforma
def setAllServerData(valores_enviados):
    for valor in valores_enviados:
        thingTemplate = objetos[valor]["thingTemplate"]
        setServerData(claves_enviar[thingTemplate],
                      valores_enviados[valor], valor)
    return valores_enviados

# Envia datos a un determinado objeto. Usamos claves y valores porque de esta forma podriamos enviar datos por separado y no
# necesariamente enviar todos los datos de una vez


def setServerData(claves, valores, objeto):
    for k in claves:
        tempurl = url + "/"+objeto+"_sise_v1/Properties/"+k
        payload = "{\n\t\""+k+"\":\""+valores[k]+"\"\n}"
        response = requests.request(
            "PUT", tempurl, data=payload, headers=headers)

# Impresor solo valido para este contexto, ya que debe hacer .items a los contenores que recibe


def printer(container, objet):
    containers = container.items()
    lista = []
    for key, value in containers:
        if objet:
            if (key != 'name' and key != 'tags' and key != 'description' and key != 'thingTemplate'):
                lista.append((str(key), value))
        else:
            lista.append((str(key), value))

    lista.sort()

    for each in lista:
        print(str(each[0]), ':', each[1])


# Este metodo imprime los elementos de un diccionario; en este codigo son los datos de un objeto.
# p.e:  printData(valores["planta1"].items())
def printData(container):
    printer(container, True)

# Este metodo imprime los datos de todos los objetos


def printAllData():
    for objeto in objetos:
        print(objeto+":")
        printer(objetos[objeto], True)
        print

# Metodo que recibe los valores en el diccionario valores_recibidos


def getAllServerData(valores_recibidos):
    for valor in valores_recibidos:

        valores_recibidos[valor] = getServerData(valor)
    return valores_recibidos

# Obtiene todas las variables y datos de un determinado objeto en thingworx y los retorna en un diccionario.
# Devuelve una estructura que aun no conocemos muy bien, pero se debe utilizar el metodo items() para leeral


def getServerData(objeto):
    tempurl = url + "/"+objeto+"_sise_v1/Services/GetPropertyValues"

    response = requests.request("POST", tempurl, headers=headers)

    result = json.loads(response.text)['rows'][0]

    for one in result:
        if(one[0] == 'a'):
            dics[str(one)] = result[one]

    return result


def setProperty(key, value):
    valores_enviar["planta1"][key] = value


def getProperty(key):
    return valores_recibir["planta1"][key]


miObjeto = "planta1"

# valores enviar s
# valores recibir a


def setValuesToThingworx(dic):

    for valoruni in valores_enviar["planta1"]:
        if(valoruni[0] == 's'):
            setProperty(valoruni, str(dic[valoruni]))


def setArduinoData():
    ras.sendArduino(dics)


def getArduinoData():

    return ras.getArduino()


def main():

    getAllServerData(objetos)
    getAllServerData(valores_recibir)

    setArduinoData()

    gett = getArduinoData()

    if(gett != None):

        now = datetime.datetime.now()
        print "\n"
        print now.strftime("%Y-%m-%d %H:%M")

        for key, value in dics.iteritems():
            print key, " : ", dics[key]

        for key, value in gett.iteritems():
            print key, " : ", gett[key]

        setValuesToThingworx(gett)

    # Enviar al servidor todos los datos
    setAllServerData(valores_enviar)
    # printAllData()


while(True):

    main()
    time.sleep(tiempo)
# por el momento se envia a thingwors los valores que el mismo pone pero deberia ser del arduino
#
