#ifndef CUCKOOFILTER_H_
#define CUCKOOFILTER_H_

#include<string.h>
#include<stdlib.h>
#include"hashfunction.h"
#include"bithack.h"

#define MaxNumKicks 500

using namespace std;

typedef struct {
	size_t index;
	uint32_t fingerprint;
} Victim;

typedef struct{
	char* bit_array;
} Bucket;



class CuckooFilter{
private:

	int capacity;
	size_t single_table_length;
	size_t fingerprint_size;
	size_t bits_per_bucket;
	size_t bytes_per_bucket;

	Bucket* bucket;

	uint32_t mask;

public:

	bool is_full;
	bool is_empty;
	int counter;
	CuckooFilter* _0_child;
	CuckooFilter* _1_child;
	// CuckooFilter* child0CF;
	// CuckooFilter* child1CF;
	CuckooFilter* front;
	int level; //代表这个CF在第几个level

	//construction and distruction function
	CuckooFilter(const size_t single_table_length, const size_t fingerprint_size, const int capacity, int curlevel);
	~CuckooFilter();

	//insert & query & delete function
	int  insertItem(const char* item, Victim &victim);
	bool insertItem(const size_t index, const uint32_t fingerprint, bool kickout, Victim &victim);
	bool queryItem(const char* item);
	bool deleteItem(const char* item);

	bool insertImpl(const size_t index, const uint32_t fingerprint, const bool kickout, Victim &victim);
	bool queryImpl(const size_t index, const uint32_t fingerprint);
	bool deleteImpl(const size_t index, const uint32_t fingerprint);

	//generate two candidate bucket addresses
	void generateIF(const char* item, size_t &index, uint32_t &fingerprint, int fingerprint_size, int single_table_length);
	void generateA(size_t index, uint32_t fingerprint, size_t &alt_index, int single_table_length);

	//read from bucket & write into bucket
	uint32_t read(const size_t index, const size_t pos);
	void write(const size_t index, const size_t pos, const uint32_t fingerprint);

	void EmptyFilter(int curlevel,int single_capacity);

};

#endif //CUCKOOFILTER_H_
