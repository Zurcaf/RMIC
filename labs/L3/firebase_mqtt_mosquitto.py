#!/usr/bin/env python

import time
import paho.mqtt.client as mqtt
from pyrebase import pyrebase

firebase_config = {
  "apiKey": "AIzaSyAM6T2Gcq0CIZyUgMjVY18popnff-YvpNE",
  "authDomain": "iot-alarm.firebaseapp.com",
  "databaseURL": "https://iot-alarm.firebaseio.com/",
  "storageBucket": ""
#  "storageBucket": "gs://iot-alarm.appspot.com"
}

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
    msg_info = mqtt_client.publish("/RMIC_G00/enable_alert_sound", val, qos=1)
    unacked_publish.add(msg_info.mid)

def on_connect(client, userdata, flags, rc, properties):
    print("Connected with result code "+str(rc))

    client.subscribe("/RMIC_G00/#")  

def on_message(client, userdata, message):
    print("Received message '" + str(message.payload) + "' on topic '" + message.topic)
    
    alarm_status = 0
    measurement = 0
    global val
    
    if message.topic == "/RMIC_G00/alarm_status":
        print("alarm_status: ")
        alarm_status = str(message.payload, 'UTF-8')
        alarm_status = alarm_status.strip()
        print(alarm_status)
        val = eval(alarm_status)
        if val == 1:
            data = {"alarm_status": True}
        else: 
            data = {"alarm_status": False}       
        db.update(data)
       

    if message.topic == "/RMIC_G00/measurement":
        print("/measurement: ")
        measurement = str(message.payload, 'UTF-8')
        measurement = measurement.strip()
        print(measurement)
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
    
    mqtt_client.connect('test.mosquitto.org', 1883, 60) 
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


