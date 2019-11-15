// FAKE
#include "cache.h"

uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    // baseline LRU replacement policy for other caches
    return lru_victim(cpu, instr_id, set, current_set, ip, full_addr, type);
}

void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    if (type == WRITEBACK) {
        if (hit) // wrietback hit does not update LRU state
            return;
    }

    return lru_update(set, way);
}

uint32_t CACHE::lru_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    uint32_t way = 0;

    // fill invalid line in current set
    for (way=0; way<NUM_WAY; way++)
    {
        if (block[set][way].valid == false)
        {

            DP ( if (warmup_complete[cpu]) {
            cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " invalid set: " << set << " way: " << way;
            cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
            cout << dec << " lru: " << block[set][way].lru << endl; });

            break;
        }
    }

    // if( cache_type == 6) cerr << "In lru_victim ways " << way <<" Num ways " << NUM_WAY << endl;

    // check for invalid line in fellow sets
    if (way == NUM_WAY && cache_type >= 6)
    {
        uint32_t fset_start = set / FELLOW_GRP_SIZE;
        uint32_t fset, fway;

        for( fset = fset_start * FELLOW_GRP_SIZE; fset < (fset_start + 1) * FELLOW_GRP_SIZE; ++fset)
        {
            if( fset == set) continue;

            for( uint32_t fway = NUM_WAY - NUM_RES; fway < NUM_WAY; ++fway)
            {
                if (block[fset][fway].valid == false)
                {
                    DP ( if (warmup_complete[cpu]) {
                    cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << "orig set"<< set << " invalid fset: " << fset << " way: " << fway;
                    cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[fset][fway].address << " data: " << block[fset][fway].data;
                    cout << dec << " lru: " << block[fset][fway].lru << endl; });

                    break;
                }
            }

            if( fway < NUM_WAY) break;
        }

        if( fset < (fset_start + 1) * FELLOW_GRP_SIZE && fway < NUM_WAY)
        {
            //swap with current LRU of the set
            BLOCK temp = BLOCK();

            // find lru
            uint32_t i;
            for (i = NUM_WAY - NUM_RES; i<NUM_WAY; i++)
            {
                if (block[set][i].lru == NUM_WAY - 1)
                    break;
            }

            temp = block[set][i];
            block[set][i] = block[fset][fway];
            block[fset][fway] = temp;

            uint32_t templru;
            templru = block[set][i].lru;
            block[set][i].lru = block[fset][fway].lru;
            block[fset][fway].lru = templru;

            way = i;
        }
        else way = NUM_WAY;
    }

    // LRU victim
    uint32_t max = 0, ind = NUM_WAY;
    if (way == NUM_WAY)
    {

        for (way= 0; way < NUM_WAY; way++)
        {
            if( max < block[set][way].lru)
            {
                max = block[set][way].lru;
                ind = way;
            }
            if (block[set][way].lru >= NUM_WAY-1 )
            {
                ind = way;

                DP ( if (warmup_complete[cpu]) {
                cout << "[" << NAME << "] " << __func__ << " instr_id: " << instr_id << " replace set: " << set << " way: " << way;
                cout << hex << " address: " << (full_addr>>LOG2_BLOCK_SIZE) << " victim address: " << block[set][way].address << " data: " << block[set][way].data;
                cout << dec << " lru: " << block[set][way].lru << endl; });

                break;
            }
        }

        way = ind;
    }


    static int j = 0;

    if( way >= NUM_WAY)
    {
        // cerr << "Address " << block[set][ntlru].address << " Full_Addr" << block[set][ntlru].full_addr << endl;
        // fprintf(stderr, "A: %x, FA: %x\n", block[set][ntlru].address, block[set][ntlru].full_addr);
        cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
        for(int k=0; k<NUM_WAY; ++k) cerr << "way:" << k << "LRU:" << block[set][k].lru << "VAlid" << (block[set][k].valid ? "T":"F") << endl;
        cerr << "WAY "<<way<<"\n";
        // assert(0);
    }

    // if (way == NUM_WAY && j < 10000)
    // {
    //     ++j;
    //     cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
    //     for(int k=0; k<NUM_WAY; ++k) cerr << "way:" << k << "LRU:" << block[set][k].lru << "VAlid" << (block[set][k].valid ? "T":"F") << endl;
    //     cerr << "\n";
    //     //assert(0);
    // }

    return way;
}

void CACHE::lru_update(uint32_t set, uint32_t way)
{
    uint32_t ntlru=0;
    int lock;
    // if( set == 689 && c)//if( set == 689 && cache_type < IS_L2C)
    // {
    //     // cerr << "Address " << block[set][ntlru].address << " Full_Addr" << block[set][ntlru].full_addr << endl;
    //     // fprintf(stderr, "A: %x, FA: %x\n", block[set][ntlru].address, block[set][ntlru].full_addr);
    //     cerr << "[" << NAME << "] " << __func__ << " entered! set: " << set <<" WAy "<< way << endl;
    //     for(int k=0; k<NUM_WAY; ++k) cerr << "way:" << k << "LRU:" << block[set][k].lru << "VAlid" << (block[set][k].valid ? "T":"F") << endl;
    //     // cerr << "\n NTLRU "<< ntlru<< "WAY "<<way<<"\n";
    //     // if(ntlru >= NUM_WAY - NUM_RES) assert(0);
    // }


    if( way < NUM_WAY - NUM_RES || cache_type < 6)
    {
        // update lru replacement state
        for (uint32_t i=0; i<NUM_WAY; i++)
        {
            if (block[set][i].lru < block[set][way].lru)    block[set][i].lru++;
        }
        block[set][way].lru = 0; // promote to the MRU position
    }
    else
    {
        // block is in RT: find lru of NT and swap with curr
        ntlru = NUM_WAY - NUM_RES; lock = 0;
        for (uint32_t i=0; i<NUM_WAY ; i++)
        {
            if (i < NUM_WAY - NUM_RES && block[set][i].lru > lock)
            {
                lock = block[set][i].lru;
                ntlru = i;
            }
            if (i != way && block[set][i].lru <= block[set][way].lru)    block[set][i].lru++;
        }

        if( ntlru >= NUM_WAY - NUM_RES)
        {
            cerr << "Address " << block[set][ntlru].address << " Full_Addr" << block[set][ntlru].full_addr << endl;
            fprintf(stderr, "A: %x, FA: %x\n", block[set][ntlru].address, block[set][ntlru].full_addr);
            cerr << "[" << NAME << "] " << __func__ << " no victim! set: " << set << endl;
            for(int k=0; k<NUM_WAY; ++k) cerr << "way:" << k << "LRU:" << block[set][k].lru << "VAlid" << (block[set][k].valid ? "T":"F") << endl;
            cerr << "\n NTLRU "<< ntlru<< "WAY "<<way<<"\n";
            assert(0);
        }


        BLOCK temp = BLOCK();

        temp = block[set][ntlru];
        block[set][ntlru] = block[set][way];
        block[set][way] = temp;

        block[set][ntlru].lru = 0;
        // block[set][way].lru = block[set][ntlru].lru;
        // block[set][ntlru].lru = 0;
    }
}

void CACHE::replacement_final_stats()
{

}

#ifdef NO_CRC2_COMPILE
void InitReplacementState()
{

}

uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    return 0;
}

void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{

}

void PrintStats_Heartbeat()
{

}

void PrintStats()
{

}
#endif
