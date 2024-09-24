# This function is used to extract the product names from the MQTT message
def extract_product_names(mqtt_message):
    lines = mqtt_message.splitlines()
    return [line.split('(')[0].strip() for line in lines]