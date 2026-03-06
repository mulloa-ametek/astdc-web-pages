<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:lxi="http://www.lxistandard.org/InstrumentIdentification/1.0">

  <xsl:output method="html" version="1.0" encoding="UTF-8" standalone="no"
      doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
      doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
      indent="yes"/>

  <xsl:template match="lxi:LXIDevice">
    <html xmlns="http://www.w3.org/1999/xhtml">
      <head>
        <title>Firmware Update</title>

        <link rel="icon" href="/Images/AMETEK.ico" type="image/x-icon"/>
        <link rel="stylesheet" type="text/css" href="/lxi_page_layout.css?v=1"/>
        <link rel="stylesheet" type="text/css" href="/update_layout.css?v=1"/>

        <script type="text/javascript" src="/jquery-1.11.1.min.js"></script>
        <script type="text/javascript" src="/update.js?v=1"></script>
      </head>

      <body>
        <div id="header">
          <div id="header_container">
            <ul id="button_list">
              <li id="home_button_element"><a id="home_button" class="header_button" href="/cgi-bin/home.cgi">Home</a></li>
              <li id="login_button_element"><a id="login_button" class="header_button" href="/cgi-bin/login.cgi">Login</a></li>
              <li id="interactive_control_element"><a id="interactive_control_button" class="header_button" href="/cgi-bin/interactive.cgi">Interactive</a></li>
              <li id="lxi_identification_element"><a id="lxi_identification_button" class="header_button" href="/cgi-bin/identification">LXI Identification</a></li>
              <li id="update_element"><a id="update_button" class="selected_header_button" href="/cgi-bin/upload.cgi">Update</a></li>
            </ul>
          </div>
        </div>

        <div id="main_content">
          <h2>Firmware Update</h2>

          <div class="update_card">
            <label for="fw_file">Select firmware file:</label><br/>
            <input type="file" id="fw_file" name="fw_file" accept=".uImage,application/octet-stream"/><br/><br/>

            <button id="upload_button" type="button">Upload</button>

            <div id="status" class="status"></div>
            <div id="progress_bar_container">
              <div id="progress_bar"></div>
            </div>
          </div>
        </div>

      </body>
    </html>
  </xsl:template>
</xsl:stylesheet>
