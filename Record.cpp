#include "Record.h"
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

Record::Record(char * data,
    int index) //: data(data), index(index)
{
//	TRACE (true);
    this->data = data;
    this->index = index;
}

Record::Record()
{
}
/*
Record::Record(std)
{
	TRACE (true);
  //  fprintf(stderr,"HELLOWFLLEIOW1");
    if (!file.is_open()) return;
    char line[constants::KEY_SIZE + constants::RECORD_SIZE];
    
    //fprintf(stderr,"HELLOWFLLEIOW2");
  //  file.getline(line,sizeof(line)/sizeof(*line));
   // fprintf(stderr,line);
    //fprintf(stderr,"POOPOO");
    while(file.peek()!=EOF)
    {
        file.getline(line,sizeof(line)/sizeof(*line));
        fprintf(stderr,line);
        fprintf(stderr,"POOPOO");
    }
    std::cout<<sizeof(line)/sizeof(*line);
    return;
}*/

Record::~Record ()
{
    delete [] data;
//	TRACE (true);
}
// Called to check sort order   
bool Record::sortsBefore(Record& other)
{
    int cmp = strncmp(this->data,other.data,RECORD_SIZE);
    bool retVal = (!(cmp > 0)); // Cmp>0 = sorts after cmp = 0 = match, cmp < 0 = sorts
    //std::cerr << "1COMP = " << cmp << " \n\t DATA = " << this->data << " \n\t OTHER = " << other.data << "\n";
    return retVal;
}

void Record::storeRecord (FILE * file, bool flush)
{
    // TODO Store to either SSD/HDD (pass file type as parameter)
    fwrite(this->data,1,RECORD_SIZE,file);
    if (flush)
    {
        fflush(file);
    }
}

void Record::exchange(Record &other)
{
    char * dataTmp = other.data;
    int indexTmp = other.index;
    other.data = this->data;
    other.index = this->index;
    this->data = dataTmp;
    this->index = indexTmp;
}

Record::Record(Record &other)
{
    this->data = new char [RECORD_SIZE];
    strcpy(this->data,other.data);
    this->index = other.index;
}
// Record::storeRecord