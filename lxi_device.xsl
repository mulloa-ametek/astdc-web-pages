<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:lxi="http://www.lxistandard.org/InstrumentIdentification/1.0">
	<!--************************************************************************************
	****	Title:			index.xsl																		****
	****	Description:	Converts the lxi device info xml file into the home page			****
	****																											****
	****	Copyright (c) AMETEK Programmable Power, 2014											****
	*************************************************************************************-->

	<!-- This will be an xhtml 1.0 Strict Document -->
	<xsl:output method="html" version="1.0" encoding="UTF-8" standalone="no"
			doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
			doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"/>

	<xsl:template match="lxi:LXIDevice">
		<!-- Extract some information that we will need shortly -->
		<xsl:variable name="manufacturer" select="normalize-space(lxi:Manufacturer[last()])"/>
		<xsl:variable name="model" select="normalize-space(lxi:Model[last()])"/>
		<xsl:comment>*******************************************************</xsl:comment>
		<xsl:comment>Title: cgi-bin/home.cgi</xsl:comment>
		<xsl:comment>Description: This is the LXI Welcome Page</xsl:comment>
		<xsl:comment>Copyright (c) AMETEK Programmable Power, 2014</xsl:comment>
		<xsl:comment>*******************************************************</xsl:comment>
		<html xmlns="http://www.w3.org/1999/xhtml">
		<head>
			<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
			<!-- Generate a title -->
			<title>
				<!-- This is the title format recommended by the LXI standard -->
				<xsl:text>LXI-</xsl:text>
				<xsl:call-template name="short_model_name"/>
				<xsl:text>-</xsl:text>
				<xsl:call-template name="serial_number"/>
				<xsl:text>-Home Page</xsl:text>
			</title>
			<link rel="shortcut icon" href="/Images/AMETEK.ico"/>
			<xsl:comment>This is a common stylesheet for all the pages</xsl:comment>
			<xsl:comment>[if lt IE 10]&gt;
				&lt;link href="/lxi_page_layout_ie.css" rel="stylesheet" type="text/css"/&gt;
				&lt;![endif]</xsl:comment>
			<xsl:comment>[if (!IE)|(gte IE 10)]&gt;&lt;!</xsl:comment>
				<link href="/lxi_page_layout.css" rel="stylesheet" type="text/css" />
			<xsl:comment>&lt;![endif]</xsl:comment>
			<xsl:comment>This is the stylesheet specific for this page</xsl:comment>
			<link href="/home_page_layout.css" rel="stylesheet" type="text/css" />
			<script type="text/javascript" src="/jquery-1.11.1.min.js"></script>
			<script type="text/javascript" src="/lxi_device.js"></script>
		</head>
		<body>
			<xsl:comment>This is the header of the web pages</xsl:comment>
			<xsl:comment>It holds the logo, device id, and the site navigation buttons</xsl:comment>
			<div id="header">
				<xsl:comment>Top bar identifying the model and fiving the manufacturer logo</xsl:comment>
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
						<xsl:text>LXI-</xsl:text>
						<xsl:call-template name="short_model_name"/>
						<xsl:text>-</xsl:text>
						<xsl:call-template name="serial_number"/>
					</span>
				</div>
				<xsl:comment>Bar with navigation buttons</xsl:comment>
				<div id="button_div">
					<ul id="button_list">
						<li id="home_button_element">
							<a id="home_button" class="selected_header_button" href="/cgi-bin/home.cgi">Home</a>
					  	</li>
						<li id="ip_configuration_element">
							<a id="ip_configuration_button" class="header_button" href="/cgi-bin/login.cgi">IP Configuration</a>
						</li>
						<li id="interactive_control_element">
							<a id="interactive_button" class="header_button" href="/cgi-bin/interactive.cgi">Interactive Control</a>
						</li>
						<li id="lxi_identification_element">
							<a id="lxi_identification_button" class="header_button" href="/lxi/identification">LXI Identification</a>
						</li>
						<li id="update_element">
							<a id="update_button" class="header_button" href="/cgi-bin/update.cgi">Update</a>
						</li>
					</ul>
				</div>
			</div>
		<div id="main_content">
			<xsl:comment>LXI logo</xsl:comment>
			<img id="lxi_logo" src="/Images/lxi_logo.png" alt="LXI"/>
			<table id="home_page_table">
				<tr>
					<th>Model:</th>
					<td>
						<xsl:value-of select="$model"/>
					</td>
				</tr>
				<tr>
					<th>Manufacturer:</th>
					<td>
						<xsl:call-template name="manufacturer_full"/>
					</td>
				</tr>
				<tr>
					<th>Serial Number:</th>
					<td>
						<xsl:call-template name="serial_number"/>
					</td>
				</tr>
				<xsl:call-template name="visa_resources"/>
				<tr>
					<th>LXI Version:</th>
					<td>
						<xsl:value-of select="lxi:LXIVersion[last()]"/>
					</td>
				</tr>
				<tr>
					<th>Host Name:</th>
					<td>
						<xsl:value-of select="lxi:Interface/lxi:Hostname[last()]"/> 
						<xsl:text>.local</xsl:text>
					</td>
				</tr>
				<tr>
					<th>Description:</th>
					<td>
						<xsl:value-of select="lxi:ManufacturerDescription[last()]"/>
					</td>
				</tr>
				<tr>
					<th>MAC Address:</th>
					<td>
						<xsl:value-of select="lxi:Interface/lxi:MACAddress[last()]"/>
					</td>
				</tr>
				<tr>
					<th>TCP/IP Address:</th>
					<td>
						<xsl:value-of select="lxi:Interface/lxi:IPAddress[last()]"/>
					</td>
				</tr>
				<tr>
					<th>Subnet Mask:</th>
					<td>
						<xsl:value-of select="lxi:Interface/lxi:SubnetMask[last()]"/>
					</td>
				</tr>
				<tr>
					<th>Gateway:</th>
					<td>
						<xsl:value-of select="lxi:Interface/lxi:Gateway[last()]"/>
					</td>
				</tr>
				<xsl:call-template name="firmware_version"/>

				<!-- Count the name servers -->
				<xsl:variable name="name_server_count" select="count(lxi:Extension/lxi:DNSServers/lxi:NameServer)"/>
				<!-- Now display them -->
				<xsl:choose>
					<!-- If there is one dns server print it as normal -->	
					<xsl:when test="$name_server_count = 1">
						<tr>
							<th>DNS Servers:</th>
							<td>
								<xsl:value-of select="lxi:Extension/lxi:DNSServers/lxi:NameServer[last()]"/>
							</td>
						</tr>
					</xsl:when>
					<xsl:when test="$name_server_count &gt; 1">
						<tr>
							<th>DNS Servers:</th>
							<td>
								<ul>
									<xsl:for-each select="lxi:Extension/lxi:DNSServers/lxi:NameServer">
										<li>
											<xsl:value-of select="."/>
										</li>
									</xsl:for-each>
								</ul>
							</td>
						</tr>
					</xsl:when>
				</xsl:choose>
				<tr>
					<th>Device Identify:</th>
					<td>
						<input type="submit" name="submit_button" value="Set" onclick="button_click()"/>
					</td>
				</tr>
			</table>
		</div>
		
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

	<xsl:template name="visa_resources">
		<xsl:variable name="visa_resource_count" select="count(lxi:Interface/lxi:InstrumentAddressString)"/>
		<xsl:choose>
			<!-- Case where there is only one resource -->
			<xsl:when test="$visa_resource_count = 1">
				<tr xmlns="http://www.w3.org/1999/xhtml">
					<th>VISA Resource:</th>
					<td>
						<xsl:value-of select="lxi:Interface/lxi:InstrumentAddressString[last()]"/>
					</td>
				</tr>
			</xsl:when>
			<xsl:when test="$visa_resource_count &gt; 1">
				<tr xmlns="http://www.w3.org/1999/xhtml">
					<th>VISA Resource:</th>
					<td>
						<ul>
							<xsl:for-each select="lxi:Interface/lxi:InstrumentAddressString">
								<li>
									<xsl:value-of select="."/>
								</li>
							</xsl:for-each>
						</ul>
					</td>
				</tr>
			</xsl:when>
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

	<xsl:template name="firmware_version">
		<tr xmlns="http://www.w3.org/1999/xhtml">
			<th>Firmware Version:</th>
			<td>
				<xsl:value-of select="lxi:FirmwareRevision"/>
			</td>
		</tr>
	</xsl:template>
</xsl:stylesheet>
