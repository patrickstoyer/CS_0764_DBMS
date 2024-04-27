#include <mmcobj.h>
#include "PriorityQueue.h"
#include "InputBuffer.h"
#include "InputStream.h"

PriorityQueue::PriorityQueue() : _dir(1)
{
    this->_arr = nullptr;
    this->_inputStreams = nullptr;
}
PriorityQueue::PriorityQueue(int capacity, int type) : _capacity(capacity), _type(type),_size(0),_dir(1)
{
    initializePQ();
}
bool PriorityQueue::isFull() const
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
    reset(true);
}
void PriorityQueue::reset()
{
    reset(0,1,false,false);
}
void PriorityQueue::reset(bool initializing)
{
    reset(0,1,false,initializing);
}
void PriorityQueue::reset(int size,int dirMult, bool resetStreams,bool initializing)
{
    _size = size;
    _dir *= dirMult;
    for (int i = 0 ; i < _capacity; i++)
    {
        char * ef = new char[1]{'!'};
        // Initialize with early fence
        if (!initializing)
        {
            _arr[i].~Record();
        }
        new (&_arr[i]) Record{ef,i};

        if ((!resetStreams)||(_type != 1)) continue;

        if (_dir == 1)
        {
            if (i >= _size) _inputStreams[i]->reset();
        }
        else
        {
            if (i < _capacity - _size) _inputStreams[i]->reset();
        }
    }
    _isReadyToNext = false;
}
void PriorityQueue::add(Record& nextRecord, int stream)
{
    bool checkDupes = REMOVE_DUPES;
    int sizeDelta=0;
    if (_type == 0)
    {
        // If new record has data, increment size
        if (!nextRecord.isSentinel())
        {
            sizeDelta++;
        }
        else
        {
            // Otherwise it is a sentinel, so decrement size
            checkDupes = false;
        }

        //_arr[MIN_NODE] will be overwritten, so if it had data, decrement size
        if  (!_arr[MIN_NODE].isSentinel())
        {
            sizeDelta--;
        }
        if ((_size < 0) || (_size > _capacity))
            throw std::invalid_argument("Size is out of bounds");

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
        if (candidate.sortsBefore(_arr[index])) continue; // cmp > 0 => candidate sorts before PQ value
        // Otherwise we can swap
        candidate.exchange(_arr[index]);

    }
    //std::cerr << "g";
    candidate.exchange(_arr[MIN_NODE]);
    _size += sizeDelta;
    //std::cerr << "h";
    //std::cerr << "i";
}

void PriorityQueue::add(int stream, InputStream& inputStream)
{
    this->incrementSize();
    _inputStreams[stream] = &inputStream;
}

void PriorityQueue::addFromStream(int stream)
{
    if ((_dir == 1) && (stream >= _size)) return;
    if ((_dir == -1) && (stream < _capacity - _size)) return;
    Record * val = _inputStreams[stream]->peek(true);
    add(*val,stream);
    delete val;
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

Record * PriorityQueue::peek()
{
    //Doesn't require delete on return val (returns pointer to value in array)
    return peek(false);
}

Record * PriorityQueue::peek(bool copy)
{
    //Requires delete if copy is true
    if (!copy) return &_arr[MIN_NODE];
    return new Record(_arr[MIN_NODE]);
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

    if (retVal->data[0] == EARLY_FENCE)
    {
        retVal = next();
    }
    if (retVal->data[0] == EARLY_FENCE)
    {
        reset();
    }
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
    if ((index >= _size) || (_type == 0 ))
    {
        char * lf = new char[1]{'!'};
        retVal = new Record(lf,index);
    } else
    {
        // Remove current winner from _inputStream it came from and replace w/ next value
        if ((_type == 2)&& (index==0)) // for _type 2, index 0 is cache level PQ, so we first need to recurse
        {
            delete _inputStreams[index]->nextAndReplace();
        }
        else
        {
            delete _inputStreams[index]->next();
        }
        retVal = _inputStreams[index]->peek(true);
    }
    replacePeek(*retVal);
    if (retVal->data[0] == EARLY_FENCE)
    {
        delete retVal;
        retVal = nextAndReplace();
    }
    if (retVal->data[0] == LATE_FENCE)
    {
        reset();
    }
    return retVal;
}
bool PriorityQueue::storeNextAndSwap (Record& record, FILE * outputFile)
{
    bool tmp = false;
    return storeNextAndSwap(record,outputFile,false,-1,tmp);
}
bool PriorityQueue::storeNextAndSwap (Record& record, FILE * outputFile, bool alwaysSwap, int lastCache, bool& wasDuplicate)
{
    if (!_isReadyToNext)
    {
        ready(lastCache);
    }
    if (_type == 0)
    {
        //Don't actually save if it duplicates last saved
        if (!REMOVE_DUPES
            || (peek()->compare(PriorityQueue::_lastSaved)!=0)
            || !peek()->isDuplicate(PriorityQueue::_lastSaved))
        {
            peek()->storeRecord(outputFile, false);
            PriorityQueue::_lastSaved.copy(*peek());
        }
        else
        {
            wasDuplicate = true;
            // Update duplicate parity
            int recSize = (USE_NEWLINES) ? RECORD_SIZE - 1 : RECORD_SIZE;
            for (int i = 0; i < recSize; i++) {
                DUPLICATE_PARITY = (char) (DUPLICATE_PARITY ^ peek()->data[i]);
            }
        }
        // If record is less than the min we just saved, we can't add it
        if (!record.sortsBefore(*peek())||alwaysSwap)
        {
            //Doesn't sort before, or we always want to swap so replace peek
            replacePeek(record);
            return true;
        }
        else
        {
            // Otherwise, call next to remove min, and return false
            delete next();
            return false;
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

        bool retVal = _inputStreams[index]->storeNextAndSwap(record, outputFile,alwaysSwap,-1,wasDuplicate);
        Record * nextRec = _inputStreams[index]->peek();

        replacePeek(*nextRec, false); // We will always want the peek of the stream to be in the array
        if ((_type==2)&&(index>0)) delete nextRec;
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

// Returns true if we finished storing records, false if we filled SSD
//  (and caller needs to call again w/ a new HDD file on a PQ w/ SSD
bool PriorityQueue::storeRecords(FILE * outputFile, int lastCache, bool isSsdGd)
{
    char * lf = new char[1]{'~'};
    Record lateFence(lf,0);

    if (!_isReadyToNext) ready(lastCache);
    Record * currentRec;
    if (!isSsdGd && (BYTES_WRITTEN_SSD + RECORD_SIZE > SSD_SIZE))
    {
        return false;
    }
    for (currentRec = nextAndReplace(); !lateFence.sortsBefore(*currentRec) ; currentRec = nextAndReplace())
    {
        if (currentRec->data[0] == EARLY_FENCE) continue;
        currentRec->storeRecord(outputFile,false);
        delete currentRec;
        if (!isSsdGd && (BYTES_WRITTEN_SSD + RECORD_SIZE > SSD_SIZE))
        {
            return false;
        }
    }
    delete currentRec;
    if (!isSsdGd)
    {
        reset(1,-1, true,false);
    }
    else
    {
        _inputStreams[0]->reset(1,-1, true,false);
    }


    return true;
}

void PriorityQueue::ready(int skipIndex)
{
    if (_isReadyToNext) return;
    // First remove anything beyond capacity (note this assumes records are added from the lowest stream to greatest)
    repair();
    if (_type != 0)
    {
        int min = 0, max = _size;
        if (_dir == -1) {
            min = _capacity - _size, max = _capacity;
        }
        for (int i = min; i < max; i++)
        {
            if ((_type == 1) && (i == skipIndex)) {
                remove(i);
                continue;
            }
            _inputStreams[i]->ready((_type <= 1)?-1:skipIndex);
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
    int min = _size, max = _capacity;
    if (_dir == -1){
        min = 0, max = _capacity - _size;
    }
    for (int i = min ; i < max ; i ++)
    {
        remove(i);
    }
}
void PriorityQueue::incrementSize() {
    _size++;
}

