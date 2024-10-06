// script.js

// Shelves rows content simulation
const shelvesData = [
    {
        id: 1,
        name: "Scaffale A",
        image: "https://via.placeholder.com/300x200", // Example image
        lastUpdate: "2024-08-26 14:30"
    },
    {
        id: 2,
        name: "Scaffale B",
        image: "https://via.placeholder.com/300x200",
        lastUpdate: "2024-08-26 14:35"
    },
    {
        id: 3,
        name: "Scaffale C",
        image: "https://via.placeholder.com/300x200",
        lastUpdate: "2024-08-26 14:40"
    }
];

// Used to create HTML elements for each shelf using Bootstrap cards
function renderShelves() {
    const shelvesContainer = document.getElementById('shelvesContainer');
    shelvesData.forEach(shelf => {
        const col = document.createElement('div');
        col.className = 'col-md-4';

        const card = document.createElement('div');
        card.className = 'card h-100';

        const img = document.createElement('img');
        img.src = shelf.image;
        img.className = 'card-img-top';
        img.alt = `Immagine di ${shelf.name}`;

        const cardBody = document.createElement('div');
        cardBody.className = 'card-body d-flex flex-column';

        const cardTitle = document.createElement('h5');
        cardTitle.className = 'card-title';
        cardTitle.textContent = shelf.name;

        const cardText = document.createElement('p');
        cardText.className = 'card-text mt-auto';
        cardText.textContent = `Ultimo aggiornamento: ${shelf.lastUpdate}`;

        cardBody.appendChild(cardTitle);
        cardBody.appendChild(cardText);

        card.appendChild(img);
        card.appendChild(cardBody);
        col.appendChild(card);
        shelvesContainer.appendChild(col);
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
