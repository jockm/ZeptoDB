/*
 * ZeptoTable.cpp
 *
 *  Created on: Apr 17, 2018
 *      Author: jock
 */

#include <ZeptoDB/ZeptoTable.h>
<<<<<<< HEAD
#include <string.h>

static enum {
	HEADER_SIZE = 100,
	MAX_FIELDS = 20,
	ID_LEN = 4,
	ZSIGNATURE = 0x5a504442, // FourCC "ZPDB"
};


ZeptoTable::ZeptoTable(const char *path, ZFieldType *dict)
{
	this->tableFile = NULL;
	this->recordLen = 0;
	this->recordCount = 0;
	this->tableExists = false;

	this->tableFile = fopen(path, "ab+");
	if(this->tableFile) {
		fseek(this->tableFile, 0, SEEK_END);
		long fileSize = ftell(this->tableFile);

		if(fileSize >= HEADER_SIZE) {
			tableExists = this->readHeader();
		}
	}
}


virtual ZeptoTable::~ZeptoTable()
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

	this->recordCount = 0;
	this->recordLen = 0;

	uint8_t header[HEADER_SIZE];
	uint8_t i = 0;
	uint16_t pos = 0;

	memset(header, 0, sizeof(header));

	header[pos++] = 'Z';
	header[pos++] = 'P';
	header[pos++] = 'D';
	header[pos++] = 'B';

	// Skip over record count for now
	++pos;

	while(dict[i].type != ZEOF && i < MAX_FIELDS) {
		this->dict[i].type = dict[i].type;
		this->dict[i].len = dict[i].len;

		header[pos++] = dict[i].type;
		header[pos++] = dict[i].len >> 8;
		header[pos++] = dict[i].len & 0xFF;

		++i;
		++this->recordCount;
		this->recordLen += dict[i].len;
	}

	header[4] = this->recordCount;

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
	// TODO Implement me
	return 0;
}



bool ZeptoTable::insert(void)
{
	// TODO Implement me
	return 0;
}



bool ZeptoTable::update(ZId id)
{
	// TODO Implement me
	return 0;
}



bool ZeptoTable::remove(ZId id)
{
	// TODO Implement me
	return 0;
}




int8_t   ZeptoTable::getInt(uint8_t fieldNo)
{
	// TODO Implement me
	return 0;
}



int64_t  ZeptoTable::getInt64(uint8_t fieldNo)
{
	// TODO Implement me
	return 0;
}



char    *ZeptoTable::getString(uint8_t fieldNo)
{
	// TODO Implement me
	return 0;
}




void ZeptoTable::setInt(uint8_t fieldNo, int32_t v)
{
	// TODO Implement me
}



void ZeptoTable::setInt64(uint8_t fieldNo, int64_t v)
{
	// TODO Implement me
}



void ZeptoTable::setString(uint8_t fieldNo, const char *v)
{
	// TODO Implement me
}


bool ZeptoTable::readHeader(void)
{
	uint8_t header[HEADER_SIZE];

	fseek(this->tableFile, 0, SEEK_SET);
	int charsRead = fread(header, sizeof(uint8_t), sizeof(header), this->tableFile);

	if(charsRead != sizeof(header)) {
		return false;
	}

	if(strncmp("ZPDB", (const char *)header, 4) != 0) {
		return false;
	}

	this->recordCount = header[4];
	if(this->recordCount == 0 || this->recordCount >= MAX_FIELDS) {
		return false;
	}

	this->recordLen = 0;
	for(uint8_t i = 0; i < this->recordCount; ++i) {
		uint8_t pos = (i * 3) + 5;

		dict[i].type = header[pos];
		dict[i].len = (header[pos + 1] << 8) | header[pos + 2];

		this->recordLen += dict[i].len;
	}

	if(this->recordLen > sizeof(this->currRec)) {
		return false;
	}

	return true;
}
=======
>>>>>>> master

