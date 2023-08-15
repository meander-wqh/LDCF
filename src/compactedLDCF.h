#ifndef CLOGARITHMICDYNAMICCUCKOOFILTER_H_
#define CLOGARITHMICDYNAMICCUCKOOFILTER_H_


#include"cuckoofilter.h"
#include"linktree.h"
#include<list>
#include<math.h>
#include<unordered_map>
#include<iostream>



class CompactedLogarithmicDynamicCuckooFilter{
private:

	int capacity;
	int single_capacity;

	int single_table_length;
	
	int level;

	double false_positive;
	double single_false_positive;

	double fingerprint_size_double;
	int fingerprint_size;

	Victim victim;

	std::unordered_map<uint32_t,CuckooFilter*> CFMap;

	// CuckooFilter* curCF;
	// CuckooFilter* child0CF; 
	// CuckooFilter* child1CF; 


public:

	//record the items inside DCF
	int counter;

	// the link tree strutcture of building blocks CF1, CF2, ...
	LinkTree* cf_tree;

	//construction & distruction functions
	CompactedLogarithmicDynamicCuckooFilter(const size_t capacity, const double false_positive, const size_t exp_block_num = 6);
	~CompactedLogarithmicDynamicCuckooFilter();

	//insert & query & delete functions
	bool insertItem(const char* item);
	CuckooFilter* getChild0CF(CuckooFilter* curCF);
	CuckooFilter* getChild1CF(CuckooFilter* curCF);
	bool queryItem(const char* item);
	bool deleteItem(const char* item);

	bool remove(CuckooFilter* cf_remove);

	//generate 2 bucket addresses
	//缺少一个level参数
	void generateIF(const char* item, size_t &index, uint32_t &fingerprint, int fingerprint_size, int single_table_length,int level);
	void generateA(size_t index, uint32_t fingerprint, size_t &alt_index, int single_table_length,int level);
	
	bool GetPre(const char* item);
	bool GetPre(const char* item,int curLevel);

	//get info of DCF
	int getFingerprintSize();
	float size_in_mb();

	//extra function to make sure the table length is the power of 2
	uint64_t upperpower2(uint64_t x);

	void info();
};







#endif //CLOGARITHMICDYNAMICCUCKOOFILTER_H
