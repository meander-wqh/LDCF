#include "cuckoofilter.h"


//妥协：每个CF实际存储的还是同样长的，但是会通过exact_fingerprint_size记录当前CF的指纹实际长度
CuckooFilter::CuckooFilter(std::string CFId, const size_t table_length, size_t fingerprint_size, const int single_capacity, int curlevel){
	std::cout<<"------new CuckooFilter------"<<std::endl;
	std::cout<<"CFId: "  <<CFId<<std::endl;
	std::cout<<"table_length: "  <<table_length<<std::endl;
	std::cout<<"single_capacity: "  <<single_capacity<<std::endl;
	exact_fingerprint_size = fingerprint_size - curlevel;
	this->fingerprint_size = fingerprint_size;
	std::cout<<"fingerprint_size: "  <<fingerprint_size<<std::endl;
	std::cout<<"exact_fingerprint_size: "  <<exact_fingerprint_size<<std::endl;
	
	bits_per_bucket = fingerprint_size*4;
	bytes_per_bucket = (fingerprint_size*4+7)>>3;
	single_table_length = table_length;
	counter = 0;//记录总数
	capacity = single_capacity;
	is_full = false;
	is_empty = true;
	level = curlevel;
	mask = (1ULL << fingerprint_size) - 1;//fingerprint长度的全1串
	this->CFId = CFId;

	bucket = new Bucket[single_table_length];
	for(size_t i = 0; i<single_table_length; i++){
		bucket[i].bit_array = new char[bytes_per_bucket];
		memset(bucket[i].bit_array, 0, bytes_per_bucket);
	}

	std::cout<<"------end------"<<std::endl;
}

CuckooFilter::~CuckooFilter(){
	delete[] bucket;
	// delete _0_child;
	// delete _1_child;
	// delete front;
}

void CuckooFilter::EmptyFilter(int curlevel,int single_capacity){
	fingerprint_size = 0l;
	bits_per_bucket = 0;
	bytes_per_bucket = 0;
	single_table_length = 0;
	counter = 0;
	capacity = single_capacity;
	is_full = false;
	is_empty = true;
	// _0_child = NULL;
	// _1_child = NULL;
	// front = NULL;
	level = curlevel;
	mask = 0;

	bucket = new Bucket[single_table_length];
	for(size_t i = 0; i<single_table_length; i++){
		bucket[i].bit_array = new char[bytes_per_bucket];
		memset(bucket[i].bit_array, 0, bytes_per_bucket);
	}

}

int CuckooFilter::insertItem(const char* item, Victim &victim){
	size_t index,alt_index;
	uint32_t fingerprint;
	generateIF(item, index, fingerprint, fingerprint_size, single_table_length);

	for(size_t count = 0; count<MaxNumKicks; count++){
		bool kickout = (count != 0);
		if(insertImpl(index, fingerprint, kickout, victim)){
			return true;
		}

		if (kickout){
			index = victim.index;
			fingerprint = victim.fingerprint;
			generateA(index, fingerprint, alt_index, single_table_length);
			index = alt_index;
		}else{
			generateA(index, fingerprint, alt_index, single_table_length);
			index = alt_index;
		}
	}

	return false;
}

bool CuckooFilter::insertItem(size_t index, uint32_t fingerprint, bool kickout, Victim &victim){
	size_t alt_index;

	for(size_t count = 0; count<MaxNumKicks; count++){
		bool kickout = (count != 0);
		if(insertImpl(index, fingerprint, kickout, victim)){
			return true;
		}

		if (kickout){
			index = victim.index;
			fingerprint = victim.fingerprint;
			generateA(index, fingerprint, alt_index, single_table_length);
			index = alt_index;
		}else{
			generateA(index, fingerprint, alt_index, single_table_length);
			index = alt_index;
		}
	}
	return false;
}

bool CuckooFilter::insertItem(size_t index, uint32_t fingerprint, Victim &victim){
	std::cout<<"bool CuckooFilter::insertItem(size_t index, uint32_t fingerprint, Victim &victim)"<<std::endl;
	size_t alt_index;
	size_t count;
	for(count = 0; count<MaxNumKicks; count++){
		bool kickout = (count != 0);
		//第一个kickout传入的是false
		if(insertImpl(index, fingerprint, kickout, victim)){
			// cout<<"count:"<<count<<endl;
			return true;
		}

		if (kickout){
			index = victim.index;
			fingerprint = victim.fingerprint;
			generateA(index, fingerprint, alt_index, single_table_length);
			index = alt_index;
		}else{
			//如果不是kickout，那么选择另一个候选index
			generateA(index, fingerprint, alt_index, single_table_length);
			index = alt_index;
		}
	}
	return false;
}

bool CuckooFilter::insertImpl(const size_t index, const uint32_t fingerprint, const bool kickout, Victim &victim){
	//std::cout<<"bool CuckooFilter::insertImpl(const size_t index, const uint32_t fingerprint, const bool kickout, Victim &victim)"<<std::endl;
	// std::cout<<index<<std::endl;
	// std::cout<<fingerprint<<std::endl;
	for(size_t pos = 0; pos<4; pos++){
		if(read(index,pos) == 0){
			//cout<<"insertImpl success: "<<fingerprint<<endl;
			write(index,pos,fingerprint);
			counter++;
			// std::cout<<"counter:"<<counter<<std::endl;
			// std::cout<<"capacity:"<<capacity<<std::endl;
			if(this->counter == capacity){
				this->is_full = true;
			}

			if(this->counter > 0 ){
				this->is_empty = false;
			}
			return true;
		}
	}
	if(kickout){
		int j = rand()%4;
		victim.index = index;
		//因为read取出的fingerprint可能是截取过的，那么需要进行补全
		victim.fingerprint = read(index,j);

		//4个位置随机写入一个
		write(index,j, fingerprint);
	}
	return false;
}

bool CuckooFilter::queryItem(const char* item){
	size_t index, alt_index;
	uint32_t fingerprint;
	generateIF(item, index, fingerprint, fingerprint_size, single_table_length);

	if(queryImpl(index, fingerprint)){
		return true;
	}
	generateA(index, fingerprint, alt_index, single_table_length);
	if(queryImpl(alt_index, fingerprint)){
		return true;
	}
	return false;
}

bool CuckooFilter::deleteItem(const char* item){
	size_t index, alt_index;
	uint32_t fingerprint;
	generateIF(item, index, fingerprint, fingerprint_size, single_table_length);

	if(deleteImpl(index, fingerprint)){
		return true;
	}
	generateA(index, fingerprint, alt_index, single_table_length);
	if(deleteImpl(alt_index, fingerprint)){
		return true;
	}

	return false;
}

bool CuckooFilter::queryImpl(const size_t index, const uint32_t fingerprint){
	if(fingerprint_size <= 4){
		const char* p = bucket[index].bit_array;
		uint64_t bits = *(uint64_t*)p;
		return hasvalue4(bits, fingerprint);
	}else if(fingerprint_size <= 8){
		const char* p = bucket[index].bit_array;
		uint64_t bits = *(uint64_t*)p;
		return hasvalue8(bits, fingerprint);
	}else if(fingerprint_size <= 12){
		const char* p = bucket[index].bit_array;
		uint64_t bits = *(uint64_t*)p;
		return hasvalue12(bits, fingerprint);
	}else if(fingerprint_size <= 16){
		const char* p = bucket[index].bit_array;
		uint64_t bits = *(uint64_t*)p;
		return hasvalue16(bits, fingerprint);
	}else{
		return false;
	}
}

bool CuckooFilter::deleteImpl(const size_t index, const uint32_t fingerprint){
	for(size_t pos = 0; pos<4; pos++){
		if(read(index, pos) == fingerprint){
			write(index, pos, 0);
			counter--;
			if(counter < this->capacity){
				this->is_full = false;
			}
			if(counter == 0){
				this->is_empty = true;
			}
			return true;
		}
	}
	return false;
}



void CuckooFilter::generateIF(const char* item, size_t &index, uint32_t &fingerprint, int fingerprint_size, int single_table_length){
	std::string  value = HashFunc::sha1(item);
	uint64_t hv = *((uint64_t*) value.c_str());

	index = ((uint32_t) (hv >> 32)) % single_table_length;
	fingerprint = (uint32_t) (hv & 0xFFFFFFFF);
	fingerprint &= ((0x1ULL<<fingerprint_size)-1);
	fingerprint += (fingerprint == 0);
}

void CuckooFilter::generateA(size_t index, uint32_t fingerprint, size_t &alt_index, int single_table_length){
	alt_index = (index ^ (fingerprint * 0x5bd1e995)) % single_table_length;
}


uint32_t CuckooFilter::read(size_t index, size_t pos){
	// std::cout<<index<<std::endl;

	const char* p = bucket[index].bit_array;
	//std::cout<<index<<std::endl;
	uint32_t fingerprint;
	if(fingerprint_size <= 4){
		p += (pos >> 1);
		uint8_t bits_8 = *(uint8_t*)p;//这里即p有可能指向某个pos，pos为0，1，2为开头的8bit
		if((pos & 1) == 0){//这个判断是判断是否为偶数，如果为偶数那么pos可能为0，可能为2
			fingerprint = (bits_8>>4) & 0xf;
		}else{
			fingerprint = bits_8 & 0xf;
		}
	}else if(fingerprint_size <= 8){
		p += pos;
		uint8_t bits_8 = *(uint8_t*)p;
		fingerprint = bits_8 & 0xff;
	}else if(fingerprint_size == 12){
		p += pos+(pos>>1);
		uint16_t bits_16 = *(uint16_t*)p;
		if((pos & 1) == 0){
			fingerprint = bits_16 & 0xfff;
		}else{
			fingerprint = (bits_16 >> 4) & 0xfff;
		}
	}else if(fingerprint_size <= 16){
		p += (pos<<1);
		uint16_t bits_16 = *(uint16_t*)p;
		fingerprint = bits_16 & 0xffff;
		//cout<<"read fingerprint:"<<fingerprint<<endl;
	}else if(fingerprint_size <= 24){
		p += pos+(pos<<1);
		uint32_t bits_32 = *(uint32_t*)p;
		fingerprint = (bits_32 >> 4);
	}else if(fingerprint_size <= 32){
		p += (pos<<2);
		uint32_t bits_32 = *(uint32_t*)p;
		fingerprint = bits_32 & 0xffffffff;
	}else{
		fingerprint =0;
	}
	return fingerprint & mask;
}

//这里会对fingerprint做处理，满足本CF的fingerprint_size
void CuckooFilter::write(size_t index, size_t pos, uint32_t fingerprint){
	char* p = bucket[index].bit_array;

	if(fingerprint_size <= 4){
		p += (pos>>1);
		if((pos & 1) == 0){
			*((uint8_t*)p) &= 0x0f;
			*((uint8_t*)p) |= (fingerprint<<4);
		}else{
			*((uint8_t*)p) &= 0xf0;
			*((uint8_t*)p) |= fingerprint;
		}
	}else if(fingerprint_size <= 8){
		((uint8_t*)p)[pos] = fingerprint;
	}else if(fingerprint_size <= 12){
		p += (pos + (pos>>1));
		if((pos & 1) == 0){
			*((uint16_t*)p) &= 0xf000; //Little-Endian
			*((uint16_t*)p) |= fingerprint;
		}else{
			*((uint16_t*)p) &= 0x000f;
			*((uint16_t*)p) |= fingerprint<<4;
		}
	}else if(fingerprint_size <= 16){
		((uint16_t*) p)[pos] = fingerprint;
	}else if(fingerprint_size <= 24){
		p += (pos+ (pos<<1));
		*((uint32_t*)p) &= 0xff000000; //Little-Endian
		*((uint32_t*)p) |= fingerprint;
	}else if(fingerprint_size <= 32){
		((uint32_t*) p)[pos] = fingerprint;
	}
}
	




