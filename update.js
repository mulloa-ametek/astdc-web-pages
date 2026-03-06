(function () {
    "use strict";

    function byId(id) {
        return document.getElementById(id);
    }

    function setStatus(message, isError) {
        var status = byId("upload_status");
        if (!status) {
            return;
        }
        status.className = isError ? "status_error" : "status_ok";
        status.textContent = message;
    }

    function uploadFile() {
        var fileInput = byId("fw_file");
        var uploadButton = byId("upload_button");

        if (!fileInput || !fileInput.files || fileInput.files.length === 0) {
            setStatus("Please select a file first.", true);
            return;
        }

        var formData = new FormData();
        formData.append("fw_file", fileInput.files[0]);

        uploadButton.disabled = true;
        debugLog("info", "UploadFile clicked");
        setStatus("Uploading...", false);

        fetch("/cgi-bin/upload.py", {
            method: "POST",
            body: formData,
            cache: "no-store"
        })
            .then(function (response) {
                return response.text().then(function (text) {
                    if (!response.ok) {
                        throw new Error(text || ("HTTP " + response.status));
                    }
                    return text;
                });
            })
            .then(function (text) {
                setStatus(text, false);
            })
            .catch(function (error) {
                setStatus("Upload failed: " + error.message, true);
            })
            .finally(function () {
                uploadButton.disabled = false;
            });
    }

    window.addEventListener("load", function () {
        var uploadButton = byId("upload_button");
        if (uploadButton) {
            uploadButton.addEventListener("click", uploadFile);
        }
    });
}());
