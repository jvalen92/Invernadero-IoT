import requests

url = "http://iot.dis.eafit.edu.co/Thingworx/Things/PruebaPlant1_sise/Properties/humedad_suelo_sise"

payload = "{\n\t\"humedad_suelo_sise\":\"20\"\n}"
headers = {
    'appKey': "d13445f2-6c11-46f9-9352-efdedf394208",
    'Content-Type': "application/json",
    'Cache-Control': "no-cache",
    'Postman-Token': "d63b80c3-cba9-4bd2-9680-0c40d0f03c05"
    }

response = requests.request("PUT", url, data=payload, headers=headers)

print(response.text)
