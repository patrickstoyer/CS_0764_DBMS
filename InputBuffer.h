#include "Iterator.h"
#include "Record.h"
#include "InputStream.h"
#include <cstdio>

class InputBuffer : public InputStream
{
public:
    InputBuffer();
    InputBuffer(const char * inputFile, char bufferType);
	~InputBuffer ();
    Record * next() override;
    Record * peek() override;
    Record * peek(bool copy) override;
    void ready(int skipIndex) override;
    void reset() override;
    void nullBuffer();
    bool storeNextAndSwap(Record& record, FILE * outputFile) override;
    bool storeNextAndSwap(Record& record, FILE * outputFile, bool alwaysSwap, int lastCache) override;
private:
	FILE * _inputFile{};
    char * _inputBuffer{};
    long long _fileSize;
    long long _lastRead;
    long long _pageSize;

    void trackBuffer();
}; // class InputBuffer
