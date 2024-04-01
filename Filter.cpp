#include "Filter.h"

FilterPlan::FilterPlan (Plan * const input) : _input (input)
{
	TRACE (true);
} // FilterPlan::FilterPlan

FilterPlan::~FilterPlan ()
{
	TRACE (true);
	delete _input;
} // FilterPlan::~FilterPlan

void FilterIterator::updateParity(Record * record)
{
	char parity = 0;
	char nextBit;
	char value;
	for (int i = 0; i < constants::KEY_SIZE; i ++)
	{
		value = record->key[i];
		while (value > 0)
		{
			nextBit = value % 2;
			value /= 2;
			parity = parity ^ nextBit;
		}
	}
	for (int i = 0; i < constants::RECORD_SIZE; i ++)
	{
		value = record->data[i];
		while (value > 0)
		{
			nextBit = value % 2;
			value /= 2;
			parity = parity ^ nextBit;
		}
	}
	
	// If previous (overall parity was 1/)
	_xorParity = _xorParity ^ parity;
	
} // FilterPlan::calcParity 

void FilterIterator::updateIsSorted(Record * nextRecord)
{
	if (!this->_lastRecord->sortsBefore(nextRecord)) _isSorted = false;
}

Iterator * FilterPlan::init () const
{
	TRACE (true);
	return new FilterIterator (this);
} // FilterPlan::init

FilterIterator::FilterIterator (FilterPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0), _xorParity (0), _isSorted (false)
{
	TRACE (true);
} // FilterIterator::FilterIterator

FilterIterator::~FilterIterator ()
{
	TRACE (true);

	delete _input;

	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
} // FilterIterator::~FilterIterator

bool FilterIterator::next ()
{
	TRACE (true);

	do
	{
		if ( ! _input->next ())  return false;
		Record * newRecord = _input->_currentRecord;
		updateIsSorted(newRecord);
		updateParity(newRecord);
		++ _consumed;
	} while (_consumed % 2 == 0);

	++ _produced;
	return true;
} // FilterIterator::next
