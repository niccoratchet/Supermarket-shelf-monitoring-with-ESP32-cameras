# SUPERMARKET SHELF MONITORING (with ESP32 cameras)

Questa repository contiene il codice creato dallo studente Redi Niccolo' per il conseguimento dell'esame di Progettazione e Produzione Multimediale (PPM). Il corso è stato svolto presso l'Università degli Studi di Firenze ed e' diretto dal Prof. Bertini.  
Il progetto ha il nome di "Supermarket shelf monitoring" ed è infatti un'applicazione web per il monitoraggio da remoto degli scaffali di un supermercato utilizzando dispositivi embedded a basso costo basati sulla collaudata piattaforma ESP32.  
Per poter funzionare, l'applicazione è composta da tre parti principali: <br />
1. Un programma C/C++ che gira sul dispositivo basato su ESP32 (per i test è stata utilizzata la Timer Camera F di M5Stack). A periodi di tempo prestabiliti, il dispositivo effettua il riconoscimento degli oggetti sulla foto scattata grazie ad un
   modello ML. Il modello è stato creato grazie al framework online Edge Impulse (https://edgeimpulse.com/) ed è stato allenato con degli oggetti che potenzialmente possono essere presenti su uno scaffale (ad esempio una confezione di panna da cucina).
   I risultati dell'inferenza verrano poi trasmessi tramite protocollo MQTT su un MQTT Broker (server). I messaggi contengono le informazioni su quali e quanti prodotti sono presenti e il grado di confidenza con cui il modello assicura
   sulla loro presenza (non si accettano risultati con un tasso di confidenza inferiore all'85%).  
2. Un back-end che conterrà due parti principali: un'applicazione web creata grazie al framework Flask che si occuperà di gestire le richieste dei front-end (vedi punto successivo) e l'altra parte che si occupa di recepire i messaggi MQTT
   inviati dalle camere e quindi intepretando i risultati.
3. Un front-end che si occupa di fornire le informazioni all'utente sullo stato di tutti gli scaffali monitorati. Grazie infatti alle informazioni date dal back-end, e' possibile vedere per ogni scaffale quali e quanti prodotti sono presenti,
   la foto sulla quale si basa il riconoscimento e il tempo passato dall'ultimo aggiornamento.
