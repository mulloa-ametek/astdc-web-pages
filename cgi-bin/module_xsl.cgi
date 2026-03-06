#!/bin/sh
module_list=`./module_list.cgi | sed "s/\(.*\),\(.*\),\(.*\),\(.*\),\(.*\)/<tr><td>\1<%td><td>\3<%td><td>\4<%td><td>\5<%td><%tr>/" | tr -d "\n"`
echo -e "Content-Type: text/xsl\r\n\r\n"
sed "s/<module_info>/$module_list/" ../module.xsl | tr % "/"
