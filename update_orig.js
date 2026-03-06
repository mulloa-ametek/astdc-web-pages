// update.js
// Upload firmware to CGI, which forwards to TCP update daemon on port 5000.
// Endpoint: /cgi-bin/update_send.cgi  (CGI compiled from update/update.c)

(function () {
  function setStatus(msg) {
    var el = document.getElementById("status");
    if (el) el.textContent = msg;
  }

  function setProgress(pct) {
    var bar = document.getElementById("progress_bar");
    if (!bar) return;
    var p = Math.max(0, Math.min(100, pct));
    bar.style.width = p + "%";
  }

  function upload() {
    var input = document.getElementById("fw_file");
    if (!input || !input.files || input.files.length === 0) {
      setStatus("Select a file first.");
      return;
    }

    var file = input.files[0];
    var form = new FormData();
    form.append("fw_file", file, file.name);

    setStatus("Uploading...");
    setProgress(0);

    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/cgi-bin/update.cgi", true);

    xhr.upload.onprogress = function (e) {
      if (e.lengthComputable) {
        var pct = (e.loaded / e.total) * 100;
        setProgress(pct);
      }
    };

    xhr.onload = function () {
      if (xhr.status !== 200) {
        setStatus("HTTP error: " + xhr.status);
        return;
      }
      // update_send.cgi responds with: "OK <hex>" or "ERR <msg>"
      setStatus(xhr.responseText.trim());
      setProgress(100);
    };

    xhr.onerror = function () {
      setStatus("Network error.");
    };

    xhr.send(form);
  }

  window.addEventListener("load", function () {
    var btn = document.getElementById("upload_btn");
    if (btn) btn.addEventListener("click", upload);
  });
})();
