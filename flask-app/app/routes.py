from flask import Blueprint, jsonify, request, current_app
from app.models import Shelf
import os

main = Blueprint('main', __name__)      # Create a Blueprint object

@main.route('/')
def home():
    return "Hello, Flask with MQTT!"

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

    if file:
        filename = file.filename
        file_path = os.path.join(current_app.config['UPLOAD_FOLDER'], filename)         # Create the file path
        try:
            file.save(file_path)
            current_app.logger.info(f" File {filename} uploaded successfully to {file_path}")
            return jsonify({"message": f" File {filename} uploaded successfully!"}), 200
        except Exception as e:
            current_app.logger.error(f"Failed to upload file {filename}: {e}")
            return jsonify({"error": "File upload failed"}), 500
