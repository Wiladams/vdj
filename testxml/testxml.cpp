#include "apphost.h"
#include "mmap.h"

#include "simd\\SimdXml.hpp"

typedef Simd::Xml::File<char> File;
typedef Simd::Xml::Document<char> Document;
typedef Simd::Xml::Node<char> Node;

void processXml()
{
	auto  xmlFile = mmap::create_shared("sample.xml");

	if (nullptr == xmlFile)
		return;

	// open XML file
	char* xmlTmp = new char[xmlFile->size() + 1];
	memcpy_s(xmlTmp, xmlFile->size(), xmlFile->data(), xmlFile->size());
	xmlTmp[xmlFile->size()] = 0;

	Document doc;
	doc.Parse<0>(xmlTmp);

	//Simd::Xml::Detail::Print<std::basic_ostream<char>, char>(doc.FirstNode());
}

void onLoad()
{
	processXml();

}
