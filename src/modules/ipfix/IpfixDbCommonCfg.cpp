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

#include "IpfixDbCommonCfg.hpp"
#include "IpfixDbCommon.hpp"

#include <core/XMLElement.h>

IpfixDbCommonCfg::IpfixDbCommonCfg() {

}

IpfixDbCommonCfg::~IpfixDbCommonCfg() {
	for (vector<IpfixDbColumn *>::iterator it = m_columns.begin(); it < m_columns.end(); it++) {
		delete *it;
	}
}

void IpfixDbCommonCfg::readSimpleType(XMLElement *element) {
	string name = element->getFirstText();

	const SimpleColumnEntry *entry = simpleColumnEntries();

	while (entry->columnName != 0) {
		if (name == entry->columnName) {
			stringstream str;
			str << entry->defaultValue;

			IpfixDbColumn *column = new IpfixDbColumn(name, entry->columnType, entry->ipfixId, entry->enterprise, true, str.str());
			column->m_serializer = constructSerializer(*column);

			m_columns.push_back(column);

			return;
		}

		entry++;
	}

	THROWEXCEPTION("Unknown column name %s", name.c_str());
}

void IpfixDbCommonCfg::readComplexType(XMLElement *element) {
	XMLNode::XMLSet<XMLElement*> set = element->getElementChildren();

	uint32_t ieEnterpriseId = 0;
	uint16_t ieId = 0;
	string name;
	string dbColumnDefinition;

	for (XMLNode::XMLSet<XMLElement*>::iterator it = set.begin();
		 it != set.end();
		 it++) {
		XMLElement* e = *it;

		if (e->matches("informationelement")) {
			XMLAttribute *enterpriseIdAttr = e->getAttribute("enterpriseId");
			if (enterpriseIdAttr != NULL)
				ieEnterpriseId = strtoul(enterpriseIdAttr->getFirstText().c_str(), NULL, 10);

			uint32_t ieId32 = strtoul(e->getFirstText().c_str(), NULL, 10);
			if (ieId32 == 0 || ieId32 > 32767) {
				THROWEXCEPTION("Invalid information element id %d", ieId);
			}

			ieId = (uint16_t) ieId;
		} else if (e->matches("type")) {
			dbColumnDefinition = e->getFirstText();
		} else if (e->matches("name")) {
			name = e->getFirstText();
		}
	}

	if (name.empty())
		THROWEXCEPTION("complextype element is missing name definition.");

	if (dbColumnDefinition.empty())
		THROWEXCEPTION("complextype element is missing type definition.");

	if (ieId == 0)
		THROWEXCEPTION("complextype element is missing informationelement id definition.");

	IpfixDbColumn *column = new IpfixDbColumn(name, dbColumnDefinition, ieId, ieEnterpriseId);
	column->m_serializer = constructSerializer(*column);

	m_columns.push_back(column);
}

#endif
