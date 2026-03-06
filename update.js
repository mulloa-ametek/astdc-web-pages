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

      if (!fileInput || !fileInput.files || fileInput.files.length === 0) {
          setStatus("Please select a file first.", true);
          debugLog("warn", "No file selected");
          return;
      }

      var file = fileInput.files[0];

      var formData = new FormData();
      // Keep filename explicit (helps some CGI parsers)
      formData.append("fw_file", file, file.name);

      uploadButton.disabled = true;
      setStatus("Uploading...", false);

      debugLog("info", "UploadFile clicked", {
          name: file.name,
          size_bytes: file.size,
          type: file.type || "(empty)",
          page: window.location.href
      });

      var url = "/cgi-bin/upload.cgi";
      var t0 = Date.now();

      fetch(url, {
          method: "POST",
          body: formData,
          cache: "no-store",
          // credentials: "same-origin" // uncomment if you rely on cookies
      })
          .then(function (response) {
              var dt = Date.now() - t0;
              debugLog("info", "Fetch response received", {
                  url: url,
                  ok: response.ok,
                  status: response.status,
                  statusText: response.statusText,
                  duration_ms: dt
              });

              // Log headers (useful for CGI debugging)
              try {
                  var headersObj = {};
                  response.headers.forEach(function (v, k) { headersObj[k] = v; });
                  debugLog("info", "Response headers", headersObj);
              } catch (e) {
                  debugLog("warn", "Could not read response headers", String(e));
              }

              return response.text().then(function (text) {
                  debugLog("info", "Response body (first 800 chars)", truncate(text, 800));

                  if (!response.ok) {
                      // Include server body in error message
                      throw new Error(text || ("HTTP " + response.status));
                  }
                  return text;
              });
          })
          .then(function (text) {
              setStatus(text, false);
              debugLog("info", "Upload completed successfully");
          })
          .catch(function (error) {
              // Network failures often show as TypeError in fetch
              debugLog("error", "Upload failed", {
                  message: error && error.message,
                  name: error && error.name,
                  stack: error && error.stack
              });
              setStatus("Upload failed: " + (error && error.message ? error.message : String(error)), true);
          })
          .finally(function () {
              uploadButton.disabled = false;
              debugLog("info", "Upload button re-enabled");
          });
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