/////////////////////////////////////////////////////////////////////
// Title:			check_password.h
// Description:	Checks the username and password passed to see
//						if they are correct
// Author:			Adam Mocarski
/////////////////////////////////////////////////////////////////////
#ifndef __CHECK_PASSWORD_H__
#define __CHECK_PASSWORD_H__
#define SUCCESS 1
#define FAILURE 0
	int user_match(const char *username, const char *password);
#endif
