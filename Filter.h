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
	Record * _lastRecord;
	bool _isSorted;
	char _xorParity;
	void updateParity(Record * record);
	void updateIsSorted(Record * nextRecord);
	RowCount _consumed, _produced;
}; // class FilterIterator
