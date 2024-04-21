#include "Iterator.h"
#include "Record.h"
#include <cstdio>
#include <random>

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
private:
	ScanPlan const * const _plan;
	RowCount _count;
	void createNextRecord ();
	static char * generateNewRecordData ();
	FILE* _file;
	char * _buffer;
	int _bufferIndex;
	static std::default_random_engine generator;
}; // class ScanIterator
