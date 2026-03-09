(function () {
  "use strict";

  // Toggle debug output
  var DEBUG = true;

  function byId(id) {
      return document.getElementById(id);
  }

  // Optional: add <pre id="debug_log"></pre> to your HTML to view logs on-page
  function debugLog(level, message, obj) {
      if (!DEBUG) return;

      var prefix = "[upload] ";
      var ts = new Date().toISOString();
      var line = ts + " " + prefix + (level || "info").toUpperCase() + ": " + message;

      // Console output
      try {
          if (obj !== undefined) {
              if (level === "error") console.error(line, obj);
              else if (level === "warn") console.warn(line, obj);
              else console.log(line, obj);
          } else {
              if (level === "error") console.error(line);
              else if (level === "warn") console.warn(line);
              else console.log(line);
          }
      } catch (e) {
          // ignore console failures
      }

      // On-page output (if present)
      var pre = byId("debug_log");
      if (pre) {
          var extra = "";
          if (obj !== undefined) {
              try {
                  extra = " | " + (typeof obj === "string" ? obj : JSON.stringify(obj));
              } catch (e2) {
                  extra = " | [unserializable]";
              }
          }
          pre.textContent += line + extra + "\n";
          pre.scrollTop = pre.scrollHeight;
      }
  }

  function setStatus(message, isError) {
      var status = byId("upload_status");
      if (!status) return;
      status.className = isError ? "status_error" : "status_ok";
      status.textContent = message;
  }

  function truncate(text, maxLen) {
      if (text == null) return "";
      text = String(text);
      return text.length > maxLen ? text.slice(0, maxLen) + "…(truncated)" : text;
  }

  function uploadFile() {
    var fileInput = byId("fw_file");
    var uploadButton = byId("upload_button");
    var progressBar = byId("progress_bar");
    var progressText = byId("progress_text");

    if (!fileInput || !fileInput.files || fileInput.files.length === 0) {
        setStatus("Please select a file first.", true);
        debugLog("warn", "No file selected");
        return;
    }

    var file = fileInput.files[0];
    var formData = new FormData();
    formData.append("fw_file", file, file.name);

    uploadButton.disabled = true;
    setStatus("Uploading...", false);

    if (progressBar) {
        progressBar.style.width = "0%";
    }
    if (progressText) {
        progressText.textContent = "0%";
    }

    debugLog("info", "UploadFile clicked", {
        name: file.name,
        size_bytes: file.size,
        type: file.type || "(empty)",
        page: window.location.href
    });

    var url = "/cgi-bin/cgi_update.cgi";
    var t0 = Date.now();
    var xhr = new XMLHttpRequest();

    xhr.upload.addEventListener("progress", function (event) {
        if (!event.lengthComputable) {
            debugLog("warn", "Upload progress not length-computable");
            return;
        }

        var percent = Math.round((event.loaded / event.total) * 100);

        if (progressBar) {
            progressBar.style.width = percent + "%";
        }
        if (progressText) {
            progressText.textContent = percent + "%";
        }

        setStatus(
            "Uploading... " + percent + "% (" +
            event.loaded + " / " + event.total + " bytes)",
            false
        );
    });

    xhr.addEventListener("load", function () {
        var dt = Date.now() - t0;

        debugLog("info", "XHR load", {
            url: url,
            status: xhr.status,
            statusText: xhr.statusText,
            duration_ms: dt,
            response: xhr.responseText
        });

        if (xhr.status >= 200 && xhr.status < 300) {
            if (progressBar) {
                progressBar.style.width = "100%";
            }
            if (progressText) {
                progressText.textContent = "100%";
            }
            setStatus(xhr.responseText || "Upload completed.", false);
        } else {
            setStatus("Upload failed: " + (xhr.responseText || ("HTTP " + xhr.status)), true);
        }

        uploadButton.disabled = false;
    });

    xhr.addEventListener("error", function () {
        debugLog("error", "XHR network error");
        setStatus("Upload failed: network error", true);
        uploadButton.disabled = false;
    });

    xhr.addEventListener("abort", function () {
        debugLog("warn", "XHR aborted");
        setStatus("Upload cancelled.", true);
        uploadButton.disabled = false;
    });

    xhr.open("POST", url, true);
    xhr.send(formData);
}

  window.addEventListener("load", function () {
      debugLog("info", "Page loaded", {
          href: window.location.href,
          ua: navigator.userAgent
      });

      var uploadButton = byId("upload_button");
      if (uploadButton) {
          uploadButton.addEventListener("click", uploadFile);
          debugLog("info", "Upload handler attached to #upload_button");
      } else {
          debugLog("warn", "Upload button #upload_button not found");
      }
  });
}());