#include "binarytree.h"
#include <string.h>
#include <iostream>
#include <openssl/sha.h>

//#define SHA256_DIGEST_LENGTH 64
using namespace std;

struct ProofNode{
  char *left,*right,*parent;
  ProofNode():left(""),right(""),parent(""){}
  ProofNode(char* _left,char* _right,char* _parent):left(_left),right(_right),parent(_parent){}
};

class merkletree{
public:
  vector<char*> tree;

  // declare function
  //vector<char*> computeTree(void (*combineFn)(char*,char*,char*),vector<char*> leaves);

  merkletree(){}
  merkletree(vector<char*> leaves){
    tree = computeTree(combin,leaves);
  }

  size_t size(){return tree.size();}
  char* root(){return tree[0];}

  void calSHA256(char* inp,char out_buff[65]){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, inp, strlen(inp));
    SHA256_Final(hash, &sha256);

    //char buffx[65];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
      sprintf(out_buff + (i * 2), "%02x", hash[i]);
    }
    out_buff[65] = 0;
    //memcpy(out_buff,buffx,65);
  }

  // combin and hash by sha256
  static void combin(char* leftData,char* rightData,char out_buff[65]){
    //concat
    char buff[strlen((const char*)leftData)+strlen((const char*)rightData)+1];
    memcpy(buff,leftData,strlen((const char*)leftData));
    memcpy(buff+strlen((const char*)leftData),rightData,strlen((const char*)rightData));

    buff[strlen((const char*)leftData)+strlen((const char*)rightData)] = 0;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buff, strlen(buff));
    SHA256_Final(hash, &sha256);

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
      sprintf(out_buff + (i * 2), "%02x", hash[i]);
    }
    out_buff[65] = 0;
  }

  vector<char*> computeTree(void (*combineFn)(char*,char*,char*),vector<char*> leaves){
    // compute nodeCount and create vector<T> tree
    int nodeCount = leafCountToNodeCount(leaves.size());
    int delta = nodeCount - leaves.size();
    vector<char*> tree(nodeCount);

    copy(leaves.begin(),leaves.end(),tree.begin()+delta);

    int idx = nodeCount-1;
    while(idx > 0){
      int parent = (idx -1)/2;

      //char*
      tree[parent] = new char[65];
      combineFn(tree[idx-1],tree[idx],tree[parent]);
      //cout<<"pass "<<&tree[parent]<<'\n';

      //tree[parent] = combinVal;

      //printf("%s %s\n",combinVal,tree[parent]);
      idx-=2;
    }

    return tree;
  }



  vector<ProofNode> proof(char* leafData){
    int idx = findLeaf(tree,leafData);
   // printf("idx %d\n",idx);
    if(idx == -1)
      return vector<ProofNode>();
    int proofArrSize = floor( log(tree.size())/ log(2) );

//    printf("proofArrSize : %d\n",proofArrSize);
    vector<ProofNode> proof(proofArrSize);
    int proofIdx = 0;
    while(idx > 0 ){
      idx = getParent(tree,idx);
      int left = getLeft(tree,idx);
      int right = getRight(tree,idx);

      proof[proofIdx++] = ProofNode(tree[left],tree[right],tree[idx]);
    }

//	printf("prooffIdx :%d \n",proofIdx);

	proof.resize(proofIdx);

//	vector<ProofNode> proof;
    return proof;
  }

  bool verifyProof(char* leaf,char* expectedMerkleRoot,vector<ProofNode> proofArr){
    if(!proofArr.size()){
      if( strcmp(leaf,expectedMerkleRoot))
        return true;
      return false;
    }

    // the merkle root should be the parent of the last part
    char* actualMekleRoot = proofArr[proofArr.size() -1].parent;

    if( strcmp(actualMekleRoot,expectedMerkleRoot)!=0 )
      return false;

    char* prevParent = leaf;
    for(int pIdx =0;pIdx<proofArr.size();pIdx++){
      ProofNode part = proofArr[pIdx];

      if( strcmp(part.left,prevParent)!=0 && strcmp(part.right,prevParent)!=0)
        return false;
      char *parentData = new char[65];
      combin(part.left,part.right,parentData);

      // Parent in proof is incorrect
      if( strcmp(parentData,part.parent) != 0 )
        return false;

      prevParent = parentData;
    }

    return strcmp(prevParent,expectedMerkleRoot) == 0;
  }


};