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
        _inputStreams = new InputStream*[1];
        _inputStreams[0] = new InputBuffer("inputfile.txt",1);
    }
    else if (_type == 1) // Cache -> Mem
    {
        _inputStreams = new InputStream*[_capacity]; // _capacity should be ~95
    } else if (_type == 2) // SSD + type 1 PQ
    {
        _inputStreams = new InputStream*[_capacity];
    } else if (_type == 3) // HDD -> HDD
    {
        _inputStreams = new InputStream*[_capacity];
    }

    for (int i = 0 ; i < _capacity; i++)
    {
        char * ef = new char[1]{'!'};
        //ef[0] = '!';
        ; // Initialize with early fence -- inserting will push up to appropriate
        _arr[i]=*(new Record{ef,i});
    }
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
    int index = _arr[MIN_NODE].index;
    char * lf = new char[1]{'~'};
    Record * retVal = new Record(lf,index);
    _arr[MIN_NODE].exchange(*retVal);
    add(_arr[MIN_NODE],index);
    if (strncmp(retVal->data,&EARLY_FENCE,1) == 0)
    {
        retVal = next();
    }
    return retVal;
}

Record * PriorityQueue::nextAndReplace ()
{
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
    return retVal;
}

void PriorityQueue::storeRecords(FILE * _outputFile)
{
    char * lf = new char[1]{'~'};
    Record lateFence(lf,0);
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

    for (Record * currentRec = nextAndReplace(); !lateFence.sortsBefore(*currentRec) ; currentRec = nextAndReplace())
    {
        if (strncmp(currentRec->data,&EARLY_FENCE,1) == 0) continue;
        currentRec->storeRecord(_outputFile,false);
    }
    fflush(_outputFile);
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
