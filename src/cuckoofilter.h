#ifndef CUCKOOFILTER_H_
#define CUCKOOFILTER_H_

#include<string.h>
#include<stdlib.h>
#include<iostream>
#include"hashfunction.h"
#include"bithack.h"
#include"uint.h"

#define MaxNumKicks 500

using namespace std;

typedef struct {
	size_t index;
	uint32_t fingerprint;
} Victim;

struct Bucket{
	char* bit_array;
	Bucket(){}
	//深度拷贝函数
	Bucket(const Bucket& b){
		//这里strlen可能会出问题，后面debug要注意
		bit_array = new char[strlen(b.bit_array) + 1];
		strcpy(bit_array,b.bit_array);
	}
	~Bucket(){
		delete[] bit_array;
	}
};



class CuckooFilter{
public:
	int capacity;
	size_t single_table_length;
	size_t fingerprint_size;//为了读写使用的8为倍数的size
	size_t exact_fingerprint_size;//由于分裂,使用的确切size
	size_t bits_per_bucket;
	size_t bytes_per_bucket;
	uint32_t mask;
	Bucket* bucket;
	bool is_full;
	bool is_empty;
	int counter;
	int level; //代表这个CF在第几个level
	std::string CFId; 

	//construction and distruction function
	// CuckooFilter(const size_t single_table_length, const size_t fingerprint_size, const int capacity, int curlevel);
	//这里传入的是
	CuckooFilter(std::string CFId,const size_t single_table_length, size_t fingerprint_size, const int capacity, int curlevel);
	//拷贝构造
	CuckooFilter(const CuckooFilter& CF):bucket(CF.bucket){
		capacity = CF.capacity;
		single_table_length = CF.single_table_length;
		fingerprint_size = CF.fingerprint_size;
		bits_per_bucket = CF.bits_per_bucket;
		bytes_per_bucket = CF.bytes_per_bucket;
		mask = CF.mask;
		is_full = CF.is_full;
		is_empty = CF.is_empty;
		counter = CF.counter;
		level = CF.level;
	}
	~CuckooFilter();

	//insert & query & delete function
	int  insertItem(const char* item, Victim &victim);
	bool insertItem(const size_t index, const uint32_t fingerprint, bool kickout, Victim &victim);
	bool insertItem(const size_t index, const uint32_t fingerprint, Victim &victim);
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
