// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_MAKEWORK_H
#define BITCOIN_MAKEWORK_H

#include <map>
#include <vector>
#include "script.h"
#include "wallet.h"

class CBlock;
class CBlockIndex;

struct WorkDesc
{
    char pmidstate[32];
    char pdata[128];
    char phash1[64];
    uint256 hashTarget;
};

struct WorkDescEx
{
    char pdata[128];
    uint256 hashTarget;
    CTransaction coinbaseTx;
    std::vector<uint256> merkle;
};

class CMakeWork
{
public:
    CMakeWork() : reservekey(pwalletMain), pindexPrev(NULL), pCurBlock(NULL), nExtraNonce(0) {};
    void UpdateWork(bool fast = false);
    void GetWork(WorkDesc &work);
    void GetWorkEx(WorkDescEx &work);
    bool SubmitWork(unsigned char *data);
    bool SubmitWorkEx(unsigned char *data, const std::vector<unsigned char> &coinbase);
private:
    typedef std::map<uint256, std::pair<CBlock*, CScript> > mapNewBlock_t;
    mapNewBlock_t mapNewBlock;
    std::vector<CBlock*> vNewBlock;
    CReserveKey reservekey;    

    unsigned int nTransactionsUpdatedLast;
    CBlockIndex* pindexPrev;
    int64 nStart;
    CBlock* pCurBlock;
    unsigned int nExtraNonce;
};

#endif
