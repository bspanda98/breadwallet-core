/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: CryptoAddClaim.proto */

#ifndef PROTOBUF_C_CryptoAddClaim_2eproto__INCLUDED
#define PROTOBUF_C_CryptoAddClaim_2eproto__INCLUDED

#include "protobuf-c.h"

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003002 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

#include "BasicTypes.pb-c.h"
#include "Duration.pb-c.h"

typedef struct _Proto__Claim Proto__Claim;
typedef struct _Proto__CryptoAddClaimTransactionBody Proto__CryptoAddClaimTransactionBody;


/* --- enums --- */


/* --- messages --- */

/*
 * A hash (presumably of some kind of credential or certificate), along with a list of keys (each of which is either a primitive or a threshold key). Each of them must reach its threshold when signing the transaction, to attach this claim to this account. At least one of them must reach its threshold to delete this Claim from this account. This is intended to provide a revocation service: all the authorities agree to attach the hash, to attest to the fact that the credential or certificate is valid. Any one of the authorities can later delete the hash, to indicate that the credential has been revoked. In this way, any client can prove to a third party that any particular account has certain credentials, or to identity facts proved about it, and that none of them have been revoked yet. 
 */
struct  _Proto__Claim
{
  ProtobufCMessage base;
  /*
   *the account to which the claim is attached
   */
  Proto__AccountID *accountid;
  /*
   * 48 byte SHA-384 hash (presumably of some kind of credential or certificate)
   */
  ProtobufCBinaryData hash;
  /*
   * list of keys: all must sign the transaction to attach the claim, and any one of them can later delete it. Each "key" can actually be a threshold key containing multiple other keys (including other threshold keys).
   */
  Proto__KeyList *keys;
  /*
   * the duration for which the claim will remain valid
   */
  Proto__Duration *claimduration;
};
#define PROTO__CLAIM__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__claim__descriptor) \
    , NULL, {0,NULL}, NULL, NULL }


/*
 * Attach the given hash to the given account. The hash can be deleted by the keys used to transfer money from the account. The hash can also be deleted by any one of the deleteKeys (where that one may itself be a threshold key made up of multiple keys). Therefore, this acts as a revocation service for claims about the account. External authorities may issue certificates or credentials of some kind that make a claim about this account. The account owner can then attach a hash of that claim to the account. The transaction that adds the claim will be signed by the owner of the account, and also by all the authorities that are attesting to the truth of that claim. If the claim ever ceases to be true, such as when a certificate is revoked, then any one of the listed authorities has the ability to delete it. The account owner also has the ability to delete it at any time.
 * In this way, it acts as a revocation server, and the account owner can prove to any third party that the claim is still true for this account, by sending the third party the signed credential, and then having the third party query to discover whether the hash of that credential is still attached to the account.
 * For a given account, each Claim must contain a different hash. To modify the list of keys in a Claim, the existing Claim should first be deleted, then the Claim with the new list of keys can be added. 
 */
struct  _Proto__CryptoAddClaimTransactionBody
{
  ProtobufCMessage base;
  /*
   * A hash of some credential/certificate, along with the keys that authorized it and are allowed to delete it
   */
  Proto__Claim *claim;
};
#define PROTO__CRYPTO_ADD_CLAIM_TRANSACTION_BODY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&proto__crypto_add_claim_transaction_body__descriptor) \
    , NULL }


/* Proto__Claim methods */
void   proto__claim__init
                     (Proto__Claim         *message);
size_t proto__claim__get_packed_size
                     (const Proto__Claim   *message);
size_t proto__claim__pack
                     (const Proto__Claim   *message,
                      uint8_t             *out);
size_t proto__claim__pack_to_buffer
                     (const Proto__Claim   *message,
                      ProtobufCBuffer     *buffer);
Proto__Claim *
       proto__claim__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__claim__free_unpacked
                     (Proto__Claim *message,
                      ProtobufCAllocator *allocator);
/* Proto__CryptoAddClaimTransactionBody methods */
void   proto__crypto_add_claim_transaction_body__init
                     (Proto__CryptoAddClaimTransactionBody         *message);
size_t proto__crypto_add_claim_transaction_body__get_packed_size
                     (const Proto__CryptoAddClaimTransactionBody   *message);
size_t proto__crypto_add_claim_transaction_body__pack
                     (const Proto__CryptoAddClaimTransactionBody   *message,
                      uint8_t             *out);
size_t proto__crypto_add_claim_transaction_body__pack_to_buffer
                     (const Proto__CryptoAddClaimTransactionBody   *message,
                      ProtobufCBuffer     *buffer);
Proto__CryptoAddClaimTransactionBody *
       proto__crypto_add_claim_transaction_body__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   proto__crypto_add_claim_transaction_body__free_unpacked
                     (Proto__CryptoAddClaimTransactionBody *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Proto__Claim_Closure)
                 (const Proto__Claim *message,
                  void *closure_data);
typedef void (*Proto__CryptoAddClaimTransactionBody_Closure)
                 (const Proto__CryptoAddClaimTransactionBody *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor proto__claim__descriptor;
extern const ProtobufCMessageDescriptor proto__crypto_add_claim_transaction_body__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_CryptoAddClaim_2eproto__INCLUDED */