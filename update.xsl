<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:lxi="http://www.lxistandard.org/InstrumentIdentification/1.0">

	<!--************************************************************************************
	****	Title:			update.xsl																		****
	****	Description:	Converts the lxi device info xml file into the login page		****
	****																											****
	****	Copyright (c) AMETEK Programmable Power, 2014											****
	*************************************************************************************-->

	<!-- This will be an xhtml 1.0 Strict Document -->
  <xsl:output method="html" version="1.0" encoding="UTF-8" standalone="no"
      doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
      doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
      indent="yes"/>

  <xsl:template match="lxi:LXIDevice">
		<!-- Header for the page -->
		<xsl:variable name="model" select="normalize-space(lxi:Model[last()])"/>
		<xsl:variable name="manufacturer" select="normalize-space(lxi:Manufacturer[last()])"/>
		<xsl:comment>*******************************************************</xsl:comment>
		<xsl:comment>Title: cgi-bin/login.cgi</xsl:comment>
		<xsl:comment>Description: This is the LXI Login Page</xsl:comment>
		<xsl:comment>Copyright (c) AMETEK Programmable Power, 2014</xsl:comment>
		<xsl:comment>*******************************************************</xsl:comment>

		<!-- start of the page -->
		<html xmlns="http://www.w3.org/1999/xhtml">
			<head>
				<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
				<!-- Generate a title -->
	
        <link rel="icon" href="/Images/AMETEK.ico" type="image/x-icon"/>
        <link rel="stylesheet" type="text/css" href="/lxi_page_layout.css?v=1"/>
        <link rel="stylesheet" type="text/css" href="/update_layout.css?v=1"/>

        <script type="text/javascript" src="/jquery-1.11.1.min.js"></script>
        <script type="text/javascript" src="/update.js?v=1"></script>
      </head>

      <body>
        <div id="header">
					<div id="product_id_bar">
						<span id="product_id_text">
							<xsl:value-of select="$model"/>
						</span>
						<!-- Only flash the AMETEK Logo if AMETEK is listed as the manufacturer -->
						<xsl:if test="$manufacturer='AMETEK'">
							<img id="AMETEK_logo" src="/Images/AMETEK_Programmable_Logo.png" alt="AMETEK Programmable Power"/>
						</xsl:if>
					</div>	
					<div id="device_id_bar">
						<span id="AMETEK_text_identifier">
							<xsl:call-template name="manufacturer_full"/>
						</span>
						<span id="device_text_identifier">
							<xsl:call-template name="formatted_name"/>
						</span>
					</div>
					<div id="button_div">
            <ul id="button_list">
              <li id="home_button_element"><a id="home_button" class="header_button" href="/cgi-bin/home.cgi">Home</a></li>
              <li id="login_button_element"><a id="login_button" class="header_button" href="/cgi-bin/login.cgi">Login</a></li>
              <li id="interactive_control_element"><a id="interactive_control_button" class="header_button" href="/cgi-bin/interactive.cgi">Interactive</a></li>
              <li id="lxi_identification_element"><a id="lxi_identification_button" class="header_button" href="/cgi-bin/identification">LXI Identification</a></li>
              <li id="update_element"><a id="update_button" class="selected_header_button" href="/cgi-bin/upload.cgi">Update</a></li>
            </ul>
          </div>
        </div>

				<!-- Unique content for the page goes here -->
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
				<!-- End of page unique content -->
				<!-- This is the footer -->
				<xsl:comment>Bottom bar that displays the copyright data</xsl:comment>
				<div id="footer">
					<span id="copyright_notice">
						<xsl:text>Copyright &#169; </xsl:text>
						<xsl:call-template name="manufacturer_full"/>
						<xsl:text> 2014</xsl:text>
					</span>
					<!-- If this is an AMETEK supply then add an additional notice -->
					<!-- <xsl:if test="$manufacturer='AMETEK'">
						<span id="device_copyright">
							<xsl:call-template name="short_model_name"/>
							<xsl:text> is a trademark of </xsl:text>
							<xsl:call-template name="manufacturer_full"/>
						</span>
					</xsl:if> -->
				</div>
		</div>
	</body>
		</html>
	</xsl:template>

	<!-- Convert an AMETEK name to AMETEK Programmable Power -->
	<xsl:template name="manufacturer_full">
		<xsl:variable name="manufacturer" select="lxi:Manufacturer[last()]"/>
		<xsl:choose>
			<xsl:when test="$manufacturer = 'AMETEK'">
				<xsl:text>AMETEK Programmable Power</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$manufacturer"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- Removes the voltage and current from the model name -->
	<xsl:template name="short_model_name">
		<xsl:variable name="model_name" select="normalize-space(lxi:Model[last()])"/>

		<xsl:choose>
			<xsl:when test="contains($model_name, ' ')">
				<xsl:value-of select="substring-before($model_name, ' ')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$model_name"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- Extracts the voltage and current out of the model name -->
	<xsl:template name="voltage_current">
		<xsl:variable name="model_name" select="normalize-space(lxi:Model[last()])"/>

		<xsl:choose>
			<xsl:when test="contains($model_name, ' ')">
				<xsl:value-of select="substring-after($model_name, ' ')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$model_name"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- Strips out the " SN# " string out of the serial number -->
	<xsl:template name="serial_number">
		<xsl:variable name="serial_number" select="normalize-space(lxi:SerialNumber[last()])"/>

		<xsl:choose>
			<xsl:when test="starts-with($serial_number, 'SN# ')">
				<xsl:value-of select="substring-after($serial_number, 'SN# ')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$serial_number"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- Generates the formatted name -->
	<xsl:template name="formatted_name">
		<xsl:param name="final_text" select="''"/>

		<xsl:text>LXI-</xsl:text>
		<xsl:call-template name="short_model_name"/>
		<xsl:text>-</xsl:text>
		<xsl:call-template name="serial_number"/>
		<xsl:if test="$final_text != ''">
			<xsl:text>-</xsl:text>
			<xsl:value-of select="$final_text"/>
		</xsl:if>
	</xsl:template>
</xsl:stylesheet>
