#include <mmcobj.h>
#include "PriorityQueue.h"
#include "InputBuffer.h"
#include "InputStream.h"

PriorityQueue::PriorityQueue()
{
    this->_arr = nullptr;
    this->_inputStreams = nullptr;
}
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
    reset();
}

void PriorityQueue::reset()
{
    for (int i = 0 ; i < _capacity; i++)
    {
        char * ef = new char[1]{'!'};
        // Initialize with early fence
        _arr[i]=*(new Record{ef,i});

    }
    _size = 0;
    _isReadyToNext = false;
}
void PriorityQueue::add(Record& nextRecord, int stream)
{
    std::cerr << "a";
    bool checkDupes = REMOVE_DUPES;
    if (_type == 0)
    {
        if ((strncmp(nextRecord.data,&LATE_FENCE,1) != 0)&&(strncmp(nextRecord.data,&EARLY_FENCE,1)!=0))
        {
            _size ++;
        }
        else
        {
            checkDupes=false;
        }
    }
    //std::cerr << "b";
    nextRecord.index = stream;
    //std::cerr << "c";
    Record candidate{};
    candidate.copy(nextRecord);
    candidate.index = nextRecord.index;
    //std::cerr << "d";
    for (int index = parent(_capacity + stream); index != 0; index = parent(index))
    {
        // std::cerr << "e";
        int cmp=candidate.compare(_arr[index]);
        if (cmp < 0) continue; // cmp > 0 => candidate sorts before PQ value
        if ((cmp == 0) && checkDupes && candidate.isDuplicate(_arr[index])) return; // cmp == 0 is a duplicate
        // Otherwise we can swap
        candidate.exchange(_arr[index]);

    }
    //std::cerr << "g";
    candidate.exchange(_arr[MIN_NODE]);
    //std::cerr << "h";
    //std::cerr << "i";
}

void PriorityQueue::add(int stream, InputStream& inputStream)
{
    _size++;
    _inputStreams[stream] = &inputStream;
}

void PriorityQueue::addFromStream(int stream)
{
    if (stream >= _size) return;
    Record val{};
    val.copy(*_inputStreams[stream]->peek());
    add(val,stream);
}
void PriorityQueue::remove(int stream)
{
    char * lf = new char[1]{'~'};
    Record sentRecord(lf,stream);
    add(sentRecord,stream);
}
int PriorityQueue::parent(int index)
{
    return  (index / 2);
}

Record * PriorityQueue::peek ()
{
    return &_arr[MIN_NODE];
}

Record * PriorityQueue::next()
{
    if (!_isReadyToNext)
    {
        ready(-1);
    }
    char * lf = new char[1]{'~'};
    auto * retVal = new Record(lf,_arr[MIN_NODE].index);
    replacePeek(*retVal);
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
        ready(-1);
    }
    int index = _arr[MIN_NODE].index;
    Record * retVal;
    if ((index > _size) || (_type == 0 ))
    {
        char * lf = new char[1]{'!'};
        retVal = new Record(lf,index);
    } else
    {
        delete _inputStreams[index]->next();
        retVal = _inputStreams[index]->next();
    }
    replacePeek(*retVal);
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

bool PriorityQueue::storeNextAndSwap (Record& record, FILE * outputFile)
{
    if (!_isReadyToNext)
    {
        ready(-1);
    }
    if (_type == 0)
    {
        peek()->storeRecord(outputFile, false);
        // If record is less than the min we just saved, we can't add it
        if (record.sortsBefore(*peek()))
        {
            // Call next to remove min, and return false
            next();
            return false;
        }
        else
        {
            // Otherwise, we can add the next record
            replacePeek(record);
            return true;
        }
    }
    else
    {
        // If not at cache level, we will in generally just recurse to that level
        // 1. Get stream index of min
        int index = peek()->index;
        // 2. Recurse to the min's input stream. This should handle:
        //    - Storing the min (note that if somehow the min w/in the _inputStreams differs from the overall PQ, this will fail)
        //    - Comparing the stored min with the new record (to see if we can swap it into the cache)
        //    - Swapping into the cache
        bool retVal = _inputStreams[index]->storeNextAndSwap(record, outputFile);
        replacePeek(*_inputStreams[index]->peek(),false); // We will always want the peek of the stream to be in the array
        return retVal;
    }
}
// Replaces arr[0] with record and re-sorts
void PriorityQueue::replacePeek(Record& record)
{
    replacePeek(record,true);
}
// Replaces arr[0] with record and re-sorts
void PriorityQueue::replacePeek(Record& record,bool swap)
{
    int index = _arr[MIN_NODE].index;
    if (swap)
    {
        _arr[MIN_NODE].exchange(record);
    }
    else // Otherwise copy record data to min node
    {
        _arr[MIN_NODE].copy(record);
    }
    add(_arr[MIN_NODE],index);
}


void PriorityQueue::storeRecords(FILE * outputFile, int lastCache)
{
    char * lf = new char[1]{'~'};
    Record lateFence(lf,0);

    if (!_isReadyToNext) ready(lastCache);

    for (Record * currentRec = nextAndReplace(); !lateFence.sortsBefore(*currentRec) ; currentRec = nextAndReplace())
    {
        if (strncmp(currentRec->data,&EARLY_FENCE,1) == 0) continue;
        currentRec->storeRecord(outputFile,false);
        delete currentRec;
    }

    reset();
}

void PriorityQueue::ready(int skipIndex)
{
    // First remove anything beyond capacity (note this assumes records are added from the lowest stream to greatest)
    repair();
    if (_type != 0)
    {
        for (int i = 0; i < _size; i++)
        {
            if (i == skipIndex) continue;
            _inputStreams[i]->ready(-1);
            addFromStream(i);
        }
    }
    _isReadyToNext = true;
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
