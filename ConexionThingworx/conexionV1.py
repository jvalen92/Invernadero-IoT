import json
import requests

url = "http://iot.dis.eafit.edu.co/Thingworx/Things/planta1_sise_v1/Services/GetPropertyValues"

headers = {
    'appKey': "d13445f2-6c11-46f9-9352-efdedf394208",
    'Accept': "application/json",
    'Cache-Control': "no-cache",
    'Postman-Token': "a78d7f25-93c9-49c2-9e7b-ef33780d3233"
    }

def getData():
    response = requests.request("POST", url, headers=headers)
    result = json.loads(response.text)
    print(result['rows'][0].items())
    #print(result['rows'][0]['s_humedad_suelo_sise'])



getData()
