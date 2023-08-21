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

	double fingerprint_size_double; //这个值是不会变的
	int fingerprint_size; //这里的fingerprint_size是初始化的最长fingerprint_size,但各个子CF会根据自己的level相应减少

	Victim victim;

	std::unordered_map<std::string,CuckooFilter*> CFMap;

public:

	//record the items inside LDCF
	int counter;
	//record the CFnumber inside LDCF
	int CFnumber = 0;
	
	// the link tree strutcture of building blocks CF1, CF2, ...
	LinkTree* cf_tree;

	//construction & distruction functions
	CompactedLogarithmicDynamicCuckooFilter(const size_t capacity, const double false_positive, const size_t exp_block_num);
	~CompactedLogarithmicDynamicCuckooFilter();

	//insert & query & delete functions
	bool insertItem(const char* item);
	bool insertItem(std::string CFId, size_t index, uint32_t fingerprint);
	// CuckooFilter* getChild0CF(CuckooFilter* curCF);
	// CuckooFilter* getChild1CF(CuckooFilter* curCF);
	//bool queryItem(const char* item);
	bool queryItem(std::string CFId, const char* item);
	
	//bool deleteItem(const char* item);
	bool deleteItem(std::string CFId,size_t index, uint32_t fingerprint);

	// bool failureHandle(Victim &victim);

	// bool remove(CuckooFilter* cf_remove);

	//generate 2 bucket addresses
	//缺少一个level参数
	void generateIF(const char* item, size_t &index, uint32_t &fingerprint, int fingerprint_size, int single_table_length);
	void generateA(size_t index, uint32_t fingerprint, size_t &alt_index, int single_table_length);
	
	bool GetPre(const char* item);
	bool GetPre(const char* item,int curLevel);

	//get info of DCF
	int getFingerprintSize();
	float size_in_mb();

	//extra function to make sure the table length is the power of 2
	uint64_t upperpower2(uint64_t x);

	//分裂函数
	bool append(std::string CFId);

	std::string getCFId(const char* item, uint32_t& fingerprint,size_t& index);
	std::string getCFId(uint32_t fingerprint);
	
	//void info();
	//std::string uint32ToString(uint32_t number, size_t numBits);
};







#endif //CLOGARITHMICDYNAMICCUCKOOFILTER_H
