#include <mmcobj.h>
#include "PriorityQueue.h"

PriorityQueue::PriorityQueue() { }
PriorityQueue::PriorityQueue(int capacity, int type) : _capacity(capacity), _type(type),_size(0)
{
    initializePQ();
}

void PriorityQueue::initializePQ()
{
    _arr = new Record[_capacity];
    _inputStreams = new InputStream*[(_type==0)? 1 : _capacity];
    for (int i = 0 ; i < _capacity; i++)
    {
        add(Record(&LATE_FENCE,i),_capacity + i);
    }
}
void PriorityQueue::add(Record nextRecord, int stream)
{
    nextRecord.index = stream;
    Record candidate = nextRecord;
    for (int index = parent(_capacity + stream); index != 0; index = parent(index))
    {
        // Compare
        // Swap if necessary
    }
}

void PriorityQueue::add(Record nextRecord, int stream, void *inputStream) {

}
void PriorityQueue::remove(int stream) {

}
int PriorityQueue::parent(int index)
{
    return  (index / 2);
}

Record PriorityQueue::peek ()
{
    return _arr[MIN_NODE];
}

Record * PriorityQueue::next ()
{
    Record * retVal = &_arr[MIN_NODE];
    //Swap arr[0] with low fence
    //add(lowFence, retVal.index)
    return retVal;

}
