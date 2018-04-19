/*
 * ZeptoQuery.cpp
 *
 *  Created on: Apr 18, 2018
 *      Author: jock
 */

#include <stdlib.h>

#include "ZeptoQuery.h"

ZeptoQuery::ZeptoQuery()
{
	this->pos = 0;
	this->clear();
}

ZeptoQuery::~ZeptoQuery()
{
	// Nothing
}

void ZeptoQuery::clear()
{
	this->pos = 0;

	uint8_t max = sizeof(this->query) / sizeof(ZeptoQueryItem);

	for(uint8_t i = 0; i < max; ++i) {
		this->query[i].op = ZNOP;
	}
}

ZeptoQuery *ZeptoQuery::eq(uint32_t id, uint64_t v)
{
	return this->add(ZEQ, id, v);
}


ZeptoQuery *ZeptoQuery::ne(uint32_t id, uint64_t v)
{
	return this->add(ZNE, id, v);
}


ZeptoQuery *ZeptoQuery::gt(uint32_t id, uint64_t v)
{
	return this->add(ZGT, id, v);
}


ZeptoQuery *ZeptoQuery::ge(uint32_t id, uint64_t v)
{
	return this->add(ZGE, id, v);
}


ZeptoQuery *ZeptoQuery::lt(uint32_t id, uint64_t v)
{
	return this->add(ZLT, id, v);
}


ZeptoQuery *ZeptoQuery::le(uint32_t id, uint64_t v)
{
	return this->add(ZLE, id, v);
}


ZeptoQuery *ZeptoQuery::add(ZQueryOp op, uint32_t id, uint64_t v)
{
	uint8_t max = sizeof(this->query) / sizeof(ZeptoQueryItem);
	if(this->pos < max) {
		this->query[this->pos].op = op;
		this->query[this->pos].id = id;
		this->query[this->pos].value = v;

		++this->pos;
	}

	return this;
}

ZeptoQueryItem *ZeptoQuery::getItem(uint8_t idx)
{
	if(idx >= this->pos) {
		return NULL;
	}

	return &this->query[idx];
}


uint8_t ZeptoQuery::getCount() {
	uint8_t ret = this->pos <= 0 ? 0 : this->pos;

	return ret;
}

