document.addEventListener("DOMContentLoaded", function() {
    const connectedCamerasList = document.getElementById("connectedCamerasList");
    const availableCamerasList = document.getElementById("availableCamerasList");
    const updateShelfForm = document.getElementById("updateShelfForm");

    // To store IDs of cameras to be removed
    let camerasToRemove = [];

    // Handle removal of connected cameras
    connectedCamerasList.addEventListener("click", function(event) {
        if (event.target.classList.contains("remove-camera")) {
            const cameraId = event.target.dataset.id;
            
            // Remove camera from UI
            event.target.parentElement.remove();

            // Add camera ID to the list of cameras to remove
            camerasToRemove.push(cameraId);
        }
    });

    // Intercept form submission to handle camera updates
    updateShelfForm.addEventListener("submit", function(event) {
        event.preventDefault();                                     // Prevent browser default form submission in order to handle it with this script

        const formData = new FormData(updateShelfForm);             // Create a FormData object to extract form data from the HTML form

        // Add cameras to be removed to form data
        formData.append('removeCameras', JSON.stringify(camerasToRemove));

        // Collect available cameras to be added (checked boxes)
        const checkedCameras = availableCamerasList.querySelectorAll('input[name="availableCameras"]:checked');
        checkedCameras.forEach(camera => {
            formData.append('availableCameras', camera.value);
        });

        // Extract shelf number from headerTitle element excluding the Update Shelf - 
        const headerElement = document.getElementById("headerTitle");
        const headerText = headerElement.innerText;
        const match = headerText.match(/-\s*(.+)/);
        const actualShelfNumber = match ? match[1] : null;
        console.log(actualShelfNumber);

        // Send the form data to the server via Fetch API
        fetch(`/update_shelf/${actualShelfNumber}`, {
            method: "POST",
            body: formData
        })
        .then(response => {
        if (response.ok) {
            window.location.href = '/home';  // Redirect to home on success
        } else {
            return response.json().then(data => {
                alert(data.error);                  // Display error message if server returns one
            });
        }
})
        .catch(error => {
            console.error('Error:', error);
            alert("There was a problem updating the shelf.");
        });
    });
});
