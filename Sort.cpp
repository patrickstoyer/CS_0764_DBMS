#include "Sort.h"

SortPlan::SortPlan (Plan * const input) : _input (input)
{
	// (true);
} // SortPlan::SortPlan

SortPlan::~SortPlan ()
{
    //TRACE (true);
	delete _input;
} // SortPlan::~SortPlan

Iterator * SortPlan::init () const
{
    //TRACE (true);
	return new SortIterator (this);
} // SortPlan::init

SortIterator::SortIterator (SortPlan const * const plan):
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0), _streamIndex(0)
{
    //TRACE (true);
	while (_input->next ()) { ++ _consumed; }
	traceprintf ("consumed %lu rows\n",
			(unsigned long) (_consumed));

	traceprintf ("consumed %lu rows\n",
			(unsigned long) (_consumed));

    _outputFile = fopen("outputfile.txt", "w");
    _outputBuffer = new char[SSD_PAGE_SIZE];
    _inputBuffer = new InputBuffer("inputfile.txt",1);
    _cacheRuns[0] = *(new PriorityQueue(CACHE_SIZE / RECORD_SIZE,0));

} // SortIterator::SortIterator

SortIterator::~SortIterator ()
{
	TRACE (true);
    fclose(_outputFile);
    delete [] _outputBuffer;
	delete _input;
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
} // SortIterator::~SortIterator

bool SortIterator::next ()
{
	//TRACE (true);

	if (_produced >= _consumed)
    {
        _cacheRuns[0].storeRecords(_outputFile);
        // Finish merge (merge SSD + MEM to HDD, HDD -> HDD if necessary)
        return false;
    }
    this->_currentRecord = *_inputBuffer->next();
    std::cerr<<this->_currentRecord.data;
    _cacheRuns[0].add(this->_currentRecord,_streamIndex++);
    if (_streamIndex == CACHE_SIZE / RECORD_SIZE) _streamIndex = 0;
    //this->_currentRecord.storeRecord(_outputFile,(_produced == _consumed - 1));
   // _inputBuffer.print();
    //std::cerr << this->_currentRecord.data;
	++ _produced;
	return true;
} // SortIterator::next
