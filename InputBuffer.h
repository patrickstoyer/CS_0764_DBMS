#pragma once
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
    Record * nextAndReplace() override;
    Record * peek() override;
    Record * peek(bool copy) override;
    void ready(int skipIndex) override;
    void reset() override;
    void nullBuffer();
    bool storeNextAndSwap(Record& record, FILE * outputFile) override;
    bool storeNextAndSwap(Record& record, FILE * outputFile, bool alwaysSwap, int lastCache, bool& wasDuplicate) override;
    void reset(int size, int dir, bool resetStreams, bool initializing) override;
private:
	FILE * _inputFile{};
    char * _inputBuffer{};
    long long _fileSize;
    long long _lastRead;
    long long _pageSize;
    long long _bytesReadCounter;

    void trackBuffer(bool flush);
}; // class InputBuffer
