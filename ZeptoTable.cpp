/*
 * ZeptoTable.cpp
 *
 *  Created on: Apr 17, 2018
 *      Author: jock
 */

#include "ZeptoTable.h"
#include <string.h>

enum {
	ZHEADER_SIZE = 100,
	ZMAX_FIELDS = 20,
	ZID_LEN = 4,
	ZSIGNATURE = 0x5a504442, // FourCC "ZPDB"
};


ZeptoTable::ZeptoTable(const char *path, ZFieldType *dict)
{
	this->query.clear();

	this->currQueryId = 0;
	this->tableFile = NULL;
	this->recordLen = 0;
	this->fieldCount = 0;
	this->tableExists = false;

	this->tableFile = fopen(path, "ab+");
	if(this->tableFile) {
		uint64_t fileSize = getFileSize();
		if(fileSize >= ZHEADER_SIZE) {
			tableExists = this->readHeader();
		}
	}
}


ZeptoTable::~ZeptoTable()
{
	this->close();
}


void ZeptoTable::close(void)
{
	if(this->tableFile) {
		fflush(this->tableFile);
		fclose(this->tableFile);
	}
}

bool ZeptoTable::exists(void)
{
	return this->tableExists;
}


bool ZeptoTable::create(ZField *dict)
{
	if(this->tableExists) {
		return false;
	}

	this->fieldCount = 0;
	this->recordLen = 0;

	uint8_t header[ZHEADER_SIZE];
	uint8_t i = 0;
	uint16_t pos = 0;

	memset(header, 0, sizeof(header));

	header[pos++] = 'Z';
	header[pos++] = 'P';
	header[pos++] = 'D';
	header[pos++] = 'B';

	// Skip over record count for now
	++pos;

	while(dict[i].type != ZEOF && i < ZMAX_FIELDS) {
		this->dict[i].type = dict[i].type;
		this->dict[i].len = dict[i].len;

		header[pos++] = dict[i].type;
		header[pos++] = dict[i].len >> 8;
		header[pos++] = dict[i].len & 0xFF;

		++i;
		++this->fieldCount;
		this->recordLen += dict[i].len;
	}

	header[4] = this->fieldCount;

	int charsWritten = fwrite(header, sizeof(uint8_t), sizeof(header), this->tableFile);
	if(charsWritten != sizeof(header)) {
		this->tableExists = false;
		this->close();
		// TODO can we delete the file as well?

		return false;
	}

	this->tableExists = true;

	return true;
}




bool ZeptoTable::fetch(ZId id)
{
	if(!this->tableExists) {
		return false;
	}

	if(id < 0 || id >= this->getTableRecordCount()) {
		return false;
	}
	uint64_t recordPos = this->getRecordPos(id);
	fseek(this->tableFile, recordPos, SEEK_SET);

	int16_t len = fread(this->currRec, sizeof(uint8_t), sizeof(this->currRec), this->tableFile);

	if(len != this->recordLen) {
		return false;
	}

	// Was the record deleted?
	if(this->getInt32(0) < 0) {
		return false;
	}

	return true;
}



bool ZeptoTable::insert(void)
{
	if(!this->tableExists) {
		return false;
	}

	ZId id = this->getTableRecordCount();
	this->setInt32(0, id);

	fseek(this->tableFile, 0, SEEK_END);
	int32_t len = fwrite(this->currRec, sizeof(uint8_t), sizeof(this->currRec), this->tableFile);

	if(len != this->recordLen) {
		return false;
	}

	return true;
}



bool ZeptoTable::update()
{
	if(!this->tableExists) {
		return false;
	}

	ZId id = this->getInt32(0);

	if(id >= this->getTableRecordCount()) {
		return false;
	}

	uint64_t recordPos = this->getRecordPos(id);
	fseek(this->tableFile, recordPos, SEEK_SET);

	int32_t len = fwrite(this->currRec, sizeof(uint8_t), sizeof(this->currRec), this->tableFile);

	if(len != this->recordLen) {
		return false;
	}

	return true;
}



bool ZeptoTable::remove(ZId id)
{
	if(!this->tableExists) {
		return false;
	}

	if(!this->fetch(id)) {
		return false;
	}

	if(id < 0) {
		return true;
	}

	this->setInt32(0, id * -1);

	bool ret = this->update();
	return ret;
}



int32_t   ZeptoTable::getInt(uint8_t fieldNo)
{
	if(!this->tableExists) {
		return false;
	}

	if(fieldNo >= this->fieldCount) {
		return 0;
	}

	if(this->dict[fieldNo].type != ZINT) {
		return 0;
	}

	int16_t fieldPos = this->getFieldPos(fieldNo);
	if(fieldPos < 0) {
		return 0;
	}

	int32_t ret = 0;

	switch(this->dict[fieldNo].len) {
		case 1:
			ret = this->getInt8(fieldPos);
			break;

		case 2:
			ret = this->getInt16(fieldPos);
			break;

		case 4:
			ret = this->getInt32(fieldPos);
			break;

		default:
			ret = this->getInt8(fieldPos);
			break;
	}


	return ret;
}



int64_t  ZeptoTable::getLong(uint8_t fieldNo)
{
	if(!this->tableExists) {
		return false;
	}

	if(fieldNo >= this->fieldCount) {
		return 0;
	}

	if(this->dict[fieldNo].type != ZINT) {
		return 0;
	}

	int16_t fieldPos = this->getFieldPos(fieldNo);
	if(fieldPos < 0) {
		return 0;
	}

	int64_t ret = 0;

	switch(this->dict[fieldNo].len) {
		case 1:
			ret = this->getInt8(fieldPos);
			break;

		case 2:
			ret = this->getInt16(fieldPos);
			break;

		case 4:
			ret = this->getInt32(fieldPos);
			break;

		case 8:
			ret = this->getInt64(fieldPos);
			break;

		default:
			ret = this->getInt8(fieldPos);
			break;
	}


	return ret;
}



char    *ZeptoTable::getString(uint8_t fieldNo)
{
	if(!this->tableExists) {
		return NULL;
	}

	if(fieldNo >= this->fieldCount) {
		return 0;
	}

	if(this->dict[fieldNo].type != ZSTRING) {
		return 0;
	}

	int16_t fieldPos = this->getFieldPos(fieldNo);
	if(fieldPos < 0) {
		return 0;
	}

	char *ret = (char *) (this->currRec + fieldPos);
	return ret;
}




void ZeptoTable::setInt(uint8_t fieldNo, int32_t v)
{
	if(!this->tableExists) {
		return;
	}

	if(fieldNo >= this->fieldCount) {
		return;
	}

	if(this->dict[fieldNo].type != ZINT) {
		return;
	}

	int16_t fieldPos = this->getFieldPos(fieldNo);
	if(fieldPos < 0) {
		return;
	}

	switch(this->dict[fieldNo].len) {
		case 1:
			this->setInt8(fieldPos, (int8_t) v);
			break;

		case 2:
			this->setInt16(fieldPos, (int16_t) v);
			break;

		case 4:
			this->setInt32(fieldPos, v);
			break;

		default:
			this->setInt8(fieldPos, (int8_t) v);
			break;
	}
}



void ZeptoTable::setLong(uint8_t fieldNo, int64_t v)
{
	if(!this->tableExists) {
		return;
	}

	if(fieldNo >= this->fieldCount) {
		return;
	}

	if(this->dict[fieldNo].type != ZINT) {
		return;
	}

	int16_t fieldPos = this->getFieldPos(fieldNo);
	if(fieldPos < 0) {
		return;
	}

	switch(this->dict[fieldNo].len) {
		case 1:
			this->setInt8(fieldPos, (int8_t) v);
			break;

		case 2:
			this->setInt16(fieldPos, (int16_t) v);
			break;

		case 4:
			this->setInt32(fieldPos, (int32_t) v);
			break;

		case 8:
			this->setInt64(fieldPos, v);
			break;

		default:
			this->setInt8(fieldPos, (int8_t) v);
			break;
	}
}



void ZeptoTable::setString(uint8_t fieldNo, const char *v)
{
	if(!this->tableExists) {
		return;
	}

	if(fieldNo >= this->fieldCount) {
		return;
	}

	if(this->dict[fieldNo].type != ZSTRING) {
		return;
	}

	int16_t fieldPos = this->getFieldPos(fieldNo);
	if(fieldPos < 0) {
		return;
	}

	strncpy((char *) (this->currRec + fieldPos), v, this->dict[fieldNo].len);
	this->currRec[fieldPos + this->dict[fieldNo].len - 1] = '\0';
}


ZeptoQuery *ZeptoTable::where()
{
	this->currQueryId = 0;

	this->query.clear();
	return &this->query;
}


bool ZeptoTable::next()
{
	int32_t recordCount = this->getTableRecordCount();
	bool matched = false;

	while(!matched && this->currQueryId < recordCount) {
		this->fetch(this->currQueryId++);

		matched = this->doesRecordMatchQuery();
	}

	return matched;
}


bool ZeptoTable::readHeader(void)
{
	uint8_t header[ZHEADER_SIZE];

	fseek(this->tableFile, 0, SEEK_SET);
	int charsRead = fread(header, sizeof(uint8_t), sizeof(header), this->tableFile);

	if(charsRead != sizeof(header)) {
		return false;
	}

	if(strncmp("ZPDB", (const char *)header, 4) != 0) {
		return false;
	}

	this->fieldCount = header[4];
	if(this->fieldCount == 0 || this->fieldCount >= ZMAX_FIELDS) {
		return false;
	}

	this->recordLen = 0;
	for(uint8_t i = 0; i < this->fieldCount; ++i) {
		uint8_t pos = (i * 3) + 5;

		dict[i].type = (ZFieldType) header[pos];
		dict[i].len = (header[pos + 1] << 8) | header[pos + 2];

		this->recordLen += dict[i].len;
	}

	if(this->recordLen > sizeof(this->currRec)) {
		return false;
	}

	return true;
}



int16_t ZeptoTable::getFieldPos(uint8_t fieldNo)
{
	if(fieldNo >= this->fieldCount) {
		return -1;
	}

	int16_t ret = 0;
	for(uint i = 0; i < fieldNo; ++i) {
		ret += this->dict[i].len;
	}

	return ret;
}


int8_t ZeptoTable::getInt8(int16_t pos)
{
	int8_t ret = this->currRec[pos];

	return ret;
}


int16_t ZeptoTable::getInt16(int16_t pos)
{
	int16_t ret = this->currRec[pos] << 8;
	ret |= this->currRec[pos + 1];

	return ret;
}

int32_t ZeptoTable::getInt32(int16_t pos)
{
	int32_t ret = this->currRec[pos] << 24;
	ret |= this->currRec[pos + 1] << 16;
	ret |= this->currRec[pos + 2] << 8;
	ret |= this->currRec[pos + 3];

	return ret;
}

int64_t ZeptoTable::getInt64(int16_t pos)
{
	int64_t ret = ((uint64_t) this->currRec[pos]) << 56;
	ret |= ((uint64_t) this->currRec[pos + 1] << 48);
	ret |= ((uint64_t) this->currRec[pos + 2] << 40);
	ret |= ((uint64_t) this->currRec[pos + 3] << 32);
	ret |= ((uint64_t) this->currRec[pos + 4] << 24);
	ret |= ((uint64_t) this->currRec[pos + 5] << 16);
	ret |= ((uint64_t) this->currRec[pos + 6] << 8);
	ret |= ((uint64_t) this->currRec[pos + 7]);

	return ret;
}


void ZeptoTable::setInt8(int16_t pos, int8_t v)
{
	this->currRec[pos] = v;
}


void ZeptoTable::setInt16(int16_t pos, int16_t v)
{
	this->currRec[pos] = v >> 8;
	this->currRec[pos + 1] = v & 0xFF;
}

void ZeptoTable::setInt32(int16_t pos, int32_t v)
{
	this->currRec[pos] = v >> 24;
	this->currRec[pos + 1] = (v >> 16) & 0xFF;
	this->currRec[pos + 2] = (v >> 8) & 0xFF;
	this->currRec[pos + 3] = v & 0xFF;
}


void ZeptoTable::setInt64(int16_t pos, int64_t v)
{
	this->currRec[pos] = v >> 56;
	this->currRec[pos + 1] = (v >> 48) & 0xFF;
	this->currRec[pos + 2] = (v >> 40) & 0xFF;
	this->currRec[pos + 3] = (v >> 32) & 0xFF;
	this->currRec[pos + 4] = (v >> 24) & 0xFF;
	this->currRec[pos + 5] = (v >> 16) & 0xFF;
	this->currRec[pos + 6] = (v >> 8) & 0xFF;
	this->currRec[pos + 7] = v & 0xFF;
}


uint64_t ZeptoTable::getRecordPos(ZId id)
{
	if(id < 0) {
		id *= -1;
	}
	uint64_t ret = ZHEADER_SIZE + (id * this->recordLen);

	return ret;
}


uint64_t ZeptoTable::getFileSize()
{
	fseek(this->tableFile, 0, SEEK_END);
	long fileSize = ftell(this->tableFile);
	return fileSize;
}


int32_t ZeptoTable::getTableRecordCount()
{
	uint32_t ret = (this->getFileSize() - ZHEADER_SIZE) / this->recordLen;

	return ret;
}


bool ZeptoTable::doesRecordMatchQuery()
{
	bool ret = true;
	uint8_t max = this->query.getCount();

	for(uint8_t i = 0; i < max; ++i) {
		ZeptoQueryItem *qi = this->query.getItem(i);
		if(!qi) {
			ret = false;
			break;
		}

		uint64_t v = this->getLong(qi->value);
		ret = ret && this->query.isTrue(i, v);
	}

	return ret;
}


