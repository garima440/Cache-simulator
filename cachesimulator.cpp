/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size in bytes)  t=32-s-b
32 bit address (MSB -> LSB): TAG || SET || OFFSET

Tag Bits   : the tag field along with the valid bit is used to determine whether the block in the cache is valid or not.
Index Bits : the set index field is used to determine which set in the cache the block is stored in.
Offset Bits: the offset field is used to determine which byte in the block is being accessed.
*/


// dont worry everything is intact
#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;

//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss
#define NOWRITEMEM 5 // no write to memory
#define WRITEMEM 6   // write to memory

struct config
{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
// You need to define your cache class here, or design your own data structure for L1 and L2 cache

/*
A single cache block:
    - valid bit (is the data in the block valid?)
    - dirty bit (has the data in the block been modified by means of a write?)
    - tag (the tag bits of the address)
    - data (the actual data stored in the block, in our case, we don't need to store the data)
*/
struct CacheBlock
{
    // we don't actually need to allocate space for data, because we only need to simulate the cache action
    // or else it would have looked something like this: vector<number of bytes> Data; 
    bitset <1> valid_bit;
    bitset <1> dirty_bit;
    int blockTag;
    string address;


};

struct State
{
    int L1state;
    int L2state;
    int memstate;
    
    State()
    {
        L1state = NA;
        L2state = NA;
        memstate = NOWRITEMEM;
    }
};

/*
A CacheSet:
    - a vector of CacheBlocks
    - a counter to keep track of which block to evict next
*/
// struct cacheSet
// {
//     // tips: 
//     // Associativity: eg. resize to 4-ways set associative cache  
//     int blockToEvict = 0; 
// };


// You can design your own data structure for L1 and L2 cache; just an example here



/*********************************** ↓↓↓ CACHE CLASS HAS BEGUN!! ↓↓↓ ******************************************/
/*********************************** ↓↓↓ CACHE CLASS HAS BEGUN!! ↓↓↓ ******************************************/
/*********************************** ↓↓↓ CACHE CLASS HAS BEGUN!! ↓↓↓ ******************************************/
/*********************************** ↓↓↓ CACHE CLASS HAS BEGUN!! ↓↓↓ ******************************************/

class cache
{
    // some cache configuration parameters.
    
    // cache L1 or L2
    // CacheSet * set;
    string addr;
    vector <vector<CacheBlock> > cacheSystem1;
    vector <vector<CacheBlock> > cacheSystem2;
    
    vector <int> blockCounterL1;
    vector <int> blockCounterL2;

    public:
    
    cache(int L1BSize, int L1SSize, int L1CSize, int L2BSize, int L2SSize, int L2CSize)
    {
        // initialize the cache according to cache parameters
        L1BlockSize = L1BSize;
        L1SetSize = L1SSize;
        L1CacheSize = L1CSize;
    
        L2BlockSize = L2BSize;
        L2SetSize = L2SSize;
        L2CacheSize = L2CSize;
    }

    void createL1()
    {
        if (L1SetSize != 0)
        {
            numOfSetsL1 = (L1CacheSize * 1024) / (L1BlockSize * L1SetSize);   
            numOfColL1 = L1SetSize;
        }
        else
        {
            numOfSetsL1 = 1;
            numOfColL1 = (L1CacheSize * 1024) / (L1BlockSize);
        }
        int rows = numOfSetsL1;
    
        blockCounterL1.resize(numOfSetsL1, 0);
        cacheSystem1.resize(rows, vector <CacheBlock> (numOfColL1));

        for (int i = 0; i < numOfSetsL1; i++)
        {
            for (int j = 0; j < numOfColL1; j++)
            {
                cacheSystem1[i][j].dirty_bit = 0;
                cacheSystem1[i][j].valid_bit = 0;
                cacheSystem1[i][j].blockTag = 0;
                cacheSystem1[i][j].address = "";
            }
        }
    }

    void createL2()
    {
        if (L2SetSize != 0)
        {
            numOfSetsL2 = (L2CacheSize * 1024) / (L2BlockSize * L2SetSize);
            numOfColL2 = L2SetSize;
        }
        else
        {
            numOfSetsL2 = 1;
            numOfColL2 = (L2CacheSize * 1024) / (L2BlockSize);
        }
    

        blockCounterL2.resize(numOfSetsL2, 0);
        cacheSystem2.resize(numOfSetsL2, vector <CacheBlock> (numOfColL2));


        for (int i = 0; i < numOfSetsL2; i++)
        {
            for (int j = 0; j < numOfColL2; j++)
            {
                cacheSystem2[i][j].dirty_bit = 0;
                cacheSystem2[i][j].valid_bit = 0;
                cacheSystem2[i][j].blockTag = 0;
                cacheSystem2[i][j].address = "";
            }
        }
    }

    bool L1_is_full(vector <vector<CacheBlock> > v, int setIndex)
    {
        bool val = true;
        for (int j = 0; j < numOfColL1; j++)
        {
            if (v[setIndex][j].valid_bit != 1)
            {
                val = false;
                break;
            }
        }
        return val;
    }

    bool L2_is_full(vector <vector<CacheBlock> > v, int setIndex)
    {
        bool val = true;
        for (int j = 0; j < numOfColL2; j++)
        {
            if (v[setIndex][j].valid_bit != 1) // if empty block is found
            {
                val = false; // return that L2 is not full at this set index
                break;
            }
        }
        return val;
    }

    int find_empty_block(string level, int setIndex) // level: L1 or L2
    {
        int empty_block;
        if (level == "L1" || level == "l1")
        {
            for (int j = 0; j < numOfColL1; j++)
            {
                if (cacheSystem1[setIndex][j].valid_bit == 0)
                {
                    empty_block = j;
                    break;
                }
            }
        }

        else if (level == "L2" || level == "l2")
        {
            for (int k = 0; k < numOfColL2; k++)
            {
                if (cacheSystem2[setIndex][k].valid_bit == 0)
                {
                    empty_block = k;
                    break;
                }
            }
        }
        return empty_block;
    }

    int parse_address(string addr, int cacheLevel, char type) // char type: 'I' for index bits; 'T' for tag bits
    {
        int return_val;
        if (cacheLevel == 1)
        {
            // Parse the address to get tag and index bits in case of L1 //
            int L1index_bits = log2(numOfSetsL1);    // number of index bits
            int L1offset_bits = log2(L1BlockSize);   // number of offset bits
            int L1tag_bits = 32 - L1index_bits - L1offset_bits; // number of offset bits

            int L1addr_tag = stoi(addr.substr(0, L1tag_bits), nullptr, 2);
            int L1addr_index;

            if (L1index_bits != 0)
            {
                L1addr_index = stoi(addr.substr(L1tag_bits, L1index_bits), nullptr, 2);
            }
            else
            {
                L1addr_index = 0;
            }

            int L1addr_offset = stoi(addr.substr(L1index_bits, L1offset_bits), nullptr, 2);

            if (type == 'i' || type == 'I')
            {
                return_val = L1addr_index;
            }
            else if (type == 'T' || type == 't')
            {
                return_val = L1addr_tag;
            }
            else
            {
                cout << "Invaid char argument.";
            }
        }

        if (cacheLevel == 2)
        {
            // Now parse the address to get tag and index bits in case of L2
            int L2index_bits = log2(numOfSetsL2);    // number of index bits
            int L2offset_bits = log2(L2BlockSize);   // number of offset bits
            int L2tag_bits = 32 - L2index_bits - L2offset_bits;  // number of offset bits

            int L2addr_tag = stoi(addr.substr(0, L2tag_bits), nullptr, 2);
            int L2addr_index;

            if (L2index_bits != 0)
            {
                L2addr_index = stoi(addr.substr(L2tag_bits, L2index_bits), nullptr, 2);
            }
            else
            {
                L2addr_index = 0; 
            }
            
            int L2addr_offset = stoi(addr.substr(L2index_bits, L2offset_bits), nullptr, 2);

            if (type == 'i' || type == 'I')
            {
                return_val = L2addr_index;
            }
            else if (type == 'T' || type == 't')
            {
                return_val = L2addr_tag;
            }
            else
            {
                cout << "Invaid char argument.";
            }
        }
        return return_val;
    }

    State write_access(string addr)
    {
        /*
        step 1: select the set in our L1 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> WriteHit!
                                                    -> Dirty Bit High
        step 3: Otherwise? -> WriteMiss!

        return WH or WM
        */
        State state;
      
        int L1index = parse_address(addr, 1, 'I');
        int L1tag = parse_address(addr, 1, 'T');

      
        int L2index = parse_address(addr, 2, 'I');
        int L2tag = parse_address(addr, 2, 'T');

        bool write_hitL1 = false;
        bool write_hitL2 = false;

        for (int i = 0; i < numOfColL1; i++)
        {
            if ((cacheSystem1[L1index][i].valid_bit == 1) && (L1tag == cacheSystem1[L1index][i].blockTag))
            {
                cacheSystem1[L1index][i].dirty_bit = 1;
                cacheSystem1[L1index][i].address = addr;
                cacheSystem1[L1index][i].blockTag = L1tag;
                state.L1state = WH;
                write_hitL1 = true;
                break;
            }
        }
        // if Write Miss in L1
        if (write_hitL1 != true)
        {
            // set L1 state to write miss and check in L2
            state.L1state = WM;
            for (int j = 0; j < numOfColL2; j++)
            {
                if ((cacheSystem2[L2index][j].valid_bit == 1) && (L2tag == cacheSystem2[L2index][j].blockTag))
                {   
                    cacheSystem2[L2index][j].dirty_bit = 1;
                    cacheSystem2[L2index][j].address = addr;
                    cacheSystem2[L2index][j].blockTag = L2tag;
                    state.L2state = WH; // write hit
                    write_hitL2 = true;
                    break;
                }
            }
            if (write_hitL2 != true)
            { 
                state.L2state = WM;
                state.memstate = WRITEMEM;
            }
        }
        return state;
    }

    State read_access(string addr)
    {
        /*
        step 1: select the set in our L1 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> ReadHit!
        step 3: Otherwise? -> ReadMiss!

        return RH or RM
        */

        int L1index = parse_address(addr, 1, 'I');
        int L1tag = parse_address(addr, 1, 'T');

        int L2index = parse_address(addr, 2, 'I');
        int L2tag = parse_address(addr, 2, 'T');
        State state;

        bool read_hit_L1 = false;
        bool read_hit_L2 = false;

        int read_hit_block;
           
        for (int i = 0; i < numOfColL1; i++)
        {
            // check in L1 cache 
            if ((cacheSystem1[L1index][i].valid_bit == 1) && (L1tag == cacheSystem1[L1index][i].blockTag))
            {
                state.L1state = RH; // read hit!
                read_hit_L1 = true;
                break;
            }
            // read miss in L1 :(
        }
            
        // if Read Miss in L1
        if (read_hit_L1 != true)
        {
            state.L1state = RM;
            for (int j = 0; j < numOfColL2; j++)
            {
                // check in l2
                if ((cacheSystem2[L2index][j].valid_bit == 1) && (L2tag == cacheSystem2[L2index][j].blockTag))
                {
                    // Read Hit in L2
                    state.L2state = RH;
                    read_hit_L2 = true;
                    read_hit_block = j;
                    break;
                }
            }
            if (read_hit_L2 == true)
            {
                // if L1 set is not full
                if (L1_is_full(cacheSystem1, parse_address(cacheSystem2[L2index][read_hit_block].address, 1, 'I')) == false)
                { 
                    // place the block in L1 at an empty place
                    int empty_way = find_empty_block("L1", parse_address(cacheSystem2[L2index][read_hit_block].address, 1, 'I'));
                    cacheSystem1[L1index][empty_way].address = cacheSystem2[L2index][read_hit_block].address;
                    cacheSystem1[L1index][empty_way].blockTag = parse_address(cacheSystem2[L2index][read_hit_block].address, 1, 'T');
                    cacheSystem1[L1index][empty_way].valid_bit = cacheSystem2[L2index][read_hit_block].valid_bit;
                    cacheSystem1[L1index][empty_way].dirty_bit = cacheSystem2[L2index][read_hit_block].dirty_bit;

                    cacheSystem2[L2index][read_hit_block].valid_bit = 0;
                }
                // If L1 is full -> evict
                else
                {
                    // find which block to evict from which set in L1
                    //string to_be_evicted_addr1 = cacheSystem1[L2index][j].address;
                    int set_index  = parse_address(cacheSystem2[L2index][read_hit_block].address, 1, 'I');
                    int way_to_evict;
                    if (blockCounterL1[set_index] < numOfColL1)
                    {
                        way_to_evict = blockCounterL1[set_index];
                        blockCounterL1[set_index]++;
                    }
                    else
                    {
                        blockCounterL1[set_index] = 0;
                        way_to_evict = blockCounterL1[set_index];
                        blockCounterL1[set_index]++;
                    }

                    // copy the contents of evicted L1 block into a temporary block to be transported to L2 cache
                    CacheBlock temp;
                    temp.address = cacheSystem1[set_index][way_to_evict].address; 
                    temp.blockTag = cacheSystem1[set_index][way_to_evict].blockTag;
                    temp.dirty_bit = cacheSystem1[set_index][way_to_evict].dirty_bit;
                    temp.valid_bit = cacheSystem1[set_index][way_to_evict].valid_bit;
                    
                    // place the L2 block in L1
                    cacheSystem1[set_index][way_to_evict].address = cacheSystem2[L2index][read_hit_block].address;
                    cacheSystem1[set_index][way_to_evict].blockTag = parse_address(cacheSystem2[L2index][read_hit_block].address, 1, 'T');
                    cacheSystem1[set_index][way_to_evict].dirty_bit = cacheSystem2[L2index][read_hit_block].dirty_bit;
                    cacheSystem1[set_index][way_to_evict].valid_bit = cacheSystem2[L2index][read_hit_block].valid_bit;

                    cacheSystem2[L2index][read_hit_block].valid_bit = 0;

                    // place the evicted block from L1 (temp block) to L2
                    // first check if L2 is full at that particular set

                    // if it is not full, look for an empty block
                    if (L2_is_full(cacheSystem2, parse_address(temp.address, 2, 'I')) == false) 
                    {
                        int temp_index = parse_address(temp.address, 2, 'I');
                        int empty_way = find_empty_block("L2", temp_index);
                        cacheSystem2[temp_index][empty_way].address = temp.address;
                        cacheSystem2[temp_index][empty_way].blockTag = parse_address(temp.address, 2, 'T');
                        cacheSystem2[temp_index][empty_way].dirty_bit = temp.dirty_bit;
                        cacheSystem2[temp_index][empty_way].valid_bit = temp.valid_bit;
                    }
                    // if L2 is full at that particular set
                    else
                    {
                        // find which block to evict in which set in L2
                        int way_to_evict;
                        int temp_index = parse_address(temp.address, 2, 'I');
                        if (blockCounterL2[temp_index] < numOfColL2)
                        {
                            way_to_evict = blockCounterL2[temp_index];
                            blockCounterL2[temp_index]++;
                        }
                        else
                        {
                            blockCounterL2[temp_index] = 0;
                            way_to_evict = blockCounterL2[temp_index];
                            blockCounterL2[temp_index]++;
                        }

                        if (cacheSystem2[temp_index][way_to_evict].dirty_bit == 1)
                        {
                            state.memstate = WRITEMEM;
                            cacheSystem2[temp_index][way_to_evict].address = temp.address;
                            cacheSystem2[temp_index][way_to_evict].blockTag = parse_address(temp.address, 2, 'T');
                            cacheSystem2[temp_index][way_to_evict].dirty_bit = temp.dirty_bit;
                            cacheSystem2[temp_index][way_to_evict].valid_bit = temp.valid_bit;
                        }
                        else
                        {
                            cacheSystem2[temp_index][way_to_evict].address = temp.address;
                            cacheSystem2[temp_index][way_to_evict].blockTag = parse_address(temp.address, 2, 'T');
                            cacheSystem2[temp_index][way_to_evict].dirty_bit = temp.dirty_bit;
                            cacheSystem2[temp_index][way_to_evict].valid_bit = temp.valid_bit;
                        }
                    }
                }
            }
            else 
            {
                state.L2state = RM;
                // read request from main memory
                if (L1_is_full(cacheSystem1, L1index) == false)
                {
                    int empty_block = find_empty_block("L1", L1index);
                    cacheSystem1[L1index][empty_block].address = addr;
                    cacheSystem1[L1index][empty_block].blockTag = L1tag;
                    cacheSystem1[L1index][empty_block].dirty_bit = 0;
                    cacheSystem1[L1index][empty_block].valid_bit = 1;
                }
                else
                {
                    int block_to_evict;
                    if (blockCounterL1[L1index] < numOfColL1)
                    {
                        block_to_evict = blockCounterL1[L1index];
                        blockCounterL1[L1index]++;
                    }
                    else
                    {
                        blockCounterL1[L1index] = 0;
                        block_to_evict = blockCounterL1[L1index];
                        blockCounterL1[L1index]++;
                    }

                    // copy the contents of the evicted L1 block
                    CacheBlock temp;
                    temp.address = cacheSystem1[L1index][block_to_evict].address; 
                    temp.blockTag = cacheSystem1[L1index][block_to_evict].blockTag;
                    temp.dirty_bit = cacheSystem1[L1index][block_to_evict].dirty_bit;
                    temp.valid_bit = cacheSystem1[L1index][block_to_evict].valid_bit;

                    // place the block in L1
                    cacheSystem1[L1index][block_to_evict].address = addr;
                    cacheSystem1[L1index][block_to_evict].blockTag = L1tag;
                    cacheSystem1[L1index][block_to_evict].dirty_bit = 0;
                    cacheSystem1[L1index][block_to_evict].valid_bit = 1;

                    // place the evicted block from L1 (temp block) to L2
                    // first check if L2 is full
                    if (L2_is_full(cacheSystem2, parse_address(temp.address, 2, 'I')) == false) // if it is not full, look for empty block
                    {
                        int temp_index = parse_address(temp.address, 2, 'I');
                        int empty_way = find_empty_block("L2", temp_index);
                        cacheSystem2[temp_index][empty_way].address = temp.address;
                        cacheSystem2[temp_index][empty_way].blockTag = parse_address(temp.address, 2, 'T');
                        cacheSystem2[temp_index][empty_way].dirty_bit = temp.dirty_bit;
                        cacheSystem2[temp_index][empty_way].valid_bit = temp.valid_bit;
                    }
                    // if L2 is full 
                    else
                    {
                        // find which block to evict in which set in L2
                        int temp_index = parse_address(temp.address, 2, 'I');
                        int way_to_evict;
                        
                        if (blockCounterL2[temp_index] < numOfColL2)
                        {
                            way_to_evict = blockCounterL2[temp_index];
                            blockCounterL2[temp_index]++;
                        }
                        else
                        {
                            blockCounterL2[temp_index] = 0;
                            way_to_evict = blockCounterL2[temp_index];
                            blockCounterL2[temp_index]++;
                        }
                
                        if (cacheSystem2[temp_index][way_to_evict].dirty_bit == 1)
                        {
                            cacheSystem2[temp_index][way_to_evict].address = temp.address;
                            cacheSystem2[temp_index][way_to_evict].blockTag = parse_address(temp.address, 2, 'T');
                            cacheSystem2[temp_index][way_to_evict].dirty_bit = temp.dirty_bit;
                            cacheSystem2[temp_index][way_to_evict].valid_bit = temp.valid_bit;   
                            state.memstate = WRITEMEM;
                        }
                        else
                        {
                            cacheSystem2[temp_index][way_to_evict].address = temp.address;
                            cacheSystem2[temp_index][way_to_evict].blockTag = parse_address(temp.address, 2, 'T');
                            cacheSystem2[temp_index][way_to_evict].dirty_bit = temp.dirty_bit;
                            cacheSystem2[temp_index][way_to_evict].valid_bit = temp.valid_bit;
                            state.memstate = NOWRITEMEM;
                        }
                    }
                }
            }
        }
        return state;
    }
    

    // some cache configuration parameters
    public:
        int L1CacheSize;
        int L1SetSize;
        int L1BlockSize;
        int L2CacheSize;
        int L2SetSize;
        int L2BlockSize;
        int numOfSetsL1;
        int numOfSetsL2;
        int numOfColL1;
        int numOfColL2;

        int L1state, L2state, memstate;
};

/*********************************** ↑↑↑ CACHE CLASS HAS ENDED ↑↑↑ ******************************************/
/*********************************** ↑↑↑ CACHE CLASS HAS ENDED ↑↑↑ ******************************************/
/*********************************** ↑↑↑ CACHE CLASS HAS ENDED ↑↑↑ ******************************************/
/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/





int main(int argc, char *argv[])
{

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while (!cache_params.eof())                   // read config file
    {
        cache_params >> dummyLine;                // L1:
        cache_params >> cacheconfig.L1blocksize;  // L1 Block size
        cache_params >> cacheconfig.L1setsize;    // L1 Associativity
        cache_params >> cacheconfig.L1size;       // L1 Cache Size
        cache_params >> dummyLine;                // L2:
        cache_params >> cacheconfig.L2blocksize;  // L2 Block size
        cache_params >> cacheconfig.L2setsize;    // L2 Associativity
        cache_params >> cacheconfig.L2size;       // L2 Cache Size
    }
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    string line;
    string accesstype;     // the Read/Write access type from the memory trace;
    string xaddr;          // the address from the memory trace store in hex;
    unsigned int addr;     // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;




/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like
    if (cacheconfig.L1blocksize!=cacheconfig.L2blocksize){
        printf("please test with the same block size\n");
        return 1;
    }
    
    // create new cache
    cache newCache(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size, cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);

    // create the Levels inside the cache
    newCache.createL1();
    newCache.createL2();

    State newState;

    int L1AcceState = NA; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = NA; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
    int MemAcceState = NOWRITEMEM; // Main Memory access state variable, can be either NA or WH;

    if (traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        { // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)){
                break;
            }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32>(addr);

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0)  // a Read request
            {
                // Implement by you:
                //   read access to the L1 Cache,
                //   and then L2 (if required),
                //   update the access state variable;
                //   return: L1AcceState L2AcceState MemAcceState
                newState = newCache.read_access(accessaddr.to_string());
                L1AcceState = newState.L1state;
                L2AcceState = newState.L2state;
                MemAcceState = newState.memstate;
        
                // For example:
                // L1AcceState = cache.readL1(addr); // read L1
                // if(L1AcceState == RM){
                //     L2AcceState, MemAcceState = cache.readL2(addr); // if L1 read miss, read L2
                // }
                // else{ ... }
            }
            else
            {   // a Write request
                // Implement by you:
                //   write access to the L1 Cache, or L2 / main MEM,
                //   update the access state variable;
                //   return: L1AcceState L2AcceState

                // For example:
                // L1AcceState = cache.writeL1(addr);
                // if (L1AcceState == WM){
                //     L2AcceState, MemAcceState = cache.writeL2(addr);
                // }
                // else if(){...}
                newState = newCache.write_access(accessaddr.to_string());
                L1AcceState = newState.L1state;
                L2AcceState = newState.L2state;
                MemAcceState = newState.memstate;
            }
            
/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/




            // Grading: don't change the code below.
            // We will print your access state of each cycle to see if your simulator gives the same result as ours.
            tracesout << L1AcceState << " " << L2AcceState << " " << MemAcceState << endl; // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
