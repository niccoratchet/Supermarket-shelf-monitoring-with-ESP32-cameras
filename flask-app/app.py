from flask import Flask
import paho.mqtt.client as mqtt
import logging
import os

app = Flask(__name__)

# MQTT Broker's configuration. The values are read from the environment variables
MQTT_BROKER = os.getenv("MQTT_BROKER", "mqtt-broker")                   # Broker MQTT container's name
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))                           # Broker MQTT port
MQTT_USER = os.getenv("MQTT_USER", "notfound-user")                     # Username for the MQTT client
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "notfound-password")         # Password for the MQTT client

# Topics to subscribe to. "inference/#" is used to receive the inference results from the cameras. "cameraConnected/#" is used to notify the Flask app when a new camera is connected
TOPICS = ["inference/#", "cameraConnected/#"]

# Called when the client connects to the broker
def on_connect(client, userdata, flags, rc):
    app.logger.info(f" Connected with result code {rc}")
    if rc == 0:
        for topic in TOPICS:
            client.subscribe(topic)
            app.logger.info(f" Subscribed to topic: {topic}")
    else:
        app.logger.error(" Failed to connect to the broker: return code {rc}")

# Called when a message is received from the broker
def on_message(client, userdata, msg):
    if msg.topic.startswith("inference/"):
        if msg.payload.decode() == " No objects found":
            app.logger.info(f" Camera n° {msg.topic[-1]} has not spotted any object")
        else:
            app.logger.info(f" Camera n° {msg.topic[-1]} has spotted: {msg.payload.decode()}")
    else:
        app.logger.info(f" A new camera has been configured. New camera's ID: {msg.topic[-1]}. Objects to scan: {msg.payload.decode()}")

logging.basicConfig(level=logging.INFO)  # Set the logging level to INFO

mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)  # Set the username and password for the MQTT client
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)  # Connect to the broker
mqtt_client.loop_start()  # Start the MQTT client's loop in a separate thread

@app.route('/')
def home():
    return "Hello, Flask with MQTT!"

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000)