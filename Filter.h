#include "Iterator.h"
#include "Record.h"
#include <stdio.h>

class FilterPlan : public Plan
{
	friend class FilterIterator;
public:
	FilterPlan (Plan * const input);
	~FilterPlan ();
	Iterator * init () const;
private:
	Plan * const _input;
	unsigned char _xorParity;
	Record * _lastRecord;
	bool _isSorted;
	void updateParity(unsigned int value);
	void updateIsSorted(Record * nextRecord);
}; // class FilterPlan

class FilterIterator : public Iterator
{
public:
	FilterIterator (FilterPlan const * const plan);
	~FilterIterator ();
	bool next ();
private:
	FilterPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced;
}; // class FilterIterator
