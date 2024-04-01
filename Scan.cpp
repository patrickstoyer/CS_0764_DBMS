#include "Scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include "Record.h"

ScanPlan::ScanPlan (RowCount const count) : _count (count)
{
	TRACE (true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
	TRACE (true);
} // ScanPlan::~ScanPlan

Iterator * ScanPlan::init () const
{
	TRACE (true);
	return new ScanIterator (this);
} // ScanPlan::init

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (true);
    _file = fopen("inputfile.txt", "w");
	_buffer = (char *)malloc(constants::PAGE_SIZE);
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	TRACE (true);
	fclose(_file);
	free(_buffer);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next ()
{
	TRACE (true);

	if (_count >= _plan->_count)
		return false;

	createNextRecord();
	++ _count;
	return true;
} // ScanIterator::next

void ScanIterator::createNextRecord()
{
	currentRecord = generateNewRecord();
	currentRecord->storeRecord(_buffer,_bufferIndex,_file,(_count == _plan->_count - 1));
} // ScanIterator::createNextRecord 


Record * ScanIterator::generateNewRecord ()
{
	static const char range[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	char * key = (char *) malloc (constants::KEY_SIZE);
	char * data = (char *) malloc (constants::RECORD_SIZE);
	
  	std::default_random_engine generator;
  	std::uniform_int_distribution<char> distribution(0,61);

	int j = 0;
	for (int i = 0; i < constants::KEY_SIZE; i++,j++)
	{
		key[i] = range[distribution(generator)];
	}
	for (int i = 0; i < constants::RECORD_SIZE; i++,j++)
	{
		data[i] = range[distribution(generator)];	
	}
	return new Record(key,data,0);// Index doesn't matter for this, just use 0
} // ScanIterator::generateNewRecord 
