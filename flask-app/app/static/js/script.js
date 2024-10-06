// Simulazione dei dati degli scaffali
const shelvesData = [
    {
        id: 1,
        name: "Scaffale A",
        image: "https://via.placeholder.com/100", // Immagine di esempio
        lastUpdate: "2024-08-26 14:30"
    },
    {
        id: 2,
        name: "Scaffale B",
        image: "https://via.placeholder.com/100", // Immagine di esempio
        lastUpdate: "2024-08-26 14:35"
    },
    {
        id: 3,
        name: "Scaffale C",
        image: "https://via.placeholder.com/100", // Immagine di esempio
        lastUpdate: "2024-08-26 14:40"
    }
];

// Funzione per creare gli elementi HTML per ogni scaffale
function renderShelves() {
    const shelvesContainer = document.getElementById('shelvesContainer');
    shelvesData.forEach(shelf => {
        const shelfElement = document.createElement('div');
        shelfElement.className = 'shelf';

        const shelfImage = document.createElement('img');
        shelfImage.src = shelf.image;
        shelfImage.alt = `Immagine di ${shelf.name}`;

        const shelfInfo = document.createElement('div');
        shelfInfo.className = 'shelf-info';

        const shelfName = document.createElement('h3');
        shelfName.textContent = shelf.name;

        const shelfLastUpdate = document.createElement('p');
        shelfLastUpdate.textContent = `Ultimo aggiornamento: ${shelf.lastUpdate}`;

        shelfInfo.appendChild(shelfName);
        shelfInfo.appendChild(shelfLastUpdate);

        shelfElement.appendChild(shelfImage);
        shelfElement.appendChild(shelfInfo);

        shelvesContainer.appendChild(shelfElement);
    });
}

// Funzione per gestire il logout
function handleLogout() {
    // Simulazione logout
    alert("Logout effettuato con successo!");
    // Reindirizzamento a una pagina di login (non implementata)
    // window.location.href = "login.html";
}

// Event Listener per il caricamento della pagina
document.addEventListener('DOMContentLoaded', function() {
    renderShelves();
    document.getElementById('logoutBtn').addEventListener('click', handleLogout);
});
