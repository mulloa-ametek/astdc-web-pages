#ifndef __CGI_LIB__H__
#define __CGI_LIB__H__
	#define CGI_BUFFER_SIZE 64
	typedef struct
	{
		char field_name[CGI_BUFFER_SIZE];
		char field_value[CGI_BUFFER_SIZE];
	} CGI_RECORD;

	int scan_input(CGI_RECORD *record_list, int size);
	char *get_record(CGI_RECORD *record, const char *field, int size);
#endif
