from app.models import Camera

# This function is used to extract the product names from the MQTT message
def extract_product_names(mqtt_message):
    lines = mqtt_message.splitlines()
    return [line.split('(')[0].strip() for line in lines]

# This function is used to get the latest update from the cameras associated with a shelf
def get_latest_update(cameras):
    latest_update = None
    latest_camera = None
    for camera in cameras:
        if camera.last_update:
            if not latest_update or camera.last_update > latest_update:
                latest_update = camera.last_update
                latest_camera = camera
    return latest_camera, latest_update