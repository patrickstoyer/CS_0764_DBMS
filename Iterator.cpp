#include "Iterator.h"
#include <chrono>

Plan::Plan () = default; // Plan::Plan

Plan::~Plan () = default; // Plan::~Plan

Iterator::Iterator () : _count (0)
{
//	TRACE (true);
} // Iterator::Iterator

Iterator::~Iterator () = default;

void Iterator::run ()
{
    const auto start = std::chrono::system_clock::now();

	while (next ())  ++ _count;
    std::chrono::duration<float> seconds_since_start = std::chrono::system_clock::now() - start;

    traceprintf ("entire plan produced %lu rows.\nTotal bytes read:              %lld\nTotal bytes written:           %lld\nTotal estimated (I/O) latency: %.1f ms\nTotal actual run time:         %.2f sec.\n\n\n",
                 (unsigned long) _count,TOTAL_READ,TOTAL_WRITTEN,TOTAL_LATENCY,seconds_since_start.count());
} // Iterator::run
