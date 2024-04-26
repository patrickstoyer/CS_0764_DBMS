#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Record.h"
#include <fstream>
#include <cstdlib>
#include <cstring>


int RECORD_SIZE = 20; // Default to 20 bytes
int KEY_SIZE = 8;
int CACHE_SIZE = 1000;// Default to 1 MB (= 1,000,000 B)
int SSD_PAGE_SIZE =  20000; // Default to 200 MB/s * (0.1 ms = 0.0001 s) = 20 KB (= 20,000 B)
int HDD_PAGE_SIZE = 500000; // Default to 100 MB/s *  (5 ms = 0.005 s) = 500 KB (= 500,000 B)
[[maybe_unused]] int MEM_SIZE =   100000;//000; // Default to 100 MB (=  100,000,000 B)
long long SSD_SIZE = 100000;//000; // Default to 10GB (= 10,000,000,000 B)
long long BYTES_WRITTEN_SSD = 0;
long long BYTES_WRITTEN_HDD = 0;
long long BYTES_WRITTEN_COUNTER = 0;
long long TOTAL_READ = 0;
long long TOTAL_WRITTEN = 0;
double TOTAL_LATENCY = 0;
bool USE_NEWLINES = false;
bool REMOVE_DUPES = true;
char DUPLICATE_PARITY = 0;
int SEED = -1;
bool ALWAYS_HDD = true;

void parseInput(int argc, char * argv [],int * count, int * size, char * outputFileName)
{
	char * nextValue;
	char valuesSet = 0;
	for (int i = 1; i < argc; i++)
	{
        nextValue = argv[i];
		if (strcmp(nextValue,"-c")==0)
		{
			i++;
			*count = (int) strtol(argv[i],nullptr,10);
            valuesSet +=0b00000001;
		} 
		else if (strcmp(nextValue,"-s")==0)
		{
			i++;
            valuesSet +=0b00000010;
			*size = (int) strtol(argv[i],nullptr,10);
		} 
		else if (strcmp(nextValue,"-seed")==0)
		{
			i++;
			SEED = (int) strtol(argv[i],nullptr,10);
		} 
		else if (strcmp(nextValue,"-o")==0)
		{
			i++;
            valuesSet +=0b00000100;
			strcpy(outputFileName, argv[i]);	
		} 
		else if (strcmp(nextValue,"-n")==0)
		{
			USE_NEWLINES = true;
		}
		else
		{
			std::cout << "Unknown argument ignored: " << nextValue << "\n";
		}
	}
	// Set defaults
	if ((valuesSet & 0b00000001) == 0)
	{
		*count = 20;
	}
	if ((valuesSet & 0b00000010) == 0)
	{
		*size = 1024;
	}
	if ((valuesSet & 0b00000100) == 0)
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
	Plan * const plan = new FilterPlan( new SortPlan ( new FilterPlan ( new ScanPlan (count) ) ) );

    Iterator * const it = plan->init ();
	it->run ();
	delete it;

	delete plan;

	return 0;
} // main