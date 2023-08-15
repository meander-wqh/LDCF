#ifndef LINKTREE_H_
#define LINKTREE_H_

struct TreeNode{
	TreeNode* Pt0 = nullptr;
	TreeNode* Pt1 = nullptr;
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
};



#endif /* LINKTREE_H_ */
