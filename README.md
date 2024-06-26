# CS_0764_DBMS

Name: Patrick Stoyer 

github ID: patrickstoyer 

Repo: https://github.com/patrickstoyer/CS_0764_DBMS 

PROJECT FEATURES IMPLEMENTED
----------------------------
This project implements (or tries to implement) the following requirements of the project:
- Tree of losers/tournament tree Priority Queue
	- See PriorityQueue class for implementation
- Duplicate removal
	- This is done during the final merge step - we compare the next Record to store with the previous one, and when there is a complete match, we do not store it and look for another record to store
	- See PriorityQueue::storeNextAndSwap and SortIterator::next
- Cache-sized mini runs 
	- These should be <= 1 MB worth of records (including any new lines that would be stored)
	- I hard coded it to use up to 95 runs at this level at any one time -- this is our memory size (100 MB) / max cache size (1 MB) times 95% (to account for any other code/storage needed)
	- See SortIterator constructor and SortIterator::moveToNextCache for the code where I create the PQs for these arrays (_cacheRuns[_cacheIndex])
- Device-optimized page sizes
	- Reading and writing from "SSD" uses 20000 bytes* (which is 200 MB/s * (0.1 ms = 0.0001 s), i.e. the latency * bandwidth)
	- Reading and writing from "HDD" uses 500000 bytes (which is 100 MB/s *  (5 ms = 0.005 s), i.e. the latency * bandwidth)
	- See usages of the global variables SSD_PAGE_SIZE and HDD_PAGE_SIZE for this feature
		- In particular, spillSsd/gracefulDegrade (in SortIterator) and the InputBuffer constructor open files and set the buffering to the corresponding page size
- Spilling from cache-sized runs to "SSD"
	- See SortIterator::gracefulDegrade() for the code that starts storing from the cache to the SSD
- Spilling from "SSD" (+ cache-sized runs) to "HDD"
	- See SortIterator::gracefulDegrade() for the code that starts storing from the cache or SSD to HDD
- Some graceful degradation during both of the spill steps 
	- Specifically, when we're on the last of the 95 cache-sized runs, we will continue to read new entries, while also spilling to SSD (if it's not full) or HDD (if SSD is full). 
	- We will only save to the final cache if a new entry is less than the last spilled value (since we cannot safely replace it into the overarching priority queue structure in that case)
	- Only if we fill the final cache in this process will we give up and spill the rest to SSD/HDD
	- See SortIterator::gracefulDegrade() 
- Verifying sort order
	- I also check character-wise XOR parity, which gives a fairly reliable indication that the final merge has the same records as the input 
	- When the input is sorted, I also check whether duplicates were removed (I just compare adjacent records, so if it is not sorted this cannot be determined)
	- See FilterIterator.cpp (in particular updateIsSorted and updateParity)

 
*Note that I consistently used decimal byte units rather than binary, e.g. I used 1000 (= 10^3) bytes for 1 KB, rather than 1024 (= 2^10)

Other features I'm unsure of whether I implemented:
- Minimum count of rows
	- I'm not really sure what this is supposed to be -- the description I got was "the tricks that you have implemented to reduce unnecessary IOs between the devices"
	- In that case, IOs are (should be) reduced by 
		- Buffering reads/writes based on device-specific page size (as described above) -> doing this means a write should only occur when we're ready to write a reasonable (based on the device) amount of data (plus a smaller final write at the EOF), and reads will always gather a reasonable amount of data to use (plus a smaller final read at the EOF)
		- Graceful degradation 
			- This means we will generate larger runs and thus potentially avoid some IOs we would have incurred from spilling records 
		- We also do not store the contents of memory to SSD or move the contents of SSD to HDD when we prepare for the final merge, which avoids 2 * 100 MB of IO to move them between levels (we just merge from whereever the records are to their final location)
- Optimized merge patterns
	- Also not really sure what this is supposed to be -- the description I got was "the constants you have defined in your code related to the data transfer between the device" - but I'm not really sure what about those is relevant for satisfying the requirement

IMPLEMENTATION
--------------
Algorithms used:
- 3-level External merge sort w/ graceful degradation (obviously)
	- Level 0 - "Cache"-sized (1MB) runs are formed by adding new records to tree of losers priority queue
	- Level 1 - If and when memory is almost filled (all but one cache-sized run filled, we start graceful degradation/spilling to SSD. 
		- Winners of each "cache"-sized run added to another tree of losers priority queue, and we incrementally move the overall winner from the level 0 runs to the SSD. 
		- If the record we stored is less than the next record, we replace the stored record with it throughout the priority queue structure; if it is not, then we add the next record to the final cache. 
		- Only if we completely fill the last cache in this process do we spill all data in memory to SSD. 
		- In my testing, this seems to have increased run size ~5-10%.
	- Level 2 - If and when SSD is filled, we start graceful degradation/spilling to HDD.
		- The winner of the priority queue in level 1 is added to another tree of losers priority queue, alongside the first (i.e. minimum) record from each SSD temporary file.
		- The winner of that priority queue is spilled to HDD -- as in the level 1 merge step, we replace it throughout the priority queue structure with the next record passed if the latter is greater, otherwise we put the next record in our final cache. Again, only if we completely fill the last cache do we completely spill to HDD. 
	- The final merge step depends on the state at the time we finishing reading inputs
		- In essence, we create a tree of losers priority containing the winner from the cache-sized runs, and, if applicable the first (i.e. minimum) record from any SSD or HDD temporary files, and get each record in turn from it.
	- In-stream duplicate removal
		- Duplicates are removed during the final merge step -- the value we're about to store is compared to the previously stored one, and if it is a duplicate we do not save it (and continue polling the PQ until we find a non-duplicate/reach the end of the inputs) (this is implemented in the storeNextAndSwap functions of PriorityQueue/InputBuffer).
- Checking sortedness/duplicates/parity (Filter.cpp)
  	- Sortedness is checked by comparison of each of the _currentRecords the SortIterator selects after each call to next() to the previously seen one
  	- Duplicates are checked similarly - we only report this if the Records are sorted, but if they are then we report duplicates if subsequent records exactly match.
  	- Parity is updated by taking XOR of previous parity with each character in each Record (i.e. parity = parity ^ nextChar) 
    
Structures used:
- Tree of losers priority queue
	- Essentially as described in papers
	- Records are added from leaves, and we make a leaf->root pass, comparing with existing records in tree and swapping the loser of the comparison into the tree if necessary
	- Prior to adding any records, the tree is initialized with early fences -- before we read from the PQ, we either add enough records to remove all those fences, or swap them with late fences, which will pull the data Records up to the correct position
	- When removing the top record from the structure, we either replace with another key value, or with a late fence (which will pull any other records in the tree up to the correct position).

General program flow:
- Most actual work is done from each Iterator class 
- Iterator constructor sets up any necessary data and does any work to prep for moving to subsequent record in next()
- next() method moves to next record (assigning _currentRecord with its value), and in some cases does something with the record (e.g. SortIterator stores it, FilterIterator compares sort order/duplication and parity), or prints metrics/information
- Iterator destructor does any necessary cleanup
- Each iterator consumes inputs from another Iterator (except Scan, which generates the inputs) -- we either consume all records in the constructor (e.g. Sort, which must generate the final merge PQ from them, or kind of Scan which generates all data in the constructor) or we do it in turn as the previous Iterator gives it input (e.g. Filter)
- The flow of Iterator/Plan types used in my final program is Filter -> Sort -> Filter -> Scan
	- So we first generate input (Scan), then "filter" it (check sortedness/parity/duplication), then sort it, then "filter" it again
   
Overview of classes:
- PriorityQueue.h/PriorityQueue.cpp - Implement most of the functionality related to the tree of losers/tournament trees
	Key methods include:
	- peek() - Gets the next winner, but leaves it in array 	
	- next() - Gets the next winner, and removes it from the array, replacing it with the late fence
	- nextAndReplace() - Gets the next winner, and -- if possible -- replaces it with the next entry from whatever stream (i.e. file or other priority queue) it came from
	- replacePeek() - Swaps the next winner with the passed Record
	- storeNextAndSwap() - Stores the next winner (unless it is a duplicate/sentinel) into a passed FILE *, and replaces it with a passed Record 
		- Either always swaps the Record into the array, or checks if it is greater than the last stored value (for graceful degradation)
	- storeRecords() - Stores all records, adding ones from the inputStreams until we have nothing more to store
	- ready() - Readies for calls to next, by removing any early fences that are present
	- reset() - Clears PQ and fills it with early fences
	
	Key fields include:
	- _arr = array of Records in tree of losers ordering
	- _inputStreams = array of InputStreams the PQ is using to get values (if applicable) -- see below
	- _type = 0 - Constitutes 1 cache-sized run, 1 - Contains the winner from each of the cache-sized runs, and those runs as inputStreams, 2 - Contains a combination of the winner from a type 1 PQ (assumed to always be in _inputStream[0]) and winners from 1 or more files (/InputBuffers)
- InputStream.h	- Defines a parent class to PriorityQueue/InputBuffer, so they can be used in the _inputStreams array
	- Mostly the same functions as described above
- InputBuffer.h/InputBuffer.cpp 
	- Essentially a wrapper for a FILE * that returns the next entry in it as a Record
	- Child class of InputStream, so uses mostly the same interface as PriorityQueue (note that trying to swap something into the InputStream always fails, so we essentially only return stored records)
- Sort.cpp/Sort.h - Most of the logic for spilling and graceful degradation is here 
	Key methods:
	- SortIterator constructor completes all but the last merge step
		- Consumes input from other iterator and adds to PQ (potentially also spilling some input to a temporary file in the process)
		- Sets up the final merge PQ
	- next() completes the last merge step and stores the next record in _currentRecord
		- Uses storeNextAndSwap() to store next (and swaps with late fence)
	- addToCacheRuns() adds the next record to the cache level runs 
		- If doing so fills up the cache run, then we call moveToNextCache to move to the next cache 
		- If we already are gracefully degrading, then filling this final cache triggers us to stop graceful degradation and call storeRecords
	- moveToNextCache() - moves forward or backwards through the cache run array, and triggers graceful degradation if we move to the final cache
	- gracefulDegrade() - Instead of directly adding to cache run PQ, we first create a new temporary SSD/HDD file (if necessary), then call storeNextAndSwap with the next Record (which stores the overall lowest record, and tries to replace it with the next Record). If that fails, we add to the cache run PQ
- Scan.cpp/Scan.h - Generates random inputs
	- Constructor - Calls createNextRecord _plan->_count times to generate that number of records and store them to inputfile.txt
	- next() - Reads from inputfile.txt and sets _currentRecord based off that
	- generateNewRecordData() - uses std::uniform_int_distribution to generate random numbers between 0 and 61, then maps them to an alphameric value
- Filter.cpp/Filter.h - Indicates if output is sorted, has duplicates, and XOR parity
	- updateParity - loops through bytes and updates parity to parity ^ nextByte
	- updateIsSorted - compares current record to last record, if there is a decrease, it is not sorted
		- If it is sorted, we also check if there is an exact duplicate
- Record.h/Record.cpp - Object containing data of size RECORD_SIZE (for record) or 1 (for sentinel), and an index to the stream it came from within a PQ
	- Some helper functions for swapping, comparisons, etc.
	- Also some miscellaneous global settings, constants, metric tracking variabes used throughout the sort that I couldn't find a better place for
- Test.cpp/Test.h - Runs program
	- Also parses input and sets the globals defined in Record.h


OTHER NOTES
-----------
1) Command line options
 - -c - count of records (-c 1000 = 1000 records) NOTE that this does NOT handle commas - 25,000 will be parsed as 25, so use 25000 instead
 - -s - size of records (-s 1024 = 1024 bytes per record) NOTE that this does NOT handle commas - 25,000 will be parsed as 25, so use 25000 instead
 - -o - trace filename (-o trace.txt = output trace to trace.txt)
 - -n - turn off newlines 
	- if not set, outputfile.txt and inputfile.txt will have each record on a new line (so the size of the files will be (RECORD_SIZE + 1 ) * NUMBER_OF_RECORDS).
    - if set, those files will have records as a continuous stream with no characters between them (so the output will be RECORD_SIZE * NUMBER_OF_RECORDS).
 - -d - turn off duplicate removal
 - -seed - set a seed for the random number generation - if set, each run should generate the same data for the records

2) Input/Output
- The input/output of a successful run should be sent to inputfile.txt and outputfile.txt resp. 
- Each file should contain either -c lines with -s random alphanumeric values, or a continuous stream of -c * -s alphanumeric values
- Output should of course be sorted and have duplicates removed (unless -d is set), whereas input should not.
- In the middle of the sort, temporary "SSD" and "HDD" files are generated that are used for spilling, and should normally be deleted on program termination 
- Example inputs/outputs are given in the Outputs folder

3) Trace file
- Reports number of rows/records produced/consumed + final bytes written/read (including writing/reading to the inputfile), estimated latency given the SSD/HDD parameters we were supposed to emulate, actual elapsed time, and some basic information about the calls being hit.
- Filter.cpp also reports whether the input was sorted, if any duplicates were found (which is not known if the input wasn't sorted based on how I checked this), and the character-wise XOR parity. 
	- This is shown twice, first for the input and then for the output
	- A correct output should be sorted, have no duplicates, and parity should match (if duplicates are removed, we will track the parity of the removed duplicates and report the parity with and without them).
- Examples of the trace files are given for several test cases in the Traces folder -- the files are named to reflect the inputs -- traceXY.txt where X is the total size of the sort (e.g. 50MB) and Y is the record size (e.g. 2000KB).

4) Testing
I tested and had successful results with key sizes of 20B, 200B and 2000B/2KB, with total sizes of 50MB, 125MB, and 12GB. At 12GB, the runs took a substantial amount of time, so I did not test with total data greater than that. 

I also tested extensively with cache/RAM/SSD size constants that were lower than the ones in the project requirements (e.g. 1/100th of the ones given us) in order to verify/debug the spilling and graceful degradation behavior.

5) Duplicate Removal
- Given the fact that data is randomly generated with each record of size N bytes containing N chars, each with 62 possible values ([A-Za-z0-9]), the number of possible records is very large (62 ^ N) and the probability of any duplicates, even with the 20 byte records and a large number of records is relatively low
- As such, most of my testing was done with small records/keys of size 2-3 bytes, and I was unable to do much testing with larger records. The behavior with these very small records was somewhat variable - usually values were removed correctly, but sometimes they were not, and I was unable to reproduce issues consistently enough to resolve them. It's possible this was some kind of general issue from having such small keys since I couldn't determine any reason the logic I was using would be in correct. 

6) "SSD" and "HDD" files
- For the SSD/HDD spilling files, I wasn't entirely sure how we were intended to emulate these -- I just did so logically, i.e. in the way I treated them, but both were ultimately just regular files on my machine.
- Similarly, for the latency metrics, I wasn't actually handling these any differently, so the I/O latency is just an estimate based on how many pages I would have written to memory (and, as I buffered the files by passing a buffer array and page size to setvbuf, this estimate may not accurately reflect how the OS is buffering the writes behind the scenes)


