#include "Iterator.h"

Plan::Plan () = default; // Plan::Plan

Plan::~Plan () = default; // Plan::~Plan

Iterator::Iterator () : _count (0)
{
//	TRACE (true);
} // Iterator::Iterator

Iterator::~Iterator () = default; // Iterator::~Iterator

void Iterator::run ()
{
    //TRACE (true);

	while (next ())  ++ _count;

	traceprintf ("entire plan produced %lu rows.\nTotal bytes read: %lld, total bytes written: %lld, total latency: %.2f\n",
			(unsigned long) _count,TOTAL_READ,TOTAL_WRITTEN,TOTAL_LATENCY);
} // Iterator::run
