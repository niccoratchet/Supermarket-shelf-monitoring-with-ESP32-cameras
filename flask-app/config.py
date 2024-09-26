import os
from flask import Flask
from flask_sqlalchemy import SQLAlchemy
import logging

app = Flask(__name__)

# Database's configuration.
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URL', 'postgresql://admin:dbpass@db:5432/shelves-db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy()

# MQTT Broker's configuration. The values are read from the environment variables
MQTT_BROKER = os.getenv("MQTT_BROKER", "mqtt-broker")
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))
MQTT_USER = os.getenv("MQTT_USER", "notfound-user")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "notfound-password")

logging.basicConfig(level=logging.INFO)  # Set the logging level to INFO
