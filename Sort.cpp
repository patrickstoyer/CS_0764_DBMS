#include "Sort.h"
#include <sys/stat.h>
#include <sys/types.h>

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
	_plan (plan), _input (plan->_input->init ()), _gracefulDegrade(false),
    _firstPass(true),_consumed (0),_produced (0), _streamIndex(0),_cacheIndex(0),
    _numCaches(1),_lastCache(94),_ssdCount(0),_hddCount(0),_newGDFile(false),_bytesWritten(0)
{
    _outputFile = fopen("outputfile.txt", "w");
    _outputBuffer = new char[HDD_PAGE_SIZE];
    setvbuf(_outputFile,_outputBuffer,_IOFBF,HDD_PAGE_SIZE);


    _cacheRuns[_cacheIndex] = *(new PriorityQueue(CACHE_SIZE / RECORD_SIZE,0));
    _cacheRunPQ = *(new PriorityQueue(95,1));
    _cacheRunPQ.add(_cacheIndex,_cacheRuns[_cacheIndex]);

    // Looping over _consumed inputs
    while (_input->next())
    {
        // Get the next record from the input
        Record nextRecord = _input->_currentRecord;
        _consumed++;
        // If we're gracefully degrading, just call subroutine to do that
        if (_gracefulDegrade)
        {
            gracefulDegrade(nextRecord);
        }
        else
        {
            // Otherwise, just add to cache runs
            addToCacheRuns(nextRecord);
            //
        }
    }
    if (_ssdCount>0) fclose(tmpOutputFile);
    if (_firstPass)
    {
        _finalPQ = _cacheRunPQ;
        _cacheRunPQ = PriorityQueue();
    }
    else
    {
        _finalPQ = *(new PriorityQueue(_ssdCount + _hddCount + 1,2));
        _finalPQ.add(0,_cacheRunPQ);
        for (int i = 1; i <= _ssdCount; i++ )
        {
            _finalPQ.add(i , *(new InputBuffer( getOutputFilename(true , i) , 2)));
        }
        for (int i = 1; i <= _hddCount; i++ )
        {
            _finalPQ.add(i + _ssdCount , *(new InputBuffer(getOutputFilename(false, i) , 1 )));
        }
    }

    // Final merge will happen in
    traceprintf("consumed %lu rows\n",
                (unsigned long) (_consumed));
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

bool SortIterator::next()
{
    if (_produced >= _consumed)
     {
        char * lf = new char[1]{'~'};
        this->_currentRecord.~Record();
        new (&this->_currentRecord) Record(lf,0);
        return false;
    }
    char * lf = new char[1]{'~'};
    if (_produced > 0) this->_currentRecord.~Record();
    new (&this->_currentRecord) Record(lf,0);
    _finalPQ.storeNextAndSwap(this->_currentRecord,_outputFile,true);
    //this->_currentRecord.storeRecord(_outputFile,(_produced == _consumed - 1));
	++ _produced;
	return true;
} // SortIterator::next


void SortIterator::moveToNextCache()
{
    _streamIndex = 0; // Start adding from the beginning
    // If we're at the second to last cache, we need to start graceful degradation
    if ((_lastCache == 0 && _cacheIndex == 1) || (_lastCache == 94 && _cacheIndex == 93))
    {
        _gracefulDegrade = true;
        _newGDFile = true;
    }
    else if (_cacheIndex == 0)
    {
        _lastCache = 94;
    }
    else if (_cacheIndex == 94)
    {
        _lastCache = 0;
    }
    (_lastCache == 0) ? _cacheIndex-- : _cacheIndex++; // Increment if we're moving to the right, decrement otherwise

    // If on first pass through caches, add PQs
    if (_firstPass)
    {
        _cacheRuns[_cacheIndex] = *(new PriorityQueue(CACHE_SIZE / RECORD_SIZE, 0));
        _cacheRunPQ.add(_cacheIndex, _cacheRuns[_cacheIndex]);
    }
}

void SortIterator::addToCacheRuns(Record& nextRecord)
{
    _cacheRuns[_cacheIndex].add(nextRecord, _streamIndex++);

    if (_cacheRuns[_cacheIndex].isFull())
    {
        if (_gracefulDegrade)
        {
            // If we're here, we filled the cache during the graceful degradation
            // We must clear the cache and stop graceful degradation
            _cacheRunPQ.storeRecords(tmpOutputFile,_lastCache);;
            _gracefulDegrade = false;
        }
        moveToNextCache();
    }
}

void SortIterator::gracefulDegrade(Record& nextRecord)
{
    if (_firstPass) _firstPass = false;

    long long ssdSpace = ssdSpaceRemaining();
    if (_newGDFile || ssdSpace < RECORD_SIZE) {
        _newGDFile = false;
        if (_ssdCount > 0)
        {
            fclose(tmpOutputFile);
            delete tmpOutputBuffer;
        }
        int bufferSize;
        if (ssdSpace > RECORD_SIZE) {
            _ssdCount++;
            bufferSize = SSD_PAGE_SIZE;
            tmpOutputFile = fopen( getOutputFilename(true, _ssdCount), "w");
        } else {
            _hddCount++;
            bufferSize = HDD_PAGE_SIZE;
            tmpOutputFile = fopen(getOutputFilename(false, _hddCount), "w");
        }
        tmpOutputBuffer = new char[bufferSize];
        setvbuf(tmpOutputFile,tmpOutputBuffer,_IOFBF,bufferSize);
        _bytesWritten = 0;
    }
    if (!_cacheRunPQ.storeNextAndSwap(nextRecord,tmpOutputFile)) // Returns true if successfully swapped in nextRecord
    {
        //_cacheRuns[_cacheIndex].add(nextRecord,_streamIndex++);
        addToCacheRuns(nextRecord);

    }
    _bytesWritten += RECORD_SIZE;
}

long long SortIterator::ssdSpaceRemaining() const
{
    if (_hddCount > 0 ) return 0;
    struct stat buffer{};
    long long retVal = SSD_SIZE;
    for (int i = 1 ; i <= _ssdCount ; i++)
    {
        if (stat (getOutputFilename(true,i),&buffer) == 0)
        {
            retVal -= buffer.st_size;
        }
        if (retVal <= 0) return retVal;
    }
    return retVal;
}

char * SortIterator::getOutputFilename(bool _type, int _count)
{
    if (_type) return new char[15]{'s','s','d','o','u','t','p','u','t',( '0' + _count),'.','t','x','t','\0'};
    return new char[15]{'h','d','d','o','u','t','p','u','t',( '0' + _count),'.','t','x','t','\0'};
}