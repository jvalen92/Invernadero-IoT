import json
import requests

url = "http://iot.dis.eafit.edu.co/Thingworx/Things/PruebaPlant1_sise/Properties/s_humedad_suelo_sise"

payload = "{\n\t\"s_humedad_suelo_sise\":\"30\"\n}"
headers = {
    'appKey': "d13445f2-6c11-46f9-9352-efdedf394208",
    'Accept': "application/json",
    'Content-Type': "application/json",
    'Cache-Control': "no-cache",
    'Postman-Token': "3dbc9089-509b-4961-914e-f3105b4f6789"
    }

response = requests.request("PUT", url, data=payload, headers=headers)

print(response.text)
