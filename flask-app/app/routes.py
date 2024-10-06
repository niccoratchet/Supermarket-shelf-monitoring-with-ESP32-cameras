from flask import Blueprint, jsonify, request, current_app, render_template
from app.models import Shelf, Camera
from datetime import datetime
from . import db
import os

main = Blueprint('main', __name__)      # Create a Blueprint object

@main.route('/home')
def home():
    return render_template('home.html')

@main.route('/shelves')
def shelves():
    shelves = Shelf.query.all()
    results = [{"number": shelf.number, "description": shelf.description} for shelf in shelves]
    return jsonify(results)

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
            db.session.commit()

            current_app.logger.info(f" File {filename} uploaded successfully to {file_path}")
            return jsonify({"message": f"File uploaded successfully!"}), 200
        except Exception as e:
            current_app.logger.error(f" Failed to upload file {filename}: {e}")
            return jsonify({"error": "File upload failed"}), 500
