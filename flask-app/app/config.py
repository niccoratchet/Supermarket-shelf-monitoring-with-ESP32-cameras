import os
import logging

class Config:
    # Database's configuration.
    SQLALCHEMY_DATABASE_URI = os.getenv('DATABASE_URL', 'postgresql://admin:dbpass@db:5432/shelves-db')
    SQLALCHEMY_TRACK_MODIFICATIONS = False

    # MQTT Broker's configuration. The values are read from the environment variables
    MQTT_BROKER = os.getenv("MQTT_BROKER", "mqtt-broker")
    MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))
    MQTT_USER = os.getenv("MQTT_USER", "notfound-user")
    MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "notfound-password")

    # Define the folder where the uploaded images will be stored and the maximum size of the uploaded files
    BASE_UPLOAD_FOLDER = os.path.join('static', 'uploads')    
    MAX_CONTENT_LENGTH = 16 * 1024 * 1024                           

logging.basicConfig(level=logging.INFO)  # Set the logging level to INFO
