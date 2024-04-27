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

    int recSize = (USE_NEWLINES) ? RECORD_SIZE - 1 : RECORD_SIZE;
	for (int i = 0; i < recSize; i ++)
	{
		value = record.data[i];
		parity = (char)(parity ^ value);
	}
	
	_xorParity = (char)(_xorParity ^ parity);
	
} // FilterPlan::calcParity 

void FilterIterator::updateIsSorted(Record& nextRecord)
{
	if (!_isSorted) return; // If we already know it's not sorted don't continue checking
    int cmp = (this->_lastRecord).compare(nextRecord);
	if (cmp > 0)
    {
        _isSorted = false;
        _hasDuplicates = (_hasDuplicates > 0) ? _hasDuplicates : -1; // If we've already seen a duplicate, leave value as is
        return;
    }
    if (_hasDuplicates > 0) return;
    if (cmp == 0)
    {
        _hasDuplicates = ((this->_lastRecord).isDuplicate(nextRecord)) ? 1 : _hasDuplicates;
    }

}

Iterator * FilterPlan::init () const
{
    //TRACE (true);
	return new FilterIterator (this);
} // FilterPlan::init

FilterIterator::FilterIterator (FilterPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),_consumed (0),
    _produced (0), _xorParity (0), _isSorted (true), _hasDuplicates(0)
{
    traceprintf("checking sortedness and parity\n");
} // FilterIterator::FilterIterator

FilterIterator::~FilterIterator ()
{
    traceprintf ("produced %lu of %lu rows.\n",
                 (unsigned long) (_produced),
                 (unsigned long) (_consumed));
	delete _input;
} // FilterIterator::~FilterIterator

void FilterIterator::printStuff()
{
    traceprintf ("\nInput was%s sorted, and parity was %d\nDoes file have duplicates? %s\n",
                 (_isSorted ? "" : " not" ),
                 (_xorParity),
                 (_hasDuplicates>0) ? "Yes" : (_hasDuplicates < 0) ? "Unknown" : "No");
    if (DUPLICATE_PARITY != 0)
    {
        printf("Parity with removed duplicates included is %d\n",(_xorParity ^ DUPLICATE_PARITY));
    }
}


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
        printStuff();
        return false;
    }
    if (_consumed > 0) this->_currentRecord.~Record();
    new (&this->_currentRecord) Record(_input->_currentRecord);
    if (_consumed == 0) this->_lastRecord = this->_currentRecord;
    else updateIsSorted(this->_currentRecord);
    updateParity(this->_currentRecord);
	if (_consumed > 0) this->_lastRecord.~Record();
	new (&_lastRecord) Record(this->_currentRecord);
	++ _consumed;
	++ _produced;
	return true;
} // FilterIterator::next
