# Usa un'immagine base di Python
FROM python:3.9-slim

# Imposta la directory di lavoro nel container
WORKDIR /app

# Copia il file dei requisiti e installa le dipendenze
COPY requirements.txt requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

# Copia il codice sorgente nel container
COPY . .

# Espone la porta 5000 per Flask
EXPOSE 5000

# Comando per avviare l'app Flask
CMD ["python", "app.py"]