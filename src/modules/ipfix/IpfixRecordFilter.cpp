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
#include "IpfixRecordFilter.h"

IpfixRecordFilter::IpfixRecordFilter(uint16_t templateId) :
	Module(), IpfixRecordDestination(), Source<IpfixRecord *>(),
	templateId(templateId) {

}

/**
  * Overwritten to pass the data record to the subscribed modules if it matches
  * the filter.
  */
void IpfixRecordFilter::onDataRecord(IpfixDataRecord *record) {
	if (record->templateInfo->templateId == templateId) {
		if (!send(record))
			record->removeReference();
	} else {
		record->removeReference();
	}
}
