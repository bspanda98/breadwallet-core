/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: QueryHeader.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "QueryHeader.pb-c.h"
void   proto__query_header__init
                     (Proto__QueryHeader         *message)
{
  static const Proto__QueryHeader init_value = PROTO__QUERY_HEADER__INIT;
  *message = init_value;
}
size_t proto__query_header__get_packed_size
                     (const Proto__QueryHeader *message)
{
  assert(message->base.descriptor == &proto__query_header__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t proto__query_header__pack
                     (const Proto__QueryHeader *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &proto__query_header__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t proto__query_header__pack_to_buffer
                     (const Proto__QueryHeader *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &proto__query_header__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Proto__QueryHeader *
       proto__query_header__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Proto__QueryHeader *)
     protobuf_c_message_unpack (&proto__query_header__descriptor,
                                allocator, len, data);
}
void   proto__query_header__free_unpacked
                     (Proto__QueryHeader *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &proto__query_header__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor proto__query_header__field_descriptors[2] =
{
  {
    "payment",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Proto__QueryHeader, payment),
    &proto__transaction__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "responseType",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_ENUM,
    0,   /* quantifier_offset */
    offsetof(Proto__QueryHeader, responsetype),
    &proto__response_type__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned proto__query_header__field_indices_by_name[] = {
  0,   /* field[0] = payment */
  1,   /* field[1] = responseType */
};
static const ProtobufCIntRange proto__query_header__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor proto__query_header__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "proto.QueryHeader",
  "QueryHeader",
  "Proto__QueryHeader",
  "proto",
  sizeof(Proto__QueryHeader),
  2,
  proto__query_header__field_descriptors,
  proto__query_header__field_indices_by_name,
  1,  proto__query_header__number_ranges,
  (ProtobufCMessageInit) proto__query_header__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCEnumValue proto__response_type__enum_values_by_number[4] =
{
  { "ANSWER_ONLY", "PROTO__RESPONSE_TYPE__ANSWER_ONLY", 0 },
  { "ANSWER_STATE_PROOF", "PROTO__RESPONSE_TYPE__ANSWER_STATE_PROOF", 1 },
  { "COST_ANSWER", "PROTO__RESPONSE_TYPE__COST_ANSWER", 2 },
  { "COST_ANSWER_STATE_PROOF", "PROTO__RESPONSE_TYPE__COST_ANSWER_STATE_PROOF", 3 },
};
static const ProtobufCIntRange proto__response_type__value_ranges[] = {
{0, 0},{0, 4}
};
static const ProtobufCEnumValueIndex proto__response_type__enum_values_by_name[4] =
{
  { "ANSWER_ONLY", 0 },
  { "ANSWER_STATE_PROOF", 1 },
  { "COST_ANSWER", 2 },
  { "COST_ANSWER_STATE_PROOF", 3 },
};
const ProtobufCEnumDescriptor proto__response_type__descriptor =
{
  PROTOBUF_C__ENUM_DESCRIPTOR_MAGIC,
  "proto.ResponseType",
  "ResponseType",
  "Proto__ResponseType",
  "proto",
  4,
  proto__response_type__enum_values_by_number,
  4,
  proto__response_type__enum_values_by_name,
  1,
  proto__response_type__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};