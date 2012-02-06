#include <iostream>
//#include<limits>
#include "limits.h"
#include <fstream>
#include <sstream>
#include "string.h"
#define DEBUG 0
#define VICTIM_NONE 0
#define VICTIM_REPLACE 1
#define VICTIM_WB 2
using namespace std;
typedef unsigned int TAG;

	long int numOpers;
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
	int incrLru(int assoc);
	TAG getTag();

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

TAG Block::getTag() {
	return tag;
}
int Block::incrLru(int assoc) {
	lruCounter = ( lruCounter + 1 )%assoc ;
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
	long int * prefReadCounter;
public:
//	Set(int assoc); since array of set have to be created default constructor can only be called
	Set();
	int init(int assoc );
	int getLRU(); // Returns the index of LRU block in the set
	int place(TAG tag,char oper, TAG & wbTag ); // places the tag at corresponding position;
	int updateRank(int i);//sets blk[i]'s rank to 0 and updates the rank of other blocks in the set
	bool search(TAG tag,char oper); //searches in the entire set and if tag is present returns true else return false
	void print();

	void printLru();

};

Set::Set() {

}

void Set::printLru() {
	for(int i=0;i<setAssoc;i++)
		cout << blk[i].getLru() << " " ;

	cout << endl;
}

void Set::print() {
	char dirtyFlag;
	for(int i=0;i<setAssoc;i++) {
		for(int j=0;j<setAssoc;j++) {
			if(blk[j].getLru() == i ) {	

				if(blk[j].isDirty())
					dirtyFlag = 'D';
				else
					dirtyFlag = ' ';
				cout << "\t" << hex << blk[j].getTag() << " " << dirtyFlag   ;
			}
		}
	}
	cout << endl;
}

int Set::init(int assoc) {
	setAssoc = assoc;
	blk = new Block[assoc];
	for(int i=0;i<setAssoc;i++) {
		blk[i].setLru(i);
//		blk[i-1].setLru(0);
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

int Set::place(TAG tag,char oper,TAG & wbTag ) {
	int status;
	int placeIndex=-1;
	for(int i=0;i<setAssoc;i++) {
		if(blk[i].isValid()==false) {

			placeIndex = i;
			break;
		}
	}

	if(placeIndex==-1) {
		placeIndex = getLRU();
		if(DEBUG)
			cout << "victim: tag " << hex << blk[placeIndex].getTag() << endl ;
		status = VICTIM_REPLACE; 
	} else {
		if(DEBUG)
			cout << "victim: none" << endl ;
        	blk[placeIndex].setValid(true);
		status = VICTIM_NONE;	
	}

	if (blk[placeIndex].isDirty() ) {
		// issue a write to next level 
		//nextLevel.write(tag);
//		cout << "Dirty block " << hex << blk[placeIndex].getTag() << " evicted" << endl;

		status = VICTIM_WB;
		wbTag = blk[placeIndex].getTag();	
		blk[placeIndex].setDirty(false);
		if(DEBUG)
			cout << ", dirty" << endl ;	
	} 


	if (oper == 'w') {
		blk[placeIndex].setDirty(true);
		if(DEBUG)
			cout << " set dirty" << endl;
	}

        blk[placeIndex].setTag(tag);
	if(DEBUG)
		cout << "Update LRU" << endl;
        updateRank(placeIndex);
		
	return status;
}

int Set::updateRank(int i) {

	int tmp = blk[i].getLru();
	for(int j=0;j<setAssoc;j++) {
		if(j==i) {
			blk[j].setLru(0);
		} else if( blk[j].getLru() < tmp ) {
			blk[j].incrLru( setAssoc );
		}
	}
}

bool Set::search(TAG tag,char oper) {

	
	for(int i=0;i<setAssoc;i++) {
		if (blk[i].checkTag(tag) ) {
			if(oper == 'w')
				blk[i].setDirty(true);
			updateRank(i);
		
			if(DEBUG) {
				cout << "Hit" << endl;			
				cout << "Update LRU" << endl;
			}
			return true;

		}
	}

	if(DEBUG)
		cout << "Miss" << endl;
	//if not found in the entire set , its MISS
/*	i = getLRU();
	blk[i].setValid(true);
	blk[i].setTag(tag);
	updateRank(i);
*/
	//Collect stats

/*	if(oper == 'r')
		numReadMisses++;
	else
		numWriteMisses++;
*/
	return false;
}

class Cache;

class StreamBuffer {
	Block *blk;
//	bool *dirty;
	int blkSize;
	int numBlks;
	int head;
	int tail;
	int lruCounter;
	int numOffsetBits;
	Cache *nextLevel;
	long int *prefReadCounter;
	
public:
//	StreamBuffer(int numBlocks) ; 
	StreamBuffer();
	int init(int numBlocks,int blockSize , Cache *c , long int * statNum );
	bool search(TAG blockNumber); // Reads the address tag
	int shift();//Shift the entire block up once , and set the new last block
	bool makeDirty(TAG blockNumber); // Forgot what to do
	int fetch(TAG blockNumber); // Fetches next numBlks blocks into buffer . (blockNumber +1 .... blockNumber + numBlks )
	TAG getHead();
	int setLru(int i);
	int getLru();
	int incrLru();
	void print();	

};

StreamBuffer::StreamBuffer() {


}

void StreamBuffer::print() {
	

	cout << "Head :" << head << "  Tail : " << tail << endl;
	if(head == -1)
		return;
	int i=head;
        do {
		cout << hex << blk[i].getTag() << ":" << blk[i].isDirty() << " " ;	

                i=(i+1)%numBlks;
        } while(i!=head);
	
	cout << endl;
}

int StreamBuffer::init(int numBlocks,int blockSize , Cache *c , long int *statNum) {
	blk = new Block[numBlocks];
	numBlks = numBlocks;
	blkSize = blockSize;
	nextLevel = c;

	prefReadCounter = statNum;

        int tmp = blkSize;
        while(tmp) {
                tmp = tmp >> 1;
                numOffsetBits++;
        }

        numOffsetBits--;

	head=-1;
	tail=-1;
}

int StreamBuffer::setLru(int i) {
        lruCounter = i;
}

int StreamBuffer::getLru() {
        return lruCounter;
}


int StreamBuffer::incrLru() {
        lruCounter++;
}

TAG StreamBuffer::getHead() {
	return blk[head].getTag();
}

bool StreamBuffer::search(TAG blockNumber) {
	if( blk[head].isDirty() == false && blk[head].getTag( ) == blockNumber) {
		cout << "ST HIT"<< endl;
		return true;
	}

	return false; 
}



bool StreamBuffer::makeDirty(TAG blockNumber) {

	if(head == -1)
		return false;

	int i=head;
	do {
		if(blk[i].getTag() == blockNumber) {
			cout << "Block : " << hex << blockNumber << " is set dirty in stream buffer" << endl;
			blk[i].setDirty(true);
			return true;
		}

		cout << "in make dirty" << endl;
		i=(i+1)%numBlks;
	} while(i!=head);

	return false;
}

class PrefetchUnit {
	int M; // number of blocks
	int N; // number of stream buffers
	int blkSize;
	Cache *nextLevel; 
	StreamBuffer *stBuf;	
public:
	PrefetchUnit(int mBlocks,int nBufs,int bSize,Cache *c, long int *statNum) ;
	int updateRank(int i);
	int getLRU();
	int prefetch(TAG blockAddr);
	bool search(TAG blockAddr);
	bool makeDirty(TAG blockAddr);
	void print();
};

PrefetchUnit::PrefetchUnit(int nBufs,int mBlocks,int bSize,Cache *c,long int *statNum) {
	M= mBlocks;
	N = nBufs;
	blkSize = bSize;
	nextLevel = c;

	stBuf = new StreamBuffer[N];
	for(int i=0;i<N;i++) {
		stBuf[i].init(M,blkSize,c,statNum);
		stBuf[i].setLru(i);
	}
	
	
}

void PrefetchUnit::print() {

	cout << "==================== Stream Buffer Contents ======================" << endl;
	for(int i=0;i<N;i++) {
		cout << i << " " ;
		stBuf[i].print();
	}
	cout << "==================================================================" << endl;
}


bool PrefetchUnit::makeDirty( TAG blockAddr ) {

	// eventhough makeDirty is called for all stream buffer , it will be present in any one and will be made dirty
	for(int i=0;i<N;i++) {
		if ( stBuf[i].makeDirty(blockAddr) ) {
			cout << "Block in stream buffer : " << i << " is made dirty" << endl;
			return true;
		}
	}	

	return false;
}


int PrefetchUnit::getLRU() {

        int max=INT_MIN;
        int index=-1;
        for(int i=0;i<N;i++) {
                if( stBuf[i].getLru() > max ) {
                        max = stBuf[i].getLru();
                        index = i;
                }
        }

        return index;
}

int PrefetchUnit::updateRank(int i) {

        for(int j=0;j<N;j++) {
                if(j!=i) {
                        stBuf[j].incrLru();
                } else {
                        stBuf[j].setLru(0);
                }
        }
}

int PrefetchUnit::prefetch(TAG blockAddr) {
	int index = getLRU();
	stBuf[index].fetch(blockAddr);
	updateRank(index);

}

bool PrefetchUnit::search(TAG blockAddr) {

	for(int i=0;i<N;i++) {
		if(stBuf[i].search(blockAddr) ) {
			cout << "Stream Buffer : "  << i << " shifted up" << endl;
			stBuf[i].shift();
			updateRank(i);
			return true;
		}
	}

	// if not found

	return false;

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
	string cacheName;
	unsigned int setsMask;
	Set *s;
	PrefetchUnit *preUnit;
	int stBufLru;
	Cache *nextLevel;
// Stats variables
/*
	long int numReads;
	long int numReadMisses;
	long int numWrites;
	long int numWriteMisses;
	long int numWriteBacks;
*/	
/*********** Stats *****************/

        long int numReads;
        long int numReadMisses;
	long int numReadHits;
        long int numWrites;
        long int numWriteMisses;
	long int numWriteHits;
        long int numWriteBacks;
	long int numReadFromPreUnit;
public:
	Cache(int blockSize,int numAssoc,int totalSize,int numStBufs,int numBlocksInStBuf,Cache *c,string name);
	int read(TAG addr); 
	int write(TAG addr);
	int fetchInstruction(char *fileName); // May not be necessary . read in the run function possible
	int getIndexOfSet(TAG address); // From the whole address mask and return the index of set alone
	int run(char *fileName); // Start the simulation
	int debugRun(); // For debugging with local values
	void printMembers();
	void printContent();
	void printStats();
};

Cache::Cache(int blockSize,int numAssoc,int totalSize,int numStBufs,int numBlocksInStBuf,Cache *c,string name) {
	blkSize = blockSize;
	assoc = numAssoc;
	cacheSize = totalSize;
	numStreamBuf = numStBufs;
	numSets = cacheSize/(blkSize * assoc);
	numBlocksInStreamBuf = numBlocksInStBuf;

        numReads=0;
        numReadMisses=0;
        numWrites=0;
        numWriteMisses=0;
        numWriteBacks=0;
	numReadFromPreUnit = 0;
	cacheName = name;
	s = new Set[numSets];

	int i=0;
	for(i=0;i<numSets;i++)
		s[i].init(assoc );



	nextLevel = c;
	
	if(numStBufs != 0 )
		preUnit = new PrefetchUnit(numStBufs,numBlocksInStBuf,blockSize,nextLevel,&numReadFromPreUnit);
	else
		preUnit = NULL;

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

void Cache::printContent() {
	for(int i=0;i<numSets;i++) {
		cout << "Set\t" << dec <<  i <<":";
//		s[i].sort();
		s[i].print();
	}

/*
	for(int i=0;i<numSets;i++) {
		cout << "Set :" << dec << i << " ";  
		s[i].printLru();
	}

*/
}

void Cache::printStats() {
        cout << "a. number of " <<  cacheName << " reads\t" << dec << numReads << endl;
        cout << "b. number of " << cacheName << " read misses \t" << dec << numReadMisses << endl;
        cout << "c. number of " << cacheName << " writes\t" << dec << numWrites << endl;
        cout << "d. number of " << cacheName << " write misses\t" << dec << numWriteMisses << endl;
        cout << "f. number of " << cacheName << " writebacks\t" << dec << numWriteBacks << endl;

	cout << "number of " << cacheName << " prefetches\t" << dec << numReadFromPreUnit << endl;
	cout << "number of " << cacheName << " reads that is not from prefetches\t" << dec << (numReads- numReadFromPreUnit) << endl;
}

int Cache::read(TAG addr) {

	int index = getIndexOfSet(addr);

	unsigned int tagAddr = addr >> (numOffsetBits+numSetsBits);
	unsigned int blockAddr = addr >> numOffsetBits;
	numReads++;
	numOpers++;

	if(DEBUG) {
		cout << "----------------------------------------"<<endl;
		cout << "# " << numOpers << " read " << hex << addr<< endl;
		cout << cacheName << " read : " << hex << (addr >> numOffsetBits) << "(tag " <<  tagAddr << ", index " << dec << index << ")" << endl ;
		
	}
	if(s[index].search(tagAddr,'r')) {
	//	cout<<"Read : In set index : " << index << ". Hit for : " << hex << blockAddr << endl;
	//	s[index].print();
	//	cout<<endl;
		numReadHits++;
	} else {
	//	cout<<"Read : In set index : " << index << ". Miss for : " << hex << blockAddr << endl;

		// check in stream Buffer
		// nextLevel->read(addr);
//The following is simultated for retrieval from next level of cache or memory
		numReadMisses++;



/***** When replacing if write back is done then make block in prefetch unit as dirty ****/
		TAG wbTag;
		if ( s[index].place(tagAddr,'r',wbTag ) == VICTIM_WB) {
		
			cout << "VICTIM_WB : tag :  " << hex << wbTag << endl;	
			unsigned int wbBlockNum;
			wbBlockNum = ( wbTag << numSetsBits ) | index ;
			cout << "VICTIM_WB : block :  " << hex << wbBlockNum << " in set : " << dec << index << endl;	
 
			numWriteBacks++;

			// issue a write to next level cache
			
			if ( preUnit!=NULL) {
				if ( preUnit->makeDirty(wbBlockNum) == false ) {
					cout << "Evicted block not found in stream buffer" << endl;
				}
			}

			TAG wbAddr;
			wbAddr = wbBlockNum << numOffsetBits;
			if(nextLevel!=NULL)
				nextLevel->write(wbAddr);

		}
		
/*****  Check in prefetch unit  *****/	
		if(preUnit != NULL) {
			if(preUnit->search(blockAddr)) {
				cout << "found in prefetch unit" << endl;
				
			} else {
				cout << "not found in prefetch unit.fetching from next level" << endl;
				// nextLevel->read(blockAddr); // l2 cache read that is not from prefetch unit
				nextLevel->read(addr);
				preUnit->prefetch(blockAddr);
			}
	
		} else {
			if(nextLevel!=NULL)
				nextLevel->read(addr);

//			if(nextLevel!=NULL)
//				nextLevel->read(addr);
		// get it from next level cache
		}
		
	//	s[index].print();
	//	cout<<endl;

		//preUnit->print();
	}
	
		 
	
}

int Cache::write(TAG addr) {

        unsigned int index = getIndexOfSet(addr);

        unsigned int tagAddr = addr >> (numOffsetBits+numSetsBits);
        unsigned int blockAddr = addr >> numOffsetBits;
	numWrites++;
	numOpers++;

	if(DEBUG) {
		cout << "----------------------------------------"<<endl;
		cout << "# " << numOpers << " write " << hex << addr<< endl;
		cout << cacheName << " write : " << hex << (addr >> numOffsetBits) << "(tag " <<  tagAddr << ", index " << dec << index << ")" << endl ;

	}
        if(s[index].search(tagAddr,'w')) {
         //       cout<<"Write : In set index : " << index << ". Hit for : " << hex << blockAddr << endl;
           //     s[index].print();
           //     cout<<endl;
		numWriteHits++;

        } else {
                //cout<<"Write : In set index : " << index << ". Miss for : " << hex << blockAddr << endl;

                // check in stream Buffer
                // nextLevel->read(addr);
//The following is simultated for retrieval from next level of cache or memory
		numWriteMisses++;

/***** When replacing if write back is done then make block in prefetch unit as dirty ****/

		TAG wbTag;
               // s[index].place(blockAddr,'w',wbTag);
               if ( s[index].place(tagAddr,'w',wbTag ) == VICTIM_WB) {

                        cout << "VICTIM_WB : tag :  " << hex << wbTag << endl;
                        unsigned int wbBlockNum;
                        wbBlockNum = ( wbTag << numSetsBits ) | index ;
                        cout << "VICTIM_WB : block :  " << hex << wbBlockNum << " in set : " << dec << index << endl;

			// issue a write to next level
			numWriteBacks++;

			if(preUnit != NULL ) {
				if ( preUnit->makeDirty(wbBlockNum) == false ) {
					cout << "Evicted block not found in stream buffer" << endl;
				}
			}

                        TAG wbAddr;
                        wbAddr = wbBlockNum << numOffsetBits;
                        if(nextLevel!=NULL)
                                nextLevel->write(wbAddr);


                }

/*****  Check in prefetch unit  *****/
                if(preUnit != NULL) {
                        if(preUnit->search(blockAddr)) {
                                cout << "found in prefetch unit" << endl;

                        } else {
                                cout << "not found in prefetch unit.fetching from next level" << endl;
                                // nextLevel->read(blockAddr); // l2 cache read that is not from prefetch unit
                                preUnit->prefetch(blockAddr);
                        }

                } else {

                // get it from next level cache
                }


/*****  Check in prefetch unit  *****/
                if(preUnit != NULL) {
                        if(preUnit->search(blockAddr)) {
                                cout << "found in prefetch unit" << endl;

                        } else {
                                cout << "not found in prefetch unit.fetching from next level" << endl;
                                // nextLevel->read(blockAddr); // l2 cache read that is not from prefetch unit
                                preUnit->prefetch(blockAddr);
                        }

                } else {
			if(nextLevel!=NULL)
				nextLevel->read(addr);

                // get it from next level cache
                }


		

        //      s[index].print();
        //      cout<<endl;

                //preUnit->print();
             
		//s[index].print();
                //cout<<endl;

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


int Cache::debugRun() {

	TAG seqAddr[9] = { 0x40007a48, 0x40007a4c ,0x40007a58 ,0x40007a48,0x40007a68,0x40007a48 ,0x40007a58 ,0x40007a5c ,0x40007a64 };

	char *oper = "wwwwwrrr";
	for(int i=0;i<9;i++ ) {
		if(oper[i]== 'r')
			read(seqAddr[i]);
		else
			write(seqAddr[i]);

	}


}

int Cache::run(char * fileName) {




	string line;
	ifstream myfile(fileName);
//	myfile.open( fileName );
	char oper;
	char address[9];
//	memset(address,0,9);
/*
	string s("400382BA");
	istringstream iss(s);
	TAG addr;
	iss >> hex >> addr; 
	cout << "address : " << hex << addr << endl;*/

	TAG addr;
//	myfile.open( "gcc_trace.txt" , ios_base::binary);
	if (myfile.is_open()) {
//		do {
		while ( getline (myfile,line) ) {
	
//			getline (myfile,line);
			oper = line.at(0);
			line.copy(address,(line.size()-2),2);
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


			memset(address,0,9);
			
		} //while ( myfile.good() );
		myfile.close();
	} else 
		cout << "Unable to open file"; 


//	cout << line << endl;
	cout << "END of Run" << endl;
}

void Cache::printMembers() {

/*        cout<< "Block Size : " << blkSize << endl;
        cout << "Assoc : " << assoc << endl;
        cout << "Cache Size : " << cacheSize << endl;
        cout << "Number of stream buffer : " << numStreamBuf << endl;
        cout << "Number of sets : " << numSets << endl;
        cout << "Number of offset bits : " << numOffsetBits << endl ;
        cout << "Number of sets bits : " << numSetsBits << endl;
        cout << "Mask to get the set index : " << setsMask << endl;
*/

	cout << "===== Simulator configuration =====" << endl;
	cout << "BLOCKSIZE:\t" << blkSize << endl;
	cout << "L1_SIZE:\t" << cacheSize << endl;
	cout << "L1_ASSOC:\t" << assoc << endl;
	cout << "L1_PREF_N:             0" << endl;
	cout << "L1_PREF_M:             0" << endl;
	cout << "L2_SIZE:               0" << endl;
	cout << "L2_ASSOC:              0" << endl;
	cout << "L2_PREF_N:             0" << endl;
	cout << "L2_PREF_M:             0" << endl;
	cout <<  "trace_file:\t" << "gcc_trace.txt" << endl;
}

int StreamBuffer::shift() {
	head=(head+1)%numBlks;

	TAG nextBlock = blk[tail].getTag() + 1 ;
	tail = (tail+1)%numBlks;

	if(nextLevel != NULL) {
		nextLevel->read(nextBlock << numOffsetBits);
		*prefReadCounter = (*prefReadCounter) + 1;
	}
	//Simulating fetching of next block
	blk[tail].setTag(nextBlock);
	
}
int StreamBuffer::fetch(TAG blockNumber) {
	int bNum;
	head=0;
	for(tail=0; tail<numBlks; tail++) {
		bNum = blockNumber + (tail +1);
		if(nextLevel != NULL) {
			*prefReadCounter = (*prefReadCounter) + 1;
			nextLevel->read(  (blockNumber << numOffsetBits)  );
		}
	
		blk[tail].setTag( bNum );
		blk[tail].setDirty(false);
	}
	tail--;

}
int main() {

	Cache l2(16,4,8192,0,0,NULL,"L2");

	Cache l1(16,1,1024,0,0,&l2,"L1") ;
	//Cache l1(16,2,1024,0,0,NULL,"L1") ;

	//For debugging 
//	Cache l1(4,2,32,2,8,NULL,"L1") ;

	//End of debugging

	l1.printMembers();
	l2.printMembers();

//	cout<< l1.getIndexOfSet(0x40007a64) << endl;
	l1.run("gcc_trace.txt");

//	l1.debugRun();
	l1.printContent();
	l2.printContent();
	l1.printStats();
	l2.printStats();
//	cout<<"chk" << endl;

	//sleep(1);
}


