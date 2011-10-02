/*
 * IPFIX Database Reader/Writer
 * Copyright (C) 2006 Jürgen Abberger
 * Copyright (C) 2006 Lothar Braun <braunl@informatik.uni-tuebingen.de>
 * Copyright (C) 2007, 2008 Gerhard Muenz
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
#ifdef DB_SUPPORT_ENABLED

#include <stdexcept>
#include <string.h>
#include <iomanip>
#include <stdlib.h>
#include "IpfixDbWriter.hpp"
#include "IpfixDbWriterCfg.h"
#include "common/msg.h"

/**
 * Compare two source IDs and check if exporter is the same (i.e., same IP address and observationDomainId
 */
bool IpfixDbWriter::equalExporter(const IpfixRecord::SourceID& a, const IpfixRecord::SourceID& b) {
    return (a.observationDomainId == b.observationDomainId) &&
            (a.exporterAddress.len == b.exporterAddress.len) &&
            (memcmp(a.exporterAddress.ip, b.exporterAddress.ip, a.exporterAddress.len) == 0 );

}


/**
 * (re)connect to database
 */
int IpfixDbWriter::connectToDB()
{
    ostringstream statement;

    dbError = true;

    // close (in the case that it was already connected)
    if (conn) mysql_close(conn);

    /** get the mysl init handle*/
    conn = mysql_init(0);
    if(conn == 0) {
        msg(MSG_FATAL,"IpfixDbWriter: Get MySQL connect handle failed. Error: %s",
            mysql_error(conn));
        return 1;
    }
    msg(MSG_DEBUG,"IpfixDbWriter: mysql init successful");

    /**Connect to Database*/
    if (!mysql_real_connect(conn, dbHost.c_str(), dbUser.c_str(), dbPassword.c_str(),
                            0, dbPort, 0, 0)) {
        msg(MSG_FATAL,"IpfixDbWriter: Connection to database failed. Error: %s",
            mysql_error(conn));
        return 1;
    }
    msg(MSG_DEBUG,"IpfixDbWriter: succesfully connected to database");

    /** make query string to create database**/
    statement << "CREATE DATABASE IF NOT EXISTS " << dbName;
    DPRINTF("SQL Query: %s", statement.str().c_str());

    /**create database*/
    if(mysql_query(conn, statement.str().c_str()) != 0 ) {
        msg(MSG_FATAL, "IpfixDbWriter: Creation of database %s failed. Error: %s",
            dbName.c_str(), mysql_error(conn));
        return 1;
    }
    msg(MSG_INFO,"IpfixDbWriter: Database %s created", dbName.c_str());

    /** use database with dbName**/
    if(mysql_select_db(conn, dbName.c_str()) !=0) {
        msg(MSG_FATAL, "IpfixDbWriter: Database %s not selectable. Error: %s",
            dbName.c_str(), mysql_error(conn));
        return 1;
    }
    msg(MSG_DEBUG,"IpfixDbWriter: Database %s selected", dbName.c_str());

    /**create table exporter*/
    statement.str("");
    statement.clear();
    statement << "CREATE TABLE IF NOT EXISTS exporter (id SMALLINT(5) NOT NULL AUTO_INCREMENT, sourceID INTEGER(10) UNSIGNED DEFAULT NULL, srcIP INTEGER(10) UNSIGNED DEFAULT NULL, PRIMARY KEY(id))";
    DPRINTF("SQL Query: %s", statement.str().c_str());
    if(mysql_query(conn, statement.str().c_str()) != 0) {
        msg(MSG_FATAL,"IpfixDbWriter: Creation of exporter table failed. Error: %s",
            mysql_error(conn));
        return 1;
    }
    msg(MSG_INFO,"IpfixDbWriter: Exporter table created");

    dbError = false;

    for (vector<IpfixDbColumn *>::iterator it = tableColumns.begin(); it < tableColumns.end(); it++) {
        IpfixDbColumn *col = *it;
        IpfixDbMySQLSerializer *serializer = (IpfixDbMySQLSerializer *) col->serializer();

        serializer->setConnection(conn);
    }
    return 0;
}


/**
 * save record to database
 */
void IpfixDbWriter::processDataDataRecord(const IpfixRecord::SourceID& sourceID,
                                          TemplateInfo& dataTemplateInfo, uint16_t length,
                                          IpfixRecord::Data* data)
{
    string rowString;
    time_t flowStartSeconds;

    DPRINTF("Processing data record");

    if (dbError) {
        connectToDB();
        if (dbError) return;
    }

    /* get new insert */
    if(srcId.observationDomainId != 0) {
        // use default source id
        rowString = getInsertString(rowString, flowStartSeconds, srcId, dataTemplateInfo, length, data);
    } else {
        rowString = getInsertString(rowString, flowStartSeconds, sourceID, dataTemplateInfo, length, data);
    }

    // if current table is not ok, write to db and get new table name
    if(!(flowStartSeconds >= currentTable.startTime && flowStartSeconds <= currentTable.endTime)) {
        if(numberOfInserts > 0) {
            msg(MSG_DEBUG, "IpfixDbWriter: Writing buffered records to database");
            writeToDb();
            numberOfInserts = 0;
        }
        if (setCurrentTable(flowStartSeconds) != 0) {
            return;
        }
    }


    // start new insert statement if necessary
    if (numberOfInserts == 0) {
        // start insert statement
        insertStatement.str("");
        insertStatement.clear();
        insertStatement << "INSERT INTO " << currentTable.name << " (" << tableColumnsString << ") VALUES " << rowString;
        numberOfInserts = 1;
    } else {
        // append insert statement
        insertStatement << "," << rowString;
        numberOfInserts++;
    }

    // write to db if maxInserts is reached
    if(numberOfInserts == maxInserts) {
        msg(MSG_DEBUG, "IpfixDbWriter: Writing buffered records to database");
        writeToDb();
        numberOfInserts = 0;
    }
}


TemplateInfo::FieldInfo *IpfixDbWriter::getField(TemplateInfo &templateInfo,
                                                 IpfixRecord::Data *&data,
                                                 InformationElement::IeId ieId,
                                                 InformationElement::IeEnterpriseNumber ieEnterprise) const {
    TemplateInfo::FieldInfo *fieldInfo = templateInfo.getFieldInfo(ieId, ieEnterprise);

    if (fieldInfo) {
        data += fieldInfo->offset;

        return fieldInfo;
    }

    fieldInfo = templateInfo.getDataInfo(ieId, ieEnterprise);

    if (fieldInfo) {
        data = templateInfo.data + fieldInfo->offset;
    }

    return fieldInfo;
}

/**
 *	loop over table columns and template to get the IPFIX values in correct order to store in database
 *	The result is written into row, the firstSwitched time is returned in flowstartsec
 */
string& IpfixDbWriter::getInsertString(string& row, time_t& flowstartsec, const IpfixRecord::SourceID& sourceID,
                                       TemplateInfo& dataTemplateInfo,uint16_t length, IpfixRecord::Data* data)
{
    string value;
    bool notfound;
    bool first = true;
    ostringstream rowStream(row);

    flowstartsec = 0;
    rowStream << "(";

    /**loop over the columname and loop over the IPFIX_TYPEID of the record
  to get the corresponding data to store and make insert statement*/
    for(vector<IpfixDbColumn *>::iterator colIter = tableColumns.begin(); colIter != tableColumns.end(); colIter++) {
        IpfixDbColumn *col = *colIter;

        if (col->ieId() == EXPORTERID) {
            // if this is the same source ID as last time, we get the exporter id from currentExporter
            ostringstream str;

            if ((currentExporter != NULL) && equalExporter(sourceID, currentExporter->sourceID)) {
                DPRINTF("Exporter is same as last time (ODID=%d, id=%d)", sourceID.observationDomainId, currentExporter->id);

                str << (uint64_t) currentExporter->id;
            } else {
                // lookup exporter buffer to get exporterID from sourcID and expIp
                str << (uint64_t) getExporterID(sourceID);
            }

            value = str.str();
        } else {
            notfound = true;

            IpfixRecord::Data *startData = data;
            TemplateInfo::FieldInfo *fieldInfo = getField(dataTemplateInfo, startData, col->ieId(), col->ieEnterpriseId());

            if (fieldInfo) {
                notfound = false;
                value = col->serializer()->serializeValue(startData, fieldInfo);
            }

            if(notfound) {
                // for some Ids, we have an alternative
                if(col->ieEnterpriseId() == 0) {
                    switch (col->ieId()) {
                    case IPFIX_TYPEID_flowStartSeconds:

                        startData = data;
                        fieldInfo = getField(dataTemplateInfo, startData, IPFIX_TYPEID_flowStartMilliSeconds, 0);

                        if (fieldInfo) {
                            value = col->serializer()->serializeValue(startData, fieldInfo);
                            notfound = false;
                        } else {
                            startData = data;
                            fieldInfo = getField(dataTemplateInfo, startData, IPFIX_TYPEID_flowStartSysUpTime, 0);

                            if (fieldInfo) {
                                value = col->serializer()->serializeValue(startData, fieldInfo);
                                notfound = false;
                            }
                        }
                        break;

                    case IPFIX_TYPEID_flowEndSeconds:
                        startData = data;
                        fieldInfo = getField(dataTemplateInfo, startData, IPFIX_TYPEID_flowEndMilliSeconds, 0);

                        if (fieldInfo) {
                            value = col->serializer()->serializeValue(startData, fieldInfo);
                            notfound = false;
                        } else {
                            startData = data;
                            fieldInfo = getField(dataTemplateInfo, startData, IPFIX_TYPEID_flowEndSysUpTime, 0);

                            if (fieldInfo) {
                                value = col->serializer()->serializeValue(startData, fieldInfo);
                                notfound = false;
                            }
                        }

                        break;
                    }
                } else if (col->ieEnterpriseId()==IPFIX_PEN_reverse) {
                    switch (col->ieId()) {
                    case IPFIX_TYPEID_flowStartSeconds:
                        startData = data;
                        fieldInfo = getField(dataTemplateInfo, startData, IPFIX_TYPEID_flowStartMilliSeconds, IPFIX_PEN_reverse);

                        if (fieldInfo) {
                            value = col->serializer()->serializeValue(startData, fieldInfo);
                            notfound = false;
                        }
                        break;
                    case IPFIX_TYPEID_flowEndSeconds:
                        startData = data;
                        fieldInfo = getField(dataTemplateInfo, startData, IPFIX_TYPEID_flowEndMilliSeconds, IPFIX_PEN_reverse);

                        if (fieldInfo) {
                            value = col->serializer()->serializeValue(startData, fieldInfo);
                            notfound = false;
                        }
                        break;
                    }
                }

                if (notfound && !col->defaultValue().empty())
                    value = col->defaultValue();
            }

            if(col->ieEnterpriseId() == 0 ) {
                switch (col->ieId()) {
                case IPFIX_TYPEID_flowStartSeconds:
                    // save time for table access
                    if (flowstartsec == 0) {
                        flowstartsec = strtoul(value.c_str(), NULL, 10);
                    }
                    break;
                case IPFIX_TYPEID_flowStartMilliSeconds:
                    // if flowStartSeconds is not stored in one of the columns, but flowStartMilliSeconds is,
                    // then we use flowStartMilliSeconds for table access
                    // This is realized by storing this value only if flowStartSeconds has not yet been seen.
                    // A later appearing flowStartSeconds will override this value.
                    if (flowstartsec == 0) {
                        flowstartsec = strtoul(value.c_str(), NULL, 10) / 1000;
                    }
                }
            }
        }

        if(!first)
            rowStream << ", ";

        rowStream << value;

        first = false;
    }

    rowStream << ")";

    if (flowstartsec == 0) {
        msg(MSG_ERROR, "IpfixDbWriter: Failed to get timing data from record. Will be saved in default table.");
    }

    row = rowStream.str();
    DPRINTF("Insert row: %s", row.c_str());

    return row;
}


/*
 * Write insertStatement to database
 */
int IpfixDbWriter::writeToDb()
{
    DPRINTF("SQL Query: %s", insertStatement.str().c_str());
    if(mysql_query(conn, insertStatement.str().c_str()) != 0) {
        msg(MSG_ERROR,"IpfixDbWriter: Insert of records failed. Error: %s", mysql_error(conn));
        return 1;
    }
    msg(MSG_DEBUG,"IpfixDbWriter: Write to database is complete");
    return 0;
}


/*
 * Sets the current table information and creates the table in the database if necessary
 */
int IpfixDbWriter::setCurrentTable(time_t flowstartsec)
{
    // generate table name
    ostringstream tableStream;
    struct tm* flowStartTime = gmtime(&flowstartsec);

    tableStream << "h_" << (flowStartTime->tm_year+1900)
                << setfill('0') << setw(2) << (flowStartTime->tm_mon+1)
                << setfill('0') << setw(2) << (flowStartTime->tm_mday) << "_"
                << setfill('0') << setw(2) << (flowStartTime->tm_hour) << "_"
                << setw(1) << (flowStartTime->tm_min<30?0:1);

    currentTable.name = tableStream.str();

    // calculate table boundaries
    if(flowStartTime->tm_min < 30) {
        flowStartTime->tm_min = 0;
        flowStartTime->tm_sec = 0;
        currentTable.startTime = timegm(flowStartTime);
    } else {
        flowStartTime->tm_min = 30;
        flowStartTime->tm_sec = 0;
        currentTable.startTime = timegm(flowStartTime);
    }
    currentTable.endTime = currentTable.startTime + 1799;

    DPRINTF("flowstartsec: %d, table name: %s, start time: %d, end time: %d", flowstartsec, currentTable.name.c_str(), currentTable.startTime, currentTable.endTime);

    // create table if exists
    ostringstream createStatement;
    createStatement << "CREATE TABLE IF NOT EXISTS " << currentTable.name << " (" << tableColumnsCreateString << ")";
    DPRINTF("SQL Query: %s", createStatement.str().c_str());
    if(mysql_query(conn, createStatement.str().c_str()) != 0) {
        msg(MSG_FATAL,"IpfixDbWriter: Creation of table failed. Error: %s", mysql_error(conn));
        dbError = true;
        return 1;
    }
    msg(MSG_DEBUG, "IpfixDbWriter: Table %s created ", currentTable.name.c_str());

    return 0;
}

/**
 *	Returns the id of the exporter table entry or 0 in the case of an error
 */
int IpfixDbWriter::getExporterID(const IpfixRecord::SourceID& sourceID)
{
    list<ExporterCacheEntry>::iterator iter;
    MYSQL_RES* dbResult;
    MYSQL_ROW dbRow;
    int id = 0;
    uint32_t expIp = 0;
    ostringstream statement;

    iter = exporterCache.begin();
    while(iter != exporterCache.end()) {
        if (equalExporter(iter->sourceID, sourceID)) {
            // found exporter in exporterCache
            DPRINTF("Exporter (ODID=%d, id=%d) found in exporter cache", sourceID.observationDomainId, iter->id);
            exporterCache.push_front(*iter);
            exporterCache.erase(iter);
            // update current exporter
            currentExporter = &exporterCache.front();
            return exporterCache.front().id;
        }
        iter++;
    }

    // convert IP address (correct host byte order since 07/2010)
    expIp = sourceID.exporterAddress.toUInt32();

    // search exporter table
    statement << "SELECT id FROM exporter WHERE sourceID=" << sourceID.observationDomainId << " AND srcIp=" << expIp;
    DPRINTF("SQL Query: %s", statement.str().c_str());

    if(mysql_query(conn, statement.str().c_str()) != 0) {
        msg(MSG_ERROR,"IpfixDbWriter: Select on exporter table failed. Error: %s",
            mysql_error(conn));
        return 0;// If a failure occurs, return 0
    }

    dbResult = mysql_store_result(conn);
    if(( dbRow = mysql_fetch_row(dbResult))) {
        // found in table
        id = atoi(dbRow[0]);
        mysql_free_result(dbResult);
        DPRINTF("ExporterID %d is in exporter table", id);
    } else {
        mysql_free_result(dbResult);
        // insert new exporter table entry
        statement.str("");
        statement.clear();
        statement << "INSERT INTO exporter (ID,sourceID,srcIP) VALUES ('NULL','" << sourceID.observationDomainId << "','" << expIp << "')";
        DPRINTF("SQL Query: %s", statement.str().c_str());
        if(mysql_query(conn, statement.str().c_str()) != 0) {
            msg(MSG_ERROR,"IpfixDbWriter: Insert in exporter table failed. Error: %s", conn);
            return 0;
        }

        id = mysql_insert_id(conn);
        msg(MSG_INFO,"IpfixDbWriter: new exporter (ODID=%d, id=%d) inserted in exporter table", sourceID.observationDomainId, id);
    }

    // insert exporter in cache
    ExporterCacheEntry tmp = {sourceID, id};
    exporterCache.push_front(tmp);

    // update current exporter
    currentExporter = &exporterCache.front();

    // pop last element if exporter cache is to long
    if(exporterCache.size() > MAX_EXPORTER)
        exporterCache.pop_back();

    return id;
}

/***** Public Methods ****************************************************/

/**
 * called on Data Record arrival
 */
void IpfixDbWriter::onDataRecord(IpfixDataRecord* record)
{
    // only treat non-Options Data Records (although we cannot be sure that there is a Flow inside)
    if((record->templateInfo->setId != TemplateInfo::NetflowTemplate)
            && (record->templateInfo->setId != TemplateInfo::IpfixTemplate)
            && (record->templateInfo->setId != TemplateInfo::IpfixDataTemplate)) {
        record->removeReference();
        return;
    }

    processDataDataRecord(*record->sourceID.get(), *record->templateInfo.get(),
                          record->dataLength, record->data);

    record->removeReference();
}


/**
 * Constructor
 */
IpfixDbWriter::IpfixDbWriter(const string& hostname, const string& dbname,
                             const string& username, const string& password,
                             unsigned port, uint32_t observationDomainId, unsigned maxStatements,
                             vector<IpfixDbColumn *>& columns)
    : currentExporter(NULL), numberOfInserts(0), maxInserts(maxStatements), tableColumns(columns),
      dbHost(hostname), dbName(dbname), dbUser(username), dbPassword(password), dbPort(port),  conn(0)
{
    // set default source id
    srcId.exporterAddress.len = 0;
    srcId.observationDomainId = observationDomainId;
    srcId.exporterPort = 0;
    srcId.receiverPort = 0;
    srcId.protocol = 0;
    srcId.fileDescriptor = 0;

    // invalide start settings for current table (to enforce table create)
    currentTable.startTime = 1;
    currentTable.endTime = 0;

    if(columns.empty())
        THROWEXCEPTION("IpfixDbWriter: cannot initiate with no columns");

    /* get columns */
    bool first = true;
    for(vector<IpfixDbColumn *>::const_iterator col = columns.begin(); col != columns.end(); col++) {
        // update tableColumnsString
        if(!first)
            tableColumnsString.append(",");
        tableColumnsString.append((*col)->name());
        // update tableColumnsCreateString
        if(!first)
            tableColumnsCreateString.append(", ");
        tableColumnsCreateString.append((*col)->name());
        tableColumnsCreateString.append(" ");
        tableColumnsCreateString.append((*col)->dbColumnTypeDefinition());

        first = false;
    }
    msg(MSG_INFO, "IpfixDbWriter: columns are %s", tableColumnsString.c_str());

    if(connectToDB() != 0)
        THROWEXCEPTION("IpfixDbWriter creation failed");
}

/**
 * Destructor
 */
IpfixDbWriter::~IpfixDbWriter()
{
    writeToDb();
    mysql_close(conn);
}

#endif
