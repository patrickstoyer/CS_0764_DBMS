#include "Sort.h"

SortPlan::SortPlan (Plan * const input) : _input (input)
{
	TRACE (true);
} // SortPlan::SortPlan

SortPlan::~SortPlan ()
{
	TRACE (true);
	delete _input;
} // SortPlan::~SortPlan

Iterator * SortPlan::init () const
{
	TRACE (true);
	return new SortIterator (this);
} // SortPlan::init

SortIterator::SortIterator (SortPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	TRACE (true);
	while (_input->next ()) { ++ _consumed; }
	traceprintf ("consumed %lu rows\n",
			(unsigned long) (_consumed));

	traceprintf ("consumed %lu rows\n",
			(unsigned long) (_consumed));
    _inputBuffer = InputBuffer("inputfile.txt",1);

} // SortIterator::SortIterator

SortIterator::~SortIterator ()
{
	TRACE (true);
	delete _input;
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
} // SortIterator::~SortIterator

bool SortIterator::next ()
{
	TRACE (true);

	if (_produced >= _consumed)  return false;

    this->_currentRecord = _inputBuffer.get();
	++ _produced;
	return true;
} // SortIterator::next
