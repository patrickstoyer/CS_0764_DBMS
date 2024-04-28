#include "Sort.h"
#include <sys/stat.h>

SortPlan::SortPlan (Plan * const input) : _input (input)
{
	// (true);
} // SortPlan::SortPlan

SortPlan::~SortPlan ()
{
    delete _input;
} // SortPlan::~SortPlan

Iterator * SortPlan::init () const
{
    //TRACE (true);
	return new SortIterator (this);
} // SortPlan::init

SortIterator::SortIterator (SortPlan const * const plan):
	_plan (plan), _input (plan->_input->init ()), _gracefulDegrade(false),
    _firstPass(true),_consumed (0),_produced (0), _streamIndex(0),_cacheIndex(0),_ssdGd(false),
    _numCaches(1),_lastCache(94),_ssdCount(0),_hddCount(0),_newGDFile(false),_bytesWritten(0)
{
    traceprintf("sorting records\n");
    _outputFile = fopen("outputfile.txt", "w");
    _outputBuffer = new char[HDD_PAGE_SIZE];
    setvbuf(_outputFile,_outputBuffer,_IOFBF,HDD_PAGE_SIZE);

    new (&_cacheRuns[_cacheIndex]) PriorityQueue(CACHE_SIZE / RECORD_SIZE,0);
    new (&_cacheRunPQ) PriorityQueue(95,1);
    _cacheRunPQ.add(_cacheIndex,_cacheRuns[_cacheIndex]);
    _currentPQ = &_cacheRunPQ;
    // Looping over _consumed inputs
    while (_input->next())
    {
        ALWAYS_HDD = false;
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
    if (_ssdCount > 0 || _hddCount > 0)
    {
        closeTmpBuffer();
    }
    if (_firstPass)
    {
        _currentPQ = &_cacheRunPQ;
    }
    else
    {
        _currentPQ = new PriorityQueue(_ssdCount + _hddCount + 1,2);
        _currentPQ->add(0,_cacheRunPQ);
        _inputBuffers = new InputBuffer[_ssdCount + _hddCount];
        for (int i = 1; i <= _ssdCount; i++ )
        {
            char * filename = getOutputFilename(true , i);
            new (&_inputBuffers[i-1]) InputBuffer(filename, 2);
            _currentPQ->add(i,_inputBuffers[i-1]);
            delete [] filename;
        }
        for (int i = 1; i <= _hddCount; i++ )
        {
            char * filename = getOutputFilename(false, i);
            new (&_inputBuffers[i + _ssdCount - 1]) InputBuffer(filename, 1);
            _currentPQ->add(i + _ssdCount, _inputBuffers[i + _ssdCount -1 ]);
            delete [] filename;
        }
    }
    ALWAYS_HDD = true;
    BYTES_WRITTEN_HDD = 0;
    BYTES_WRITTEN_COUNTER = 0;

    // Final merge will happen when calling next
    traceprintf("consumed %lu rows. Beginning final merge step.\n",
                (unsigned long) (_consumed));
} // SortIterator::SortIterator

SortIterator::~SortIterator ()
{
    fclose(_outputFile);
    delete [] _outputBuffer;
	delete _input;
    removeTmpFiles(false);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
} // SortIterator::~SortIterator

bool SortIterator::next()
{
    if (_produced >= _consumed)
    {
        finalNextCleanup();
        return false;
    }
    char * lf = new char[1]{'~'};
    if (_produced > 0) this->_currentRecord.~Record();
    new (&this->_currentRecord) Record(lf,0);
    // Keep calling StoreNextAndSwap till we got a non-duplicate
    bool wasDupe = false;
    _currentPQ->storeNextAndSwap(this->_currentRecord,_outputFile,true,-1,wasDupe);
    while (wasDupe)
    {
        wasDupe = false;
        lf = new char[1]{'~'};
        this->_currentRecord.~Record();
        new (&this->_currentRecord) Record(lf,0);
        _currentPQ->storeNextAndSwap(this->_currentRecord,_outputFile,true,-1,wasDupe);
    }
    if (this->_currentRecord.isSentinel())
    {
        finalNextCleanup();
        return false;
    }
    ++ _produced;
    return true;
} // SortIterator::next

void SortIterator::finalNextCleanup()
{
    if (_firstPass)
    {
        _currentPQ = nullptr;
    }
    else
    {
        delete [] _inputBuffers;
    }
    char * lf = new char[1]{'~'};
    this->_currentRecord.~Record();
    new (&this->_currentRecord) Record(lf,0);
}


void SortIterator::moveToNextCache()
{
    _streamIndex = 0; // Start adding from the beginning
    // If we're at the second to last cache, we need to start graceful degradation
    if ((_lastCache == 0 && _cacheIndex == 1) || (_lastCache == 94 && _cacheIndex == 93))
    {
        _gracefulDegrade = true;
        _newGDFile = true;
        traceprintf("starting graceful degradation\n");
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
        new (&_cacheRuns[_cacheIndex]) PriorityQueue(CACHE_SIZE / RECORD_SIZE, 0);
        _cacheRunPQ.add(_cacheIndex, _cacheRuns[_cacheIndex]);
    }
    else {
        _cacheRuns[_cacheIndex].reset();
        _cacheRunPQ.incrementSize();
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
            if (!_currentPQ->storeRecords(tmpOutputFile,_lastCache,_ssdGd))
            {
                closeTmpBuffer();
                spillSsd();
                _currentPQ->storeRecords(tmpOutputFile,_lastCache,_ssdGd);
            }

            if (_ssdGd)
            {
                _ssdGd = false;
                removeTmpFiles(true);
                BYTES_WRITTEN_SSD = 0;
                _currentPQ->_inputStreams[0] = nullptr; // This would be _cacheRunPQ itself, which we oughtn't delete
                delete _currentPQ;
                _currentPQ = &_cacheRunPQ;
                delete [] _inputBuffers;
            }
            _gracefulDegrade = false;
        }
        moveToNextCache();
    }
}

void SortIterator::gracefulDegrade(Record& nextRecord)
{
    if (_firstPass) _firstPass = false;

    if (_newGDFile || (!hasSsdSpaceRemaining() &&!_ssdGd)) {
        _newGDFile = false;
        if (_ssdCount > 0 || _hddCount > 0)
        {
            closeTmpBuffer();
        }

        if (hasSsdSpaceRemaining()) {
            traceprintf("spilling to SSD\n");
            _ssdCount++;
            int bufferSize = (BYTES_WRITTEN_SSD + SSD_PAGE_SIZE > SSD_SIZE) ? (int)(SSD_SIZE - BYTES_WRITTEN_SSD) : SSD_PAGE_SIZE;
            char * filename = getOutputFilename(true, _ssdCount);
            tmpOutputFile = fopen(filename, "w");
            delete [] filename;
            tmpOutputBuffer = new char[bufferSize];
            setvbuf(tmpOutputFile,tmpOutputBuffer,_IOFBF,bufferSize);
            _bytesWritten = 0;
        } else {
            spillSsd();
        }
    }
    bool tmp = false;
    if (!_currentPQ->storeNextAndSwap(nextRecord,tmpOutputFile,false,_lastCache,tmp)) // Returns true if successfully swapped in nextRecord
    {
        //_cacheRuns[_cacheIndex].add(nextRecord,_streamIndex++);
        addToCacheRuns(nextRecord);

    }
    _bytesWritten += RECORD_SIZE;
}

bool SortIterator::hasSsdSpaceRemaining()
{
    return (BYTES_WRITTEN_SSD + RECORD_SIZE < SSD_SIZE);
}

char * SortIterator::getOutputFilename(bool _type, int _count)
{
    if (_type) return new char[17]{'s','s','d','o','u','t','p','u','t',(char)( '0' + _count/100),(char)( '0' + (_count/10)%10),(char)( '0' + _count % 10),'.','t','x','t','\0'};
    return new char[17]{'h','d','d','o','u','t','p','u','t',(char)( '0' + _count/100),(char)( '0' + (_count/10)%10),(char)( '0' + _count % 10),'.','t','x','t','\0'};
}

void SortIterator::spillSsd()
{
    char * filename;
    //Make ssd spill pq
    _currentPQ = new PriorityQueue(_ssdCount + 1,2);
    _currentPQ->add(0,_cacheRunPQ);
    _inputBuffers = new InputBuffer[_ssdCount];
    for (int i = 1; i <= _ssdCount; i++ )
    {
        filename = getOutputFilename(true , i);
        new (&_inputBuffers[i-1]) InputBuffer(filename, 2);
        _currentPQ->add(i,_inputBuffers[i-1]);
        delete [] filename;
    }
    _ssdGd = true;
    traceprintf("spilling to HDD\n");
    _hddCount++;
    int bufferSize = HDD_PAGE_SIZE;
    filename = getOutputFilename(false, _hddCount);
    tmpOutputFile = fopen(filename, "w");
    delete [] filename;
    tmpOutputBuffer = new char[bufferSize];
    setvbuf(tmpOutputFile,tmpOutputBuffer,_IOFBF,bufferSize);
    _bytesWritten = 0;
}

void SortIterator::closeTmpBuffer()
{
    fclose(tmpOutputFile);
    delete [] tmpOutputBuffer;
    if (BYTES_WRITTEN_COUNTER > 0)
    {
        double latency = (_hddCount > 0) ? 5 : 0.1;
        TOTAL_LATENCY += latency;
       // traceprintf("%s write of %lld bytes with latency %.1f ms (total I/O latency: %.1f)\n",(_hddCount > 0) ? "HDD" : "SSD",BYTES_WRITTEN_COUNTER,latency,TOTAL_LATENCY);
        BYTES_WRITTEN_COUNTER = 0;
    }
}

void SortIterator::removeTmpFiles(bool ssdOnly)
{
    for (int i = 1; i <= _ssdCount; i ++)
    {
        char * filename = getOutputFilename(true , i);
        remove(filename);
        delete [] filename;
    }
    _ssdCount = 0;
    if (ssdOnly) return;
    for (int i = 1; i <= _hddCount; i ++)
    {
        char * filename = getOutputFilename(false , i);
        remove(filename);
        delete [] filename;
    }
    _hddCount = 0;
}
