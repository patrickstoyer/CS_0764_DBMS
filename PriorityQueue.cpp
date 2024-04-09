#include <mmcobj.h>
#include "PriorityQueue.h"
#include "Defs.h"
#include "InputBuffer.h"
#include "InputStream.h"

PriorityQueue::PriorityQueue() { }
PriorityQueue::PriorityQueue(int capacity, int type) : _capacity(capacity), _type(type),_size(0)
{
    initializePQ();
}
bool PriorityQueue::isFull()
{
    return (_capacity == _size);
}

void PriorityQueue::initializePQ()
{
    _arr = new Record[_capacity];
    if (_type==0) // HDD -> Cache
    {
        _inputStreams = nullptr;
    }
    else if ((_type == 1) || (_type == 2)) // 1. Cache -> Mem or 2. SSD + HDD + type 1 PQ
    {
        _inputStreams = new InputStream * [_capacity]; // _capacity should be ~95
    }
}

void PriorityQueue::reset()
{
    for (int i = 0 ; i < _capacity; i++)
    {
        char * ef = new char[1]{'!'};
        ; // Initialize with early fence
        _arr[i]=*(new Record{ef,i});

    }
    _isReadyToNext = false;
}
void PriorityQueue::add(Record& nextRecord, int stream)
{
    if ((_type == 0)&&(strncmp(nextRecord.data,&LATE_FENCE,1) != 0))
    {
        _size ++;
    }

    nextRecord.index = stream;
    Record candidate = nextRecord;
    for (int index = parent(_capacity + stream); index != 0; index = parent(index))
    {
        // Compare
        if (!candidate.sortsBefore(_arr[index]))
        {
            candidate.exchange(_arr[index]);
        }
    }
    candidate.exchange(_arr[MIN_NODE]);

}

void PriorityQueue::add(int stream, InputStream& inputStream)
{
    //add(nextRecord,stream);
    _size++;
    _inputStreams[stream] = &inputStream;
}

void PriorityQueue::addFromStream(int stream)
{
    if (stream >= _size) return;
    add(*_inputStreams[stream]->next(),stream);
}
void PriorityQueue::remove(int stream)
{
    char * lf = new char[1]{'~'};
    Record * sentRecord = new Record(lf,stream);
    add(*sentRecord,stream);
}
int PriorityQueue::parent(int index)
{
    return  (index / 2);
}

Record & PriorityQueue::peek ()
{
    return _arr[MIN_NODE];
}

Record * PriorityQueue::next()
{
    if (!_isReadyToNext)
    {
        ready();
    }
    int index = _arr[MIN_NODE].index;
    char * lf = new char[1]{'~'};
    Record * retVal = new Record(lf,index);
    _arr[MIN_NODE].exchange(*retVal);
    add(_arr[MIN_NODE],index);
    if (strncmp(retVal->data,&EARLY_FENCE,1) == 0)
    {
        retVal = next();
    }
    if (strncmp(retVal->data,&EARLY_FENCE,1)==0)
    {
        reset();
    }
    _lastReturnedIndex = retVal->index;
    return retVal;
}

Record * PriorityQueue::nextAndReplace ()
{
    if (!_isReadyToNext)
    {
        ready();
    }
    int index = _arr[MIN_NODE].index;
    Record * retVal;
    if ((index > _size) || (_type == 0 ))
    {
        char * lf = new char[1]{'!'};
        retVal = new Record(lf,index);
    } else
    {
        retVal = _inputStreams[index]->next();
    }
    _arr[MIN_NODE].exchange(*retVal);
    add(_arr[MIN_NODE],index);
    if (strncmp(retVal->data,&EARLY_FENCE,1) == 0)
    {
        retVal = nextAndReplace();
    }
    if (strncmp(retVal->data,&LATE_FENCE,1)==0)
    {
        reset();
    }
    _lastReturnedIndex = retVal->index;
    return retVal;
}

bool PriorityQueue::storeNextAnd (Record& swapVal)
{
    if (!_isReadyToNext)
    {
        ready();
    }
    if (_type != 2) return next();
    // If swapVal is
    if (swapVal.sortsBefore(peek()))
    {
        return next();
    }
    int index = _arr[MIN_NODE].index;
    _arr[MIN_NODE].exchange(swapVal);
    _inputStreams[index]->add(_arr[MIN_NODE]);
    add(_arr[MIN_NODE],index);
    _lastReturnedIndex = swapVal.index;
    return &swapVal;
}

void PriorityQueue::storeRecords(FILE * outputFile, int lastCache)
{
    char * lf = new char[1]{'~'};
    Record lateFence(lf,0);

    ready();

    for (Record * currentRec = nextAndReplace(); !lateFence.sortsBefore(*currentRec) ; currentRec = nextAndReplace())
    {
        if (strncmp(currentRec->data,&EARLY_FENCE,1) == 0) continue;
        currentRec->storeRecord(_outputFile,false);
    }
    fflush(_outputFile);
}

void PriorityQueue::ready()
{
    // First remove anything beyond capacity (note this assumes records are added from lowest stream to greatest)
    repair();
    if (_type != 0)
    {
        for (int i = 0; i < _size; i++)
        {
            _inputStreams[i]->repair();
            addFromStream(i);
        }
    }
}

PriorityQueue::~PriorityQueue()
{
    delete [] _arr;
    delete [] _inputStreams;
}

void PriorityQueue::repair()
{
    for (int i = _size ; i < _capacity ; i ++)
    {
        remove(i);
    }
}
