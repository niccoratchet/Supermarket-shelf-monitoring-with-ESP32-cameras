from app.models import db, Camera, Product, Camera_Product, Product_Shelf
from collections import Counter
from app.utils import extract_product_names
from sqlalchemy import text

# Topics to subscribe to. "inference/#" is used to receive the inference results from the cameras. "cameraConnected/#" is used to notify the Flask app when a new camera is connected
# Regarding the "cameraConnected/#" if a new camera is connected, at the wildcard place will be present 'N'. It indicates that the camera has to be configured and needs a new ID.
TOPICS = ["inference/#", "cameraConnected/#"]

def init_mqtt_handlers(app, mqtt_client):
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
            if msg.payload.decode() == "No objects found":
                app.logger.info(f" Camera n° {cameraID} has not spotted any object")
                updateCameraProductQuantity(cameraID, "No objects found")
            else:
                app.logger.info(f" Camera n° {cameraID} has spotted: {msg.payload.decode()}")
                updateCameraProductQuantity(cameraID, msg.payload.decode())
        elif msg.topic.endswith("N"):
            app.logger.info(f" Camera with objects to scan: {msg.payload.decode()} is now connected. Extracting a new ID from the database...")
            newCamera = newCameraConfiguration(client, userdata, msg)
            linkProductsWithCamera(newCamera, msg.payload.decode())
        else:
            app.logger.info(f" Camera with ID: {cameraID} and objects to scan: {msg.payload.decode()} is now connected")

    # Called when a new camera is connected and needs a new ID extracted from the database
    def newCameraConfiguration(client, userdata, msg):  # This function is called when a new camera is connected and needs to be configured
        with app.app_context():
            app.logger.info("A new camera needs to be configured. Generating a new ID...")
            newCamera = Camera(shelf_number="1", description="ESP32")                           # TODO: Shelf's number and camera's description should be changed by the user
            db.session.add(newCamera)
            db.session.commit()
            mqtt_client.publish(f"newID/{newCamera.id}", newCamera.id)  # Publish the new camera's ID to the topic newID
            app.logger.info(f" Camera with ID: {newCamera.id} and objects to scan: {msg.payload.decode()} is now connected")
            return newCamera

    # Called when a new camera is configured. Products (not already present) are added to the database and the camera is linked to them
    def linkProductsWithCamera(newCamera, objectsString):
        with app.app_context():
            app.logger.info("Updating the shelf information...")
            objects = objectsString.split(" ")                          # Split the objects string into a list of objects
            presenceList = [False] * len(objects)                       # Create a list of booleans to check if the objects are present on the shelf
            listOfProducts = db.session.query(Product).all()            # Get all the products from the database
            for product in listOfProducts:
                if product.name in objects:
                    app.logger.info(f" Product {product.name} is already present on shelves")
                    newCameraProduct = Camera_Product(camera_id=newCamera.id, product_id=product.id)
                    newProductShelf = Product_Shelf(product_id=product.id, shelf_number=newCamera.shelf_number)
                    presenceList[objects.index(product.name)] = True
                    db.session.add(newCameraProduct)
                    db.session.add(newProductShelf)
                    db.session.commit()
            for i in range(len(objects)):
                if presenceList[i] == False:
                    app.logger.info(f" Product {objects[i]} is not present on the shelf. Adding it to the database...")
                    newProduct = Product(name=objects[i], quantity= 0, category="Pasta")
                    db.session.add(newProduct)
                    db.session.commit()
                    newCameraProduct = Camera_Product(camera_id=newCamera.id, product_id=newProduct.id)
                    newProductShelf = Product_Shelf(product_id=newProduct.id, shelf_number=newCamera.shelf_number)
                    db.session.add(newCameraProduct)
                    db.session.add(newProductShelf)
                    db.session.commit()

    def updateCameraProductQuantity(cameraID, msgPayload):  # This function is called when a camera sends the quantity of a product
        with app.app_context():
            if msgPayload != "No objects found":
                productNames = extract_product_names(msgPayload)                    # Extract the product names from the MQTT message
                productCounts = Counter(productNames)                               # Count the number of occurrences of each product

                monitoredProducts = db.session.query(Camera_Product).filter_by(camera_id=cameraID).all()    # Get the products monitored by the camera

                updatedProductIDs = set()                                           # Create a set to store the IDs of the products that have been updated
                for productName in productNames:
                    productID = db.session.query(Product.id).filter_by(name=productName).first()            # Get the ID of the spotted product
                    if productID is not None:
                        cameraProduct = db.session.query(Camera_Product).filter(Camera_Product.camera_id == cameraID, Camera_Product.product_id == productID[0]).first()        # Get the camera_product row for the spotted product
                        if cameraProduct is not None:
                            cameraProduct.quantity = productCounts[productName]
                            updatedProductIDs.add(productID[0])

                for cameraProduct in monitoredProducts:                         # Set the quantity of the products that have not been spotted to 0
                    if cameraProduct.product_id not in updatedProductIDs:
                        app.logger.info(f" Product {cameraProduct.product_id} has not been spotted by the camera. Setting the quantity to 0")
                        cameraProduct.quantity = 0
            else:
                monitoredProducts = db.session.query(Camera_Product).filter_by(camera_id=cameraID).all()
                for cameraProduct in monitoredProducts:
                    cameraProduct.quantity = 0
                    app.logger.info(f" Product {cameraProduct.product_id} has not been spotted by the camera. Setting the quantity to 0")
            db.session.commit()
            updateShelfProductQuantities(cameraID)                          # Update the quantity of the products in the Shelf_Product table

    # This function is used to update the quantity of the products in Shelf_Product table  
    def updateShelfProductQuantities(cameraID):
        with app.app_context():
            camera = db.session.query(Camera).filter_by(id=cameraID).first()
            if camera is not None:
                query= text("""SELECT cp.product_id AS product_id, SUM(cp.quantity) as total_quantity
                               FROM Camera c, Camera_Product cp
                               WHERE shelf_number = :shelf_number AND c.id = cp.camera_id AND cp.camera_id = :camera_id
                               GROUP BY product_id
                         """)
                result = db.session.execute(query, {'shelf_number': camera.shelf_number, 'camera_id': cameraID}).fetchall()  # Execute the query and fetch all the results
                product_quantities = {row[0]: row[1] for row in result}       # Converting the result into a dictionary {product_id: total_quantity}
                for product_id, total_quantity in product_quantities.items():                       # Update the quantity of the products in the Shelf_Product table
                    query = text("""
                        UPDATE Product_Shelf
                        SET quantity = :total_quantity
                        WHERE shelf_number = :shelf_number
                        AND product_id = :product_id
                    """)
                    db.session.execute(query, {
                        'total_quantity': total_quantity,
                        'shelf_number': camera.shelf_number,
                        'product_id': product_id
                    })
                db.session.commit()
            else:
                app.logger.error(f" Camera with ID {cameraID} not found")
    
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message