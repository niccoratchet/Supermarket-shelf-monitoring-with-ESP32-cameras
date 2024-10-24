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
        event.preventDefault();                                     // Prevent browser default form submission in order to handle dele

        const formData = new FormData(updateShelfForm);             // Create a FormData object to extract form data from the HTML form

        // Add cameras to be removed to form data
        formData.append('removeCameras', JSON.stringify(camerasToRemove));

        // Collect available cameras to be added (checked boxes)
        const checkedCameras = availableCamerasList.querySelectorAll('input[name="availableCameras"]:checked');
        checkedCameras.forEach(camera => {
            formData.append('availableCameras', camera.value);
        });

        // Send the form data to the server via Fetch API
        fetch(`/update_shelf/${document.getElementById("shelfNumber").value}`, {
            method: "POST",
            body: formData
        })
        .then(response => {
            if (response.ok) {
                window.location.href = '/home';  // Redirect to home on success
            } else {
                alert("Error updating shelf.");
            }
        })
        .catch(error => {
            console.error('Error:', error);
            alert("There was a problem updating the shelf.");
        });
    });
});
