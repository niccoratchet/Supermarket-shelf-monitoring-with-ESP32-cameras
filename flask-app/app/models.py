from config import db

class Shelf(db.Model):          # This class represents the Shelf table in the database
    __tablename__ = 'shelf'
    number = db.Column(db.Text, primary_key=True)
    description = db.Column(db.Text, nullable=True)
    def __repr__(self):
        return f'<Shelf {self.number} - {self.description}>'
    
class Camera(db.Model):         # This class represents the Camera table in the database
    __tablename__ = 'camera'
    id = db.Column(db.Integer, primary_key=True)
    shelf_number = db.Column(db.Text, db.ForeignKey('shelf.number'), nullable=False)
    description = db.Column(db.Text, nullable=True)
    def __repr__(self):
        return f'<Camera {self.id} - {self.shelf_number} - {self.description}>'
    
class Product(db.Model):        # This class represents the Product table in the database
    __tablename__ = 'product'
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.Text, nullable=False)
    quantity = db.Column(db.Integer, nullable=False, default=0)
    category = db.Column(db.Text, nullable=False)
    def __repr__(self):
        return f'<Product {self.id} - {self.name} - {self.category_name}>'

class Camera_Product(db.Model):  # This class represents the Camera_Product table in the database
    __tablename__ = 'camera_product'
    camera_id = db.Column(db.Integer, db.ForeignKey('camera.id'), primary_key=True)
    product_id = db.Column(db.Integer, db.ForeignKey('product.id'), primary_key=True)
    quantity = db.Column(db.Integer, nullable=False, default=0)
    def __repr__(self):
        return f'<Camera_Product {self.camera_id} - {self.product_id}>'

class Product_Shelf(db.Model):  # This class represents the Product_Shelf table in the database
    __tablename__ = 'product_shelf'
    product_id = db.Column(db.Integer, db.ForeignKey('product.id'), primary_key=True)
    shelf_number = db.Column(db.Text, db.ForeignKey('shelf.number'), primary_key=True)
    quantity = db.Column(db.Integer, nullable=False, default=0)
    def __repr__(self):
        return f'<Product_Shelf {self.product_id} - {self.shelf_number}>'