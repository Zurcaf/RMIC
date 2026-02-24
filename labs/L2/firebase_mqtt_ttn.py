#!/usr/bin/env python

import time
import paho.mqtt.client as mqtt
import pyrebase
import json
import base64
import os

firebase_config = {
  "apiKey": "AIzaSyAM5T2Gcq0CIZxUgMjVY08popnff-YvpNE",
  "authDomain": "iot-alarm-app.firebaseapp.com",
  "databaseURL": "https://iot-alarm-app.firebaseio.com/",
  "storageBucket": ""
#  "storageBucket": "gs://iot-alarm-app.appspot.com"
}

APPEUI = "70B3D57ED0009EC1"
APPID  = "iot-alarm-app@ttn"
PSW    = 'NNSXS.Q3VWD7FA2MHJKDGRUYAJVJK6ZTXVIR4UNJVCMCA.T7TLIWX56FTQNZVHY4MP2T3SQ4QLFDADUEPRJBEWMZR3322LR65Q'
DEVID = 'eui-0097cf9eb38628aa'

firebase = pyrebase.initialize_app(firebase_config)
db = firebase.database()

mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
unacked_publish = set()

def fb_stream_handler(message):
#    print(message["event"]) # put
#    print(message["path"]) # /-K7yGTTEp7O549EzTYtI
#    print(message["data"]) # {'title': 'Pyrebase', "body": "etc..."}
    print (message)
    enable_alert_sound = db.child("enable_alert_sound").get().val()
    print("/enable_alert_sound: ", enable_alert_sound)
    if enable_alert_sound == True:
        val = 1
    else:
        val = 0 
    enable_alert_sound_b64 = base64.b64encode(val.to_bytes(1, 'big'))
    x = { "downlinks": [{"f_port": 1, "frm_payload": enable_alert_sound_b64.decode(), "priority": "NORMAL" }]}
    print("x:", x)
    down_payload = json.dumps(x)
    print("down_payload:", down_payload)
    msg_info = mqtt_client.publish("v3/"+APPID+"/devices/" + DEVID + "/down/replace", down_payload, qos=1)
    unacked_publish.add(msg_info.mid)

def on_connect(client, userdata, flags, rc, properties):
    print("Connected with result code "+str(rc))
    print("Subscribe to topic: ", "v3/"+APPID+"/devices/" + DEVID +"/up")

    client.subscribe("v3/"+APPID+"/devices/" + DEVID +"/up")

def on_message(client, userdata, message):
    print("Received message '" + str(message.payload) + "' on topic '" + message.topic)
    
    alarm_status = 0
    measurement = 0
    
    y = json.loads(message.payload.decode('utf-8'))
    print(y)

    if message.topic == "v3/"+APPID+"/devices/" + DEVID +"/up":
        alarm_status = y['uplink_message']['decoded_payload']['decoded']['alarm_status']
        print("alarm_status:", alarm_status)
        val = bool(alarm_status) 
        data = {"alarm_status": val}       
        db.update(data)    
        measurement = y['uplink_message']['decoded_payload']['decoded']["measurement"]
        print("measurement:", measurement)
        val = int(measurement)
        data = {"measurement": val}
        db.update(data)

def on_publish(client, userdata, mid, reason_code, properties):
    # reason_code and properties will only be present in MQTTv5. It's always unset in MQTTv3
    try:
        userdata.remove(mid)
    except KeyError:
        print("on_publish() is called with a mid not present in unacked_publish")
        print("This is due to an unavoidable race-condition:")
        print("* publish() return the mid of the message sent.")
        print("* mid from publish() is added to unacked_publish by the main thread")
        print("* on_publish() is called by the loop_start thread")
        print("While unlikely (because on_publish() will be called after a network round-trip),")
        print(" this is a race-condition that COULD happen")
        print("")
        print("The best solution to avoid race-condition is using the msg_info from publish()")
        print("We could also try using a list of acknowledged mid rather than removing from pending list,")
        print("but remember that mid could be re-used !")      


def main():
    my_stream = db.child("enable_alert_sound").stream(fb_stream_handler, stream_id="new_posts")
    
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
#    mqtt_client.on_publish = on_publish
    
    mqtt_client.username_pw_set(APPID, PSW)
    mqtt_client.connect('eu1.cloud.thethings.network', 1883, 60) 
    mqtt_client.loop_start() 
    

    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:       
        print("Program interrupted by the user.")
        mqtt_client.loop_stop()
        my_stream.close()

       

if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()


