//////////////////////////////////////////////////////////////////////////
// Title:						lxi_device.xsl
// Description:				javascript for handling the lxi home page
//									This is mainly going to consist of handling
//									the lxi identification
//
// Copyright (C) AMETEK Programmable Power 2014
//////////////////////////////////////////////////////////////////////////
function button_click()
{
	var button = $("input[name=submit_button]");
	var current_setting = button.val();
	var set_string = 'Set';
	var unset_string = 'Unset';

	// Send CGI POST message
	$.post('/cgi-bin/blink.cgi', { blink: current_setting });

	// Change the text on the submission button
	if(current_setting == set_string)
	{
		button.prop('value', unset_string);
	}
	else
	{
		button.prop('value', set_string);
	}
}
