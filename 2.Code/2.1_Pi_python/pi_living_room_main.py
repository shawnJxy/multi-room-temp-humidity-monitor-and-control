import time
import sys
import board
import adafruit_dht
import paho.mqtt.client as mqtt
import json
import asyncio
from kasa import SmartPlug
import dht
        
#setup smart plug entity
plug = SmartPlug("192.168.4.44")


#setup MQTT with thingsboard
THINGSBOARD_HOST = 'demo.thingsboard.io'
ACCESS_TOKEN = 'rK5mm66QTtCNOrAtHyzf'


def on_connect(client, userdata, rc, *extra_parms):
    print('Connected with result code' + str(rc))
    # Subscribing to receive RPC requests
    client.subscribe('v1/devices/me/rpc/request/+')
    #time.sleep(1)
    

def indoorStatus():
    t,h = dht.getdata()
    msg = str({'Temperature':t,'Humidity':h})
    print(msg)
    return msg

async def plugUpdate():
    await plug.update()
    if plug.is_on == True:
        #global plugStatus
        plugStatus = True
    else:
        #global plugStatus
        plugStatus = False
    print ("plugStatus: " + str(plugStatus))
    return plugStatus   
    
async def plugSwitch(plugSet):
    if plugSet == True:
        await plug.turn_on()
        print ("Plug On")
    else:
        await plug.turn_off()
        print ("Plug Off")

async def plugAuto():
    global plugSet_old
    global plugStatus
    t, h = dht.getdata()
    if t > 27:
        await plug.turn_on()
        plugSet_old = True
        plugStatus = True
        print ("Auto Fan: On")
    elif t <= 27:
        await plug.turn_off()
        plugSet_old = False
        plugStaus = False
        print ("Auto Fan: Off")
        

def on_message(client, userdata, msg):
    print ('Topic: ' + msg.topic + '\nMessage: ' + str(msg.payload))
    # Decode JSON request
    data = json.loads(msg.payload)
    # Check request method
    if data['method'] == 'getValue':
        if plugStatus == False:
            client.publish(msg.topic.replace('request', 'response'), json.dumps({'data': 0}), 1)
        else:          
            client.publish(msg.topic.replace('request', 'response'), json.dumps({'data': 1}), 1)
    elif data['method'] == 'setValue':
        global plugSet
        global plugSet_time
        if data['params'] == True:
            #global plugSet
            plugSet = True
            plugSet_time = time.time()
            print ("plugSet: " + str(plugSet))
        elif data['params'] == False:
            #global plugSet
            plugSet = False
            plugSet_time = time.time()
            print ("plugSet: " + str(plugSet))

client = mqtt.Client()
#Register connect callback
client.on_connect = on_connect
# Registed publish message callback
client.on_message = on_message
# Set access token
client.username_pw_set(ACCESS_TOKEN)
# Connect to ThingsBoard using default MQTT port and 60 seconds keepalive interval
client.connect(THINGSBOARD_HOST, 1883, 60)


async def main():
    global plugStatus
    global plugSet
    global plugSet_old
    global plugSet_time
    #check plug status
    plugStatus = await plugUpdate()
    print ("plug Status: " + str(plugStatus))
    #initialize plugSet
    plugSet_old = plugStatus
    plugSet = plugStatus
    plugSet_time = 0
    print ("initialized" + str(plugSet_old))
    while client.loop() ==0:
        try: 
            #publish room temp and humidity data
            client.publish('v1/devices/me/telemetry', indoorStatus(), qos=0, retain=False)
            print("payload sent")
            #update plug status
            #print (str(time.time()))
            #print (str(plugSet_time))
            #delta = time.time() - plugSet_time
            #print ("timeDelat: " + str(delta))
            if plugSet_time == 0:
                await plugAuto()
            elif time.time() - plugSet_time  > 3600:
                await plugAuto()
                plugSet_time = 0
            else:
                if plugSet == plugSet_old:
                    print ("plug Set: No Action")
                else:
                    await plugSwitch(plugSet)
                    plugSet_old = plugSet
                    plugSet_time = time.time()
                    print ("plug Set" + str(plugSet))              
            time.sleep(2)
        except KeyboardInterrupt:
            print("Disconnect")
            client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())