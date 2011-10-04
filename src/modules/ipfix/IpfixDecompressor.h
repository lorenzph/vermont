/*
 *
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

#ifndef IPFIXDECOMPRESSOR_H_
#define IPFIXDECOMPRESSOR_H_

#ifdef WITH_DECOMPRESSION
#include <string>
#include "IpfixPacketProcessor.hpp"

class Decompressor {
public:
	virtual int decompress(const boost::shared_array<uint8_t> source,
						   uint16_t sourceLength,
						   boost::shared_array<uint8_t> &destination,
						   uint16_t &destinationLength) = 0;
};

class IpfixDecompressor : public IpfixPacketProcessor, public Sensor {
public:
	IpfixDecompressor(const std::string &algorithm);
	~IpfixDecompressor();

	int processPacket(boost::shared_array<uint8_t> &message, uint16_t &length, boost::shared_ptr<IpfixRecord::SourceID> sourceId);

private:
	Decompressor *decompressor;
};
#endif

#endif
