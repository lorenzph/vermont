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
#ifndef IPFIXDBCOMMONCFG_H
#define IPFIXDBCOMMONCFG_H

#include <vector>
#include <stdint.h>
#include <core/Cfg.h>

#include "IpfixRecord.hpp"

class Xmlelement;
class IpfixDbColumn;
class IpfixDbSerializer;

class IpfixDbCommonCfg {
public:
	IpfixDbCommonCfg();
	virtual ~IpfixDbCommonCfg();
protected:

	/**
	  * Parses a simple column type from the given XMLElement.
	  *
	  * The parsed column information is appended to the m_columns vector.
	  */
	void readSimpleType(XMLElement *element);

	/**
	  * Parses a complex column type from the given XMLElement.
	  *
	  * The parsed column information is appended to the m_columns vector.
	  *
	  * This function may throw an exception via THROWEXCEPTION if the input data is invalid.
	  */
	void readComplexType(XMLElement *element);

	virtual IpfixDbSerializer *constructSerializer(const IpfixDbColumn &column) const = 0;
protected:
	struct SimpleColumnEntry {
		const char* columnName; 	/** column name */
		const char* columnType;		/** column data type in database */
		uint64_t defaultValue;       	/** default value */
		InformationElement::IeId ipfixId; /** IPFIX_TYPEID */
		InformationElement::IeEnterpriseNumber enterprise; /** enterprise number */
	};

	virtual const SimpleColumnEntry *simpleColumnEntries() const = 0;
protected:
	vector<IpfixDbColumn *> m_columns;
};

#endif
#endif
