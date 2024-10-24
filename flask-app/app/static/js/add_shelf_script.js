document.addEventListener("DOMContentLoaded", function() {
    const availableCameras = JSON.parse(document.getElementById('cameraData').dataset.cameras);     // Get available cameras from the data attribute passed from the server
    let selectedCameras = [];
    var numberOfAdditionalSelectBoxes = 1;

    document.querySelector('.camera-select').addEventListener('change', function(event) {           // Add event listener to the first select box to add new select boxes
        const selectedCameraId = event.target.value;
        if (selectedCameraId && !selectedCameras.includes(selectedCameraId)) {                      // Check if a camera has been selected and it is not already selected
            selectedCameras.push(selectedCameraId);
            addNewCameraChoice();
        }
    });

    // Function to add a new select box for selecting a camera when at least the first camera has been selected
    function addNewCameraChoice() {
        
        const cameraSelectContainer = document.getElementById('additionalCamerasContainer');        

        // Create a new select element
        const newSelect = document.createElement('select');
        newSelect.classList.add('form-select', 'mb-3');
        newSelect.name = `camera${numberOfAdditionalSelectBoxes + 1}`;          // Set the name attribute of the select box to camera1, camera2, camera3, etc.

        // Create an empty option
        const emptyOption = document.createElement('option');
        emptyOption.value = '';
        emptyOption.textContent = 'Select a camera';
        newSelect.appendChild(emptyOption);

        // Add remaining available cameras to the new select box
        availableCameras.forEach(camera => {
            if (!selectedCameras.includes(camera.id.toString())) {
                const option = document.createElement('option');
                option.value = camera.id;
                option.textContent = `Camera ${camera.id}`;
                newSelect.appendChild(option);
            }
        });

        // Append the new select box to the container
        cameraSelectContainer.appendChild(newSelect);

        // Attach change event listener to the new select box
        newSelect.addEventListener('change', function(event) {
            const selectedCameraId = event.target.value;
            if (selectedCameraId && !selectedCameras.includes(selectedCameraId)) {
                selectedCameras.push(selectedCameraId);
                addNewCameraChoice();
            }
        });
        numberOfAdditionalSelectBoxes++;        // Increment the number of additional select boxes
    }
});

document.getElementById('backBtn').addEventListener('click', () => {
    window.location.href = '/home';
});
