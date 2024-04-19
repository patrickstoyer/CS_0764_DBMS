#include "Record.h"
#include "InputStream.h"

class PriorityQueue : public InputStream {
public:
    PriorityQueue();
    PriorityQueue(int capacity, int type);
    ~PriorityQueue();
    void storeRecords(FILE * outputFile, int lastCache);
    bool storeNextAndSwap(Record& record, FILE * outputFile) override;
    Record * peek () override; // Returns arr[0] (the min value)
    void add(Record& nextRecord, int stream); // Adds a new node from _inputStreams[stream], assumes stream exists
    void add(int stream, InputStream& inputStream); // Adds a new node from inputStream, and sets _inputStreams[stream] = inputStream
    void ready(int skipIndex) override;
    bool isFull();
    Record * next () override; // Return arr[0] and replace with late_fence
    Record * nextAndReplace(); // Return arr[0] and replaces it from the stream it came from
private:
    void initializePQ();
    void remove(int stream); // Removes the value that came from stream
    static int parent(int index); // Gets index of parent of index
    int _size{}; // Number of streams/inputs currently being used
    int _capacity{}; // Maximum size of array
    int _type{} ; // 0 = Cache level. (CAPACITY = 1 MB / RECORD_SIZE)
                 //     Each node represents a record that was loaded
                 //     Only 1 input stream (_inputStreams[0]) of type InputBuffer (from HDD)
                 // 1 = Mem level (CAPACITY = ~97, (100 MB - a few MB for buffers, code, etc.)/ 1 MB per cache)
                 //     Each node represents the min value of a cache level PQ.
                 //     Each input stream is a cache level
                 // 2 = X SSD input streams + 1 Mem level PQ
                 //     Each node in arr represents the next seen value from a sorted SSD-size run stored on HDD
                 //     All _inputStreams should be of type InputBuffer (from HDD)
    Record * _arr{}; // Sorted data - Size of arr = size of streams (except cache level PQ)
    InputStream ** _inputStreams{}; // Pointers to streams of data _inputStreams[0] should be a
    int _lastReturnedIndex{};
    bool _isReadyToNext{}; // If true, we have repaired the array since the last early fence insertion

    void addFromStream(int stream);
    void reset();
    void repair();
    void replacePeek(Record &record);
    void replacePeek(Record &record,bool swap);
};

static char LATE_FENCE = '~'; // Sorts after [A-Za-z0-9]
static char EARLY_FENCE = '!'; // Sorts before [A-Za-z0-9]
static int MIN_NODE = 0; // Index of minimum of PQ