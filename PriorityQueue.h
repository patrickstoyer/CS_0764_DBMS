#include "Record.h"
#include "InputStream.h"

class PriorityQueue : InputStream {
public:
    PriorityQueue();
    PriorityQueue(int capacity, int type);
    ~PriorityQueue();
    Record * next (); // Return arr[0] and replaces it from the stream it came from
private:
    void initializePQ();
    void add(Record nextRecord, int stream); // Adds a new node from _inputStreams[stream], assumes stream exists
    void add(Record nextRecord, int stream, void * inputStream); // Adds a new node from inputStream, and sets _inputStreams[stream] = inputStream
    void remove(int stream); // Removes the value that came from stream
    static int parent(int index); // Gets index of parent of index
    Record peek (); // Returns arr[0] (the min value)
    int _size; // Number of streams/inputs currently being used
    int _capacity; // Maximum size of array
    int _type ; // 0 = Cache level. (CAPACITY = 1 MB / RECORD_SIZE)
                 //     Each node represents a record that was loaded
                 //     Only 1 input stream (_inputStreams[0]) of type InputBuffer (from HDD)
                 // 1 = Mem level (CAPACITY = ~97, (100 MB - a few MB for buffers, code, etc.)/ 1 MB per cache)
                 //     Each node represents the min value of a cache level PQ.
                 //     Each input stream is a cache level
                 // 2 = SSD
                 // 3 = HDD -> HDD final pass merge.
                 //     Each node in arr represents the next seen value from a sorted SSD-size run stored on HDD
                 //     All _inputStreams should be of type InputBuffer (from HDD)
    Record * _arr; // Sorted data - Size of arr = size of streams (except cache level PQ)
    InputStream ** _inputStreams; // Pointers to streams of data _inputStreams[0] should be a
    //
};

static char LATE_FENCE = '~'; // Sorts after [A-Za-z0-9]
static char EARLY_FENCE = '!'; // Sorts before [A-Za-z0-9]
static int MIN_NODE = 0; // Index of minimum of PQ