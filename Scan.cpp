#include "Scan.h"
#include <cstdio>
#include <chrono>
#include <random>
#include "Record.h"
#include "InputBuffer.h"

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
	_plan (plan), _count (0)
{

    _file = fopen("inputfile.txt", "w");
	_buffer = new char[HDD_PAGE_SIZE];
    setvbuf(_file,_buffer,_IOFBF,HDD_PAGE_SIZE);
	if (SEED > 0) generator.seed(SEED);

    traceprintf("generating %lld records of size %d\n",_plan->_count,(USE_NEWLINES) ? RECORD_SIZE - 1: RECORD_SIZE);
    ALWAYS_HDD = true;
	for (int i = 0; i < _plan->_count; i++)
    {
        createNextRecord(i);
    }
    fclose(_file);
    delete [] _buffer;
    new (&_inputBuffer) InputBuffer("inputfile.txt",1);

} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
    traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next ()
{
    if (_count >= _plan->_count)
    {
        _inputBuffer.nullBuffer();
        return false;
    }
    Record * next = _inputBuffer.next();
    this->_currentRecord.copy(*next);
    delete next;
    ++ _count;
	return true;
} // ScanIterator::next

void ScanIterator::createNextRecord(int count)
{
    Record next(generateNewRecordData(),0);
    next.storeRecord(_file,(count == _plan->_count - 1));
} // ScanIterator::createNextRecord

char * ScanIterator::generateNewRecordData ()
{
	static const char range[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	char * data = new char[RECORD_SIZE];
	// If appending new line at end, we'll generate RECORD_SIZE - 1 byte random, then newline;
	int max = (USE_NEWLINES) ? RECORD_SIZE - 1 : RECORD_SIZE;
	std::uniform_int_distribution<char> distribution(0,61);
  	for (int i = 0; i < max; i++)
	{
		char next = distribution(generator);
		data[i] = range[next];	
	}
	// Add newline
	if (USE_NEWLINES) data[max] = '\n';
	return data;// Index doesn't matter for this, just use 0
} // ScanIterator::generateNewRecord 
