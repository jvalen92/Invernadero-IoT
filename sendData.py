#-*-coding:utf8-*-
"""
    Carriots.com
    Created 11 Jan 2013

    This sketch sends streams to Carriots according to the values read by a LDR sensor
"""

#import RPi.GPIO as GPIO
from urllib2 import urlopen, Request
from time import mktime, sleep
from datetime import datetime
from json import dumps


class Client (object):
    api_url = "http://api.carriots.com/streams"

    def __init__(self, api_key=None, client_type='json'):
        self.client_type = client_type
        self.api_key = api_key
        self.content_type = "application/vnd.carriots.api.v2+%s" % self.client_type
        self.headers = {'User-Agent': 'Raspberry-Carriots',
                        'Content-Type': self.content_type,
                        'Accept': self.content_type,
                        'Carriots.apikey': self.api_key}
        self.data = None
        self.response = None

    def send(self, data):
        self.data = dumps(data)
        request = Request(Client.api_url, self.data, self.headers)
        self.response = urlopen(request)
        return self.response




def main():
    #GPIO.setmode(GPIO.BCM)
    #Tiempo actual
    timestamp = int(mktime(datetime.utcnow().timetuple()))

    device = "defaultDevice@jvalen92.jvalen92"  # Replace with the id_developer of your device
    apikey = "c9d9fbc64533e1b01a52005714cb40777f9075b985431a20801d8d2e1a65b4ba"  # Replace with your Carriots apikey

    client_carriots = Client(apikey)

    #valor de variable a mandar
    value="Enviado desde la raspberry!!!"

    data = {"protocol": "v2", "device": device, "at": timestamp, "data":value }
    carriots_response = client_carriots.send(data)
    print (carriots_response.read())

if __name__ == '__main__':
    main()


