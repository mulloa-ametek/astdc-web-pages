#!/bin/sh

#####################################################################################
# Title:				get_password.sh
# Author:			Adam Mocarski
# Description:		Get the password for the specified usernam
#
# Copyright (C) AMETEK Programmable Power, 2014
#####################################################################################
PASSWORD_FILE='/home/usrpasswd.ini' # This file contains 

# Extract the password from the password file
exec sed -n "/user=$1:/s/^user=$1:\([[:alnum:]]*\):.*$/\1/p" $PASSWORD_FILE
