import requests

url = "http://iot.dis.eafit.edu.co/Thingworx/Things/PruebaPlant1_sise/Properties/humedad_suelo_sise"

headers = {
    'appKey': "d13445f2-6c11-46f9-9352-efdedf394208",
    'Content-Type': "application/json",
    'Cache-Control': "no-cache",
    'Postman-Token': "68d2ded6-ae9e-41a0-b647-480957b7bd01"
    }

response = requests.request("GET", url, headers=headers)

print(response.text)
