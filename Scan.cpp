#include "Scan.h"
#include <stdio.h>
#include "Record.h"

ScanPlan::ScanPlan (RowCount const count) : _count (count)
{
	TRACE (true);
    _file = fopen("file.binary", "w");
	_buffer.reserve(constants::PAGE_SIZE);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
	TRACE (true);
	fclose(_file);
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
	currentRecord->storeRecord(_plan->_buffer,_plan->_file,(_count == _plan->_count - 1));
} // ScanIterator::createNextRecord 


Record * ScanIterator::generateNewRecord ()
{
	//return new Record(random(),random(),0);// Index doesn't matter for this
} // ScanIterator::generateNewRecord 
