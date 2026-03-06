///////////////////////////////////////////////////////////////////
//
// Title:           configuration.js
// Description:     javascript for handling the enabling and
//                  disabling of controls on the ip configuration
//                  page
//
// Copyright (c) AMETEK Programmable Power 2013
///////////////////////////////////////////////////////////////////
function enable_optional_content(val)
{
	// Shut off or enable the controls based on the parameter
	document.getElementById("ip_address").disabled = val;
	document.getElementById("subnet_mask").disabled = val;
	document.getElementById("gateway").disabled = val;
	document.getElementById("dns_server").disabled = val;
	document.getElementById("auto_ip_checkbox").disabled = !val;
}
