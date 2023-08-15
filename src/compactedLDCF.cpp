#include "compactedLDCF.h"


using namespace std;


CompactedLogarithmicDynamicCuckooFilter::CompactedLogarithmicDynamicCuckooFilter(const size_t item_num, const double fp, const size_t exp_block_num){

	capacity = item_num;

	single_table_length = upperpower2(capacity/4.0/exp_block_num);
	single_capacity = single_table_length*0.9375*4;//s=6 1920 s=12 960 s=24 480 s=48 240 s=96 120

	false_positive = fp;
	single_false_positive = 1-pow(1.0-false_positive, ((double)single_capacity/capacity));

	fingerprint_size_double = ceil(log(8.0/single_false_positive)/log(2));
	counter = 0;


	curCF = new CuckooFilter(single_table_length, fingerprint_size, single_capacity, 0);
	child0CF = new CuckooFilter(single_table_length, fingerprint_size, single_capacity, 1);
	child1CF = new CuckooFilter(single_table_length, fingerprint_size, single_capacity, 1);

	//>>>>>>这里先假设curlevel为0吧
	cf_tree = new LinkTree(single_table_length, fingerprint_size, single_capacity,0);
	cf_tree->cf_pt = curCF;
	cf_tree->child0_pt = child0CF;
	cf_tree->child1_pt = child1CF;
}

CompactedLogarithmicDynamicCuckooFilter::~CompactedLogarithmicDynamicCuckooFilter(){
	delete curCF;
	delete child0CF;
	delete child1CF;
	delete cf_tree;
}



//这个函数伪代码中是从root开始遍历的，需要重写
bool CompactedLogarithmicDynamicCuckooFilter::insertItem(const char* item){
	int curLevel = 0;
	curCF = cf_tree->cf_pt; //从root开始
	while(curCF->is_empty == true){
		//如果curCF为空，那么说明该CF已经分裂，值已经分裂到后面两个子过滤器中
		++curLevel;
		if(GetPre(item,curLevel)){
			curCF = getChild0CF(curCF);
		}else{
			curCF = getChild1CF(curCF);
		}
	}
	//目前找到了对应的CF
	if(curCF->insertItem(item,victim)){
		counter++;
	}else{
		//如果没有插入成功，缺少一个failureHandle函数，需要自己写，文章提到如果没有成功那么说明需要分裂了
		//TODO
		counter++;
	}


	// //下面是他们写的，完全不对
	// if(curCF->is_full == true){
	// 	if(GetPre(item))
	// 		curCF = getChild0CF(curCF);
	// 	else
	// 		curCF = getChild1CF(curCF);
	// }

	// if(curCF->insertItem(item, victim)){
	// 	counter++;
	// }else{
	// 	counter++;
	// }

	return true;
}

//TODO需要写一个缺少的append函数

//修改名称，curCF中并没有child0CF,名称不统一
//在curCF满了的前提下才会触发这个函数
CuckooFilter* CompactedLogarithmicDynamicCuckooFilter::getChild0CF(CuckooFilter* curCF){
	CuckooFilter* _child0CF = NULL;
	if(curCF->_0_child == NULL){
		//没有初始化_child0CF，没有传入curlevel参数
		_child0CF = new CuckooFilter(single_table_length, fingerprint_size, single_capacity, curCF->level+1);
		curCF->_0_child = _child0CF;
		_child0CF->front = curCF;
		cf_tree->_0_child = _child0CF;
	}else{
		_child0CF = curCF->_0_child;
	}
	return _child0CF;
}

//重复名称，应该是getChild1CF
CuckooFilter* CompactedLogarithmicDynamicCuckooFilter::getChild0CF(CuckooFilter* curCF){
	if(curCF->child1CF == NULL){
		_child1CF = new CuckooFilter(single_table_length, fingerprint_size, single_capacity);
		curCF->child1CF = _child1CF;
		_child1CF->front = curCF;
		cf_list->child1_pt = _child1CF;
	}else{
		_child1CF = curCF->child1CF;
	}
	return _child1CF;
}

bool CompactedLogarithmicDynamicCuckooFilter::queryItem(const char* item){
	size_t index, alt_index;
	uint32_t fingerprint;

	generateIF(item, index, fingerprint, fingerprint_size, single_table_length, level);
	generateA(index, fingerprint, alt_index, single_table_length, level);

	CuckooFilter* query_pt = cf_tree->cf_pt;
	for(int count = 0; count<cf_tree->level; count++){

		if(query_pt->queryImpl(index, fingerprint)){
			return true;
		}else if(query_pt->queryImpl(alt_index, fingerprint)){
			return true;
		}else{
			if(GetPre(item)
				query_pt = query_pt->child0;
			else
				query_pt = query_pt->child1;
		}
		if(query_pt == 0){
			break;
		}

	}
	return false;
}

bool CompactedLogarithmicDynamicCuckooFilter::deleteItem(const char* item){
	size_t index, alt_index;
	uint32_t fingerprint;

	generateIF(item, index, fingerprint, fingerprint_size, single_table_length);
	generateA(index, fingerprint, alt_index, single_table_length);
	CuckooFilter* delete_pt = cf_tree->cf_pt;
	for(int count = 0; count<cf_tree->level; count++){
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
		}else{
			if(GetPre(item)
				query_pt = query_pt->child0;
			else
				query_pt = query_pt->child1;
		}
	}
	return false;
}



void CompactedLogarithmicDynamicCuckooFilter::generateIF(const char* item, size_t &index, uint32_t &fingerprint, int fingerprint_size, int single_table_length, int level){
	std::string  value = HashFunc::sha1(item);
	uint64_t hv = *((uint64_t*) value.c_str());

	index = ((uint32_t) (hv >> 32)) % single_table_length;
	fingerprint = (uint32_t) (hv & 0xFFFFFFFF);
	fingerprint &= ((0x1ULL<<fingerprint_size)-1);
	fingerprint += (fingerprint == 0);
	fingerprint = fingerprint >> level;
}

void CompactedLogarithmicDynamicCuckooFilter::generateA(size_t index, uint32_t fingerprint, size_t &alt_index, int single_table_length, int level){
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
	if ((fingerprint >> (fingerprint_size - curLevel)) & 1 ==0)
		return true;
	return false;
}



int CompactedLogarithmicDynamicCuckooFilter::getFingerprintSize(){
	return fingerprint_size;
}

float CompactedLogarithmicDynamicCuckooFilter::size_in_mb(){
	return fingerprint_size * 4.0 * single_table_length * cf_list->num / 8 / 1024 / 1024;
}

uint64_t LogarithmicDynamicCuckooFilter::upperpower2(uint64_t x) {
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
