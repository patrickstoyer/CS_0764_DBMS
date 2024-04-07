#include "Iterator.h"
#include "InputBuffer.h"
#include "PriorityQueue.h"

class SortPlan : public Plan
{
	friend class SortIterator;
public:
	SortPlan (Plan * const input);
	~SortPlan ();
	Iterator * init () const;
private:
	Plan * const _input;
}; // class SortPlan

class SortIterator : public Iterator
{
public:
	SortIterator (SortPlan const * const plan);
	~SortIterator ();
	bool next ();
private:
	SortPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced;
    InputBuffer * _inputBuffer;
    FILE* _outputFile;
    char * _outputBuffer;
    int _bufferIndex;
    PriorityQueue _cacheRuns[95];
    PriorityQueue _memRuns; //_memRuns[0]~_cacheRuns, _memRuns[1-N] = ~mem-size runs on SSD
    PriorityQueue _ssdRuns; //_
    int _streamIndex;
}; // class SortIterator
