from flask import Blueprint, json, jsonify, request, current_app, render_template, url_for
from app.models import Shelf, Camera, Product_Shelf, Product
from datetime import datetime
from app.utils import get_latest_update
from . import db
import os

main = Blueprint('main', __name__)      # Create a Blueprint object

@main.route('/home')                    # Route to the home page
def home():
    return render_template('home.html')

@main.route('/shelves')                 # Route to get all the shelves from the database, in order to display them in the home page
def shelves():

    try:
        shelves = Shelf.query.all()
        current_app.logger.info(f" Retrieved {len(shelves)} shelves from the database.")
        shelves_list = []
        for shelf in shelves:

            cameras = Camera.query.filter_by(shelf_number=shelf.number).all()       # Get all the cameras associated with the shelf

            latest_camera = None
            latest_update = None
            latest_image_url = None

            for camera in cameras:                  # Get the latest image from the cameras associated with the shelf
                if camera.image_path:
                    image_full_path = os.path.join(current_app.config['BASE_UPLOAD_FOLDER'], f'shelf_{camera.shelf_number}', f'camera_{camera.id}', camera.image_path)
                    if os.path.exists(image_full_path):
                        try:
                            if camera.last_update:
                                timestamp = camera.last_update
                            else:
                                # Fallback se last_update non è disponibile
                                timestamp_str = camera.image_path.split('_')[1] + camera.image_path.split('_')[2]
                                timestamp = datetime.strptime(timestamp_str, "%Y%m%d%H%M%S")

                            if not latest_update or timestamp > latest_update:
                                latest_update = timestamp
                                latest_camera = camera
                                latest_image_url = url_for('static', filename=f'uploads/shelf_{camera.shelf_number}/camera_{camera.id}/{camera.image_path}')
                        except (IndexError, ValueError) as e:
                            current_app.logger.error(f" Error parsing timestamp from image path: {camera.image_path} - {e}")
                            continue

            shelves_list.append({                   # Append the shelf data to the shelves_list
                "number": shelf.number,
                "description": shelf.description,
                "image": latest_image_url if latest_image_url else url_for('static', filename='images/placeholder.jpg'),  # If there is no image, use a placeholder
                "lastUpdate": latest_update.strftime("%Y-%m-%d %H:%M:%S") if latest_update else "N/A"
            })

        current_app.logger.info(f" Returning {len(shelves_list)} shelves as JSON.")
        return jsonify(shelves_list)
    except Exception as e:
        current_app.logger.error(f" Error in /shelves route: {e}")
        return jsonify({"error": "Internal Server Error"}), 500
                        
@main.route('/shelves/<number>')                                    # Route to get the details of a specific shelf (when a shelf is clicked)
def shelf_details(number):

    shelf = Shelf.query.filter_by(number=number).first()
    if not shelf:
        return "Shelf not found", 404

    # Fetch the cameras associated with the shelf
    cameras = Camera.query.filter_by(shelf_number=number).all()
    last_update_camera, last_update = get_latest_update(cameras)

    # Fetch the products associated with the shelf
    products = db.session.query(Product, Product_Shelf.quantity).join(Product_Shelf).filter(Product_Shelf.shelf_number == number).all()

    return render_template('shelf_details.html', shelf=shelf, cameras=cameras, products=products, last_update_camera=last_update_camera, last_update=last_update)                             

@main.route('/upload', methods=['POST'])        # Route to upload an image from a POST request by the ESP32 camera
def upload_image():
    
    if 'file' not in request.files:                         # Check if the request contains a file part
        print("No file part")
        return jsonify({"error": "No file part"}), 400

    file = request.files['file']

    if file.filename == '':                                 # Check if the file name is empty
        print("No selected file")
        return jsonify({"error": "No selected file"}), 400
    
    camera_id = request.form.get('camera_id')               # Get the camera_id from the request in order to save the image in the correct folder
    if not camera_id:
        current_app.logger.error("No camera_id provided")
        return jsonify({"error": "No camera_id provided"}), 400

    try:
        camera = Camera.query.get(int(camera_id))          # Get the camera row from the database with the provided camera_id
        if not camera:
            current_app.logger.error(f"Camera with id {camera_id} not found")
            return jsonify({"error": " Camera not found"}), 404
    except ValueError:
        current_app.logger.error(" Invalid camera_id format")
        return jsonify({"error": "Invalid camera_id"}), 400

    if file:

        shelf_id = camera.shelf_number                                                                         # Get the shelf of the camera in order to put the image in the correct folder
        shelf_folder = os.path.join(current_app.config['BASE_UPLOAD_FOLDER'], f'shelf_{shelf_id}')       # Create the shelf folder if it doesn't exist
        camera_folder = os.path.join(shelf_folder, f'camera_{camera.id}')
        os.makedirs(camera_folder, exist_ok=True)

        if camera.image_path:                                                                        # If the camera already has an image, delete it (we only need the last image)
            existing_image_path = os.path.join(camera_folder, camera.image_path)
            if os.path.exists(existing_image_path):
                os.remove(existing_image_path)


        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")                           # Get the current timestamp
        filename = f'photo_{timestamp}_{camera_id}.jpg'                                # Create a filename with the timestamp and camera_id
        file_path = os.path.join(camera_folder, filename)                              # Path to save the uploaded file in the camera's folder
        try:
            file.seek(0, os.SEEK_END)
            file_length = file.tell()
            file.seek(0)                                                               # Reset the file pointer to the beginning
            current_app.logger.info(f" File size: {file_length} bytes")
            file.save(file_path)

            relative_path = filename                                  # Save the relative path to the image in the camera row
            camera.image_path = relative_path
            camera.last_update = datetime.now()                      # Update the last_update field with the current timestamp
            db.session.commit()

            current_app.logger.info(f" File {filename} uploaded successfully to {file_path}")
            return jsonify({"message": f" File uploaded successfully!"}), 200
        except Exception as e:
            current_app.logger.error(f" Failed to upload file {filename}: {e}")
            return jsonify({"error": "File upload failed"}), 500
        
@main.route('/add_shelf_form')                   # Route to display the form to add a new shelf
def add_shelf_form():

    cameras = Camera.query.filter_by(shelf_number=None).all()       # Extracting not connected cameras from the database
    # Loop in order to display available cameras to connect to the shelf
    available_cameras = []
    for camera in cameras:
        available_cameras.append({
            "id": camera.id,
            "description": camera.description
        })
    return render_template('add_shelf.html', available_cameras=available_cameras)

@main.route('/add_shelf', methods=['POST'])      # Route to add a new shelf to the database
def add_shelf():

    shelf_number = request.form.get('shelfNumber')          # Get the shelf number from the form
    shelf_name = request.form.get('shelfName')    # Get the shelf description from the form

    # Check if the form fields are empty
    if not shelf_number or not shelf_name:
        return jsonify({"error": "All fields are required"}), 400
    
    # Check if the shelf number already exists in the database
    shelf = Shelf.query.filter_by(number=shelf_number).first()    
    if shelf:
        return jsonify({"error": "Shelf already exists"}), 400
    
    # Extract cameras id from the form (every select box has a name formed by 'camera_id_' + camera_id)
    cameras_to_update = []
    areAllCamerasCHecked = False
    while areAllCamerasCHecked == False:
        camera_id = request.form.get(f'camera{len(cameras_to_update) + 1}')
        current_app.logger.info(f" Camera id: {camera_id} with index {len(cameras_to_update) + 1}")
        if camera_id:
            if camera_id != 'Select a camera':              # Check if the camera is selected
                cameras_to_update.append(camera_id)
        else:
            areAllCamerasCHecked = True
    
    new_shelf = Shelf(number=shelf_number, description=shelf_name)    # Create a new shelf object
    db.session.add(new_shelf)                                               # Add the new shelf to the session
    db.session.commit()

    current_app.logger.info(f" New shelf has {len(cameras_to_update)} cameras connected to it.")
    # Update the cameras with the new shelf number
    for camera_id in cameras_to_update:
        camera = Camera.query.get(int(camera_id))
        camera.shelf_number = shelf_number
        db.session.commit()

    return render_template('home.html')                                      # Redirect to the home page

@main.route('/update_shelf_form/<number>')        # Route to display the form to update a shelf
def update_shelf_form(number):

    shelf = Shelf.query.filter_by(number=number).first()        # Get the shelf from the database
    if not shelf:
        return "Shelf not found", 404

    # Extracting not connected cameras to any shelf
    cameras = Camera.query.filter_by(shelf_number=None).all()
    available_cameras = []
    for camera in cameras:
        available_cameras.append({
            "id": camera.id,
            "description": camera.description
        })

    # Extracting cameras connected to the shelf
    cameras = Camera.query.filter_by(shelf_number=number).all()
    connected_cameras = []
    for camera in cameras:
        connected_cameras.append({
            "id": camera.id,
            "description": camera.description
        })

    return render_template('update_shelf.html', shelf=shelf, available_cameras=available_cameras, connected_cameras=connected_cameras)

@main.route('/update_shelf/<number>', methods=['POST'])
def update_shelf(number):
    shelf = Shelf.query.filter_by(number=number).first()
    if not shelf:
        return "Shelf not found", 404

    # Update the shelf description and number
    shelf.description = request.form['shelfName']
    shelf.number = request.form['shelfNumber']
    current_app.logger.info(f" Updating shelf {shelf.number} with name {shelf.description}")

    # Remove cameras disconnected from the shelf
    remove_cameras_json = request.form.get('removeCameras', '[]')
    try:
        remove_cameras = json.loads(remove_cameras_json)            # JavaScript code responds with JSON format data. Convert JSON string to list
        current_app.logger.info(f" Removing {len(remove_cameras)} cameras from shelf {shelf.number}")
        for camera_id in remove_cameras:
            camera = Camera.query.filter_by(id=int(camera_id)).first()
            if camera:
                camera.shelf_number = None  # Disconnect the camera from the shelf
    except ValueError as e:
        return f"Error processing camera removal: {e}", 400

    # Add new cameras
    new_cameras = request.form.getlist('availableCameras')
    for camera_id in new_cameras:
        camera = Camera.query.filter_by(id=camera_id).first()
        if camera:
            camera.shelf_number = shelf.number      # Connects the camera to the shelf

    db.session.commit()
    return render_template('home.html')                                      # Redirect to the home page
