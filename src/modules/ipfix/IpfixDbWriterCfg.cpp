/*
 * Vermont Configuration Subsystem
 * Copyright (C) 2009 Vermont Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#ifdef DB_SUPPORT_ENABLED

#include "IpfixDbWriterCfg.h"

const IpfixDbWriterCfg::SimpleColumnEntry IpfixDbWriterCfg::SimpleColumnMap[] = {
	{CN_dstIP, 		"INTEGER(10) UNSIGNED", 	0, IPFIX_TYPEID_destinationIPv4Address, 0},
	{CN_srcIP, 		"INTEGER(10) UNSIGNED", 	0, IPFIX_TYPEID_sourceIPv4Address, 0},
	{CN_srcPort, 		"SMALLINT(5) UNSIGNED", 	0, IPFIX_TYPEID_sourceTransportPort, 0},
	{CN_dstPort, 		"SMALLINT(5) UNSIGNED", 	0, IPFIX_TYPEID_destinationTransportPort, 0},
	{CN_proto, 		"TINYINT(3) UNSIGNED", 		0, IPFIX_TYPEID_protocolIdentifier, 0 },
	{CN_dstTos, 		"TINYINT(3) UNSIGNED", 		0, IPFIX_TYPEID_classOfServiceIPv4, 0},
	{CN_bytes, 		"BIGINT(20) UNSIGNED", 		0, IPFIX_TYPEID_octetDeltaCount, 0},
	{CN_pkts, 		"BIGINT(20) UNSIGNED", 		0, IPFIX_TYPEID_packetDeltaCount, 0},
	{CN_firstSwitched, 	"INTEGER(10) UNSIGNED", 	0, IPFIX_TYPEID_flowStartSeconds, 0}, // default value is invalid/not used for this ent
	{CN_lastSwitched, 	"INTEGER(10) UNSIGNED", 	0, IPFIX_TYPEID_flowEndSeconds, 0}, // default value is invalid/not used for this entry
	{CN_firstSwitchedMillis, "SMALLINT(5) UNSIGNED", 	0, IPFIX_TYPEID_flowStartMilliSeconds, 0},
	{CN_lastSwitchedMillis, "SMALLINT(5) UNSIGNED", 	0, IPFIX_TYPEID_flowEndMilliSeconds, 0},
	{CN_tcpControlBits,  	"SMALLINT(5) UNSIGNED", 	0, IPFIX_TYPEID_tcpControlBits, 0},
	//TODO: use enterprise number for the following extended types (Gerhard, 12/2009)
	{CN_revbytes, 		"BIGINT(20) UNSIGNED", 		0, IPFIX_TYPEID_octetDeltaCount, IPFIX_PEN_reverse},
	{CN_revpkts, 		"BIGINT(20) UNSIGNED", 		0, IPFIX_TYPEID_packetDeltaCount, IPFIX_PEN_reverse},
	{CN_revFirstSwitched, 	"INTEGER(10) UNSIGNED", 	0, IPFIX_TYPEID_flowStartSeconds, IPFIX_PEN_reverse}, // default value is invalid/not used for this entry
	{CN_revLastSwitched, 	"INTEGER(10) UNSIGNED", 	0, IPFIX_TYPEID_flowEndSeconds, IPFIX_PEN_reverse}, // default value is invalid/not used for this entry
	{CN_revFirstSwitchedMillis, "SMALLINT(5) UNSIGNED", 	0, IPFIX_TYPEID_flowStartMilliSeconds, IPFIX_PEN_reverse},
	{CN_revLastSwitchedMillis, "SMALLINT(5) UNSIGNED", 	0, IPFIX_TYPEID_flowEndMilliSeconds, IPFIX_PEN_reverse},
	{CN_revTcpControlBits,  "SMALLINT(5) UNSIGNED", 	0, IPFIX_TYPEID_tcpControlBits, IPFIX_PEN_reverse},
	{CN_maxPacketGap,  	"BIGINT(20) UNSIGNED", 		0, IPFIX_ETYPEID_maxPacketGap, IPFIX_PEN_vermont|IPFIX_PEN_reverse},
	{CN_exporterID, 	"SMALLINT(5) UNSIGNED", 	0, EXPORTERID, 0},
	{0} // last entry must be 0
};


const boost::regex IpfixDbMySQLSerializer::integerRegex("^\\s*(BIGINT|INTEGER|INT|MEDIUMINT|TINYINT|SMALLINT)");
const boost::regex IpfixDbMySQLSerializer::floatRegex("^\\s*(FLOAT|REAL|DOUBLE PRECISION|DECIMAL|NUMERIC)");

IpfixDbMySQLSerializer::IpfixDbMySQLSerializer(const IpfixDbColumn &type, SerializationType serializationType) :
	IpfixDbSerializer(type , serializationType), integerType(false), floatType(false), unsignedType(false) {
	if (boost::regex_search(m_type.dbColumnTypeDefinition(), integerRegex)) {
		// Seems to be an integer type - check if it is unsigned or signed
		integerType = true;
		unsignedType = m_type.dbColumnTypeDefinition().rfind("UNSIGNED") != std::string::npos;
	} else if(boost::regex_search(m_type.dbColumnTypeDefinition(), floatRegex)) {
		floatType = true;
	}

}

IpfixDbSerializer::SerializationType IpfixDbMySQLSerializer::guessType(const IpfixRecord::Data *data, size_t dataLength) const {
	if (integerType) {
		if (dataLength == 1)
			return (unsignedType ? unsigned8 : signed8);
		else if(dataLength <= 2)
			return (unsignedType ? unsigned16 : signed16);
		else if(dataLength <= 4)
			return (unsignedType ? unsigned32 : signed32);
		else if(dataLength <= 8)
			return (unsignedType ? unsigned64 : signed64);
		else
			return unknownType;
	} else if(floatType) {
		switch (dataLength) {
		case 4:
			return float32;
		case 8:
			return float64;
		default:
			return unknownType;
		}
	} else if (m_type.ieId() >= IPFIX_TYPEID_basicList && m_type.ieId() <= IPFIX_TYPEID_subTemplateMultiList) {
		return structuredData;
	}

	return unknownType;
}

std::string IpfixDbMySQLSerializer::escape(SerializationType type, const std::string &input) {
	switch (type) {
	case unsigned8:
	case unsigned16:
	case unsigned32:
	case unsigned64:
	case signed8:
	case signed16:
	case signed32:
	case signed64:
	case float32:
	case float64:
		return input;
	default: {
		char *out = new char[input.size() * 2 + 1 + 2];
		mysql_real_escape_string(connection, out, input.c_str(), input.size());

		std::string result = std::string("'") + std::string(out) + std::string("'");
		delete [] out;

		return result;
	}
	}
}

uint64_t IpfixDbMySQLSerializer::transformValue(uint64_t value, TemplateInfo::FieldInfo *fieldInfo) {
	// Transformations are only applied to keep compatiblity to old configs
	if (!m_type.fromSimpleType())
		return value;

	InformationElement::IeEnterpriseNumber fieldEId = fieldInfo->type.enterprise;
	InformationElement::IeId fieldId = fieldInfo->type.id;
	InformationElement::IeEnterpriseNumber typeEId = m_type.ieEnterpriseId();
	InformationElement::IeId typeId = m_type.ieId();

	if (fieldEId == 0 && typeEId == 0) {
		if (((fieldId == IPFIX_TYPEID_flowEndMilliSeconds && typeId == IPFIX_TYPEID_flowEndSeconds) ||
			 (fieldId == IPFIX_TYPEID_flowStartMilliSeconds && typeId == IPFIX_TYPEID_flowStartSeconds)))
			return value / 1000;
		else if ((fieldId == IPFIX_TYPEID_flowStartMilliSeconds && typeId == IPFIX_TYPEID_flowStartMilliSeconds) ||
				 (fieldId == IPFIX_TYPEID_flowEndMilliSeconds && typeId == IPFIX_TYPEID_flowEndMilliSeconds))
			return value % 1000;

	} else if (fieldEId == IPFIX_PEN_reverse && typeEId == IPFIX_PEN_reverse) {
		if ((typeId == IPFIX_TYPEID_flowStartSeconds && fieldId == IPFIX_TYPEID_flowStartMilliSeconds) ||
				(typeId == IPFIX_TYPEID_flowEndSeconds && fieldId == IPFIX_TYPEID_flowEndMilliSeconds)) {
			return value / 1000;
		} else if((typeId == IPFIX_TYPEID_flowStartMilliSeconds && fieldId == IPFIX_TYPEID_flowStartMilliSeconds) ||
				  (typeId == IPFIX_TYPEID_flowEndMilliSeconds && fieldId == IPFIX_TYPEID_flowEndMilliSeconds)) {
			return value % 1000;
		}
	}

	if (fieldInfo->type.length == 5) {
		// Strip the netmask from 5 byte IP addresses
		value = htonll(value);
		return ntohl(*((uint32_t *) (((uint8_t *) &value) + 3)));
	}

	return value;
}

IpfixDbWriterCfg* IpfixDbWriterCfg::create(XMLElement* e)
{
	assert(e);
	assert(e->getName() == getName());
	return new IpfixDbWriterCfg(e);
}


IpfixDbWriterCfg::IpfixDbWriterCfg(XMLElement* elem)
	: CfgHelper<IpfixDbWriter, IpfixDbWriterCfg>(elem, "ipfixDbWriter"),
	  port(0), bufferRecords(30), observationDomainId(0)
{
	if (!elem) return;

	XMLNode::XMLSet<XMLElement*> set = _elem->getElementChildren();
	for (XMLNode::XMLSet<XMLElement*>::iterator it = set.begin();
		 it != set.end();
		 it++) {
		XMLElement* e = *it;

		if (e->matches("host")) {
			hostname = e->getFirstText();
		} else if (e->matches("port")) {
			port = getInt("port");
		} else if (e->matches("dbname")) {
			dbname = e->getFirstText();
		} else if (e->matches("username")) {
			user = e->getFirstText();
		} else if (e->matches("password")) {
			password = e->getFirstText();
		} else if (e->matches("bufferrecords")) {
			bufferRecords = getInt("bufferrecords");
		} else if (e->matches("columns")) {
			readColumns(e);
		} else if (e->matches("observationDomainId")) {
			observationDomainId = getInt("observationDomainId");
		} else if (e->matches("next")) { // ignore next
		} else {
			msg(MSG_FATAL, "Unknown IpfixDbWriter config statement %s\n", e->getName().c_str());
			continue;
		}
	}
	if (hostname=="") THROWEXCEPTION("IpfixDbWriterCfg: host not set in configuration!");
	if (port==0) THROWEXCEPTION("IpfixDbWriterCfg: port not set in configuration!");
	if (dbname=="") THROWEXCEPTION("IpfixDbWriterCfg: dbname not set in configuration!");
	if (user=="") THROWEXCEPTION("IpfixDbWriterCfg: username not set in configuration!");
}

void IpfixDbWriterCfg::readColumns(XMLElement* elem) {
	m_columns.clear();
	XMLNode::XMLSet<XMLElement*> set = elem->getElementChildren();
	for (XMLNode::XMLSet<XMLElement*>::iterator it = set.begin();
		 it != set.end();
		 it++) {
		XMLElement* e = *it;

		if (e->matches("name")) {
			readSimpleType(e);
		} else if (e->matches("complextype")) {
			readComplexType(e);
		} else {
			msg(MSG_INFO, "Unknown IpfixDbWriter config statement %s\n", e->getName().c_str());
			continue;
		}
	}

}

IpfixDbWriterCfg::~IpfixDbWriterCfg()
{
}


IpfixDbWriter* IpfixDbWriterCfg::createInstance()
{
	instance = new IpfixDbWriter(hostname, dbname, user, password, port, observationDomainId, bufferRecords, m_columns);
	return instance;
}


bool IpfixDbWriterCfg::deriveFrom(IpfixDbWriterCfg* old)
{
	return false;
}

IpfixDbSerializer *IpfixDbWriterCfg::constructSerializer(const IpfixDbColumn &column) const {
	return new IpfixDbMySQLSerializer(column);
}

#endif /*DB_SUPPORT_ENABLED*/
