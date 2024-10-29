# Use the official Eclipse Mosquitto image from the Docker Hub
FROM eclipse-mosquitto:latest

# Copy the custom configuration file to the mosquitto directory
COPY config/mosquitto.conf /mosquitto/config/mosquitto.conf

# Copy the password file to the mosquitto directory
COPY config/pwfile /mosquitto/config/pwfile

# Impost the permissions of the configuration file and the password file
RUN chmod 644 /mosquitto/config/mosquitto.conf /mosquitto/config/pwfile

# Run the Mosquitto broker with the custom configuration file
CMD ["/usr/sbin/mosquitto", "-c", "/mosquitto/config/mosquitto.conf"]