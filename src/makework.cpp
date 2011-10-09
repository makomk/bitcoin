// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2011 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include "headers.h"
#include "db.h"
#include "net.h"
#include "init.h"
#undef printf
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
#include "makework.h"
#define printf OutputDebugStringF

using namespace std;
using namespace json_spirit;

Object JSONRPCError(int code, const string& message);


void CMakeWork::UpdateWork(bool fast)
{
    if (pindexPrev != pindexBest)
    {
        // Deallocate old blocks since they're obsolete now
        mapNewBlock.clear();
        BOOST_FOREACH(CBlock* pblock, vNewBlock)
            delete pblock;
         vNewBlock.clear();
    }
    nTransactionsUpdatedLast = nTransactionsUpdated;
    pindexPrev = pindexBest;
    nStart = GetTime();

    // Create new block
    pCurBlock = CreateNewBlock(reservekey);
    if (!pCurBlock)
        throw JSONRPCError(-7, "Out of memory");
    vNewBlock.push_back(pCurBlock);
}

void CMakeWork::GetWork(WorkDesc &work)
{
    // Update block
    if (pindexPrev != pindexBest ||
        (nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 60))
    {
        UpdateWork();
    }

    // Update nTime
    pCurBlock->nTime = max(pindexPrev->GetMedianTimePast()+1, GetAdjustedTime());
    pCurBlock->nNonce = 0;

    // Update nExtraNonce
    static unsigned int nExtraNonce = 0;
    IncrementExtraNonce(pCurBlock, pindexPrev, nExtraNonce);

    // Save
    mapNewBlock[pCurBlock->hashMerkleRoot] = make_pair(pCurBlock, pCurBlock->vtx[0].vin[0].scriptSig);

    // Prebuild hash buffers
    FormatHashBuffers(pCurBlock, work.pmidstate, work.pdata, work.phash1);

    work.hashTarget = CBigNum().SetCompact(pCurBlock->nBits).getuint256();
}

bool CMakeWork::SubmitWork(unsigned char *data)
{
    CBlock* pdata = (CBlock*)data;

    // Byte reverse
    for (int i = 0; i < 128/4; i++)
        ((unsigned int*)pdata)[i] = ByteReverse(((unsigned int*)pdata)[i]);

    // Get saved block
    if (!mapNewBlock.count(pdata->hashMerkleRoot))
        return false;
    CBlock* pblock = mapNewBlock[pdata->hashMerkleRoot].first;

    pblock->nTime = pdata->nTime;
    pblock->nNonce = pdata->nNonce;
    pblock->vtx[0].vin[0].scriptSig = mapNewBlock[pdata->hashMerkleRoot].second;
    pblock->hashMerkleRoot = pblock->BuildMerkleTree();

    return CheckWork(pblock, *pwalletMain, reservekey);
}
