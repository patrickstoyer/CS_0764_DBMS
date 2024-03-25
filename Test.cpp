#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Record.h"
#include <fstream>

int main (int argc, char * argv [])
{
	TRACE (true);
	std::fstream f("data.txt");
	std::cout << constants::KEY_SIZE << constants::RECORD_SIZE << constants::CACHE_SIZE << constants::PAGE_SIZE << constants::MEM_SIZE << constants::SSD_SIZE;
  	//Record * record = new Record(f);
	Plan * const plan = new ScanPlan (7);
	// new FilterPlan ( new SortPlan ( new FilterPlan ( new ScanPlan (7) ) ) );

	// TODO:
	// 1) Create (Sort)Iterator from (Sort)Plan->init. This will automatically:
	//   -> Call SortIterator constructor (SortPlan::init)
	//     -> Call FilterPlan->init (SortIterator::SortIterator)
	//   	 -> Call FilterIterator constructor (FilterPlan::init)
	//		   -> Call ScanPlan->init (FilterIterator::FilterIterator)
	//		     -> Call ScanIterator constructor (ScanPlan::init)
	//	   -> Call FilterIterator->next till done (SortIterator::SortIterator)
	// 	      -> Call ScanIterator->next (twice) till done (FilterIterator:next)
	// 2) Run the (Sort)Iterator. This will automatically:
	//   -> Call SortIterator->next till done (Iterator->run)
	// 
	// (ACTUALLY, will have FilterPlan->init/FilterIterator->run, the former should be equivalent to caling SortPlan->init (i.e. sets up data/1st filter info), latter will call SortIterator->next)
	//
	// At end of 1, I think we should have the records on stable storage ("HDD").
	// SortIterator will then have to read from this file, before adding to sorting data structure(s)
	// So ScanIterator->next should:
	//  - Create record data
	//  - Make sure currentRecord is publically accessible, for FilterIterator to compute parity/sortedness 
	//  - Store data 
	//		- Move to buffer
	//  	- If buffer full (page size), write to file and clear
	// FilterIterator->next should:
	//  - Update overall parity for each call to ScanIterator->next
	//  - Update isSorted (compare each record key with last key) (store last record seen's key)
	//  - On final iteration, output sorted/parity
	// SortIterator->next should:
	//  - Read record from file 
	// 		- If input buffer is empty, read from file to it
	//		- Read next record from input buffer
	//  - See notes app -- in essence, either add to cache-level array (memory not full), or do some more complicated logic, then add to cache-level array (memory full).
	Iterator * const it = plan->init ();
	it->run ();
	delete it;

	delete plan;

	return 0;
} // main
