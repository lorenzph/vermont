/*
 * IPFIX Database Reader/Writer
 * Copyright (C) 2006 Jürgen Abberger
 * Copyright (C) 2006 Lothar Braun <braunl@informatik.uni-tuebingen.de>
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

/* Some constants that are common to IpfixDbWriter and IpfixDbReader */
/**
 * STARTLEN     : Length of statement for INSERT IN.., CREATE TABL.., CREATE DATA.., SELECT FROM...
 * TABLE_WIDTH  : Length of table name string
 * COL_WIDTH    : Length of the string denotes the name of the single columns
 *                and datatype to store in table
 * INS_WIDTH    : Length of the string for insert statement depending of count columns
 * EXPORTER_WIDTH    : Length of the string for operations on exporter table
 */

#if defined(DB_SUPPORT_ENABLED) || defined(PG_SUPPORT_ENABLED)

#define STARTLEN         60
#define TABLE_WIDTH      16
#define COL_WIDTH        40
#define INS_WIDTH        25
#define EXPORTER_WIDTH   80

// column names
#define CN_srcIP 	"srcIP"
#define CN_dstIP 	"dstIP"
#define CN_srcPort 	"srcPort"
#define CN_dstPort 	"dstPort"
#define CN_proto	"proto"
#define CN_dstTos       "dstTos"
#define CN_bytes       	"bytes"
#define CN_pkts       	"pkts"
#define CN_firstSwitched 	"firstSwitched"
#define CN_lastSwitched 	"lastSwitched"
#define CN_firstSwitchedMillis 	"firstSwitchedMillis"
#define CN_lastSwitchedMillis 	"lastSwitchedMillis"
#define CN_exporterID 	"exporterID"

#define CN_tcpControlBits	"tcpControlBits"
#define CN_revbytes		"revbytes"
#define CN_revpkts		"revpkts"
#define CN_revFirstSwitched	"revFirstSwitched"
#define CN_revLastSwitched	"revLastSwitched"
#define CN_revFirstSwitchedMillis	"revFirstSwitchedMillis"
#define CN_revLastSwitchedMillis	"revLastSwitchedMillis"
#define CN_revTcpControlBits	"revTcpControlBits"
#define CN_maxPacketGap		"maxPacketGap"

#include "IpfixRecord.hpp"

class IpfixDbColumn;

using namespace std;

class IpfixDbSerializer {
public:
	enum SerializationType {
		Automatic, // Serialization is performed based on the type's dbColumnDefinition and data length,
		unsigned8,
		unsigned16,
		unsigned32,
		unsigned64,
		signed8,
		signed16,
		signed32,
		signed64,
		float32,
		float64,
		boolean,
		macAddress,
		octetArray,
		string,
		/*
		dateTimeSeconds,
		dateTimeMilliseconds,
		dateTimeMicroseconds,
		dateTimeNanoseconds,
		ipv4Address,
		ipv6Address,
		*/
		unknownType
	};

public:
	/**
	  * Creates a new serializer for the given \a type.
	  *
	  */
	IpfixDbSerializer(const IpfixDbColumn &type, SerializationType serializationType = Automatic);

	/**
	  * Serializes the data value beginning at \a data appending the serialized value to the given
	  * \a insertString.
	  */
	virtual std::string serializeValue(const IpfixRecord::Data *data, TemplateInfo::FieldInfo *fieldInfo);
protected:
	/**
	  * Implement this method to infer the SerializationType from the given data. This method is invoked
	  * if the serialization type is set to Automatic.
	  */
	virtual SerializationType guessType(const IpfixRecord::Data *data, size_t dataLength) const = 0;

	/**
	  * Escapes the input value. E.g. adding '' around string values.
	  */
	virtual std::string escape(SerializationType type, const std::string &input) = 0;

	/**
	  * Enables the application of transformations before serializing a value.
	  */
	virtual uint64_t transformValue(uint64_t value, TemplateInfo::FieldInfo *fieldInfo) { return value; }
	virtual int64_t transformValue(int64_t value, TemplateInfo::FieldInfo *fieldInfo) { return value; }

	virtual float transformValue(float value, TemplateInfo::FieldInfo *fieldInfo) { return value; }
	virtual double transformValue(double value, TemplateInfo::FieldInfo *fieldInfo) { return value; }

protected:
	uint64_t toUint64(const IpfixRecord::Data *data, size_t dataLength, bool &ok) const;

	int64_t toInt64(const IpfixRecord::Data *data, size_t dataLength, bool &ok) const;

	float toFloat(const IpfixRecord::Data *data, size_t dataLength, bool &ok) const;
	double toDouble(const IpfixRecord::Data *data, size_t dataLength, bool &ok) const;
protected:
	const IpfixDbColumn &m_type;
	SerializationType m_serializationType;
};

class IpfixDbColumn {
public:
	IpfixDbColumn(string name, string dbColumnDefinition, InformationElement::IeId ieId,
				  InformationElement::IeEnterpriseNumber ieEnterpriseId = 0, bool fromSimpleType = false,
				  string defaultValue = "") :
		m_fromSimpleType(fromSimpleType), m_name(name), m_dbColumnDefinition(dbColumnDefinition), m_ieId(ieId),
		m_ieEnterpriseId(ieEnterpriseId), m_serializer(NULL), m_defaultValue(defaultValue) {}

	virtual ~IpfixDbColumn();

	/**
	  * \return Returns the column name of this complex type.
	  */
	const string &name() const { return m_name; }

	/**
	  * \return Returns the column defintion for this complex type (e.g. the type which is used to create the
	  *         column in the database.
	  */
	const string &dbColumnTypeDefinition() const { return m_dbColumnDefinition; }

	/**
	  * \return Returns the information element id of this complex type.
	  */
	InformationElement::IeId ieId() const { return m_ieId; }

	/**
	  * \return Returns the enterprise id of this information element or 0 if no enterprise id is set.
	  */
	InformationElement::IeEnterpriseNumber ieEnterpriseId() const { return m_ieEnterpriseId; }

	IpfixDbSerializer *serializer() const { return m_serializer; }

	/**
	  * \return Returns true if this column definition was derived from a simple type in the XML config.
	  */
	bool fromSimpleType() const { return m_fromSimpleType; }

	const string &defaultValue() const { return m_defaultValue; }

private:
	friend class IpfixDbCommonCfg;

	bool m_fromSimpleType;

	string m_name;
	string m_dbColumnDefinition;

	InformationElement::IeId m_ieId;
	InformationElement::IeEnterpriseNumber m_ieEnterpriseId;

	IpfixDbSerializer *m_serializer;

	string m_defaultValue;
};
#endif
