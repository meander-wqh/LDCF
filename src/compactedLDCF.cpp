#include "compactedLDCF.h"


using namespace std;


CompactedLogarithmicDynamicCuckooFilter::CompactedLogarithmicDynamicCuckooFilter(const size_t item_num, const double fp, const size_t exp_block_num){
	std::cout<<"------new CompactedLDCF------"<<std::endl;
	capacity = item_num;
	
	//exp_block_num是期望有多少个CF?
	single_table_length = upperpower2(capacity/4.0/exp_block_num);
	// cout<<exp_block_num<<endl;
	// cout<<single_table_length<<endl;
	single_capacity = single_table_length*0.9375*4;//s=6 1920 s=12 960 s=24 480 s=48 240 s=96 120

	false_positive = fp;
	single_false_positive = 1-pow(1.0-false_positive, ((double)single_capacity/capacity));
	fingerprint_size_double = ceil(log(8.0/single_false_positive)/log(2));
	if(fingerprint_size_double>0 && fingerprint_size_double<=4){
		fingerprint_size = 4;
	}else if(fingerprint_size_double>4 && fingerprint_size_double<=8){
		fingerprint_size = 8;
	}else if(fingerprint_size_double>8 && fingerprint_size_double<=12){
		fingerprint_size = 12;
	}else if(fingerprint_size_double>12 && fingerprint_size_double<=16){
		fingerprint_size = 16;
	}else if(fingerprint_size_double>16 && fingerprint_size_double<=24){
		fingerprint_size = 16;
	}else if(fingerprint_size_double>24 && fingerprint_size_double<=32){
		fingerprint_size = 16;
	}else{
		cout<<"fingerprint out of range!!!"<<endl;
		fingerprint_size = 16;
	}
	counter = 0;

	std::cout<<"total_capacity: "<<capacity<<std::endl;
	std::cout<<"single_table_length: "<<single_table_length<<std::endl;
	std::cout<<"single_capacity: "<<single_capacity<<std::endl;
	std::cout<<"false_positive: "<<false_positive<<std::endl;
	std::cout<<"single_false_positive: "<<single_false_positive<<std::endl;
	std::cout<<"fingerprint_size: "<<fingerprint_size<<std::endl;

	cf_tree = new LinkTree();
	std::cout<<"------end------"<<std::endl;

	CFMap[""] = new CuckooFilter("",single_table_length, fingerprint_size, single_capacity, 0);	
	CFnumber++;
}

CompactedLogarithmicDynamicCuckooFilter::~CompactedLogarithmicDynamicCuckooFilter(){
	delete cf_tree;
	for(auto it = CFMap.begin();it!=CFMap.end();it++){
		delete it->second;
	}
}


//插入CF的层级，候选index，指纹, 层级代表id的位数，第0层为空字符串，第1层为"0"或者"1"
bool CompactedLogarithmicDynamicCuckooFilter::insertItem(std::string CFId, size_t index, uint32_t fingerprint){
	std::cout<<"bool CompactedLogarithmicDynamicCuckooFilter::insertItem(int level, size_t index, uint32_t fingerprint)"<<std::endl;
	int level = CFId.length();
	std::cout<<"fingerprint: "<<fingerprint<<endl;
	std::cout<<"insertItem in:"<<CFId<<std::endl;
	//这里传入的指纹长度是完整的，在插入的时候才会进行截取
	// uint32_t uCFId = fingerprint >> (fingerprint_size-level);
	// std::string CFId = uint32ToString(uCFId,level);
	if(CFMap.find(CFId) == CFMap.end()){
		std::cout<<"cannot find CFId:"<<CFId<<std::endl;
		return false;
	}
	if(CFMap[CFId]->is_full == true){
		//分裂
		std::cout<<"CFId:"<<CFId<<" is full"<<std::endl;
		append(CFId);
		// if(CFMap["1"] == nullptr){
		// 	std::cout<<"testnullptr"<<endl;
		// }
		//std::cout<<CFMap["1"]->CFId<<std::endl;
		CFId = CFId + uint32ToString(fingerprint >> (fingerprint_size-(level+1)),1);//修改需要插入的CF
	}
	if(CFMap[CFId]->insertItem(index,fingerprint,victim)){
		std::cout<<"insertItem success"<<std::endl;
		counter++;
	}else{
		//错误处理：分裂
		std::cout<<"handle failure"<<std::endl;
		std::string next =  uint32ToString(victim.fingerprint >> (CFMap[CFId]->exact_fingerprint_size-1),1);
		append(CFId);
		CFId = CFId + next;
		insertItem(CFId,victim.index,victim.fingerprint);
	}

	return true;
}
std::string CompactedLogarithmicDynamicCuckooFilter::getCFId(const char* item, uint32_t& fingerprint, size_t& index){
	std::string  value = HashFunc::sha1(item);
	uint64_t hv = *((uint64_t*) value.c_str());
	index = ((uint32_t) (hv >> 32)) % single_table_length;
	fingerprint = (uint32_t) (hv & 0xFFFFFFFF);
	fingerprint &= ((0x1ULL<<fingerprint_size)-1);
	fingerprint += (fingerprint == 0);

	return cf_tree->getCFId(fingerprint,fingerprint_size);
}


std::string CompactedLogarithmicDynamicCuckooFilter::getCFId(uint32_t fingerprint){
	return cf_tree->getCFId(fingerprint,fingerprint_size);
}

// std::string CompactedLogarithmicDynamicCuckooFilter::uint32ToString(uint32_t number, size_t numBits) {
//     std::bitset<32> bits(number);
//     std::string binaryStr = bits.to_string();
//     return binaryStr.substr(32 - numBits); // 截取有效位数部分
// }

//分裂append函数,改成先构造两个空的，然后再遍历老的再往里面填充bucket
bool CompactedLogarithmicDynamicCuckooFilter::append(std::string CFId){
	CFnumber++;
	//cf_tree分裂
	cf_tree->append(CFId);

	string child0Id = CFId + "0";
	string child1Id = CFId + "1";

	CuckooFilter* child0 = new CuckooFilter(child0Id, single_table_length,fingerprint_size,single_capacity,CFMap[CFId]->level+1);
	CuckooFilter* child1 = new CuckooFilter(child1Id, single_table_length,fingerprint_size,single_capacity,CFMap[CFId]->level+1);

	//处理每个CF中的fingerprint
	for(int index=0;index<single_table_length;++index){
		for(int pos=0;pos<4;++pos){
			uint32_t fingerprint = CFMap[CFId]->read(index,pos);
			//std::cout<<"fingerprint:"<<fingerprint<<std::endl;
			uint32_t fingerprint_0 = 0;
			uint32_t fingerprint_1 = 0;
			// cout<<"fingerprint:"<<fingerprint<<endl;
			// cout<<"移位："<<CFMap[CFId]->exact_fingerprint_size-1<<" fingerprint:"<<(fingerprint >> (CFMap[CFId]->exact_fingerprint_size-1))<<endl;
			//根据下一位进行分配
			if(((fingerprint >> (CFMap[CFId]->exact_fingerprint_size-1)) & 1) == 0){
				fingerprint_0 = fingerprint;
			}else{
				fingerprint_1 = fingerprint;
			}
			// uint32_t fingerprint_0 = fingerprint & ((1<<(CFMap[CFId]->exact_fingerprint_size-1)) - ((~fingerprint >> (CFMap[CFId]->exact_fingerprint_size - 1)) & 1));
			// fingerprint_0 = fingerprint_0 & ((1<<(CFMap[CFId]->exact_fingerprint_size-1))-1);
			// //std::cout<<"fingerprint_0:"<<fingerprint_0<<std::endl;
			// uint32_t fingerprint_1 = fingerprint & ((1<<(CFMap[CFId]->exact_fingerprint_size-1)) - ((fingerprint >> (CFMap[CFId]->exact_fingerprint_size - 1)) & 1));
			// fingerprint_1 = fingerprint_1 & ((1<<(CFMap[CFId]->exact_fingerprint_size-1))-1);
			// //std::cout<<"fingerprint_1:"<<fingerprint_1<<std::endl;
			child0->write(index,pos,fingerprint_0);
			child1->write(index,pos,fingerprint_1);
			if(fingerprint_0 != 0){
				child0->counter++;
			}
			if(fingerprint_1 != 0){
				child1->counter++;
			}
		}
	}
	// std::cout<<"child0_counter: "<<child0->counter<<std::endl;
	// std::cout<<"child1_counter: "<<child1->counter<<std::endl;
	CFMap[child0Id] = child0;
	CFMap[child1Id] = child1;

	//删除CFMap中对应的老的map
	auto it = CFMap.find(CFId);
	if(it != CFMap.end()){
		CFMap.erase(it);
	}
	return true;
}

// bool CompactedLogarithmicDynamicCuckooFilter::queryItem(const char* item){
// 	size_t index, alt_index;
// 	uint32_t fingerprint;


// 	generateIF(item, index, fingerprint, fingerprint_size, single_table_length, level);
// 	generateA(index, fingerprint, alt_index, single_table_length, level);

// 	CuckooFilter* query_pt = cf_tree->cf_pt;
// 	for(int count = 0; count<cf_tree->level; count++){

// 		if(query_pt->queryImpl(index, fingerprint)){
// 			return true;
// 		}else if(query_pt->queryImpl(alt_index, fingerprint)){
// 			return true;
// 		}else{
// 			if(GetPre(item)
// 				query_pt = query_pt->child0;
// 			else
// 				query_pt = query_pt->child1;
// 		}
// 		if(query_pt == 0){
// 			break;
// 		}

// 	}
// 	return false;
// }

bool CompactedLogarithmicDynamicCuckooFilter::queryItem(std::string CFId, const char* item){
	//std::cout<<"bool CompactedLogarithmicDynamicCuckooFilter::queryItem(std::string CFId, const char* item)"<<std::endl;
	return CFMap[CFId]->queryItem(item);
}

// bool CompactedLogarithmicDynamicCuckooFilter::deleteItem(const char* item){
// 	size_t index, alt_index;
// 	uint32_t fingerprint;

// 	generateIF(item, index, fingerprint, fingerprint_size, single_table_length);
// 	generateA(index, fingerprint, alt_index, single_table_length);
// 	CuckooFilter* delete_pt = cf_tree->cf_pt;
// 	for(int count = 0; count<cf_tree->level; count++){
// 		if(delete_pt->queryImpl(index, fingerprint)){
// 			if(delete_pt->deleteImpl(index, fingerprint)){
// 				counter--;
// 				return true;
// 			}
// 		}else if(delete_pt->queryImpl(alt_index, fingerprint)){
// 			if(delete_pt->deleteImpl(alt_index ,fingerprint)){
// 				counter--;
// 				return true;
// 			}
// 		}else{
// 			if(GetPre(item)
// 				query_pt = query_pt->child0;
// 			else
// 				query_pt = query_pt->child1;
// 		}
// 	}
// 	return false;
// }

bool CompactedLogarithmicDynamicCuckooFilter::deleteItem(std::string CFId,size_t index, uint32_t fingerprint){
	size_t alt_index;
	generateA(index, fingerprint, alt_index, single_table_length);
	CuckooFilter* delete_pt = CFMap[CFId];
	if(delete_pt->queryImpl(index, fingerprint)){
		if(delete_pt->deleteImpl(index, fingerprint)){
			counter--;
			return true;
		}
	}else if(delete_pt->queryImpl(alt_index, fingerprint)){
		if(delete_pt->deleteImpl(alt_index ,fingerprint)){
			counter--;
			return true;
		}
	}
	return false;
}


//生成index 和 fingerprint
void CompactedLogarithmicDynamicCuckooFilter::generateIF(const char* item, size_t &index, uint32_t &fingerprint, int fingerprint_size, int single_table_length){
	std::string  value = HashFunc::sha1(item);
	uint64_t hv = *((uint64_t*) value.c_str());

	index = ((uint32_t) (hv >> 32)) % single_table_length;
	fingerprint = (uint32_t) (hv & 0xFFFFFFFF);
	fingerprint &= ((0x1ULL<<fingerprint_size)-1);
	fingerprint += (fingerprint == 0);
}

//生成alt_index
void CompactedLogarithmicDynamicCuckooFilter::generateA(size_t index, uint32_t fingerprint, size_t &alt_index, int single_table_length){
	alt_index = (index ^ (fingerprint * 0x5bd1e995)) % single_table_length;
}

//这部分还不太懂
bool CompactedLogarithmicDynamicCuckooFilter::GetPre(const char* item){
	std::string  value = HashFunc::sha1(item);
	uint64_t hv = *((uint64_t*) value.c_str());
	uint32_t fingerprint = (uint32_t) (hv & 0xFFFFFFFF);
	if (fingerprint >> (fingerprint_size -1) ==0)
		return true;
	return false;
}

//我的版本
bool CompactedLogarithmicDynamicCuckooFilter::GetPre(const char* item,int curLevel){
	std::string  value = HashFunc::sha1(item);
	uint64_t hv = *((uint64_t*) value.c_str());
	uint32_t fingerprint = (uint32_t) (hv & 0xFFFFFFFF);
	//这里还需要对移位后的uint32_t fingerprint取最低位，方法是&1
	if ((fingerprint >> (fingerprint_size - curLevel)) & 1 == 0)
		return true;
	return false;
}

int CompactedLogarithmicDynamicCuckooFilter::getFingerprintSize(){
	return fingerprint_size;
}

float CompactedLogarithmicDynamicCuckooFilter::size_in_mb(){
	float b = 0.0;
	for(auto it = CFMap.begin();it!=CFMap.end();++it){
		b += (fingerprint_size-it->second->level)*4.0*single_table_length ;
	}

	return b / 8.0 / 1024 / 1024;
}

uint64_t CompactedLogarithmicDynamicCuckooFilter::upperpower2(uint64_t x) {
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;
  x++;
  return x;
}
