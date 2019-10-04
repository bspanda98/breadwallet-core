//
//  BRGenericHandlers.h
//  Core
//
//  Created by Ed Gamble on 6/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRGenericHandlers_h
#define BRGenericHandlers_h

#include "support/BRInt.h" // UInt256
#include "support/BRArray.h"
#include "support/BRKey.h"
#include "support/BRFileService.h"
#include "BRGenericBase.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {
        GENERIC_SYNC_TYPE_TRANSACTION,
        GENERIC_SYNC_TYPE_TRANSFER
    } BRGenericAPISyncType;

    typedef struct BRGenericHandersRecord *BRGenericHandlers;

    typedef struct BRGenericAccountWithTypeRecord {
        BRGenericHandlers handlers;
        BRGenericAccount base;
    } *BRGenericAccountWithType;;

    typedef struct {
        BRGenericHandlers handlers;
    } BRGenericNetworkRecord;

    // MARK: - Generic Account

    typedef void * (*BRGenericAccountCreate) (const char *type, UInt512 seed);
    typedef void * (*BRGenericAccountCreateWithPublicKey) (const char *type, BRKey key);
    typedef void * (*BRGenericAccountCreateWithSerialization) (const char *type, uint8_t *bytes, size_t bytesCount);
    typedef void (*BRGenericAccountFree) (BRGenericAccount account);
    typedef BRGenericAddress (*BRGenericAccountGetAddress) (BRGenericAccount account);
    typedef uint8_t * (*BRGenericAccountGetSerialization) (BRGenericAccount account, size_t *bytesCount);
    typedef void (*BRGenericAccountSerializeTransfer) (BRGenericAccount account, BRGenericTransfer transfer, UInt512 seed);

    // MARK: - Generic Address
    typedef char * (*BRGenericAddressAsString) (BRGenericAddress address);
    typedef int (*BRGenericAddressEqual) (BRGenericAddress address1,
                                          BRGenericAddress address2);

    // MARK: - Generic Treansfer
    typedef BRGenericTransfer (*BRGenericTransferCreate) (BRGenericAddress source,
                                                          BRGenericAddress target,
                                                          UInt256 amount);
    typedef void (*BRGenericTransferFree) (BRGenericTransfer transfer);
    typedef BRGenericAddress (*BRGenericTransferGetSourceAddress) (BRGenericTransfer transfer);
    typedef BRGenericAddress (*BRGenericTransferGetTargetAddress) (BRGenericTransfer transfer);
    typedef UInt256 (*BRGenericTransferGetAmount) (BRGenericTransfer transfer);
    typedef UInt256 (*BRGenericTransferGetFee) (BRGenericTransfer transfer);
    typedef BRGenericFeeBasis (*BRGenericTransferGetFeeBasis) (BRGenericTransfer transfer);
    typedef BRGenericHash (*BRGenericTransferGetHash) (BRGenericTransfer transfer);
    typedef uint8_t * (*BRGenericTransferGetSerialization) (BRGenericTransfer transfer, size_t *bytesCount);

    // MARK: - Generic Wallet
    typedef BRGenericWallet (*BRGenericWalletCreate) (BRGenericAccount account);
    typedef void (*BRGenericWalletFree) (BRGenericWallet wallet);
    typedef UInt256 (*BRGenericWalletGetBalance) (BRGenericWallet wallet);

    // MARK: - Generic Wallet Manager
    // Create a transfer from the
    typedef BRGenericTransfer (*BRGenericWalletManagerRecoverTransfer) (const char *hash,
                                                                        const char *from,
                                                                        const char *to,
                                                                        const char *amount,
                                                                        const char *currency,
                                                                        uint64_t timestamp,
                                                                        uint64_t blockHeight);

    typedef BRArrayOf(BRGenericTransfer) (*BRGenericWalletManagerRecoverTransfersFromRawTransaction) (uint8_t *bytes,
                                                                        size_t   bytesCount);

    typedef void (*BRGenericWalletManagerInitializeFileService) (BRFileServiceContext context,
                                                                 BRFileService fileService);

    typedef BRArrayOf(BRGenericTransfer) (*BRGenericWalletManagerLoadTransfers) (BRFileServiceContext context,
                                                                                 BRFileService fileService);

    typedef BRGenericAPISyncType (*BRGenericWalletManagerGetAPISyncType) (void);

    // MARK: - FeeBasis
    typedef UInt256 (*BRGenericFeeBasisGetPricePerCostFactor) (BRGenericFeeBasis feeBasis);
    typedef double (*BRGenericFeeBasisGetCostFactor) (BRGenericFeeBasis feeBasis);
    typedef uint32_t (*BRGenericFeeBasisIsEqual) (BRGenericFeeBasis fb1, BRGenericFeeBasis fb2);
    typedef void (*BRGenericFeeBasisFree) (BRGenericFeeBasis feeBasis);

    // MARK: - Network

    typedef BRGenericAddress (*BRGenericNetworkAddressCreate) (const char * address);
    typedef void (*BRGenericNetworkAddressFree) (BRGenericAddress address);

    // MARK: - Generic Handlers

    struct BRGenericHandersRecord {
        const char *type;
        struct {
            BRGenericAccountCreate create;
            BRGenericAccountCreateWithPublicKey createWithPublicKey;
            BRGenericAccountCreateWithSerialization createWithSerialization;
            BRGenericAccountFree free;
            BRGenericAccountGetAddress getAddress;
            BRGenericAccountGetSerialization getSerialization;
            BRGenericAccountSerializeTransfer serializeTransfer;
        } account;

        struct {
            BRGenericAddressAsString asString;
            BRGenericAddressEqual equal;
        } address;
        
        struct {
            BRGenericTransferCreate create;
            BRGenericTransferFree   free;
            BRGenericTransferGetSourceAddress sourceAddress;
            BRGenericTransferGetTargetAddress targetAddress;
            BRGenericTransferGetAmount amount;
            BRGenericTransferGetFee fee;
            BRGenericTransferGetFeeBasis feeBasis;
            BRGenericTransferGetHash hash;
            BRGenericTransferGetSerialization getSerialization;
        } transfer;

        struct {
            BRGenericWalletCreate create;
            BRGenericWalletFree free;
            BRGenericWalletGetBalance balance;
            // set balance

            // ...
        } wallet;

        struct {
            BRGenericWalletManagerRecoverTransfer transferRecover;
            BRGenericWalletManagerRecoverTransfersFromRawTransaction transfersRecoverFromRawTransaction;
            BRGenericWalletManagerInitializeFileService fileServiceInit;
            BRGenericWalletManagerLoadTransfers fileServiceLoadTransfers;
            BRGenericWalletManagerGetAPISyncType apiSyncType;
        } manager;

        struct {
            BRGenericFeeBasisGetPricePerCostFactor pricePerCostFactor;
            BRGenericFeeBasisGetCostFactor costFactor;
            BRGenericFeeBasisIsEqual feeBasisIsEqual;
            BRGenericFeeBasisFree free;
        } feebasis;

        struct {
            BRGenericNetworkAddressCreate networkAddressCreate;
            BRGenericNetworkAddressFree networkAddressFree;
        } network;
    };

    extern void
    genericHandlersInstall (const BRGenericHandlers handlers);

    extern const BRGenericHandlers
    genericHandlerLookup (const char *symbol);

#ifdef __cplusplus
}
#endif

#endif /* BRGenericHandlers_h */
