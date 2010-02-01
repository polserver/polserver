#ifndef CLIB_XERCES_INC_H

#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/SaxParseException.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

namespace xerces = xercesc_2_2;

using xerces::AttributeList;
using xerces::BinInputStream;
using xerces::chLatin_L;
using xerces::chLatin_S;
using xerces::chNull;
using xerces::DocumentHandler;
using xerces::DOMBuilder;
using xerces::DOMDocument;
using xerces::DOMElement;
using xerces::DOMImplementation;
using xerces::DOMImplementationLS;
using xerces::DOMImplementationRegistry;
using xerces::DOMNamedNodeMap;
using xerces::DOMNode;
using xerces::DOMText;
using xerces::DOMWriter;
using xerces::HandlerBase;
using xerces::InputSource;
using xerces::Janitor;
using xerces::MemBufFormatTarget;
using xerces::MemBufInputSource;
using xerces::SAXParser;
using xerces::SAXParseException;
using xerces::XercesDOMParser;
using xerces::XMLFormatTarget;
using xerces::XMLFormatter;
using xerces::XMLPlatformUtils;
using xerces::XMLPScanToken;
using xerces::XMLString;
#endif
