/*
 * Vermont Configuration Subsystem
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
#include "IpfixRecordFilterCfg.h"

IpfixRecordFilterCfg::IpfixRecordFilterCfg(XMLElement *elem)
	: CfgHelper<IpfixRecordFilter, IpfixRecordFilterCfg>(elem, "ipfixRecordFilter")
{
	if (!elem)
		return;

	XMLNode::XMLSet<XMLElement*> set = elem->getElementChildren();
	for (XMLNode::XMLSet<XMLElement*>::iterator it = set.begin();
		 it != set.end();
		 it++) {

		XMLElement* e = *it;

		if (e->matches("templateId")) {
			templateId = getUInt32("templateId");
		}
	}

	if (templateId <= 255)
		THROWEXCEPTION("Invalid template id specified in IpfixRecordFilter (%d)", templateId);
}

IpfixRecordFilterCfg::~IpfixRecordFilterCfg() {

}

bool IpfixRecordFilterCfg::deriveFrom(IpfixRecordFilterCfg* old)
{
	return false;
}

IpfixRecordFilter *IpfixRecordFilterCfg::createInstance()
{
	instance = new IpfixRecordFilter(templateId);
	return instance;
}

IpfixRecordFilterCfg * IpfixRecordFilterCfg::create(XMLElement *elem)
{
	assert(elem);
	assert(elem->getName() == getName());
	return new IpfixRecordFilterCfg(elem);
}
