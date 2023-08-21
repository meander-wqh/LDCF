#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <vector>
#include "compactedLDCF.h"

using namespace std;

typedef struct{
	size_t item_num;
	double exp_FPR;
	string dataset_path;
	size_t exp_block_num;
}Config;


typedef struct{
	int exp_BBN;
	double actual_FPR;
	int actual_BBN;
	int F_size;
	double space_cost;
	double I_time;
	double Q_time;
	double D_time;
	double C_rate;
}Metric;



Metric test(const Config config, string *data){

	Metric metric;
	//DynamicCuckooFilter* dcf = new DynamicCuckooFilter(config.item_num, config.exp_FPR);
	//CompactedLogarithmicDynamicCuckooFilter* cldcf = new CompactedLogarithmicDynamicCuckooFilter(config.item_num, config.exp_FPR);
	CompactedLogarithmicDynamicCuckooFilter* cldcf = new CompactedLogarithmicDynamicCuckooFilter(config.item_num, config.exp_FPR,config.exp_block_num);
	size_t capacity = config.item_num;
	size_t exp_block_num = config.exp_block_num;
	metric.exp_BBN = config.exp_block_num; 
	uint64_t single_table_length = cldcf->upperpower2(capacity/4.0/exp_block_num);


	//**********insert**********
	std::cout<<"**********insert**********"<<std::endl;
	metric.I_time = clock();
	for(size_t i = 0; i<config.item_num; i++){
		std::cout<<"data: "<<data[i]<<std::endl;
		uint32_t fingerprint;
		size_t index;
		//由于要通过fingerprint找到对应的CFId，同时是通过SGX生成用来查找的，所以这里生成的fingerprint是完整的长度
		std::string CFId = cldcf->getCFId(data[i].c_str(),fingerprint,index);
		cldcf->insertItem(CFId, index, fingerprint);
	}
	metric.I_time = clock() - metric.I_time;
	metric.I_time = metric.I_time/CLOCKS_PER_SEC;

	metric.space_cost = cldcf->size_in_mb();

	//**********query**********

	std::cout<<"**********query**********"<<std::endl;
	int false_positive_count = 0;
	int found_count = 0;

	metric.Q_time = clock();
	for(size_t i = 0; i<config.item_num; i++){
		uint32_t fingerprint;
		size_t index;
		std::string CFId = cldcf->getCFId(data[i].c_str(),fingerprint,index);
		std::cout<<"data: "<<data[i]<<std::endl;
		std::cout<<"CFId: "<<CFId<<std::endl;
		std::cout<<"fingerprint: "<<fingerprint<<std::endl;
		if(cldcf->queryItem(CFId,data[i].c_str()) == false){
			cout << "Can't found Item: "<<data[i]<<" in "<<CFId<< endl;
		}else{
			cout << "Found Item:" <<data[i] <<" in "<<CFId<< endl;
			found_count++;
		}
	}
	metric.Q_time = clock() - metric.Q_time;
	metric.Q_time = metric.Q_time/CLOCKS_PER_SEC;

	//calculate false
	for(size_t i = 0; i<config.item_num; i++){
		char item[10] = {0};
		sprintf(item, "%ld", i + 1000000);
		uint32_t fingerprint;
		size_t index;
		std::string CFId = cldcf->getCFId((const char*)item,fingerprint,index);
		if(cldcf->queryItem(CFId,item)){
			false_positive_count++;
		}
	}
	metric.actual_FPR = (double)false_positive_count/config.item_num;

	std::cout<<"ItemCounter: "<<cldcf->counter<<std::endl;
	cout<<"found_count:"<<found_count<<endl;
	std::cout<<"CFnumber: "<<cldcf->CFnumber<<std::endl;
	metric.actual_BBN = cldcf->CFnumber;
	cout<<"false_positive:"<< (double)false_positive_count/config.item_num<<endl;



	//**********delete**********

	std::cout<<"**********delete**********"<<std::endl;


	size_t count = 0;
	metric.D_time = clock();
	while(count < config.item_num){
		uint32_t fingerprint;
		size_t index;
		std::string CFId = cldcf->getCFId(data[count].c_str(),fingerprint,index);
		cldcf->deleteItem(CFId,index,fingerprint);
		count += 1; //delete all the items
	}
	metric.D_time = clock() - metric.D_time;
	metric.D_time = metric.D_time/CLOCKS_PER_SEC;

	return metric;

}



string Get_Value(string config_buff){
	string value;
	int pos = config_buff.find("=", 0);
	if (pos != -1)
	{
		pos++;
		value = config_buff.substr(pos, config_buff.length());
	} else {
		exit(1);
	}

	while(1){
		pos = value.find(" ", 0);
		if(pos >= 0){
			value = value.substr(pos+1, config_buff.length());
		}
		else{
			break;
		}
	}

	return value;
}


Config Read_Config(const string path){
	ifstream in_config(path.c_str());
	string config_buff;
	Config configuration;
	getline(in_config, config_buff);
	configuration.exp_FPR = atof(Get_Value(config_buff).c_str());
	getline(in_config, config_buff);
	configuration.item_num = atof(Get_Value(config_buff).c_str());
	getline(in_config, config_buff);
	configuration.dataset_path = Get_Value(config_buff);
	getline(in_config, config_buff);
	configuration.exp_block_num = atoi(Get_Value(config_buff).c_str());
	
	return configuration;
}


string* Read_Dataset(const Config config, const string path){
	ifstream in_input(path.c_str());
	string input_buff;
	if(!getline(in_input, input_buff)){
		cout << "Read File Error!" << endl;
	}
	string buff;
	stringstream ss(input_buff);

	vector<string> tokens;
	size_t item_count = 0;
	string *input_data = new string[config.item_num];
	while (item_count < config.item_num){
		ss >> buff;
		tokens.push_back(buff);
		input_data[item_count] = buff;
		item_count ++;
	}
	return input_data;
}

void Print_Info(Config config, Metric metric){

	ofstream out("./result/result.txt");

	out << "item_num" << setw(20) << "exp_FPR"
		<< setw(20) << "actual_FPR" << setw(20) << "actual_BBN" << setw(20) << "F_size(bits)"
		<< setw(20) << "space_cost(MB)"
		<< setw(20) << "I_time(s)" << setw(20) << "Q_time(s)" << setw(20) << "D_time(s)" << setw(20) << "C_rate"
		<< endl;

	out << config.item_num << setw(20) << config.exp_FPR
		<< setw(20) << metric.actual_FPR << setw(20) << metric.actual_BBN << setw(20) << metric.F_size
		<< setw(20) << metric.space_cost
		<< setw(20) << metric.I_time << setw(20) << metric.Q_time << setw(20) << metric.D_time << setw(20) << metric.C_rate
		<< endl;
}




int main(int argc, char* argv[]){

	string config_path = "./configuration/config.txt";
	Config config = Read_Config(config_path);

	string dataset_path = config.dataset_path;
	string *data = Read_Dataset(config, dataset_path);


    Metric metric = test(config, data);

    Print_Info(config, metric);

}

