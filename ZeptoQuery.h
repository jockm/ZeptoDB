/*
 * ZeptoQuery.h
 *
 *  Created on: Apr 18, 2018
 *      Author: jock
 */

#ifndef ZEPTODB_ZEPTOQUERY_H_
#define ZEPTODB_ZEPTOQUERY_H_

#include <stdint.h>

typedef enum zqueryop {
	ZNOP = 0,
	ZEQ  = 1,
	ZNE  = 2,
	ZLT  = 3,
	ZLE  = 4,
	ZGT  = 5,
	ZGE  = 6,
} ZQueryOp;


typedef struct {
		ZQueryOp op;
		int32_t id;
		int64_t value;
} ZeptoQueryItem;

class ZeptoQuery
{
	public:
		ZeptoQuery();
		virtual ~ZeptoQuery();

		void clear();

		ZeptoQuery *eq(uint32_t id, int64_t v);
		ZeptoQuery *ne(uint32_t id, int64_t v);
		ZeptoQuery *gt(uint32_t id, int64_t v);
		ZeptoQuery *ge(uint32_t id, int64_t v);
		ZeptoQuery *lt(uint32_t id, int64_t v);
		ZeptoQuery *le(uint32_t id, int64_t v);

		ZeptoQueryItem *getItem(uint8_t idx);

		bool isTrue(uint8_t qIdx, int64_t v);

		uint8_t getCount() {
			return this->pos <= 0 ? 0 : this->pos;
		}

	private:
		ZeptoQuery *add(ZQueryOp op, uint32_t id, int64_t v);

		ZeptoQueryItem query[5];
		uint8_t        pos;
};

#endif /* ZEPTODB_ZEPTOQUERY_H_ */
