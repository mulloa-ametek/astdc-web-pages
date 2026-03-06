#include "lxi_identification.h"

int main(int argc, char **argv)
{
	const char *cgi_header = "Content-Type: text/xml\r\n\r\n";
	switch(argc)
	{
	case 1:
		printf(cgi_header);
		print_identification_document(NULL);
		break;
	case 2:
		printf(cgi_header);
		print_identification_document(argv[1]);
		break;
	default:
		fputs("Illegal number of arguments.", stderr);
		return -1;
	}

	return 0;
}
