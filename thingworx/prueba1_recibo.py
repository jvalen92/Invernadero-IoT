import json
import requests

url = "http://iot.dis.eafit.edu.co/Thingworx/Things/PruebaPlant1_sise/Properties/s_humedad_suelo_sise"

headers = {
    'appKey': "d13445f2-6c11-46f9-9352-efdedf394208",
    'Accept': "application/json",
    'Cache-Control': "no-cache",
    'Postman-Token': "61fa9c6f-8250-48b9-ad37-2faac0519e1f"
    }

response = requests.request("GET", url, headers=headers)

# print(response.text)

cont = json.loads(response.text)
print(cont['rows'][0]['s_humedad_suelo_sise'])
