#include "Record.h"

class InputStream {
public:
    virtual Record * next () = 0;
};