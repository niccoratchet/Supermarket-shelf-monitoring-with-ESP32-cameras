from flask import Blueprint, jsonify
from app.models import Shelf

main = Blueprint('main', __name__)      # Create a Blueprint object

@main.route('/')
def home():
    return "Hello, Flask with MQTT!"

@main.route('/shelves')
def shelves():
    shelves = Shelf.query.all()
    results = [{"number": shelf.number, "description": shelf.description} for shelf in shelves]
    return jsonify(results)
