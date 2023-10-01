#include "hashfunction.h"

string HashFunc::sha1(const char* key){
	// EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	// unsigned char md_value[EVP_MAX_MD_SIZE];
	// unsigned int md_len;

	// EVP_DigestInit(mdctx, EVP_sha1());
	// EVP_DigestUpdate(mdctx, (const void*) key, sizeof(key));
	// EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	// EVP_MD_CTX_free(mdctx);

	// return std::string((char*)md_value, (size_t)md_len);
	unsigned char res[32];
	SHA256_CTX* ctx = new SHA256_CTX();
    SHA256_Init(ctx);
    SHA256_Update(ctx,key,strlen(key));
    SHA256_Final(res,ctx);
	//cout<<"外部data: ";
	//print_bytes((uint8_t*)data, strlen((const char*)data));
	// print_bytes((uint8_t*)res, strlen((const char*)res));
	std::string sha256_hash;
	for(int i=0;i<32;i++){
		sha256_hash+=res[i];
	}
    return sha256_hash;
}

string HashFunc::md5(const char* key){
	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len;

	EVP_DigestInit(mdctx, EVP_md5());
	EVP_DigestUpdate(mdctx, (const void*) key, sizeof(key));
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	EVP_MD_CTX_free(mdctx);

	return std::string((char*)md_value, (size_t)md_len);
}
