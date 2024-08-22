from flask import Flask, jsonify
import paho.mqtt.client as mqtt
import logging
import os
from flask_sqlalchemy import SQLAlchemy

app = Flask(__name__)

# Database's configuration.
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URL', 'postgresql://admin:dbpass@db:5432/shelves-db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db = SQLAlchemy(app)

# MQTT Broker's configuration. The values are read from the environment variables
MQTT_BROKER = os.getenv("MQTT_BROKER", "mqtt-broker")                   # Broker MQTT container's name
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))                           # Broker MQTT port
MQTT_USER = os.getenv("MQTT_USER", "notfound-user")                     # Username for the MQTT client
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "notfound-password")         # Password for the MQTT client

# Topics to subscribe to. "inference/#" is used to receive the inference results from the cameras. "cameraConnected/#" is used to notify the Flask app when a new camera is connected
# Reguarding the "cameraConnected/#" if a new camera is connected, at the wildcard place will be present 'N'. It indicates that the camera has to be configured and needs a new ID.
TOPICS = ["inference/#", "cameraConnected/#"]

# Called when the flask app connects to the broker
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
        cameraID = msg.topic.split("/")[1]              # Extract the camera ID from the topic
        if msg.payload.decode() == " No objects found":
            app.logger.info(f" Camera n° {cameraID} has not spotted any object")
        else:
            app.logger.info(f" Camera n° {cameraID} has spotted: {msg.payload.decode()}")
    elif msg.topic.endswith("N"):
        newCameraConfiguration(client, userdata, msg)
    else:
        app.logger.info(f" Camera with ID: {cameraID} and objects to scan: {msg.payload.decode()} is now connected")       # TODO: Include shelf information in the log

def newCameraConfiguration(client, userdata, msg):  # This function is called when a new camera is connected and needs to be configured
    with app.app_context():
        app.logger.info("A new camera needs to be configured. Generating a new ID...")
        newCamera = Camera(shelf_number="1", description="ESP32")
        db.session.add(newCamera)
        db.session.commit()
        mqtt_client.publish(f"newID/{newCamera.id}", newCamera.id)  # Publish the new camera's ID to the topic newID
    

logging.basicConfig(level=logging.INFO)  # Set the logging level to INFO

mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)  # Set the username and password for the MQTT client
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)  # Connect to the broker
mqtt_client.loop_start()  # Start the MQTT client's loop in a separate thread

class Shelf(db.Model):          # This class represents the Shelf table in the database

    __tablename__ = 'shelf'
    number = db.Column(db.Text, primary_key=True)
    description = db.Column(db.Text, nullable=True)
    def __repr__(self):
        return f'<Shelf {self.number} - {self.description}>'
    
class Camera(db.Model):         # This class represents the Camera table in the database

    __tablename__ = 'camera'
    id = db.Column(db.Integer, primary_key=True)
    shelf_number = db.Column(db.Text, db.ForeignKey('shelf.number'), nullable=False)
    description = db.Column(db.Text, nullable=True)
    def __repr__(self):
        return f'<Camera {self.id} - {self.shelf_number} - {self.description}>'

    
def initialize_database():
    with app.app_context():                                             # app.app_context() is used to create a context in which the application is configured outside of the request/response cycle
        db.create_all()                                                 # Create the tables in the database
        if Shelf.query.count() == 0:                                    # If the Shelf table is empty, populate it with some initial values
            inital_shelves = [Shelf(number="1", description="Pasta"),
                               Shelf(number="2", description="Bath")]
            db.session.bulk_save_objects(inital_shelves)                # Used to insert multiple objects in the database from a list
            db.session.commit()                                         # Commit the changes to the database                     


@app.route('/')
def home():
    return "Hello, Flask with MQTT!"

@app.route('/shelves')
def shelves():
    shelves = Shelf.query.all()
    results = [
        {
            "number": shelf.number,
            "description": shelf.description
        } for shelf in shelves
    ]

    return jsonify(results)

if __name__ == "__main__":
    initialize_database()
    app.run(host='0.0.0.0', port=5000)