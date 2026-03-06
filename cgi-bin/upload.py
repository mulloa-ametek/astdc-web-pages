#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys
from datetime import datetime

cgitb.enable()

UPLOAD_DIR = os.environ.get("UPLOAD_DIR", "/tmp/web_uploads")
FIELD_NAME = "fw_file"


def respond(status_code, message):
    print(f"Status: {status_code}")
    print("Content-Type: text/plain")
    print("Cache-Control: no-store")
    print()
    print(message)


def safe_name(name):
    base = os.path.basename(name or "")
    if not base:
        return None
    return base.replace("\x00", "")


def main():
    method = os.environ.get("REQUEST_METHOD", "")
    if method != "POST":
        respond("405 Method Not Allowed", "Use POST.")
        return

    form = cgi.FieldStorage()

    if FIELD_NAME not in form:
        respond("400 Bad Request", "Missing file field 'fw_file'.")
        return

    file_item = form[FIELD_NAME]
    if not getattr(file_item, "filename", None):
        respond("400 Bad Request", "No file selected.")
        return

    filename = safe_name(file_item.filename)
    if not filename:
        respond("400 Bad Request", "Invalid filename.")
        return

    os.makedirs(UPLOAD_DIR, exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    dest_path = os.path.join(UPLOAD_DIR, f"{timestamp}_{filename}")

    total = 0
    with open(dest_path, "wb") as out_file:
        while True:
            chunk = file_item.file.read(65536)
            if not chunk:
                break
            out_file.write(chunk)
            total += len(chunk)

    respond("200 OK", f"Saved {total} bytes to {dest_path}")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        respond("500 Internal Server Error", f"Upload failed: {exc}")
