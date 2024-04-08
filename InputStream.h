#pragma once
#include "Record.h"

class InputStream {
public:
    virtual Record * next () = 0;
    virtual void repair () = 0;
};