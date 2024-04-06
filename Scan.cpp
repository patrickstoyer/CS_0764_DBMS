#include "Scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <random>
#include "Record.h"

ScanPlan::ScanPlan (RowCount const count) : _count (count)
{
//	TRACE (true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
//	TRACE (true);
} // ScanPlan::~ScanPlan

Iterator * ScanPlan::init () const
{
//	TRACE (true);
	return new ScanIterator (this);
} // ScanPlan::init

std::default_random_engine ScanIterator::generator(std::chrono::system_clock::now().time_since_epoch().count());

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0), _bufferIndex(0)
{
//	TRACE (true);
    _file = fopen("inputfile.txt", "w");
	_buffer = new char[HDD_PAGE_SIZE];
    setvbuf(_file,_buffer,_IOFBF,HDD_PAGE_SIZE);
	if (SEED > 0) generator.seed(SEED);
	
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	TRACE (true);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next ()
{
    //TRACE (true);

	if (_count >= _plan->_count)
    {
        fclose(_file);
        delete [] _buffer;
        return false;
    }
	createNextRecord();
	++ _count;
	return true;
} // ScanIterator::next

void ScanIterator::createNextRecord()
{
	this->_currentRecord = *generateNewRecord();
	this->_currentRecord.storeRecord(_file,(_count == _plan->_count - 1));
} // ScanIterator::createNextRecord 


Record * ScanIterator::generateNewRecord ()
{
	static const char range[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	char * data = new char[RECORD_SIZE];
	// If appending new line at end, we'll generate RECORD_SIZE - 1 byte random, then newline;
	int max = (USE_NEWLINES) ? RECORD_SIZE - 1 : RECORD_SIZE;
	std::uniform_int_distribution<char> distribution(0,61);
  	for (int i = 0; i < RECORD_SIZE; i++)
	{
		char next = distribution(generator);
		data[i] = range[next];	
	}
	// Add newline
	if (USE_NEWLINES) data[max] = '\n';
	return new Record(data,0);// Index doesn't matter for this, just use 0
} // ScanIterator::generateNewRecord 
