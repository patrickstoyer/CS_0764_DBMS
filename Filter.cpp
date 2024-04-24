#include "Filter.h"

FilterPlan::FilterPlan (Plan * const input) : _input (input)
{
    //TRACE (true);
} // FilterPlan::FilterPlan

FilterPlan::~FilterPlan ()
{
//	TRACE (true);
	delete _input;
} // FilterPlan::~FilterPlan

void FilterIterator::updateParity(Record& record)
{
	char parity = 0;
	//char nextBit;
	char value;

	for (int i = 0; i < RECORD_SIZE; i ++)
	{
		value = record.data[i];
		parity = (char)(parity ^ value);
	}
	
	_xorParity = (char)(_xorParity ^ parity);
	
} // FilterPlan::calcParity 

void FilterIterator::updateIsSorted(Record& nextRecord)
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
    traceprintf("checking sortedness and parity\n");
} // FilterIterator::FilterIterator

FilterIterator::~FilterIterator ()
{
	TRACE (true);
	delete _input;

	traceprintf ("produced %lu of %lu rows.\nInput was%s sorted, and parity was %d\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed),
			(_isSorted ? "" : " not" ),
			(_xorParity));
    if (DUPLICATE_PARITY != 0)
    {
        printf("parity with removed duplicates included is %d\n",(_xorParity ^ DUPLICATE_PARITY));
    }

} // FilterIterator::~FilterIterator

bool FilterIterator::next ()
{
    if ( ! _input->next ())
    {
        char * lf = new char[1]{'~'};
        this->_currentRecord.~Record();
        new (&this->_currentRecord) Record(lf,0);
        lf = new char[1]{'~'};
        this->_lastRecord.~Record();
        new (&this->_lastRecord) Record(lf,0);
        return false;
    }
    if (_consumed > 0) this->_currentRecord.~Record();
    new (&this->_currentRecord) Record(_input->_currentRecord);
    if (_consumed == 0) this->_lastRecord = this->_currentRecord;

    updateParity(this->_currentRecord);
	updateIsSorted(this->_currentRecord);
    if (_consumed > 0) this->_lastRecord.~Record();
	new (&_lastRecord) Record(this->_currentRecord);
	++ _consumed;
	++ _produced;
	return true;
} // FilterIterator::next
