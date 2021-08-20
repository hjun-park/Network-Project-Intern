
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filereadstream.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include "rapid.h"


using namespace rapidjson;
using namespace std;

Parser parsing() {
	FILE *fp;
	char buffer[1024];

	Parser *parser;
	parser = (Parser *)malloc(sizeof(Parser));


	// TODO : Parsing error 나는 부분을 수정 
//	fp = fopen("dhcp_config.json", "r");
	fp = fopen("dhcp_config.json", "rb");
	
//	fread(buffer, 1024, 1, fp);
	FileReadStream is(fp, buffer, sizeof(buffer));

    Document document; 
	vector<string> symbols;

	fclose(fp);

    if (document.ParseInsitu(buffer).HasParseError())
		printf("error\n");

    printf("\n[+] Parsing to \"dhcp_config.json\" succeeded.\n");

	Value *results = &(document["net1"]);
	memcpy(parser->interfaces, (*results)["interfaces"].GetString(), sizeof(parser->interfaces));
	parser->valid_lifetime = ((*results)["valid-lifetime"].GetInt());
	memcpy(parser->pool, (*results)["pool"].GetString(), sizeof(parser->pool));
	memcpy(parser->subnet, (*results)["subnet"].GetString(), sizeof(parser->subnet));
	memcpy(parser->router, (*results)["router"].GetString(), sizeof(parser->router));
	memcpy(parser->dns_server, (*results)["dns-server"].GetString(), sizeof(parser->dns_server));

    return *parser;
}
