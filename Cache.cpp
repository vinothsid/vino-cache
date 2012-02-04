#include <iostream>
//#include<limits>
#include "limits.h"
#include <fstream>
#include <sstream>
#include "string.h"
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
	int setTag( TAG tagAddr);
	bool isValid();
	bool isDirty();
	bool checkTag(TAG tagAddr);
	int setLru(int i);
	int getLru();
	int incrLru();
	int getTag();

};

Block::Block() {
	valid = false;
	dirty = false;
	lruCounter=-1;
	
}

int Block::setLru(int i) {
	lruCounter = i;
}

int Block::getLru() {
	return lruCounter;
}

int Block::getTag() {
	return tag;
}
int Block::incrLru() {
	lruCounter++;
}
 
int Block::setValid(bool value) {
	valid  = value;
}

int Block::setDirty(bool value) {
	dirty = value;
}

int Block::setTag( TAG tagAddr) {
	tag = tagAddr;
}

bool Block::isValid() {
	return (valid==true);
}

bool Block::isDirty() {
	return (dirty==true);
}

bool Block::checkTag(TAG tagAddr) {
	return isValid() && (tag==tagAddr);
}

//Set class which holds the #ASSOC blocks;
class Set {
	Block *blk;
	int setAssoc;
public:
//	Set(int assoc); since array of set have to be created default constructor can only be called
	Set();
	int init(int assoc);
	int getLRU(); // Returns the index of LRU block in the set
	int place(TAG tag,char oper); // places the tag at corresponding position;
	int updateRank(int i);//sets blk[i]'s rank to 0 and updates the rank of other blocks in the set
	bool search(TAG tag,char oper); //searches in the entire set and if tag is present returns true else return false
	void print();


};

Set::Set() {

}

void Set::print() {
	for(int i=0;i<setAssoc;i++) {
		cout << hex << blk[i].getTag() << ":" << blk[i].isDirty() << " " ;
	}
	cout << endl;
}

int Set::init(int assoc) {
	setAssoc = assoc;
	blk = new Block[assoc];
	for(int i=0;i<setAssoc;i++) {
		blk[i].setLru(i);
	}
}

int Set::getLRU() {

	int max=INT_MIN;
	int index=-1;
	for(int i=0;i<setAssoc;i++) {
		if( blk[i].getLru() > max ) {
			max = blk[i].getLru();
			index = i;
		}
	}

	return index;
}

int Set::place(TAG tag,char oper) {
	int placeIndex=-1;
	for(int i=0;i<setAssoc;i++) {
		if(blk[i].isValid()==false) {

			placeIndex = i;
			break;
		}
	}

	if(placeIndex==-1) {
		placeIndex = getLRU();

	} else {
        	blk[placeIndex].setValid(true);
		
	}

	if (blk[placeIndex].isDirty() ) {
		// issue a write to next level 
		//nextLevel.write(tag);
		cout << "Dirty block " << hex << blk[placeIndex].getTag() << " evicted" << endl;
		blk[placeIndex].setDirty(false);
	}

	if (oper == 'w') {
		blk[placeIndex].setDirty(true);
	}
        blk[placeIndex].setTag(tag);
        updateRank(placeIndex);
		

}

int Set::updateRank(int i) {

	for(int j=0;j<setAssoc;j++) {
		if(j!=i) {
			blk[j].incrLru();
		} else {
			blk[j].setLru(0);
		}
	}
}

bool Set::search(TAG tag,char oper) {

	
	for(int i=0;i<setAssoc;i++) {
		if (blk[i].checkTag(tag) ) {
			if(oper == 'w')
				blk[i].setDirty(true);
			updateRank(i);
			return true;

		}
	}

	//if not found in the entire set , its MISS
/*	i = getLRU();
	blk[i].setValid(true);
	blk[i].setTag(tag);
	updateRank(i);
*/
	return false;
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
	int numBlocksInStreamBuf;
	int numSets;
	int numOffsetBits;
	int numSetsBits;
	unsigned int setsMask;
	Set *s;
	StreamBuffer *stBuf;
	Cache *nextLevel;
	
public:
	Cache(int blockSize,int numAssoc,int totalSize,int numStBufs,int numBlocksInStBuf,Cache *c);
	int read(TAG addr); 
	int write(TAG addr);
	int fetchInstruction(char *fileName); // May not be necessary . read in the run function possible
	int getIndexOfSet(TAG address); // From the whole address mask and return the index of set alone
	int run(char *fileName); // Start the simulation
	void printMembers();
};

Cache::Cache(int blockSize,int numAssoc,int totalSize,int numStBufs,int numBlocksInStBuf,Cache *c) {
	blkSize = blockSize;
	assoc = numAssoc;
	cacheSize = totalSize;
	numStreamBuf = numStBufs;
	numSets = cacheSize/(blkSize * assoc);
	numBlocksInStreamBuf = numBlocksInStBuf;
	s = new Set[numSets];

	int i=0;
	for(i=0;i<numSets;i++)
		s[i].init(assoc);


	//initialise blk of each s;
	stBuf = new StreamBuffer[numStreamBuf];
	//initialise all blk of stBuf
	for(i=0;i<numStreamBuf;i++)
		stBuf[i].init(numBlocksInStreamBuf,c);

	nextLevel = c;
	
	numOffsetBits=0;
	int tmp = blkSize;
	while(tmp) {
		tmp = tmp >> 1;
		numOffsetBits++;
	}

	numOffsetBits--;

	tmp = numSets;
	numSetsBits = 0;
	while(tmp) {
		tmp = tmp >> 1;
		numSetsBits++;
	}

	numSetsBits--;
	setsMask = ( numSets-1 ) << numOffsetBits;	
}

int Cache::read(TAG addr) {

	unsigned int index = getIndexOfSet(addr);

	unsigned int blockAddr = addr >> (numOffsetBits+numSetsBits);
	if(s[index].search(blockAddr,'r')) {
		cout<<"Read : In set index : " << index << ". Hit for : " << hex << blockAddr << endl;
		s[index].print();
		cout<<endl;

	} else {
		cout<<"Read : In set index : " << index << ". Miss for : " << hex << blockAddr << endl;

		// check in stream Buffer
		// nextLevel->read(addr);
//The following is simultated for retrieval from next level of cache or memory
		s[index].place(blockAddr,'r');
		s[index].print();
		cout<<endl;

	}
		 
	
}

int Cache::write(TAG addr) {

        unsigned int index = getIndexOfSet(addr);

        unsigned int blockAddr = addr >> (numOffsetBits+numSetsBits);
        if(s[index].search(blockAddr,'w')) {
                cout<<"Write : In set index : " << index << ". Hit for : " << hex << blockAddr << endl;
		//s[index].setDirty(true);
                s[index].print();
                cout<<endl;

        } else {
                cout<<"Write : In set index : " << index << ". Miss for : " << hex << blockAddr << endl;

                // check in stream Buffer
                // nextLevel->read(addr);
//The following is simultated for retrieval from next level of cache or memory
                s[index].place(blockAddr,'w');
                s[index].print();
                cout<<endl;

        }

}

int Cache::fetchInstruction(char *fileName) {

}
	
int Cache::getIndexOfSet(TAG address) {

	unsigned int tmp = address;
	tmp = tmp & setsMask;
//	cout<<"After masking : " << tmp << endl;
	tmp = tmp >> numOffsetBits;

	return tmp;		
		
}

int Cache::run(char * fileName) {

/*
	TAG seqAddr[9] = { 0x40007a48, 0x40007a4c ,0x40007a58 ,0x40007a48,0x40007a68,0x40007a48 ,0x40007a58 ,0x40007a5c ,0x40007a64 };

	for(int i=0;i<9;i++ ) {
		read(seqAddr[i]);

	}
	for(int i=0;i<9;i++ ) {
		write(seqAddr[i]);

	}

*/

	string line;
	ifstream myfile;
	myfile.open( fileName );
	char oper;
	char address[9];
	memset(address,0,9);
/*
	string s("400382BA");
	istringstream iss(s);
	TAG addr;
	iss >> hex >> addr; 
	cout << "address : " << hex << addr << endl;*/

	TAG addr;
	//myfile.open( "gcc_trace.txt" , ios_base::binary);
	if (myfile.is_open()) {
		while ( getline (myfile,line) ) {
			//getline (myfile,line);
			oper = line.at(0);
			line.copy(address,line.size(),2);
//			cout << line << endl;
			
			istringstream iss(address);			
			iss >> hex >> addr;	
//			cout << oper << " " << hex << addr << endl;

			if (oper == 'r' )
				read(addr);
			else if (oper == 'w')
				write(addr);
			else {
				cout << "Error : Invalid operation encountered" << endl;
			}
			
		}
		myfile.close();
	} else 
		cout << "Unable to open file"; 


}

void Cache::printMembers() {

        cout<< "Block Size : " << blkSize << endl;
        cout << "Assoc : " << assoc << endl;
        cout << "Cache Size : " << cacheSize << endl;
        cout << "Number of stream buffer : " << numStreamBuf << endl;
        cout << "Number of sets : " << numSets << endl;
        cout << "Number of offset bits : " << numOffsetBits << endl ;
        cout << "Number of sets bits : " << numSetsBits << endl;
        cout << "Mask to get the set index : " << setsMask << endl;

}

int main() {

	Cache l1(16,2,1024,0,0,NULL) ;
	l1.printMembers();

	cout<< l1.getIndexOfSet(0x40007a64) << endl;
	l1.run("gcc_trace.txt");
	cout<<"chk" << endl;

}


