document.addEventListener("DOMContentLoaded", function () {
    const editButton = document.getElementById("editBtn");
    const backButton = document.getElementById("backBtn");
    const deleteButton = document.getElementById("deleteBtn");

    // Funzione per tornare alla home
    backButton.addEventListener("click", () => {
        window.location.href = "/home";
    });

    // Funzione per modificare lo scaffale
    editButton.addEventListener("click", () => {
        const shelfNumber = deleteButton.dataset.shelfNumber;
        window.location.href = `/update_shelf_form/${shelfNumber}`;
    });

    // Funzione per eliminare lo scaffale
    deleteButton.addEventListener("click", () => {
        const shelfNumber = deleteButton.dataset.shelfNumber;
        if (confirm("Are you sure you want to delete this shelf?")) {
            fetch(`/delete_shelf/${shelfNumber}`, {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify({ shelfNumber: shelfNumber }),
            })
            .then((response) => {
                if (response.ok) {
                    alert("Shelf deleted successfully.");
                    window.location.href = "/home";
                } else {
                    alert("Failed to delete the shelf.");
                }
            })
            .catch((error) => console.error("Error:", error));
        }
    });
});
