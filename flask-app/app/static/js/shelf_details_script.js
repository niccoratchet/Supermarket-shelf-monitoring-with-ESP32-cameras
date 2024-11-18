document.addEventListener("DOMContentLoaded", function () {
    const editButton = document.getElementById("editBtn");
    const backButton = document.getElementById("backBtn");
    const deleteButton = document.getElementById("deleteBtn");

    // Function to go back to the home page
    backButton.addEventListener("click", () => {
        window.location.href = "/home";
    });

    // Function to redirect to the form to update the shelf
    editButton.addEventListener("click", () => {
        const shelfNumber = deleteButton.dataset.shelfNumber;
        window.location.href = `/update_shelf_form/${shelfNumber}`;
    });

    // Function to delete the shelf
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
