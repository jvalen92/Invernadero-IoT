import requests

url = "https://academic.cloud.thingworx.com/Thingworx/Things/NorhWeatherStation_evinley/Properties/WindSpeed"

headers = {
    'appKey': "052b8a49-091f-4f83-b7df-5f90a3394be0",
    'Accept': "application/json",
    'Cache-Control': "no-cache",
    'Postman-Token': "61b72666-485b-4bc4-8581-ed4a37efbfaa"
    }

response = requests.request("GET", url, headers=headers)

print(response.text)
