//////////////////////////////////////////////////////////////////////////
// Title:						interactive.js
// Description:				javascript for handling the interactive
//									web page
//
// Copyright (C) AMETEK Programmable Power 2014
//////////////////////////////////////////////////////////////////////////
var command_array = []; // Command history
var command_index = 0; // currently indexed command in the history

// Takes the input text and creates a span with the given class
// and adds a line break
function surround_in_span(text, text_class)
{
	var newline = "<br/>";

	return "<span class=\"" + text_class + "\">" + text + newline + "</span>";
}

// Scroll to the bottom of the page
function scroll_bottom(scroller)
{
	scroller.prop("scrollTop", scroller.prop("scrollHeight"));
}

// event handler for when the user clicks the send command button
// This is also called when the user hits the enter key
function send_button_click()
{
	var command_text = $("#command_text").prop("value");
	var result = $("#result_text");
	var sent_class = "sent_class";
	var receive_class = "receive_class";
	var error_class = "error_class";
	var error_message = "Failed to get a response";
	var query_failure = "failed_query";
	var success_message = "done";

	// Reset the command index
	command_index = 0;

	// Only process if there is a command in the text box
	if(command_text && command_text.length > 0)
	{
		// Move the command to the history and adjust scroll bar
		result.append(surround_in_span(command_text, sent_class));
		scroll_bottom(result);

		// send out the CGI POST message
		$.post('/cgi-bin/interact.cgi', { command: command_text }, function(post_result) {
			switch(post_result)
			{
			case query_failure:
				// Print a failure message
				result.append(surround_in_span(error_message, error_class));
				scroll_bottom(result);
				break;
			case success_message:
				// Add the command to the command history
				add_unique_command(command_text);
				break;
			default:
				// Print the command in the history
				result.append(surround_in_span(post_result, receive_class));
				scroll_bottom(result);
				// And add the command in the command history
				add_unique_command(command_text);
				break;
			}
		});
	}
}

function add_unique_command(command)
{
	// If the command isn't in our history
	// add it to the history
	if(unique_command(command))
	{
		command_array.unshift(command);
	}
}

function unique_command(find_me)
{
	var i;

	// Loop through the array looking for the command
	for(i = 0; i != command_array.length; ++i)
	{
		if(command_array[i] == find_me)
		{
			return false;
		}
	}

	// If we got here the element isnt in the list
	return true;
}

// Run this code after load
$(document).ready(function(){
	$("#command_text").keydown(function(e){
		switch(e.which)
		{
		case 13:
			// Enter key was pressed
			// Enter the command as if the send button were clicked
			send_button_click();

			// Clear the command line
			$("#command_text").prop("value", "");
			break;
		case 38:
			// The up arrow was clicked
			if(command_array[command_index] == $("#command_text").prop("value"))
			{
				if(command_index + 1 < command_array.length)
				{
					$("#command_text").prop("value", command_array[++command_index]);
				}
			}
			else
			{
				if(command_index < command_array.length)
				{
					$("#command_text").prop("value", command_array[command_index]);
				}
				if(command_index + 1 < command_array.length)
				{
					++command_index;
				}
			}
			break;
		case 40:
			// The down arrow was clicked
			if(command_index > 0)
			{
				$("#command_text").prop("value", command_array[--command_index]);
			}
			break;
		default:
			// Some other key was clicked
			// Ignore it
			break;
		}
	});});
