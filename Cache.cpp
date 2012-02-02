#include <iostream>
using namespace std;
typedef unsigned int TAG;

class Block {
	bool valid;
	bool dirty;
	int lruCounter;
	TAG tag;

public:
	Block() ;
	int setValid(bool value);
	int setDirty(bool value);
	int setTag( TAG tag);
	bool isValid();
	bool isDirty();

};

Block::Block() {
	valid = false;
	dirty = false;
	lruCounter=-1;
	
}

int Block::setValid(bool value) {

}

int Block::setDirty(bool value) {

}

int Block::setTag( TAG tag) {

}

bool Block::isValid() {

}

bool Block::isDirty() {

}

//Set class which holds the #ASSOC blocks;
class Set {
	Block *blk;

public:
//	Set(int assoc); since array of set have to be created default constructor can only be called
	Set();
	int init(int assoc);
	int getLRU(); // Returns the index of LRU block in the set
	int replace(int i,TAG tag); // Replaces the tag at the index i with the tag
	int updateRank(int i);//sets blk[i]'s rank to 0 and updates the rank of other blocks in the set
	bool search(TAG tag); //searches in the entire set and if tag is present returns true else return false


};

Set::Set() {

}

int Set::init(int assoc) {
	blk = new Block[assoc];
}

int Set::getLRU() {

}

int Set::replace(int i,TAG tag) {

}

int Set::updateRank(int i) {

}

bool Set::search(TAG tag) {

}

class Cache;

class StreamBuffer {
	Block *blk;
	Cache *nextLevel;

public:
//	StreamBuffer(int numBlocks) ; 
	StreamBuffer();
	int init(int numBlocks,Cache *c);
	int read(TAG tag); // Reads the address tag
	int write(TAG tag); //Writes the address tag
	int shift();//Shift the entire block up once , and set the new last block
	int makeDirty(TAG tag); // Forgot what to do
};

StreamBuffer::StreamBuffer() {

}

int StreamBuffer::init(int numBlocks,Cache *c) {
	blk = new Block[numBlocks];
	nextLevel = c;
}

int StreamBuffer::read(TAG tag) {

}

int StreamBuffer::write(TAG tag) {

}

int StreamBuffer::shift() {

}

int StreamBuffer::makeDirty(TAG tag) {

}

class Cache {
	int blkSize;
	int assoc;
	int cacheSize;
	int numStreamBuf;
	int numSets;
	Set *s;
	StreamBuffer *stBuf;
	Cache *nextLevel;
	
public:
	Cache(int blockSize,int numAssoc,int totalSize,int numStBufs,Cache *c);
	int read(TAG addr); 
	int write(TAG addr);
	int fetchInstruction(char *fileName); // May not be necessary . read in the run function possible
	int getIndexOfSet(TAG address); // From the whole address mask and return the index of set alone
	int run(char *fileName); // Start the simulation
};

Cache::Cache(int blockSize,int numAssoc,int totalSize,int numStBufs,Cache *c) {
	blkSize = blockSize;
	assoc = numAssoc;
	cacheSize = totalSize;
	numStreamBuf = numStBufs;
	numSets = cacheSize/(blkSize * assoc);
	s = new Set[numSets];

	//initialise blk of each s;
	stBuf = new StreamBuffer[numStreamBuf];
	//initialise all blk of stBuf
	nextLevel = c;
}

int Cache::read(TAG addr) {

}

int Cache::write(TAG addr) {

}

int Cache::fetchInstruction(char *fileName) {

}

int Cache::run(char *fileName) {

}

int main() {

//	Cache l1;
	cout<<"chk";

}


