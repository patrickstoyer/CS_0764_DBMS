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
    InputBuffer * _inputBuffer{};
    FILE* _outputFile;
    char * _outputBuffer;
    int _bufferIndex{};
    PriorityQueue _cacheRuns[95];
    int _lastCache;
    PriorityQueue _cacheRunPQ;
    PriorityQueue _finalPQ;
    //PriorityQueue _memRuns; //_memRuns[0]~_cacheRuns, _memRuns[1-N] = ~mem-size runs on SSD
    //PriorityQueue _ssdRuns;
    int _streamIndex;
    int _cacheIndex;
    int _numCaches;
    int _ssdCount;
    int _hddCount;
    void gracefulDegrade(Record& nextRecord);
    bool _gracefulDegrade;
    bool _firstPass;
    Record _lastStored;

    void moveToNextCache();
    void addToCacheRuns(Record &nextRecord);
    static char *getOutputFilename(bool _type, int _count);
    long long int ssdSpaceRemaining() const;
    int _bytesWritten;
    bool _newGDFile;
    FILE * tmpOutputFile{};
    char * tmpOutputBuffer{};
}; // class SortIterator
