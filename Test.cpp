#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Record.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>


int RECORD_SIZE = 20; // Default to 20 bytes
int KEY_SIZE = 8;
int CACHE_SIZE = 1000;//000;
int SSD_PAGE_SIZE =  20000; // Default to 200 MB/s * (0.1 ms = 0.0001 s) = 20 KB (= 20,000 B)
int HDD_PAGE_SIZE = 500000; // Default to 100 MB/s *  (5 ms = 0.005 s) = 500 KB (= 500,000 B)
int MEM_SIZE =   100000;//000; // Default to 100 MB (=  100,000,000 B)
long long SSD_SIZE = 10000000;//000; // Default to 10GB (= 10,000,000,000 B)
bool USE_NEWLINES = false;
bool REMOVE_DUPES = true;
int SEED = -1;

void parseInput(int argc, char * argv [],int * count, int * size, char * outputFileName)
{
	char * nextval;
	char valsSet = 0;
	for (int i = 1; i < argc; i++)
	{
		nextval = argv[i];
		if (strcmp(nextval,"-c")==0)
		{
			i++;
			*count = (int) strtol(argv[i],NULL,10);
			valsSet +=0b00000001;
		} 
		else if (strcmp(nextval,"-s")==0)
		{
			i++;
			valsSet +=0b00000010;
			*size = (int) strtol(argv[i],NULL,10);
		} 
		else if (strcmp(nextval,"-seed")==0)
		{
			i++;
			SEED = (int) strtol(argv[i],NULL,10);
		} 
		else if (strcmp(nextval,"-o")==0)
		{
			i++;
			valsSet +=0b00000100;
			strcpy(outputFileName, argv[i]);	
		} 
		else if (strcmp(nextval,"-n")==0)
		{
			USE_NEWLINES = true;
		}
		else
		{
			std::cout << "Unknown argument ignored: " << nextval << "\n";
		}
	}
	// Set defaults
	if ((valsSet & 0b00000001) == 0)
	{
		*count = 20;
	}
	if ((valsSet & 0b00000010) == 0)
	{
		*size = 1024;
	}
	if ((valsSet & 0b00000100) == 0)
	{
		char arr[] = "outputFile.txt";
		strcpy(outputFileName, arr);
	}
    KEY_SIZE = (RECORD_SIZE > 8) ? 8 : RECORD_SIZE;
    REMOVE_DUPES = false;
} // parseInput

int main (int argc, char * argv [])
{	
	char outputFileName[64];
	int count,size;
	parseInput(argc,argv,&count,&size,outputFileName);
	if ((count <= 0) || (size <= 0))
	{
		return 0;
	}
	else 
	{
		RECORD_SIZE = size;
		if (USE_NEWLINES) RECORD_SIZE += 1;
	}
    //TRACE (true);

	//std::fstream f("data.txt");
	//Record * record = new Record(f);
	Plan * const plan = new SortPlan ( new FilterPlan ( new ScanPlan (count) ) );
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