#pragma once
#include "Record.h"

class InputStream {
public:
    virtual Record * next () = 0;
    virtual Record * peek () = 0;
    virtual void repair () = 0;
    virtual bool storeNextAndSwap(Record& record, FILE * outputFile) = 0;
};