# SUPERMARKET SHELVES MONITORING (with ESP32 cameras)

Questa repository contiene il codice creato dallo studente Redi Niccolo' per il conseguimento dell'esame di Progettazione e Produzione Multimediale (PPM). Il corso è stato svolto presso l'Università degli Studi di Firenze ed e' diretto dal Prof. Bertini.  
Il progetto ha il nome di "Supermarket shelves monitoring" ed è infatti un'applicazione web per il monitoraggio da remoto degli scaffali di un supermercato utilizzando dispositivi embedded a basso costo basati sulla collaudata piattaforma ESP32.  
Per poter funzionare, l'applicazione è composta da tre parti principali: <br />
1. Un programma C/C++ che gira sul dispositivo basato su ESP32 (per i test è stata utilizzata la Timer Camera F di M5Stack). A periodi di tempo prestabiliti, il dispositivo effettua il riconoscimento degli oggetti sulla foto scattata grazie ad un
   modello ML. Il modello è stato creato grazie al framework online Edge Impulse (https://edgeimpulse.com/) ed è stato allenato con degli oggetti che potenzialmente possono essere presenti su uno scaffale (ad esempio una confezione di panna da cucina).
   I risultati dell'inferenza verrano poi trasmessi tramite protocollo MQTT su un MQTT Broker. I messaggi contengono le informazioni su quali e quanti prodotti sono presenti e il grado di confidenza con cui il modello afferma la loro presenza (i risultati con un tasso di confidenza inferiore all'85% non vengano inviati al broker).  
2. Un back-end sul quale viene eseguito un webserver creato grazie al framework Flask che si occuperà sia di gestire le richieste dei front-end (vedi punto successivo) sia della ricezione dei messaggi MQTT e delle foto usate per il riconoscimento tramite endpoint REST. 
   Grazie all'utilizzo di un DB relazionale, il backend tiene traccia dello stato di: scaffali, prodotti e camere.
3. Un front-end che, tramite una semplice interfaccia, fornisce le informazioni sullo stato di tutti gli scaffali monitorati. Grazie alle informazioni inviate dal back-end, viene mostrato per ogni scaffale quali e quanti prodotti sono presenti insieme all'ultima foto 
   ricevuta dall'insieme di camere potenzialmente legate ad uno scaffale.

Per il backend vengono utilizzati 3 container Docker che gestiscono 3 servizi distinti. In particolare abbiamo il container:
1. Flask WebServer
2. PostGreSQL
3. pgAdmin (per gestire il DB attraverso un'interfaccia web)

La parte frontend è stata invece sviluppata scrivendo codice HTML, CSS e JS grazie soprattutto all'aiuto della libreria Bootstrap.
