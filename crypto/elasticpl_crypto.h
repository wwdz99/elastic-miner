/*
* Copyright 2016 sprocket
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*/

#ifndef ELASTICPLCRYPTO_H_
#define ELASTICPLCRYPTO_H_

#include <stdint.h>

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>

extern void whirlpool_hash(const uint8_t *message, uint32_t len, uint8_t *hash);

extern uint32_t epl_sha256(int idx, int len, int32_t *mem);
extern uint32_t epl_sha512(int idx, int len, int32_t *mem);
extern uint32_t epl_md5(int idx, int len, int32_t *mem);
extern uint32_t epl_ripemd160(int idx, int len, int32_t *mem);
extern uint32_t epl_whirlpool(int idx, int len, int32_t *mem);
extern uint32_t epl_ec_priv_to_pub(size_t idx, bool compressed, int32_t *mem, int nid, size_t len);
extern uint32_t epl_ec_add(size_t idx1, bool comp1, size_t idx2, bool comp2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz);
extern uint32_t epl_ec_sub(size_t idx1, bool comp1, size_t idx2, bool comp2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz);
extern uint32_t epl_ec_neg(size_t idx1, bool comp1, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz);
extern uint32_t epl_ec_mult(size_t idx1, bool comp1, size_t idx2, size_t n2, bool comp, int32_t *mem, int nid, size_t comp_sz, size_t uncomp_sz);

#endif // ELASTICPLCRYPTO_H_
