from config import app, db, MQTT_BROKER, MQTT_PORT, MQTT_USER, MQTT_PASSWORD
from app.routes import main as main_blueprint
from app.models import Shelf
from app.mqtt_handlers import init_mqtt_handlers
import paho.mqtt.client as mqtt

def create_app():

    # Database's configuration
    db.init_app(app)

    # Blueprint's registration
    app.register_blueprint(main_blueprint)

    # MQTT Client initialization
    mqtt_client = mqtt.Client()
    init_mqtt_handlers(app, mqtt_client)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_start()                            # Start the MQTT client's loop in a separate thread

    # Database initialization
    with app.app_context():
        db.create_all()
        if Shelf.query.count() == 0:                                    # If the Shelf table is empty, populate it with some initial values
            initial_shelves = [Shelf(number="1", description="Pasta"),
                               Shelf(number="2", description="Bath")]
            db.session.bulk_save_objects(initial_shelves)                # Used to insert multiple objects in the database from a list
            db.session.commit()                                         # Commit the changes to the database 

    return app
