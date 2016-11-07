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

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>

extern uint32_t epl_sha256(int idx, int len, int32_t *mem);
extern uint32_t epl_ec_priv_to_pub(size_t idx, bool compressed, int32_t *mem, int nid, size_t len);

#endif // ELASTICPLCRYPTO_H_
