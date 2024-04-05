#include "Filter.h"

FilterPlan::FilterPlan (Plan * const input) : _input (input)
{
	//TRACE (true);
} // FilterPlan::FilterPlan

FilterPlan::~FilterPlan ()
{
	//TRACE (true);
	delete _input;
} // FilterPlan::~FilterPlan

void FilterIterator::updateParity(Record * record)
{
	char parity = 0;
	//char nextBit;
	char value;
	for (int i = 0; i < RECORD_SIZE; i ++)
	{
		value = record->data[i];
		parity = parity ^ value;
		/*while (value > 0)
		{
			nextBit = value % 2;
			value /= 2;
			parity = parity ^ nextBit;
		}*/
	}
	
	_xorParity = _xorParity ^ parity;
	
} // FilterPlan::calcParity 

void FilterIterator::updateIsSorted(Record * nextRecord)
{
	if (!_isSorted) return; // If we already know it's not sorted don't continue checking
	if ((this->_lastRecord).sortsBefore(nextRecord)) return;
	_isSorted = false;
}

Iterator * FilterPlan::init () const
{
	
	//TRACE (true);
	return new FilterIterator (this);
} // FilterPlan::init

FilterIterator::FilterIterator (FilterPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0), _xorParity (0), _isSorted (true)
{
	//TRACE (true);
} // FilterIterator::FilterIterator

FilterIterator::~FilterIterator ()
{
	//TRACE (true);

	delete _input;

	traceprintf ("produced %lu of %lu rows\nInput was %ssorted, and parity was %d\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed),
			(_isSorted ? "" : "not " ),
			(_xorParity));
} // FilterIterator::~FilterIterator

bool FilterIterator::next ()
{
	// TRACE (true);

	//do
	//{
	if ( ! _input->next ())  return false;
	
	Record newRecord = _input->_currentRecord;
	if (_consumed == 0) this->_lastRecord = newRecord;
	
	updateParity(&newRecord);
	updateIsSorted(&newRecord);
	//std::cout << "HERE2";
	_lastRecord = newRecord;
	//std::cout << "HERE3";
	++ _consumed;
	//} while (_consumed % 2 == 0);
	//std::cout << "HERE4";
	++ _produced;
	return true;
} // FilterIterator::next
