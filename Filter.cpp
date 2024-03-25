#include "Filter.h"

FilterPlan::FilterPlan (Plan * const input) : _input (input), _xorParity (0), _isSorted (false)
{
	TRACE (true);
} // FilterPlan::FilterPlan

FilterPlan::~FilterPlan ()
{
	TRACE (true);
	delete _input;
} // FilterPlan::~FilterPlan

void FilterPlan::updateParity(unsigned int value)
{
	unsigned char parity = 0;
	unsigned char nextBit;
	while (value > 0)
	{
		nextBit = value % 2;
		value /= 2;
		parity = parity ^ nextBit;
	}
	// If previous (overall parity was 1/)
	_xorParity = _xorParity ^ parity;
	
} // FilterPlan::calcParity 

void FilterPlan::updateIsSorted(Record * nextRecord)
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
	_consumed (0), _produced (0)
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
		_plan->updateIsSorted(newRecord);
		_plan->updateParity(newRecord->key);
		_plan->updateParity(newRecord->data);
		++ _consumed;
	} while (_consumed % 2 == 0);

	++ _produced;
	return true;
} // FilterIterator::next
