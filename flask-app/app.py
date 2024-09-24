from flask import Flask
import paho.mqtt.client as mqtt
import logging
import os
from app.models import db, Shelf
from app.routes import main as main_blueprint
from app.mqtt_handlers import init_mqtt_handlers
from app.utils import extract_product_names

app = Flask(__name__)

# Database's configuration.
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URL', 'postgresql://admin:dbpass@db:5432/shelves-db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db.init_app(app)
app.register_blueprint(main_blueprint)                          # Register the blueprint in the Flask app

# MQTT Broker's configuration. The values are read from the environment variables
MQTT_BROKER = os.getenv("MQTT_BROKER", "mqtt-broker")                   # Broker MQTT container's name
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))                           # Broker MQTT port
MQTT_USER = os.getenv("MQTT_USER", "notfound-user")                     # Username for the MQTT client
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "notfound-password")         # Password for the MQTT client

logging.basicConfig(level=logging.INFO)  # Set the logging level to INFO

mqtt_client = mqtt.Client()
init_mqtt_handlers(app, mqtt_client)  # Initialize the MQTT handlers
mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)  # Set the username and password for the MQTT client
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)  # Connect to the broker
mqtt_client.loop_start()  # Start the MQTT client's loop in a separate thread

def initialize_database():
    with app.app_context():                                             # app.app_context() is used to create a context in which the application is configured outside of the request/response cycle
        db.create_all()                                                 # Create the tables in the database
        if Shelf.query.count() == 0:                                    # If the Shelf table is empty, populate it with some initial values
            initial_shelves = [Shelf(number="1", description="Pasta"),
                               Shelf(number="2", description="Bath")]
            db.session.bulk_save_objects(initial_shelves)                # Used to insert multiple objects in the database from a list
            db.session.commit()                                         # Commit the changes to the database                     

if __name__ == "__main__":
    initialize_database()
    app.run(host='0.0.0.0', port=5000)
