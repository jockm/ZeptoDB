/*
 * ZeptoTable.h
 *
 *  Created on: Apr 17, 2018
 *      Author: jock
 */

#ifndef ZEPTODB_ZEPTOTABLE_H_
#define ZEPTODB_ZEPTOTABLE_H_

#include <stdio.h>

typedef enum _zfieldtype {
	ZID      = 0,
	ZSTRING = 1,
	ZINT    = 2,
	ZEOL    = 0xFF,
} ZFieldType;


typedef uint32_t ZId;


struct {
	ZFieldType type;
	uint16_t   len;
};


class ZeptoTable
{
	public:
		ZeptoTable(const char *path);

		bool exists(void);
		bool create(ZFieldType *dict);

		bool fetch(ZId id);
		bool insert(void);
		bool update(ZId id);
		bool remove(ZId id);

		int8_t   getInt(uint8_t fieldNo);
		int64_t  getInt64(uint8_t fieldNo);
		char    *getString(uint8_t fieldNo);

		void setInt(uint8_t fieldNo, int32_t v);
		void setInt64(uint8_t fieldNo, int64_t v);
		void setString(uint8_t fieldNo, const char *v);


	private:
		FILE *tableFile;
		uint16_t recordLen;
		ZFieldType dict[20];
};

#endif /* ZEPTODB_ZEPTOTABLE_H_ */
