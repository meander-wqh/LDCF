#ifndef LINKTREE_H_
#define LINKTREE_H_
#include"uint.h"

struct TreeNode{
	TreeNode* Pt0;
	TreeNode* Pt1;
	TreeNode() :Pt0(nullptr), Pt1(nullptr) {}
};



class LinkTree{
public:
	int level;
	int num;
	TreeNode* root;
	LinkTree(){
		level = 0;
		num = 0;
		root = new TreeNode();
	}
	std::string getCFId(uint32_t fingerprint,size_t exact_fingerprint_size){
		std::string CFId = "";
		TreeNode* curNode = root;
		std::string sfingerprint = uint32ToString(fingerprint,exact_fingerprint_size);
		int index = 0;
		while(curNode->Pt0 != nullptr){
			if(sfingerprint[index] == '0'){
				curNode = curNode->Pt0;
			}else{
				curNode = curNode->Pt1;
			}
			CFId = CFId + sfingerprint[index];
			++index;
		}
		return CFId;
	}

	//分裂，input:为分裂的节点路径
	bool append(std::string path){
		TreeNode* curNode = root;
		for(int i=0;i<path.length();++i){
			if(path[i] == '0'){
				curNode = curNode->Pt0;
			}else{
				curNode = curNode->Pt1;
			}
		}
		curNode->Pt0 = new TreeNode();
		curNode->Pt1 = new TreeNode();
		return true;
	}
	void releaseTree(TreeNode* root) {
		if (root == nullptr) {
			return;
		}

		// 递归释放左子树
		releaseTree(root->Pt0);

		// 递归释放右子树
		releaseTree(root->Pt1);

		// 释放当前节点
		delete root;
	}
	~LinkTree(){
		releaseTree(root);
	}
};



#endif /* LINKTREE_H_ */
