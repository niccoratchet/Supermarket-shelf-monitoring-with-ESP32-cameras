// script.js

// Function used to render shelves content in the page
function renderShelves(shelves) {
    const shelvesContainer = document.getElementById('shelvesContainer');
    shelvesContainer.innerHTML = '';                                        //  Clear the container before rendering new shelves

    console.log('Rendering shelves:', shelves);

    if (!Array.isArray(shelves)) {
        console.error('Shelves data is not an array:', shelves);
        alert('Error during shelves data rendering: data is not an array');
        return;
    }

    shelves.forEach(shelf => {
        const col = document.createElement('div');
        col.className = 'col-md-4';

        const card = document.createElement('div');
        card.className = 'card h-100';
        card.style.cursor = 'pointer'; // Set cursor to pointer to indicate it's clickable

        // Add event listener for the click
        card.addEventListener('click', () => {
            window.location.href = `/shelves/${shelf.number}`;              // Redirect to the details page for the specific shelf
        });

        const img = document.createElement('div');                          // Use a div instead of img
        img.className = 'card-img-top card-img-placeholder';                // Add a placeholder class

        if (!shelf.image.includes('placeholder')) {                            // Check if the image is not a placeholder
            img.style.backgroundImage = `url(${shelf.image})`;
        } else {
            img.textContent = 'No image available';
        }

        const cardBody = document.createElement('div');
        cardBody.className = 'card-body d-flex flex-column';

        const cardTitle = document.createElement('h5');
        cardTitle.className = 'card-title';
        cardTitle.textContent = 'Shelf n° ' + shelf.number;

        const cardText = document.createElement('p');
        cardText.className = 'card-text mt-auto';
        cardText.innerHTML = `<strong>Last update:</strong> ${shelf.lastUpdate}`;

        const cardDescription = document.createElement('p');
        cardDescription.className = 'card-text';
        cardDescription.innerHTML = `<strong>Products category:</strong> ${shelf.description}`;

        cardBody.appendChild(cardTitle);
        cardBody.appendChild(cardDescription);
        cardBody.appendChild(cardText);

        card.appendChild(img);
        card.appendChild(cardBody);
        col.appendChild(card);
        shelvesContainer.appendChild(col);
    });

    renderAddShelfCard();        // Render the card to add a new shelf

}

function renderAddShelfCard() {
    const col = document.createElement('div');      // Add col-md-4 class to make the card occupy 1/3 of the row
    col.className = 'col-md-4';

    const card = document.createElement('div');     // Add text-center class to center the content
    card.className = 'card h-100 text-center';
    card.style.cursor = 'pointer'; 

    const cardBody = document.createElement('div');         // Add d-flex, flex-column, justify-content-center and align-items-center classes to center the content
    cardBody.className = 'card-body d-flex flex-column justify-content-center align-items-center';
    cardBody.style.backgroundColor = '#f0f0f0';

    const icon = document.createElement('i');       // Add bi-plus-lg class to use the plus icon from Bootstrap Icons
    icon.className = 'bi bi-plus-lg';
    icon.style.fontSize = '48px';
    icon.style.color = '#6c757d';

    const text = document.createElement('p');           // Add mt-3 class to add margin top
    text.className = 'mt-3';
    text.textContent = 'Add shelf';
    text.style.color = '#6c757d';
    text.style.fontSize = '16px';

    cardBody.appendChild(icon);
    cardBody.appendChild(text);

    card.appendChild(cardBody);
    col.appendChild(card);

    // Add event listener for the click on the card to add a new shelf
    card.addEventListener('click', () => {
        window.location.href = `/add_shelf_form`;              // Redirect to the form to add a new shelf
    });

    shelvesContainer.appendChild(col);
}


// Function used to fetch shelves data from the backend using AJAX (fetch) 
async function fetchShelves() {
    try {
        const response = await fetch('/shelves');
        console.log('Fetch response status:', response.status);
        if (!response.ok) {
            const errorText = await response.text();
            console.error('Fetch error response:', errorText);
            throw new Error('Error during shelves data fetch');
        }
        const shelves = await response.json();
        console.log('Shelves data received:', shelves);
        renderShelves(shelves);
    }
    catch (error) {
        console.error('Error:', error);
        alert('Error during shelves data fetch');
    }
}

// Event listener for the logout button
document.addEventListener('DOMContentLoaded', function() {
    fetchShelves();
});
