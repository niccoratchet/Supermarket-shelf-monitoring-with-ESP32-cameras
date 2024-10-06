import os
from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from .config import Config
import paho.mqtt.client as mqtt

db = SQLAlchemy()                # SQLAlchemy instance initialization without the Flask app

def create_app():
    
    app = Flask(__name__)               # Creating the Flask app
    app.config.from_object(Config)      # Load the configuration from the Config class

    app.logger.info(f" BASE_UPLOAD_FOLDER: {app.config.get('BASE_UPLOAD_FOLDER')}")            # Log the upload folder path

    if not os.path.exists(app.config['BASE_UPLOAD_FOLDER']):                             # Check if the upload folder exists, if not create it
        os.makedirs(app.config['BASE_UPLOAD_FOLDER'])
        app.logger.info(f" Created upload folder at {app.config['BASE_UPLOAD_FOLDER']}")
    else:
        app.logger.info(f" Upload folder already exists at {app.config['BASE_UPLOAD_FOLDER']}")

    db.init_app(app)  # Connect the SQLAlchemy instance to the Flask app

    # Blueprint's registration
    from app.routes import main as main_blueprint                           # Importing here to avoid circular imports
    app.register_blueprint(main_blueprint)

    # MQTT Client initialization
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(app.config['MQTT_USER'], app.config['MQTT_PASSWORD'])
    from app.mqtt_handlers import init_mqtt_handlers                                        # Importing here to avoid circular imports
    init_mqtt_handlers(app, mqtt_client)                                                 
    mqtt_client.connect(app.config['MQTT_BROKER'], app.config['MQTT_PORT'], 60)
    mqtt_client.loop_start()                                                                # Start the MQTT client in a new thread

    # Database initialization
    with app.app_context():
        db.create_all()
        from app.models import Shelf                                                  # Importing here to avoid circular imports
        if Shelf.query.count() == 0:                                    # If the Shelf table is empty, populate it with some initial values
            initial_shelves = [Shelf(number="1", description="Pasta"),
                               Shelf(number="2", description="Bath")]
            db.session.bulk_save_objects(initial_shelves)                # Used to insert multiple objects in the database from a list
            db.session.commit()                                         # Commit the changes to the database 

    return app
