#include "lxi_identification.h"

static const char *xml_declaration = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
static const char *identification_data =
"<LXIDevice xmlns=\"http://www.lxistandard.org/InstrumentIdentification/1.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.lxistandard.org/InstrumentIdentification/1.0 http://10.11.24.53/lxi/schema\">\n"
"\t<Manufacturer>AMETEK</Manufacturer>\n"
"\t<Model>ViPR</Model>\n"
"\t<SerialNumber>333333</SerialNumber>\n"
"\t<FirmwareRevision>1.0</FirmwareRevision>\n"
"\t<ManufacturerDescription>DC Power Supply</ManufacturerDescription>\n"
"\t<HomepageURL>http://programmablepower.com</HomepageURL>\n"
"\t<DriverURL>http://www.programmablepower.com/products/ViPR/drivers</DriverURL>\n"
"\t<UserDescription>Lab Power Supply</UserDescription>\n"
"\t<IdentificationURL>http://10.11.24.53/lxi/identification</IdentificationURL>\n"
"\t<Interface xsi:type=\"NetworkInformation\" InterfaceType=\"LXI\" IPType=\"IPv4\" InterfaceName=\"eth0\">\n"
"\t\t<InstrumentAddressString>TCPIP0::10.11.24.53::inst0::INSTR</InstrumentAddressString>\n"
"\t\t<InstrumentAddressString>TCPIP0::10.11.24.53::52000::SOCKET</InstrumentAddressString>\n"
"\t\t<Hostname>AMOC</Hostname>\n"
"\t\t<IPAddress>10.11.24.53</IPAddress>\n"
"\t\t<SubnetMask>255.255.0.0</SubnetMask>\n"
"\t\t<MACAddress>00:20:4A:9E:BE:7A</MACAddress>\n"
"\t\t<Gateway>10.11.0.1</Gateway>\n"
"\t\t<DHCPEnabled>true</DHCPEnabled>\n"
"\t\t<AutoIPEnabled>true</AutoIPEnabled>\n"
"\t</Interface>\n"
"\t<Domain>0</Domain>\n"
"\t<LXIVersion>1.4</LXIVersion>\n"
"</LXIDevice>";

static inline void print_xml_declaration(void);
static inline void print_xsl_stylesheet(const char *stylesheet);

static inline void print_xsl_stylesheet(const char *stylesheet)
{
	(void)printf("<?xml-stylesheet type=\"text/xsl\" href=\"%s\"?>", stylesheet);
}

static inline void print_xml_declaration(void)
{
	(void)puts(xml_declaration);
}

void print_identification_document(char *stylesheet)
{
	// Standard header
	print_xml_declaration();

	// Wont have a stylesheet if this is just the standard
	// LXI identification document
	if(stylesheet != NULL)
	{
		print_xsl_stylesheet(stylesheet);
	}

	// Rest of the static data
	(void)puts(identification_data);
}
