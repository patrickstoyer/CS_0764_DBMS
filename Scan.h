#include "Iterator.h"
#include "Record.h"
#include <stdio.h>

class ScanPlan : public Plan
{
	friend class ScanIterator;
public:
	ScanPlan (RowCount const count);
	~ScanPlan ();
	Iterator * init () const;
private:
	RowCount const _count;
}; // class ScanPlan

class ScanIterator : public Iterator
{
public:
	ScanIterator (ScanPlan const * const plan);
	~ScanIterator ();
	bool next ();
	Record * currentRecord;
private:
	ScanPlan const * const _plan;
	RowCount _count;
	void createNextRecord ();
	Record * generateNewRecord ();
	FILE* _file;
	char * _buffer;
	int _bufferIndex;
}; // class ScanIterator
