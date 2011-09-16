/*
 * Copyright (C) 2011 Vermont Project
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


#if defined(DB_SUPPORT_ENABLED) || defined(PG_SUPPORT_ENABLED)

#include <limits>
#include <common/jsoncpp/json/json.h>

#include "IpfixDbCommon.hpp"

IpfixDbColumn::~IpfixDbColumn() {
	if (m_serializer != NULL)
		delete m_serializer;
}

IpfixDbSerializer::IpfixDbSerializer(const IpfixDbColumn &type, SerializationType serializationType) :
	m_type(type), m_serializationType(serializationType) {

}

std::string IpfixDbSerializer::serializeValue(const IpfixRecord::Data *data, TemplateInfo::FieldInfo *fieldInfo) {
	SerializationType type = m_serializationType;
	size_t dataLength = fieldInfo->type.length;

	if (type == Automatic)
		type = guessType(data, dataLength);

	ostringstream str;
	bool ok = false;

	switch (type) {
	case unsigned8:
	case unsigned16:
	case unsigned32:
	case unsigned64:
		str << transformValue(toUint64(data, dataLength, ok), fieldInfo);
		break;
	case signed8:
	case signed16:
	case signed32:
	case signed64:
		str << transformValue(toInt64(data, dataLength, ok), fieldInfo);
		break;
	case float32:
		str << transformValue(toFloat(data, dataLength, ok), fieldInfo);
		break;
	case float64:
		str << transformValue(toDouble(data, dataLength, ok), fieldInfo);
		break;
	case boolean:
		str << (transformValue(toUint64(data, dataLength, ok), fieldInfo) ? 1 : 0);
		break;
	case macAddress:
	case octetArray:
	case string:
		str << std::string((const char *) data, dataLength);
		break;
	case structuredData:
		ok = true;
		str << encodeStructuredData(data, fieldInfo);
	}

	DPRINTF("Serializing field with type %d to %s", type, str.str().c_str());

	if (!ok) {
		str.str("");
		str.clear();
		str << "NULL";
	}

	return escape(type, str.str());
}

string IpfixDbSerializer::encodeStructuredData(const IpfixRecord::Data *data, TemplateInfo::FieldInfo *fieldInfo) const {
	Json::Value root;

	root["type"] = fieldInfo->type.id;

	Json::Value rowArray;

	for (uint16_t rowIndex = 0; rowIndex < fieldInfo->rowCount; rowIndex++) {
		Json::Value row;
		Json::Value fields;
		TemplateInfo::StructuredDataRow *rowInfo = &fieldInfo->rows[rowIndex];

		row["templateId"] = rowInfo->templateId;

		for (uint16_t fieldIndex = 0; fieldIndex < rowInfo->fieldCount; fieldIndex++) {
			Json::Value field;
			TemplateInfo::FieldInfo *fieldInfo = &rowInfo->fields[fieldIndex];

			field["id"] = fieldInfo->type.id;
			field["enterpriseId"] = fieldInfo->type.enterprise;
			field["value"] = Json::Value(std::string((const char *) data + fieldInfo->offset, fieldInfo->type.length));

			fields.append(field);
		}

		row["fields"] = fields;

		rowArray.append(row);
	}

	root["rows"] = rowArray;

	Json::FastWriter writer;

	return writer.write(root);
}

uint64_t IpfixDbSerializer::toUint64(const IpfixRecord::Data *data, size_t dataLength, bool &ok) const {
	if (dataLength > sizeof(uint64_t)) {
		ok = false;

		return 0;
	}

	ok = true;

	uint64_t value = 0;
	memcpy(((uint8_t *) &value) + (sizeof(value) - dataLength), data, dataLength);
	return ntohll(value);
}

int64_t IpfixDbSerializer::toInt64(const IpfixRecord::Data *data, size_t dataLength, bool &ok) const {
	return (int64_t) toUint64(data, dataLength, ok);
}

float IpfixDbSerializer::toFloat(const IpfixRecord::Data *data, size_t dataLength, bool &ok) const {
	if (dataLength < sizeof(float)) {
		ok = false;

		return 0;
	}

	if (std::numeric_limits<float>::is_iec559) {
		ok = false;

		msg(MSG_ERROR, "Platform must support IEEE 754 floating point numbers.");
		return 0.0;
	}

	ok = true;

	return *((float *) data);
}


double IpfixDbSerializer::toDouble(const IpfixRecord::Data *data, size_t dataLength, bool &ok) const {
	if (dataLength < sizeof(double)) {
		ok = false;

		return 0;
	}

	if (std::numeric_limits<double>::is_iec559) {
		ok = false;

		msg(MSG_ERROR, "Platform must support IEEE 754 floating point numbers.");
		return 0.0;
	}

	ok = true;

	return *((double *) data);
}
#endif
