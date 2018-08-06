//
//  BBREthereumBlock.c
//  breadwallet-core Ethereum
//
//  Created by Ed Gamble on 3/23/2018.
//  Copyright (c) 2018 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "BRArray.h"
#include "../base/BREthereumLogic.h"
#include "BREthereumBlock.h"
#include "BREthereumLog.h"
#include "../BREthereumPrivate.h"

//#define BLOCK_LOG_ALLOC_COUNT

#if defined (BLOCK_LOG_ALLOC_COUNT)
static unsigned int blockAllocCount = 0;
#endif

//#define BLOCK_HEADER_LOG_ALLOC_COUNT

#if defined (BLOCK_HEADER_LOG_ALLOC_COUNT)
static unsigned int blockHeaderAllocCount = 0;
#endif

// MARK: - Block Status

static void
blockStatusInitialize (BREthereumBlockStatus *status,
                       BREthereumHash hash) {
    memset (status, 0, sizeof (BREthereumBlockStatus));
    status->error = ETHEREUM_BOOLEAN_FALSE;
    status->hash = hash;
}

static void
blockStatusRelease (BREthereumBlockStatus *status) {
    if (NULL != status->transactions) array_free(status->transactions);
    if (NULL != status->logs) array_free(status->logs);
}

static BRRlpItem
blockStatusRlpEncode (BREthereumBlockStatus status,
                      BRRlpCoder coder);

static BREthereumBlockStatus
blockStatusRlpDecode (BRRlpItem item,
                      BRRlpCoder coder);

// GETH:
/*
 type Header struct {
    ParentHash  common.Hash    `json:"parentHash"       gencodec:"required"`
    UncleHash   common.Hash    `json:"sha3Uncles"       gencodec:"required"`
    Coinbase    common.Address `json:"miner"            gencodec:"required"`
    Root        common.Hash    `json:"stateRoot"        gencodec:"required"`
    TxHash      common.Hash    `json:"transactionsRoot" gencodec:"required"`
    ReceiptHash common.Hash    `json:"receiptsRoot"     gencodec:"required"`
    Bloom       Bloom          `json:"logsBloom"        gencodec:"required"`
    Difficulty  *big.Int       `json:"difficulty"       gencodec:"required"`
    Number      *big.Int       `json:"number"           gencodec:"required"`
    GasLimit    uint64         `json:"gasLimit"         gencodec:"required"`
    GasUsed     uint64         `json:"gasUsed"          gencodec:"required"`
    Time        *big.Int       `json:"timestamp"        gencodec:"required"`
    Extra       []byte         `json:"extraData"        gencodec:"required"`
    MixDigest   common.Hash    `json:"mixHash"          gencodec:"required"`
    Nonce       BlockNonce     `json:"nonce"            gencodec:"required"`
}

// HashNoNonce returns the hash which is used as input for the proof-of-work search.
func (h *Header) HashNoNonce() common.Hash {
    return rlpHash([]interface{}{
        h.ParentHash,
        h.UncleHash,
        h.Coinbase,
        h.Root,
        h.TxHash,
        h.ReceiptHash,
        h.Bloom,
        h.Difficulty,
        h.Number,
        h.GasLimit,
        h.GasUsed,
        h.Time,
        h.Extra,
    })
}
*/
/**
 * An Ethereum Block header.
 *
 * As per (2018-05-04 https://ethereum.github.io/yellowpaper/paper.pdf). [Documentation from
 * that reference]
 */
struct BREthereumBlockHeaderRecord {
    // THIS MUST BE FIRST to support BRSet operations.
    //
    // The Keccak256-bit hash (of this Block).
    BREthereumHash hash;

    // The Keccak256-bit hash of the parent block’s header, in its entirety; formally Hp.
    BREthereumHash parentHash;

    // The Keccak 256-bit hash of the ommers list portion of this block; formally Ho.
    BREthereumHash ommersHash;

    // The 160-bit address to which all fees collected from the successful mining of this block
    // be transferred; formally Hc.
    BREthereumAddress beneficiary;

    // The Keccak 256-bit hash of the root node of the state trie, after all transactions are
    // executed and finalisations applied; formally Hr.
    BREthereumHash stateRoot;

    // The Keccak 256-bit hash of the root node of the trie structure populated with each
    // transaction in the transactions list portion of the block; formally Ht.
    BREthereumHash transactionsRoot;

    // The Keccak 256-bit hash of the root node of the trie structure populated with the receipts
    // of each transaction in the transactions list portion of the block; formally He.
    BREthereumHash receiptsRoot;

    // The Bloom filter composed from indexable information (logger address and log topics)
    // contained in each log entry from the receipt of each transaction in the transactions list;
    // formally Hb.
    BREthereumBloomFilter logsBloom;

    // A scalar value corresponding to the difficulty level of this block. This can be calculated
    // from the previous block’s difficulty level and the timestamp; formally Hd.
    UInt256 difficulty;

    // A scalar value equal to the number of ancestor blocks. The genesis block has a number of
    // zero; formally Hi.
    uint64_t number;

    // A scalar value equal to the current limit of gas expenditure per block; formally Hl.
    uint64_t gasLimit; // BREthereumGas

    // A scalar value equal to the total gas used in transactions in this block; formally Hg.
    uint64_t gasUsed; // BREthereumGas

    // A scalar value equal to the reasonable output of Unix’s time() at this block’s inception;
    // formally Hs.
    uint64_t timestamp;

    // An arbitrary byte array containing data relevant to this block. This must be 32 bytes or
    // fewer; formally Hx.
    uint8_t extraData [32];
    uint8_t extraDataCount;

    // A 256-bit hash which, combined with the nonce, proves that a sufficient amount of
    // computation has been carried out on this block; formally Hm.
    BREthereumHash mixHash;

    // A 64-bitvalue which, combined with the mixHash, proves that a sufficient amount of
    // computation has been carried out on this block; formally Hn.
    uint64_t nonce;
};

static BREthereumBlockHeader
createBlockHeaderMinimal (BREthereumHash hash, uint64_t number, uint64_t timestamp) {
    BREthereumBlockHeader header = (BREthereumBlockHeader) calloc (1, sizeof (struct BREthereumBlockHeaderRecord));
    header->number = number;
    header->timestamp = timestamp;
    header->hash = hash;

#if defined (BLOCK_HEADER_LOG_ALLOC_COUNT)
    eth_log ("MEM", "Block Header Create Minimal: %d", ++blockHeaderAllocCount);
#endif

    return header;
}

static BREthereumBlockHeader
createBlockHeader (void) {
    BREthereumBlockHeader header = (BREthereumBlockHeader) calloc (1, sizeof (struct BREthereumBlockHeaderRecord));

    //         EMPTY_DATA_HASH = sha3(EMPTY_BYTE_ARRAY);
    //         EMPTY_LIST_HASH = sha3(RLP.encodeList());

    // transactionRoot, receiptsRoot
    //         EMPTY_TRIE_HASH = sha3(RLP.encodeElement(EMPTY_BYTE_ARRAY));
#if defined (BLOCK_HEADER_LOG_ALLOC_COUNT)
    eth_log ("MEM", "Block Header Create Empty: %d", ++blockHeaderAllocCount);
#endif

    return header;
}

extern BREthereumBlockHeader
blockHeaderCopy (BREthereumBlockHeader source) {
    BREthereumBlockHeader header = (BREthereumBlockHeader) calloc (1, sizeof (struct BREthereumBlockHeaderRecord));
    *header = *source;
#if defined (BLOCK_HEADER_LOG_ALLOC_COUNT)
    eth_log ("MEM", "Block Header Copy  %d", ++blockHeaderAllocCount);
#endif
    return header;
}

extern void
blockHeaderRelease (BREthereumBlockHeader header) {
#if defined (BLOCK_HEADER_LOG_ALLOC_COUNT)
    eth_log ("MEM", "Block Header Release %d", --blockHeaderAllocCount);
#endif
    assert (ETHEREUM_BOOLEAN_IS_FALSE(hashEqual(header->hash, hashCreateEmpty())));
    memset (header, 0, sizeof(struct BREthereumBlockHeaderRecord));
    free (header);
}

extern void
blockHeaderReleaseForSet (void *ignore, void *item) {
    blockHeaderRelease ((BREthereumBlockHeader) item);
}

extern BREthereumBoolean
blockHeaderIsValid (BREthereumBlockHeader header) {
    return ETHEREUM_BOOLEAN_TRUE;
}

static int64_t max(int64_t x, int64_t y) { return x >= y ? y : x; }

// See https://ethereum.github.io/yellowpaper/paper.pdf Section 4.3.3 'Block Header Validity
static UInt256
blockHeaderCanonicalDifficulty (BREthereumBlockHeader header,
                                BREthereumBlockHeader parent,
                                size_t parentOmmersCount,
                                BREthereumBlockHeader genesis) {
    if (0 == header->number) return genesis->difficulty;

    uint32_t rem; int overflow = 0;
    UInt256 x = divUInt256_Small(parent->difficulty, 2048, &rem);

    uint64_t delay_scaled = (header->timestamp - parent->timestamp) / 9;
    uint64_t y = parentOmmersCount == 0 ? 1 : 2;
    int64_t sigma_2 = max (y - delay_scaled, -99);
    assert (sigma_2 <= INT32_MAX && INT32_MIN <= sigma_2);
    UInt256 x_sigma = mulUInt256_Small(x, (uint64_t) (sigma_2 < 0 ? -sigma_2 : sigma_2), &overflow);
    assert (0 == overflow);

    uint64_t fake_block_number = header->number > 3000000 ? (header->number - 3000000) : 0;
    int64_t epsilon_exponent = (fake_block_number / 1000000)  - 2;
    assert (epsilon_exponent < 256 && epsilon_exponent > -256);
    UInt256 epsilon = createUInt256Power2(epsilon_exponent < 0 ? -epsilon_exponent : epsilon_exponent);

    UInt256 r;
    if (sigma_2 > 0)
        r = addUInt256_Overflow(parent->difficulty, x_sigma, &overflow);
    else
        r = subUInt256_Negative(parent->difficulty, x_sigma, &overflow);

    assert (0 == overflow);
    if (epsilon_exponent > 0)
        r = addUInt256_Overflow(r, epsilon, &overflow);
    else
        r = subUInt256_Negative(r, epsilon , &overflow);

    assert (0 == overflow);

    return gtUInt256(r, genesis->difficulty) ? r : genesis->difficulty;
}

// See https://ethereum.github.io/yellowpaper/paper.pdf Section 4.3.3 'Block Header Validity
extern BREthereumBoolean
blockHeaderIsConsistent (BREthereumBlockHeader header,
                         BREthereumBlockHeader parent,
                         size_t parentOmmersCount,
                         BREthereumBlockHeader genesis) {
    if (NULL == parent) return ETHEREUM_BOOLEAN_TRUE;

//    UInt256 canonicalDifficulty = blockHeaderCanonicalDifficulty(header,
//                                                                 parent,
//                                                                 parentOmmersCount,
//                                                                 genesis);

//    return AS_ETHEREUM_BOOLEAN (// nonce
//                                eqUInt256(header->difficulty, canonicalDifficulty) &&
//                                //  difficulty -- geUInt256(header->difficulty, parent->difficulty));
//                                header->gasUsed <= header->gasLimit &&
//                                header->gasLimit < parent->gasLimit + (parent->gasLimit / 1024) &&
//                                header->gasLimit > parent->gasLimit - (parent->gasLimit / 1024) &&
//                                header->gasLimit >= 5000 &&
//                                header->timestamp > parent->timestamp &&
//                                header->number == 1 + parent->number &&
//                                header->extraDataCount <= 32);

    return ETHEREUM_BOOLEAN_TRUE;
}

extern BREthereumHash
blockHeaderGetHash (BREthereumBlockHeader header) {
    return header->hash;
}

extern BREthereumHash
blockHeaderGetParentHash (BREthereumBlockHeader header) {
    return header->parentHash;
}

extern uint64_t
blockHeaderGetNumber (BREthereumBlockHeader header) {
    return header->number;
}

extern uint64_t
blockHeaderGetTimestamp (BREthereumBlockHeader header) {
    return header->timestamp;
}

extern UInt256
blockHeaderGetDifficulty (BREthereumBlockHeader header) {
    return header->difficulty;
}

extern uint64_t
blockHeaderGetGasUsed (BREthereumBlockHeader header) {
    return header->gasUsed;
}

// ...

extern uint64_t
blockHeaderGetNonce (BREthereumBlockHeader header) {
    return header->nonce;
}

extern BREthereumBoolean
blockHeaderMatch (BREthereumBlockHeader header,
                  BREthereumBloomFilter filter) {
    return bloomFilterMatch(header->logsBloom, filter);
}

extern BREthereumBoolean
blockHeaderMatchAddress (BREthereumBlockHeader header,
                         BREthereumAddress address) {
    return (ETHEREUM_BOOLEAN_IS_TRUE(blockHeaderMatch(header, bloomFilterCreateAddress(address)))
            || ETHEREUM_BOOLEAN_IS_TRUE(blockHeaderMatch(header, logTopicGetBloomFilterAddress(address)))
            ? ETHEREUM_BOOLEAN_TRUE
            : ETHEREUM_BOOLEAN_FALSE);
}

extern size_t
blockHeaderHashValue (const void *h)
{
    return hashSetValue(&((BREthereumBlockHeader) h)->hash);
}

extern int
blockHeaderHashEqual (const void *h1, const void *h2) {
    return h1 == h2 || hashSetEqual (&((BREthereumBlockHeader) h1)->hash,
                                     &((BREthereumBlockHeader) h2)->hash);
}

extern BREthereumComparison
blockHeaderCompare (BREthereumBlockHeader h1,
                    BREthereumBlockHeader h2) {
    return (h1->number < h2->number
            ? ETHEREUM_COMPARISON_LT
            : (h1->number > h2->number
               ? ETHEREUM_COMPARISON_GT
               : (h1->timestamp < h2->timestamp
                  ? ETHEREUM_COMPARISON_LT
                  : (h1->timestamp > h2->timestamp
                     ? ETHEREUM_COMPARISON_GT
                     : ETHEREUM_COMPARISON_EQ))));
}

//
// Block Header RLP Encode / Decode
//
//
extern BRRlpItem
blockHeaderRlpEncode (BREthereumBlockHeader header,
                      BREthereumBoolean withNonce,
                      BREthereumRlpType type,
                      BRRlpCoder coder) {
    BRRlpItem items[15];
    size_t itemsCount = ETHEREUM_BOOLEAN_IS_TRUE(withNonce) ? 15 : 13;

    items[ 0] = hashRlpEncode(header->parentHash, coder);
    items[ 1] = hashRlpEncode(header->ommersHash, coder);
    items[ 2] = addressRlpEncode(header->beneficiary, coder);
    items[ 3] = hashRlpEncode(header->stateRoot, coder);
    items[ 4] = hashRlpEncode(header->transactionsRoot, coder);
    items[ 5] = hashRlpEncode(header->receiptsRoot, coder);
    items[ 6] = bloomFilterRlpEncode(header->logsBloom, coder);
    items[ 7] = rlpEncodeItemUInt256 (coder, header->difficulty, 0);
    items[ 8] = rlpEncodeItemUInt64(coder, header->number, 0);
    items[ 9] = rlpEncodeItemUInt64(coder, header->gasLimit, 0);
    items[10] = rlpEncodeItemUInt64(coder, header->gasUsed, 0);
    items[11] = rlpEncodeItemUInt64(coder, header->timestamp, 0);
    items[12] = rlpEncodeItemBytes(coder, header->extraData, header->extraDataCount);

    if (ETHEREUM_BOOLEAN_IS_TRUE(withNonce)) {
        items[13] = hashRlpEncode(header->mixHash, coder);
        items[14] = rlpEncodeItemUInt64(coder, header->nonce, 0);
    }

    return rlpEncodeListItems(coder, items, itemsCount);
}

extern BREthereumBlockHeader
blockHeaderRlpDecode (BRRlpItem item,
                      BREthereumRlpType type,
                      BRRlpCoder coder) {
    BREthereumBlockHeader header = (BREthereumBlockHeader) calloc (1, sizeof(struct BREthereumBlockHeaderRecord));

    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);
    assert (13 == itemsCount || 15 == itemsCount);

    header->hash = hashCreateEmpty();

    header->parentHash = hashRlpDecode(items[0], coder);
    header->ommersHash = hashRlpDecode(items[1], coder);
    header->beneficiary = addressRlpDecode(items[2], coder);
    header->stateRoot = hashRlpDecode(items[3], coder);
    header->transactionsRoot = hashRlpDecode(items[4], coder);
    header->receiptsRoot = hashRlpDecode(items[5], coder);
    header->logsBloom = bloomFilterRlpDecode(items[6], coder);
    header->difficulty = rlpDecodeItemUInt256(coder, items[7], 0);
    header->number = rlpDecodeItemUInt64(coder, items[8], 0);
    header->gasLimit = rlpDecodeItemUInt64(coder, items[9], 0);
    header->gasUsed = rlpDecodeItemUInt64(coder, items[10], 0);
    header->timestamp = rlpDecodeItemUInt64(coder, items[11], 0);

    BRRlpData extraData = rlpDecodeItemBytes(coder, items[12]);
    memset (header->extraData, 0, 32);
    memcpy (header->extraData, extraData.bytes, extraData.bytesCount);
    header->extraDataCount = extraData.bytesCount;
    rlpDataRelease(extraData);

    if (15 == itemsCount) {
        header->mixHash = hashRlpDecode(items[13], coder);
        header->nonce = rlpDecodeItemUInt64(coder, items[14], 0);
    }

#if defined (BLOCK_HEADER_LOG_ALLOC_COUNT)
    eth_log ("MEM", "Block Header Create RLP: %d", ++blockHeaderAllocCount);
#endif

    BRRlpData data = rlpGetDataSharedDontRelease(coder, item);
    header->hash = hashCreateFromData(data);
    // Safe to ignore data release.

    return header;

}

//
// An Ethereum Block
//
// The block in Ethereum is ...
struct BREthereumBlockRecord {
    // THIS MUST BE FIRST to support BRSet operations. (its first field is BREthereumHash)
    BREthereumBlockStatus status;

    // ... the collection of relevant pieces of information (known as the block header), H,
    BREthereumBlockHeader header;

    // ... together with information corresponding to the comprised transactions, T,
    BREthereumTransaction *transactions;

    // ... and a set of other block headers U that are known to have a parent equal to the present
    // block’s parent’s parent (such blocks are known as ommers).
    BREthereumBlockHeader *ommers;

    /**
     *
     */
    BREthereumBlock next;
};

extern BREthereumBlock
blockCreateMinimal(BREthereumHash hash,
                   uint64_t number,
                   uint64_t timestamp) {
    return blockCreate(createBlockHeaderMinimal(hash, number, timestamp));
}

extern BREthereumBlock
blockCreateFull (BREthereumBlockHeader header,
                 BREthereumBlockHeader ommers[], size_t ommersCount,
                 BREthereumTransaction transactions[], size_t transactionCount) {
    BREthereumBlock block = blockCreate(header);

    BREthereumBlockHeader *blockOmmers;
    array_new (blockOmmers, ommersCount);
    for (int i = 0; i < ommersCount; i++)
        array_add (blockOmmers, ommers[i]);

    BREthereumTransaction *blockTransactions;
    array_new (blockTransactions, transactionCount);
    for (int i = 0; i < transactionCount; i++)
        array_add (blockTransactions,  transactions[i]);

    blockUpdateBody(block, blockOmmers, blockTransactions);

    return block;
}

extern BREthereumBlock
blockCreate (BREthereumBlockHeader header) {
    BREthereumBlock block = (BREthereumBlock) calloc (1, sizeof (struct BREthereumBlockRecord));

    block->header = header;
    block->ommers = NULL;
    block->transactions = NULL;

    blockStatusInitialize(&block->status, blockHeaderGetHash(block->header));
    block->next = BLOCK_NEXT_NONE;

#if defined (BLOCK_LOG_ALLOC_COUNT)
    eth_log ("MEM", "Block Create: %d", ++blockAllocCount);
#endif
    return block;
}

extern void
blockUpdateBody (BREthereumBlock block,
                 BREthereumBlockHeader *ommers,
                 BREthereumTransaction *transactions) {
    block->ommers = ommers;
    block->transactions = transactions;
}

extern void
blockRelease (BREthereumBlock block) {
    blockHeaderRelease(block->header);

    if (NULL != block->ommers) {
        for (size_t index = 0; index < array_count(block->ommers); index++)
            blockHeaderRelease(block->ommers[index]);
        array_free(block->ommers);
        block->ommers = NULL;
    }

    if (NULL != block->transactions) {
        for (size_t index = 0; index < array_count(block->transactions); index++)
            transactionRelease(block->transactions[index]);
        array_free(block->transactions);
        block->transactions = NULL;
    }
    blockStatusRelease(&block->status);
    block->next = BLOCK_NEXT_NONE;

#if defined (BLOCK_LOG_ALLOC_COUNT)
    eth_log ("MEM", "Block Release: %d", --blockAllocCount);
#endif

    free (block);
}

extern BREthereumBoolean
blockIsValid (BREthereumBlock block,
              BREthereumBoolean skipHeaderValidation) {
    if (ETHEREUM_BOOLEAN_IS_FALSE(skipHeaderValidation)
        && ETHEREUM_BOOLEAN_IS_FALSE(blockHeaderIsValid(blockGetHeader(block))))
        return ETHEREUM_BOOLEAN_FALSE;

    // TODO: Validate transactions - Merkle Root
    return ETHEREUM_BOOLEAN_TRUE;
}

extern BREthereumBlockHeader
blockGetHeader (BREthereumBlock block) {
    return block->header;
}

extern unsigned long
blockGetTransactionsCount (BREthereumBlock block) {
    return array_count(block->transactions);
}

extern BREthereumTransaction
blockGetTransaction (BREthereumBlock block, size_t index) {
    return (index < array_count(block->transactions)
            ? block->transactions[index]
            : NULL);
}

static BREthereumHash
blockGetTransactionRootHash (BREthereumBlock block,
                             size_t index,
                             size_t count) {
    assert (count > 0);

    struct ConcatenatedHashPair {
        BREthereumHash hashLeft;
        BREthereumHash hashRight;
    } concatenatedHashPair;

    // If count is down to 1 or 2, we'll use the tranactions hashes directly, otherwise...
    if (1 == count || 2 == count) {
        concatenatedHashPair.hashLeft  = transactionGetHash (block->transactions [index]);
        concatenatedHashPair.hashRight = transactionGetHash (block->transactions [index + (2 == count)]);
    }

    // ... recurse by repeatedly splitting count.
    else {
        size_t middleCount = count / 2;     // if count == 3, middleCount = 1

        // Ensure that the 'left count' is always even.
        if (1 == middleCount % 2) middleCount += 1;   // if count == 3; middleCount = 2

        concatenatedHashPair.hashLeft  = blockGetTransactionRootHash (block, index, middleCount);
        concatenatedHashPair.hashRight = blockGetTransactionRootHash (block,
                                                                      index + middleCount,
                                                                      count - middleCount);
    }

    BRRlpData concatenatedHashes = { sizeof (concatenatedHashPair), (uint8_t*) &concatenatedHashPair };
    return hashCreateFromData(concatenatedHashes);

//    return block->header->transactionsRoot;
}

extern BREthereumBoolean
blockTransactionsAreValid (BREthereumBlock block) {
    return hashEqual (block->header->transactionsRoot,
                      blockGetTransactionRootHash(block, 0, array_count(block->transactions)));
}

extern unsigned long
blockGetOmmersCount (BREthereumBlock block) {
    return NULL == block->ommers ? 0 : array_count(block->ommers);
}

extern BREthereumBlockHeader
blockGetOmmer (BREthereumBlock block, unsigned int index) {
    return (index < array_count(block->ommers)
            ? block->ommers[index]
            : NULL);
}

extern BREthereumHash
blockGetHash (BREthereumBlock block) {
    return  block->header->hash;
}

extern uint64_t
blockGetNumber (BREthereumBlock block) {
    return block->header->number;
}

extern uint64_t
blockGetTimestamp (BREthereumBlock block) {
    return block->header->timestamp;
}

extern void
blockLinkLogsWithTransactions (BREthereumBlock block) {
    assert (ETHEREUM_BOOLEAN_IS_TRUE (blockHasStatusLogsRequest(block, BLOCK_REQUEST_COMPLETE)) &&
            ETHEREUM_BOOLEAN_IS_TRUE (blockHasStatusTransactionsRequest(block, BLOCK_REQUEST_COMPLETE)));

    size_t logsCount = array_count(block->status.logs);
    for (size_t index = 0; index < logsCount; index++) {
        BREthereumLog log = block->status.logs[index];
        BREthereumTransactionStatus status = logGetStatus(log);
        uint64_t transactionIndex; size_t logIndex;
        assert (transactionStatusExtractIncluded(&status, NULL, NULL, NULL, &transactionIndex));

        BREthereumTransaction transaction = block->transactions[transactionIndex];
        logExtractIdentifier(log, NULL, &logIndex);
        logInitializeIdentifier(log, transactionGetHash(transaction), logIndex);
    }
}

//
// Block RLP Encode / Decode
//
static BRRlpItem
blockTransactionsRlpEncode (BREthereumBlock block,
                            BREthereumNetwork network,
                            BREthereumRlpType type,
                            BRRlpCoder coder) {
    size_t itemsCount = (NULL == block->transactions ? 0 : array_count(block->transactions));
    BRRlpItem items[itemsCount];

    for (int i = 0; i < itemsCount; i++)
        items[i] = transactionRlpEncode(block->transactions[i],
                                        network,
                                        type,
                                        coder);

    return rlpEncodeListItems(coder, items, itemsCount);
}

extern BREthereumTransaction *
blockTransactionsRlpDecode (BRRlpItem item,
                            BREthereumNetwork network,
                            BREthereumRlpType type,
                            BRRlpCoder coder) {
    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);

    BREthereumTransaction *transactions;
    array_new(transactions, itemsCount);

    for (int i = 0; i < itemsCount; i++) {
        BREthereumTransaction transaction = transactionRlpDecode(items[i],
                                                                     network,
                                                                     type,
                                                                     coder);
        array_add(transactions, transaction);
    }

    return transactions;
}

static BRRlpItem
blockOmmersRlpEncode (BREthereumBlock block,
                      BREthereumRlpType type,
                      BRRlpCoder coder) {
    size_t itemsCount = (NULL == block->ommers ? 0 : array_count(block->ommers));
    BRRlpItem items[itemsCount];

    for (int i = 0; i < itemsCount; i++)
        items[i] = blockHeaderRlpEncode (block->ommers[i],
                                         ETHEREUM_BOOLEAN_TRUE,
                                         type,
                                         coder);

    return rlpEncodeListItems(coder, items, itemsCount);
}

extern BREthereumBlockHeader *
blockOmmersRlpDecode (BRRlpItem item,
                      BREthereumNetwork network,
                      BREthereumRlpType type,
                      BRRlpCoder coder) {
    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);

    BREthereumBlockHeader *headers;
    array_new(headers, itemsCount);

    for (int i = 0; i < itemsCount; i++) {
        BREthereumBlockHeader header = blockHeaderRlpDecode(items[i], type, coder);
        array_add(headers, header);
    }

    return headers;
}

//
// Block Encode
//
extern BRRlpItem
blockRlpEncode (BREthereumBlock block,
                BREthereumNetwork network,
                BREthereumRlpType type,
                BRRlpCoder coder) {
    BRRlpItem items[4];

    items[0] = blockHeaderRlpEncode(block->header, ETHEREUM_BOOLEAN_TRUE, type, coder);
    items[1] = blockTransactionsRlpEncode(block, network, RLP_TYPE_TRANSACTION_SIGNED, coder);
    items[2] = blockOmmersRlpEncode(block, type, coder);

    if (RLP_TYPE_ARCHIVE == type)
        items[3] = blockStatusRlpEncode(block->status, coder);

    return rlpEncodeListItems(coder, items, (RLP_TYPE_ARCHIVE == type ? 4 : 3));
}

//
// Block Decode
//
extern BREthereumBlock
blockRlpDecode (BRRlpItem item,
                BREthereumNetwork network,
                BREthereumRlpType type,
                BRRlpCoder coder) {
    BREthereumBlock block = calloc (1, sizeof(struct BREthereumBlockRecord));
    memset (block, 0, sizeof(struct BREthereumBlockRecord));

    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);
    assert ((3 == itemsCount && RLP_TYPE_NETWORK == type) ||
            (4 == itemsCount && RLP_TYPE_ARCHIVE == type));

    block->header = blockHeaderRlpDecode(items[0], type, coder);
    block->transactions = blockTransactionsRlpDecode(items[1], network, RLP_TYPE_TRANSACTION_SIGNED, coder);
    block->ommers = blockOmmersRlpDecode(items[2], network, type, coder);

    if (RLP_TYPE_ARCHIVE == type)
        block->status = blockStatusRlpDecode(items[3], coder);
    else
        blockStatusInitialize(&block->status, blockHeaderGetHash(block->header));

    block->next = BLOCK_NEXT_NONE;

#if defined (BLOCK_LOG_ALLOC_COUNT)
    eth_log ("MEM", "Block Create RLP: %d", ++blockAllocCount);
#endif

    return block;
}

//
// MARK: - Block As Set
//
extern size_t
blockHashValue (const void *b)
{
    return hashSetValue(& ((BREthereumBlock) b)->status.hash);
}

extern int
blockHashEqual (const void *b1, const void *b2) {
    return b1 == b2 || hashSetEqual (& ((BREthereumBlock) b1)->status.hash,
                                     & ((BREthereumBlock) b2)->status.hash);
}

extern void
blockReleaseForSet (void *ignore, void *item) {
    blockRelease((BREthereumBlock) item);
}

//
// MARK: - Block Next (Chaining)
//

extern BREthereumBlock
blockGetNext (BREthereumBlock block) {
    return block->next;
}

extern BREthereumBlock // old 'next'
blockSetNext (BREthereumBlock block,
              BREthereumBlock newNext) {
    BREthereumBlock oldNext = block->next;
    block->next = newNext;
    return oldNext;
}

extern BREthereumBoolean
blockHasNext (BREthereumBlock block) {
    return AS_ETHEREUM_BOOLEAN (BLOCK_NEXT_NONE != block->next);
}

//
// MARK: - Block Status
//

extern BREthereumBlockStatus
blockGetStatus (BREthereumBlock block) {
    return block->status;
}

extern BREthereumBoolean
blockHasStatusComplete (BREthereumBlock block) {
    return AS_ETHEREUM_BOOLEAN(block->status.transactionRequest != BLOCK_REQEUST_PENDING &&
                               block->status.logRequest != BLOCK_REQEUST_PENDING &&
                               block->status.accountStateRequest != BLOCK_REQEUST_PENDING);
}

extern BREthereumBoolean
blockHasStatusError (BREthereumBlock block) {
    return block->status.error;
}

extern void
blockReportStatusError (BREthereumBlock block,
                        BREthereumBoolean error) {
    block->status.error = error;
}

//
// Transaction Request
//
extern BREthereumBoolean
blockHasStatusTransactionsRequest (BREthereumBlock block,
                                   BREthereumBlockRequestState request) {
    return AS_ETHEREUM_BOOLEAN(block->status.transactionRequest == request);
}

extern void
blockReportStatusTransactionsRequest (BREthereumBlock block,
                                      BREthereumBlockRequestState request) {
    block->status.transactionRequest = request;
}

extern void
blockReportStatusTransactions (BREthereumBlock block,
                               BREthereumTransaction *transactions) {
    assert (block->status.transactionRequest == BLOCK_REQEUST_PENDING);
    block->status.transactionRequest = BLOCK_REQUEST_COMPLETE;
    block->status.transactions = transactions;
}

//
// Log Request
//
extern BREthereumBoolean
blockHasStatusLogsRequest (BREthereumBlock block,
                           BREthereumBlockRequestState request) {
    return AS_ETHEREUM_BOOLEAN(block->status.logRequest == request);
}

extern void
blockReportStatusLogsRequest (BREthereumBlock block,
                              BREthereumBlockRequestState request) {
    block->status.logRequest = request;
}

extern void
blockReportStatusLogs (BREthereumBlock block,
                       BREthereumLog *logs) {
    assert (block->status.logRequest == BLOCK_REQEUST_PENDING);
    block->status.logRequest = BLOCK_REQUEST_COMPLETE;
    block->status.logs = logs;
}

extern BREthereumBoolean
blockHasStatusAccountStateRequest (BREthereumBlock block,
                                   BREthereumBlockRequestState request) {
    return AS_ETHEREUM_BOOLEAN(block->status.accountStateRequest == request);
}

extern void
blockReportStatusAccountStateRequest (BREthereumBlock block,
                                      BREthereumBlockRequestState request) {
    block->status.accountStateRequest = request;
}

extern void
blockReportStatusAccountState (BREthereumBlock block,
                               BREthereumAccountState accountState) {
    assert (block->status.accountStateRequest == BLOCK_REQEUST_PENDING);
    block->status.accountStateRequest = BLOCK_REQUEST_COMPLETE;
    block->status.accountState = accountState;
}

extern BREthereumBoolean
blockHasStatusTransaction (BREthereumBlock block,
                           BREthereumTransaction transaction) {
    if (block->status.transactionRequest != BLOCK_REQUEST_COMPLETE) return ETHEREUM_BOOLEAN_FALSE;

    for (size_t index = 0; index < array_count(block->status.transactions); index++)
        if (transactionHashEqual(transaction, block->status.transactions[index]))
            return ETHEREUM_BOOLEAN_TRUE;

    return ETHEREUM_BOOLEAN_FALSE;
}

extern BREthereumBoolean
blockHasStatusLog (BREthereumBlock block,
                   BREthereumLog log) {
    if (block->status.logRequest != BLOCK_REQUEST_COMPLETE) return ETHEREUM_BOOLEAN_FALSE;

    for (size_t index = 0; index < array_count(block->status.logs); index++)
        if (logHashEqual(log, block->status.logs[index]))
            return ETHEREUM_BOOLEAN_TRUE;

    return ETHEREUM_BOOLEAN_FALSE;
}


static BRRlpItem
blockStatusRlpEncode (BREthereumBlockStatus status,
                      BRRlpCoder coder) {
    BRRlpItem items[6];

    uint64_t flags = ((status.transactionRequest << 4) |
                      (status.logRequest << 2) |
                      (status.accountStateRequest << 0));

    items[0] = hashRlpEncode(status.hash, coder);
    items[1] = rlpEncodeItemUInt64(coder, flags, 1);

    // TODO: Fill out
    items[2] = rlpEncodeItemString(coder, "");
    items[3] = rlpEncodeItemString(coder, "");
    items[4] = accountStateRlpEncode(status.accountState, coder);

    items[5] = rlpEncodeItemUInt64(coder, status.error, 0);

    return rlpEncodeListItems(coder, items, 6);
}

static BREthereumBlockStatus
blockStatusRlpDecode (BRRlpItem item,
                      BRRlpCoder coder) {
    BREthereumBlockStatus status;

    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);
    assert (6 == itemsCount);

    status.hash = hashRlpDecode(items[0], coder);

    uint64_t flags = rlpDecodeItemUInt64(coder, items[1], 1);
    status.transactionRequest = 0x3 & (flags >> 4);
    status.logRequest = 0x3 & (flags >> 2);
    status.accountStateRequest = 0x3 & (flags >> 0);

    // TODO: Fill Out
    status.transactions = NULL;  // items [2]
    status.logs = NULL; // items [3]
    status.accountState = accountStateRlpDecode(items[4], coder);

    status.error = (BREthereumBoolean) rlpDecodeItemUInt64(coder, items[5], 0);
    return status;
}

/* Block Headers (10)
 ETH: LES:   L 10: [
 ETH: LES:     L 15: [
 ETH: LES:       I 32: 0xb853f283c777f628e28be62a80850d98a5a5e9c4e86afb0e785f7a222ebf67f8
 ETH: LES:       I 32: 0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347
 ETH: LES:       I 20: 0xb2930b35844a230f00e51431acae96fe543a0347
 ETH: LES:       I 32: 0x24dc4e9f66f026b0569270e8ef95d34c275721ff6eecab029afe11c43249e046
 ETH: LES:       I 32: 0xa7f796d7cd98ed2f3fb70ecb9a48939825f1a3d0364eb995c49151761ce9659c
 ETH: LES:       I 32: 0xed45b3ad4bb2b46bf1e49a7925c63042aa41d5af7372db334142152d5a7ec422
 ETH: LES:       I256: 0x00a61039e24688a200002e10102021116002220040204048308206009928412802041520200201115014888000c00080020a002021308850c60d020d00200188062900c83288401115821a1c101200d00318080088df000830c1938002a018040420002a22201000680a391c91610e4884682a00910446003da000b9501020009c008205091c0b04108c000410608061a07042141001820440d404042002a4234f00090845c1544820140430552592100352140400039000108e052110088800000340422064301701c8212008820c4648a020a482e90a0268480000400021800110414680020205002400808012c6248120027c4121119802240010a2181983
 ETH: LES:       I  7: 0x0baf848614eb16
 ETH: LES:       I  3: 0x5778a9
 ETH: LES:       I  3: 0x7a121d
 ETH: LES:       I  3: 0x793640
 ETH: LES:       I  4: 0x5b1596f8
 ETH: LES:       I  5: 0x73696e6731
 ETH: LES:       I 32: 0xbcefde2594b8b501c985c2f0f411c69baee727c4f90a74ef28b7f2b59a00f7c2
 ETH: LES:       I  8: 0x07ab2de40005d6f7
 ETH: LES:     ]
 ETH: LES:     L 15: [
 ETH: LES:       I 32: 0x8996b1f91f060302350f1cb9014a11d48fd1b42eeeacf18ce4762b94c69656fa
 ETH: LES:       I 32: 0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347
 ETH: LES:       I 20: 0xea674fdde714fd979de3edf0f56aa9716b898ec8
 ETH: LES:       I 32: 0x4a91466c8a43f6c61d47ae2680072ec0ca9d4077752b947bc0913f6f52823476
 ETH: LES:       I 32: 0xfc9df67c1dc39852692763387da8039d824e9956d936338184f51c6734e8cc9f
 ETH: LES:       I 32: 0x2cfe437b4cac28ccbb68373b6107e4e1c8fabdbfe8941d256367d4ab9e97e3e4
 ETH: LES:       I256: 0xe01310094f66290e1101240443c1f801110021114120280c00088524322a016c2c2212b0012302001094b000009404a10018c03040208082c600c64c01280101039141e008a2c9198186044c1882541400580c36026194088033a15a08003400c5200624020c010033453168429059cd066310252a04680618226215548466e4006180038a24544804c209e11046012c008046b100065c648050084c0a15222aba6e800030c0148c2301162034298812550c060c20018470190a4141280920c110124052001d31444a30030116a42b0001c36427a888c817281110482046a04003183121a4b00042a4c6008048208fa444200204280222cc008c148446101092
 ETH: LES:       I  7: 0x0bab23f73e93e6
 ETH: LES:       I  3: 0x5778a7
 ETH: LES:       I  3: 0x7a121d
 ETH: LES:       I  3: 0x7a0453
 ETH: LES:       I  4: 0x5b1596ed
 ETH: LES:       I 21: 0x65746865726d696e652d6177732d61736961312d33
 ETH: LES:       I 32: 0x057ca051122a8e18dbd6dadaade0f7c5b877623a4ae706facce5de7d1f924858
 ETH: LES:       I  8: 0xcd9b80400100b43c
 ETH: LES:     ]
 ETH: LES:    ...
 ETH: LES:  ]
 */


/* BLock Bodies
ETH: LES:   L  2: [
ETH: LES:     L  2: [
ETH: LES:       L117: [
ETH: LES:         L  9: [
ETH: LES:           I  1: 0x09
ETH: LES:           I  5: 0x0ba43b7400
ETH: LES:           I  2: 0x5208
ETH: LES:           I 20: 0x5521a68d4f8253fc44bfb1490249369b3e299a4a
ETH: LES:           I  8: 0x154f1f6cc6457c00
ETH: LES:           I  0: 0x
ETH: LES:           I  1: 0x26
ETH: LES:           I 32: 0x317428668e86eedc101a2ac3344c66dca791b078557e018a4524d86da3529de2
ETH: LES:           I 32: 0x40446aa978d382ad2a12549713ad94cbe654aa4853ab68414566a0638689f6a9
ETH: LES:         ]
ETH: LES:         L  9: [
ETH: LES:           I  2: 0x0308
ETH: LES:           I  5: 0x0ba43b7400
ETH: LES:           I  2: 0xb78d
ETH: LES:           I 20: 0x58a4884182d9e835597f405e5f258290e46ae7c2
ETH: LES:           I  0: 0x
ETH: LES:           I 68: 0xa9059cbb0000000000000000000000004a2ce805877250dd17e14f4421d66d2a9717725a0000000000000000000000000000000000000000000000273f96e31883e34000
ETH: LES:           I  1: 0x26
ETH: LES:           I 32: 0xf02eac3ab7c93ccdea73b6cbcc3483d0d03cdb374c07b3021461e8bb108234fa
ETH: LES:           I 32: 0x511e4c85506e11c46198fd61982f5cc05c9c06f792aaee47f5ddf14ced764b1d
ETH: LES:         ]
ETH: LES:         ...
ETH: LES:       ]
ETH: LES:       L  0: []
ETH: LES:     ]
ETH: LES:    ...
ETH: LES   ]
*/

//
// MARK: Genesis Blocks
//

// We should extract these blocks from the Ethereum Blockchain so as to have the definitive
// data.  

/*
 Appendix I. Genesis Block and is specified thus:
 The genesis block is 15 items,
 (0-256 , KEC(RLP()), 0-160 , stateRoot, 0, 0, 0-2048 , 2^17 , 0, 0, 3141592, time, 0, 0-256 , KEC(42), (), ()􏰂
 Where 0256 refers to the parent hash, a 256-bit hash which is all zeroes; 0160 refers to the
 beneficiary address, a 160-bit hash which is all zeroes; 02048 refers to the log bloom,
 2048-bit of all zeros; 217 refers to the difficulty; the transaction trie root, receipt
 trie root, gas used, block number and extradata are both 0, being equivalent to the empty
 byte array. The sequences of both ommers and transactions are empty and represented by
 (). KEC(42) refers to the Keccak hash of a byte array of length one whose first and only byte
 is of value 42, used for the nonce. KEC(RLP()) value refers to the hash of the ommer list in
 RLP, both empty lists.

 The proof-of-concept series include a development premine, making the state root hash some
 value stateRoot. Also time will be set to the initial timestamp of the genesis block. The
 latest documentation should be consulted for those values.
 */

// Ideally, we'd statically initialize the Genesis blocks.  But, we don't have static
// initializer for BREthereumHash, BREthereumAddress nor BREthereumBlookFilter.  Therefore,
// will define `initializeGenesisBlocks(void)` to convert hex-strings into the binary values.
//
// The following is expanded for illustration only; it is filled below.
static struct BREthereumBlockHeaderRecord genesisMainnetBlockHeaderRecord = {

    // BREthereumHash hash;
    EMPTY_HASH_INIT,

    // BREthereumHash parentHash;
    EMPTY_HASH_INIT,

    // BREthereumHash ommersHash;
    EMPTY_HASH_INIT,

    // BREthereumAddressRaw beneficiary;
    EMPTY_ADDRESS_INIT,

    // BREthereumHash stateRoot;
    EMPTY_HASH_INIT,

    // BREthereumHash transactionsRoot;
    EMPTY_HASH_INIT,

    // BREthereumHash receiptsRoot;
    EMPTY_HASH_INIT,

    // BREthereumBloomFilter logsBloom;
    EMPTY_BLOOM_FILTER_INIT,

    // uint256_t difficulty;
    UINT256_INIT (1 << 17),

    // uint64_t number;
    0,

    // uint64_t gasLimit;
    0x1388,

    // uint64_t gasUsed;
    0,

    // uint64_t timestamp;
    0,

    // uint8_t extraData [32];
    { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
      0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 },

    // uint8_t extraDataCount;
    0,

#if BLOCK_HEADER_NEEDS_SEED_HASH == 1
//    BREthereumHash seedHash;
    EMPTY_HASH_INIT,
#endif

    // BREthereumHash mixHash;
    EMPTY_HASH_INIT,

    // uint64_t nonce;
    0x0000000000000042
};
static struct BREthereumBlockHeaderRecord genesisTestnetBlockHeaderRecord;
static struct BREthereumBlockHeaderRecord genesisRinkebyBlockHeaderRecord;

const BREthereumBlockHeader ethereumMainnetBlockHeader = &genesisMainnetBlockHeaderRecord;
const BREthereumBlockHeader ethereumTestnetBlockHeader = &genesisTestnetBlockHeaderRecord;
const BREthereumBlockHeader ethereumRinkebyBlockHeader = &genesisRinkebyBlockHeaderRecord;

static void
initializeGenesisBlocks (void);

extern BREthereumBlockHeader
networkGetGenesisBlockHeader (BREthereumNetwork network) {
    static int needInitializeGenesisBlocks = 1;

    if (needInitializeGenesisBlocks) {
        needInitializeGenesisBlocks = 0;
        initializeGenesisBlocks();
    }

    BREthereumBlockHeader genesisHeader =
    (network == ethereumMainnet
     ? ethereumMainnetBlockHeader
     : (network == ethereumTestnet
        ? ethereumTestnetBlockHeader
        : (network == ethereumRinkeby
           ? ethereumRinkebyBlockHeader
           : NULL)));

    return genesisHeader == NULL ? NULL : blockHeaderCopy(genesisHeader);
}

extern BREthereumBlock
networkGetGenesisBlock (BREthereumNetwork network) {
    return blockCreate(networkGetGenesisBlockHeader(network));
}

static void
initializeGenesisBlocks (void) {
    BREthereumBlockHeader header;

    // Mainnet
    /*
    {"jsonrpc":"2.0","id":1,"result":
        {
            "hash":"0xd4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3",
            "parentHash":"0x0000000000000000000000000000000000000000000000000000000000000000",
            "sha3Uncles":"0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347",
            "miner":"0x0000000000000000000000000000000000000000",
            "stateRoot":"0xd7f8974fb5ac78d9ac099b9ad5018bedc2ce0a72dad1827a1709da30580f0544",
            "transactionsRoot":"0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
            "receiptsRoot":"0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
            "logsBloom":"0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",

            "difficulty":"0x400000000",
            "number":"0x0",
            "gasLimit":"0x1388",
            "gasUsed":"0x0",
            "timestamp":"0x0",

            "extraData":"0x11bbe8db4e347b4e8c937c1c8370e4b5ed33adb3db69cbdb7a38e1e50b1b82fa",
            "mixHash":"0x0000000000000000000000000000000000000000000000000000000000000000",
            "nonce":"0x0000000000000042",

            "size":"0x21c",
            "totalDifficulty":"0x400000000",

            "transactions":[],
            "uncles":[]
        }}
     */
    header = &genesisMainnetBlockHeaderRecord;
    header->hash = hashCreate("0xd4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3");
    header->parentHash = hashCreate("0x0000000000000000000000000000000000000000000000000000000000000000");
    header->ommersHash = hashCreate("0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347");
    header->beneficiary = addressCreate("0x0000000000000000000000000000000000000000");
    header->stateRoot = hashCreate("0xd7f8974fb5ac78d9ac099b9ad5018bedc2ce0a72dad1827a1709da30580f0544");
    header->transactionsRoot = hashCreate("0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421");
    header->receiptsRoot = hashCreate("0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421");
    header->logsBloom = bloomFilterCreateString("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    header->difficulty = createUInt256 (0x400000000);
    header->number = 0x0;
    header->gasLimit = 0x1388;
    header->gasUsed = 0x0;
    header->timestamp = 0x0;
    decodeHex(header->extraData, 32, "11bbe8db4e347b4e8c937c1c8370e4b5ed33adb3db69cbdb7a38e1e50b1b82fa", 64);
    header->extraDataCount = 32;
    header->mixHash = hashCreate("0x0000000000000000000000000000000000000000000000000000000000000000");
    header->nonce = 0x0000000000000042;

    // Testnet
    /*
    {"jsonrpc":"2.0","id":1,"result":
        {"difficulty":"0x100000",
            "extraData":"0x3535353535353535353535353535353535353535353535353535353535353535",
            "gasLimit":"0x1000000",
            "gasUsed":"0x0",
            "hash":"0x41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d",
            "logsBloom":"0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
            "miner":"0x0000000000000000000000000000000000000000",
            "mixHash":"0x0000000000000000000000000000000000000000000000000000000000000000",
            "nonce":"0x0000000000000042",
            "number":"0x0",
            "parentHash":"0x0000000000000000000000000000000000000000000000000000000000000000",
            "receiptsRoot":"0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
            "sha3Uncles":"0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347",
            "size":"0x21c",
            "stateRoot":"0x217b0bbcfb72e2d57e28f33cb361b9983513177755dc3f33ce3e7022ed62b77b",
            "timestamp":"0x0",
            "totalDifficulty":"0x100000",
            "transactions":[],
            "transactionsRoot": "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
            "uncles":[]}}
     */
    header = &genesisTestnetBlockHeaderRecord;
    header->hash = hashCreate("0x41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d");
    header->parentHash = hashCreate("0x0000000000000000000000000000000000000000000000000000000000000000");
    header->ommersHash = hashCreate("0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347");
    header->beneficiary = addressCreate("0x0000000000000000000000000000000000000000");
    header->stateRoot = hashCreate("0x217b0bbcfb72e2d57e28f33cb361b9983513177755dc3f33ce3e7022ed62b77b");
    header->transactionsRoot = hashCreate("0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421");
    header->receiptsRoot = hashCreate("0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421");
    header->logsBloom = bloomFilterCreateString("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    header->difficulty = createUInt256 (0x100000);
    header->number = 0x0;
    header->gasLimit = 0x1000000;
    header->gasUsed = 0x0;
    header->timestamp = 0x0;
    decodeHex(header->extraData, 32, "3535353535353535353535353535353535353535353535353535353535353535", 64);
    header->extraDataCount = 32;
    header->mixHash = hashCreate("0x0000000000000000000000000000000000000000000000000000000000000000");
    header->nonce = 0x0000000000000042;

    // Rinkeby
    /*
    {"jsonrpc":"2.0","id":1,"result":
        {"difficulty":"0x1",
            "extraData":"0x52657370656374206d7920617574686f7269746168207e452e436172746d616e42eb768f2244c8811c63729a21a3569731535f067ffc57839b00206d1ad20c69a1981b489f772031b279182d99e65703f0076e4812653aab85fca0f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
            "gasLimit":"0x47b760",
            "gasUsed":"0x0",
            "hash":"0x6341fd3daf94b748c72ced5a5b26028f2474f5f00d824504e4fa37a75767e177",
            "logsBloom":"0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
            "miner":"0x0000000000000000000000000000000000000000",
            "mixHash":"0x0000000000000000000000000000000000000000000000000000000000000000",
            "nonce":"0x0000000000000000",
            "number":"0x0",
            "parentHash":"0x0000000000000000000000000000000000000000000000000000000000000000",
            "receiptsRoot":"0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
            "sha3Uncles":"0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347",
            "size":"0x29a",
            "stateRoot":"0x53580584816f617295ea26c0e17641e0120cab2f0a8ffb53a866fd53aa8e8c2d",
            "timestamp":"0x58ee40ba",
            "totalDifficulty":"0x1",
            "transactions":[],
            "transactionsRoot":"0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
            "uncles":[]}}
     */
    header = &genesisRinkebyBlockHeaderRecord;
    header->hash = hashCreate("0x6341fd3daf94b748c72ced5a5b26028f2474f5f00d824504e4fa37a75767e177");
    header->parentHash = hashCreate("0x0000000000000000000000000000000000000000000000000000000000000000");
    header->ommersHash = hashCreate("0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347");
    header->beneficiary = addressCreate("0x0000000000000000000000000000000000000000");
    header->stateRoot = hashCreate("0x53580584816f617295ea26c0e17641e0120cab2f0a8ffb53a866fd53aa8e8c2d");
    header->transactionsRoot = hashCreate("0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421");
    header->receiptsRoot = hashCreate("0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421");
    header->logsBloom = bloomFilterCreateString("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    header->difficulty = createUInt256 (0x1);
    header->number = 0x0;
    header->gasLimit = 0x47b760;
    header->gasUsed = 0x0;
    header->timestamp = 0x58ee40ba;
    // TODO: Rinkeby ExtraData is oversized... ignore.
    decodeHex(header->extraData, 32, "3535353535353535353535353535353535353535353535353535353535353535", 64);
    header->extraDataCount = 32;
    header->mixHash = hashCreate("0x0000000000000000000000000000000000000000000000000000000000000000");
    header->nonce = 0x0000000000000000;
}

//
// MARK: Block Checkpoint
//

static const BREthereumBlockCheckpoint
ethereumMainnetCheckpoints [] = {
    {       0, HASH_INIT("d4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3"),  0 }, // 1061 days  6 hrs ago (Jul-30-2015 03:26:13 PM +UTC)
#if 0
    { 5000000, HASH_INIT("7d5a4369273c723454ac137f48a4f142b097aa2779464e6505f1b1c5e37b5382"),  0 }, //  146 days  8 hrs ago (Jan-30-2018 01:41:33 PM +UTC)
    { 5500000, HASH_INIT("2d3a154eee9f90666c6e824f11e15f2d60b05323a81254f60075c34a61ef124d"),  0 }, //   61 days 22 hrs ago (Apr-24-2018 11:07:01 PM +UTC)

    // Has three+ logs
    { 5506600, HASH_INIT("4f91cdbd4eb27b12b7959daa8b4300d88a5a88efc8256c413534fe16fe9eee2b"),  0 }, //   17 days 23 hrs ago (Jun-07-2018 10:25:41 PM +UTC)
    { 5750000, HASH_INIT("9645ed6cd4994b1e734eb25abbc225005dce591cbbe9e083cd2587b27cfe908f"),  0 }, //

    // Has three+ eth - 5795662
    { 5795660, HASH_INIT("5eeddeff1bfdde5859a63f47fbd3a4ff929be9dc21dd48a52a8cd08d560cc3b5"),  0 }, //   11 days 21 hrs ago (Jun-15-2018 10:43:11 PM +UTC)

    // Head - 5860000
    { 5860000, HASH_INIT("ef888507717b8d59d3abb24f618a7809cf58d5a723d691979769a4a4cf39f63c"),  0 }, //
    { 5865000, HASH_INIT("9f573bfa8b0ffaca5210b45eb01c12e4d0f6ffc3a8c4d13bea8176b1266f5d53"),  0 }, //    1 hr 25 mins ago (Jun-27-2018 07:38:02 PM +UTC)

    // 2018-07-02
    { 5893500, HASH_INIT("a8ed4df2446a09d85b5867253a7c4a0870a65d858cf43539070186e180b4f7d2"),  0 },

    // 2018-07-03
    { 5899000, HASH_INIT("e7b0051c152755f03e9e98b5f41540d79abee02c5770c22299b7250a2d647507"),  0 },

    // 2018-07-11
    { 5947250, HASH_INIT("8af864db3ad85a966b23841a235568312b1d06a1e51bc3e3b253722187317590"),  0 }, //
#endif
};
#define CHECKPOINT_MAINNET_COUNT      (sizeof (ethereumMainnetCheckpoints) / sizeof (BREthereumBlockCheckpoint))

static const BREthereumBlockCheckpoint
ethereumTestnetCheckpoints [] = {
    {       0, HASH_INIT("41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d"),  0 }, // 1061 days  6 hrs ago (Jul-30-2015 03:26:13 PM +UTC)
    { 3500000, HASH_INIT("afeb3a16e527470f325a0d152db7779c90490788fb2485d4e87d4eda41e93574"),  0 }, //    1 day 14 hrs ago (Jun-24-2018 08:02:44 AM +UTC)
};
#define CHECKPOINT_TESTNET_COUNT      (sizeof (ethereumTestnetCheckpoints) / sizeof (BREthereumBlockCheckpoint))

static const BREthereumBlockCheckpoint
ethereumRinkebyCheckpoints [] = {
    {       0, HASH_INIT("6341fd3daf94b748c72ced5a5b26028f2474f5f00d824504e4fa37a75767e177"),  0x58ee40ba }, //  439 days  6 hrs ago (Apr-12-2017 03:20:50 PM +UTC)
    { 2500000, HASH_INIT("40ecdd747a7a2c39eed10991af78f3d294c3aee778cd171550042d84d6cb3b7a"),  0x58ee40ba }, //    4 days 13 hrs ago (Jun-21-2018 08:34:46 AM +UTC)
};
#define CHECKPOINT_RINKEBY_COUNT      (sizeof (ethereumRinkebyCheckpoints) / sizeof (BREthereumBlockCheckpoint))

static const BREthereumBlockCheckpoint *
blockCheckpointFindForNetwork (BREthereumNetwork network,
                               size_t *count) {
    assert (NULL != count);

    if (network == ethereumMainnet) {
        *count = CHECKPOINT_MAINNET_COUNT;
        return ethereumMainnetCheckpoints;
    }

    if (network == ethereumTestnet) {
        *count = CHECKPOINT_TESTNET_COUNT;
        return ethereumTestnetCheckpoints;
    }

    if (network == ethereumRinkeby) {
        *count = CHECKPOINT_RINKEBY_COUNT;
        return ethereumRinkebyCheckpoints;
    }
    *count = 0;
    return NULL;
}

extern const BREthereumBlockCheckpoint *
blockCheckpointLookupLatest (BREthereumNetwork network) {
    size_t count;
    const BREthereumBlockCheckpoint *checkpoints = blockCheckpointFindForNetwork(network, &count);
    return &checkpoints[count - 1];
}

extern const BREthereumBlockCheckpoint *
blockCheckpointLookupByNumber (BREthereumNetwork network,
                               uint64_t number) {
    size_t count;
    const BREthereumBlockCheckpoint *checkpoints = blockCheckpointFindForNetwork(network, &count);
    for (size_t index = count; index > 0; index--)
        if (checkpoints[index - 1].number <= number)
            return &checkpoints[index - 1];
    return NULL;
}

extern const BREthereumBlockCheckpoint *
blockCheckpointLookupByTimestamp (BREthereumNetwork network,
                                  uint64_t timestamp) {
    size_t count;
    const BREthereumBlockCheckpoint *checkpoints = blockCheckpointFindForNetwork(network, &count);
    for (size_t index = count; index > 0; index--)
        if (checkpoints[index - 1].timestamp <= timestamp)
            return &checkpoints[index - 1];
    return NULL;
}

extern BREthereumBlockHeader
blockCheckpointCreatePartialBlockHeader (const BREthereumBlockCheckpoint *checkpoint) {
    return createBlockHeaderMinimal (checkpoint->hash, checkpoint->number, checkpoint->timestamp);
}
