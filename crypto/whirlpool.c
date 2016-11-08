/*
* Whirlpool hash in C
*
* Copyright (c) 2014 Project Nayuki
* https://www.nayuki.io/page/fast-whirlpool-hash-in-x86-assembly
*
* (MIT License)
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
* - The above copyright notice and this permission notice shall be included in
*   all copies or substantial portions of the Software.
* - The Software is provided "as is", without warranty of any kind, express or
*   implied, including but not limited to the warranties of merchantability,
*   fitness for a particular purpose and noninfringement. In no event shall the
*   authors or copyright holders be liable for any claim, damages or other
*   liability, whether in an action of contract, tort or otherwise, arising from,
*   out of or in connection with the Software or the use or other dealings in the
*   Software.
*/

#include <stdint.h>
#include <string.h>


static void whirlpool_round(uint64_t block[8], const uint64_t key[8]);


static uint64_t ROUND_CONSTANTS[32] = {
	UINT64_C(0x4F01B887E8C62318), UINT64_C(0x52916F79F5D2A636), UINT64_C(0x357B0CA38E9BBC60), UINT64_C(0x57FE4B2EC2D7E01D),
	UINT64_C(0xDA4AF09FE5377715), UINT64_C(0x856BA0B10A29C958), UINT64_C(0x67053ECBF4105DBD), UINT64_C(0xD8957DA78B4127E4),
	UINT64_C(0x9E4717DD667CEEFB), UINT64_C(0x33835AAD07BF2DCA), UINT64_C(0xD94919C871AA0263), UINT64_C(0xB032269A885BE3F2),
	UINT64_C(0x4834CDBE80D50FE9), UINT64_C(0xAE1A68205F907AFF), UINT64_C(0x1273F164229354B4), UINT64_C(0x3D8DA1DBECC30840),
	UINT64_C(0x1BD682762BCF0097), UINT64_C(0xEF30F345506AAFB5), UINT64_C(0xC02FBA65EAA2553F), UINT64_C(0x8A0675924DFD1CDE),
	UINT64_C(0x96A8D4621F0EE6B2), UINT64_C(0x4C3972845925C5F9), UINT64_C(0x61E2A5D18C38785E), UINT64_C(0x04FCC7431E9C21B3),
	UINT64_C(0x247EDFFA0D6D9951), UINT64_C(0xEBB74E8F11CEAB3B), UINT64_C(0xD32C13B9F794813C), UINT64_C(0xA97F445603C46EE7),
	UINT64_C(0x6C9D0BDC53C1BB2A), UINT64_C(0xE11489AC46F67431), UINT64_C(0xEDD0B67009693A16), UINT64_C(0x86F85C28A49842CC),
};


void whirlpool_compress(uint8_t state[64], const uint8_t block[64]) {
	const int NUM_ROUNDS = 10;  // Any number from 0 to 32 is allowed
	uint64_t tempState[8];
	uint64_t tempBlock[8];
	int i;

	// Initialization
	for (i = 0; i < 8; i++) {
		int j = i << 3;
		uint64_t x = (uint64_t)state[j + 0] << 0
			| (uint64_t)state[j + 1] << 8
			| (uint64_t)state[j + 2] << 16
			| (uint64_t)state[j + 3] << 24
			| (uint64_t)state[j + 4] << 32
			| (uint64_t)state[j + 5] << 40
			| (uint64_t)state[j + 6] << 48
			| (uint64_t)state[j + 7] << 56;
		uint64_t y = (uint64_t)block[j + 0] << 0
			| (uint64_t)block[j + 1] << 8
			| (uint64_t)block[j + 2] << 16
			| (uint64_t)block[j + 3] << 24
			| (uint64_t)block[j + 4] << 32
			| (uint64_t)block[j + 5] << 40
			| (uint64_t)block[j + 6] << 48
			| (uint64_t)block[j + 7] << 56;
		tempState[i] = x;
		tempBlock[i] = x ^ y;
	}

	// Hashing rounds
	uint64_t rcon[8];
	memset(rcon + 1, 0, sizeof(rcon[0]) * 7);
	for (i = 0; i < NUM_ROUNDS; i++) {
		rcon[0] = ROUND_CONSTANTS[i];
		whirlpool_round(tempState, rcon);
		whirlpool_round(tempBlock, tempState);
	}

	// Final combining
	for (i = 0; i < 64; i++)
		state[i] ^= block[i] ^ (uint8_t)(tempBlock[i >> 3] >> ((i & 7) << 3));
}


// The combined effect of gamma (SubBytes) and theta (MixRows)
static uint64_t MAGIC_TABLE[256] = {
	UINT64_C(0xD83078C018601818), UINT64_C(0x2646AF05238C2323), UINT64_C(0xB891F97EC63FC6C6), UINT64_C(0xFBCD6F13E887E8E8), UINT64_C(0xCB13A14C87268787), UINT64_C(0x116D62A9B8DAB8B8), UINT64_C(0x0902050801040101), UINT64_C(0x0D9E6E424F214F4F),
	UINT64_C(0x9B6CEEAD36D83636), UINT64_C(0xFF510459A6A2A6A6), UINT64_C(0x0CB9BDDED26FD2D2), UINT64_C(0x0EF706FBF5F3F5F5), UINT64_C(0x96F280EF79F97979), UINT64_C(0x30DECE5F6FA16F6F), UINT64_C(0x6D3FEFFC917E9191), UINT64_C(0xF8A407AA52555252),
	UINT64_C(0x47C0FD27609D6060), UINT64_C(0x35657689BCCABCBC), UINT64_C(0x372BCDAC9B569B9B), UINT64_C(0x8A018C048E028E8E), UINT64_C(0xD25B1571A3B6A3A3), UINT64_C(0x6C183C600C300C0C), UINT64_C(0x84F68AFF7BF17B7B), UINT64_C(0x806AE1B535D43535),
	UINT64_C(0xF53A69E81D741D1D), UINT64_C(0xB3DD4753E0A7E0E0), UINT64_C(0x21B3ACF6D77BD7D7), UINT64_C(0x9C99ED5EC22FC2C2), UINT64_C(0x435C966D2EB82E2E), UINT64_C(0x29967A624B314B4B), UINT64_C(0x5DE121A3FEDFFEFE), UINT64_C(0xD5AE168257415757),
	UINT64_C(0xBD2A41A815541515), UINT64_C(0xE8EEB69F77C17777), UINT64_C(0x926EEBA537DC3737), UINT64_C(0x9ED7567BE5B3E5E5), UINT64_C(0x1323D98C9F469F9F), UINT64_C(0x23FD17D3F0E7F0F0), UINT64_C(0x20947F6A4A354A4A), UINT64_C(0x44A9959EDA4FDADA),
	UINT64_C(0xA2B025FA587D5858), UINT64_C(0xCF8FCA06C903C9C9), UINT64_C(0x7C528D5529A42929), UINT64_C(0x5A1422500A280A0A), UINT64_C(0x507F4FE1B1FEB1B1), UINT64_C(0xC95D1A69A0BAA0A0), UINT64_C(0x14D6DA7F6BB16B6B), UINT64_C(0xD917AB5C852E8585),
	UINT64_C(0x3C677381BDCEBDBD), UINT64_C(0x8FBA34D25D695D5D), UINT64_C(0x9020508010401010), UINT64_C(0x07F503F3F4F7F4F4), UINT64_C(0xDD8BC016CB0BCBCB), UINT64_C(0xD37CC6ED3EF83E3E), UINT64_C(0x2D0A112805140505), UINT64_C(0x78CEE61F67816767),
	UINT64_C(0x97D55373E4B7E4E4), UINT64_C(0x024EBB25279C2727), UINT64_C(0x7382583241194141), UINT64_C(0xA70B9D2C8B168B8B), UINT64_C(0xF6530151A7A6A7A7), UINT64_C(0xB2FA94CF7DE97D7D), UINT64_C(0x4937FBDC956E9595), UINT64_C(0x56AD9F8ED847D8D8),
	UINT64_C(0x70EB308BFBCBFBFB), UINT64_C(0xCDC17123EE9FEEEE), UINT64_C(0xBBF891C77CED7C7C), UINT64_C(0x71CCE31766856666), UINT64_C(0x7BA78EA6DD53DDDD), UINT64_C(0xAF2E4BB8175C1717), UINT64_C(0x458E460247014747), UINT64_C(0x1A21DC849E429E9E),
	UINT64_C(0xD489C51ECA0FCACA), UINT64_C(0x585A99752DB42D2D), UINT64_C(0x2E637991BFC6BFBF), UINT64_C(0x3F0E1B38071C0707), UINT64_C(0xAC472301AD8EADAD), UINT64_C(0xB0B42FEA5A755A5A), UINT64_C(0xEF1BB56C83368383), UINT64_C(0xB666FF8533CC3333),
	UINT64_C(0x5CC6F23F63916363), UINT64_C(0x12040A1002080202), UINT64_C(0x93493839AA92AAAA), UINT64_C(0xDEE2A8AF71D97171), UINT64_C(0xC68DCF0EC807C8C8), UINT64_C(0xD1327DC819641919), UINT64_C(0x3B92707249394949), UINT64_C(0x5FAF9A86D943D9D9),
	UINT64_C(0x31F91DC3F2EFF2F2), UINT64_C(0xA8DB484BE3ABE3E3), UINT64_C(0xB9B62AE25B715B5B), UINT64_C(0xBC0D9234881A8888), UINT64_C(0x3E29C8A49A529A9A), UINT64_C(0x0B4CBE2D26982626), UINT64_C(0xBF64FA8D32C83232), UINT64_C(0x597D4AE9B0FAB0B0),
	UINT64_C(0xF2CF6A1BE983E9E9), UINT64_C(0x771E33780F3C0F0F), UINT64_C(0x33B7A6E6D573D5D5), UINT64_C(0xF41DBA74803A8080), UINT64_C(0x27617C99BEC2BEBE), UINT64_C(0xEB87DE26CD13CDCD), UINT64_C(0x8968E4BD34D03434), UINT64_C(0x3290757A483D4848),
	UINT64_C(0x54E324ABFFDBFFFF), UINT64_C(0x8DF48FF77AF57A7A), UINT64_C(0x643DEAF4907A9090), UINT64_C(0x9DBE3EC25F615F5F), UINT64_C(0x3D40A01D20802020), UINT64_C(0x0FD0D56768BD6868), UINT64_C(0xCA3472D01A681A1A), UINT64_C(0xB7412C19AE82AEAE),
	UINT64_C(0x7D755EC9B4EAB4B4), UINT64_C(0xCEA8199A544D5454), UINT64_C(0x7F3BE5EC93769393), UINT64_C(0x2F44AA0D22882222), UINT64_C(0x63C8E907648D6464), UINT64_C(0x2AFF12DBF1E3F1F1), UINT64_C(0xCCE6A2BF73D17373), UINT64_C(0x82245A9012481212),
	UINT64_C(0x7A805D3A401D4040), UINT64_C(0x4810284008200808), UINT64_C(0x959BE856C32BC3C3), UINT64_C(0xDFC57B33EC97ECEC), UINT64_C(0x4DAB9096DB4BDBDB), UINT64_C(0xC05F1F61A1BEA1A1), UINT64_C(0x9107831C8D0E8D8D), UINT64_C(0xC87AC9F53DF43D3D),
	UINT64_C(0x5B33F1CC97669797), UINT64_C(0x0000000000000000), UINT64_C(0xF983D436CF1BCFCF), UINT64_C(0x6E5687452BAC2B2B), UINT64_C(0xE1ECB39776C57676), UINT64_C(0xE619B06482328282), UINT64_C(0x28B1A9FED67FD6D6), UINT64_C(0xC33677D81B6C1B1B),
	UINT64_C(0x74775BC1B5EEB5B5), UINT64_C(0xBE432911AF86AFAF), UINT64_C(0x1DD4DF776AB56A6A), UINT64_C(0xEAA00DBA505D5050), UINT64_C(0x578A4C1245094545), UINT64_C(0x38FB18CBF3EBF3F3), UINT64_C(0xAD60F09D30C03030), UINT64_C(0xC4C3742BEF9BEFEF),
	UINT64_C(0xDA7EC3E53FFC3F3F), UINT64_C(0xC7AA1C9255495555), UINT64_C(0xDB591079A2B2A2A2), UINT64_C(0xE9C96503EA8FEAEA), UINT64_C(0x6ACAEC0F65896565), UINT64_C(0x036968B9BAD2BABA), UINT64_C(0x4A5E93652FBC2F2F), UINT64_C(0x8E9DE74EC027C0C0),
	UINT64_C(0x60A181BEDE5FDEDE), UINT64_C(0xFC386CE01C701C1C), UINT64_C(0x46E72EBBFDD3FDFD), UINT64_C(0x1F9A64524D294D4D), UINT64_C(0x7639E0E492729292), UINT64_C(0xFAEABC8F75C97575), UINT64_C(0x360C1E3006180606), UINT64_C(0xAE0998248A128A8A),
	UINT64_C(0x4B7940F9B2F2B2B2), UINT64_C(0x85D15963E6BFE6E6), UINT64_C(0x7E1C36700E380E0E), UINT64_C(0xE73E63F81F7C1F1F), UINT64_C(0x55C4F73762956262), UINT64_C(0x3AB5A3EED477D4D4), UINT64_C(0x814D3229A89AA8A8), UINT64_C(0x5231F4C496629696),
	UINT64_C(0x62EF3A9BF9C3F9F9), UINT64_C(0xA397F666C533C5C5), UINT64_C(0x104AB13525942525), UINT64_C(0xABB220F259795959), UINT64_C(0xD015AE54842A8484), UINT64_C(0xC5E4A7B772D57272), UINT64_C(0xEC72DDD539E43939), UINT64_C(0x1698615A4C2D4C4C),
	UINT64_C(0x94BC3BCA5E655E5E), UINT64_C(0x9FF085E778FD7878), UINT64_C(0xE570D8DD38E03838), UINT64_C(0x980586148C0A8C8C), UINT64_C(0x17BFB2C6D163D1D1), UINT64_C(0xE4570B41A5AEA5A5), UINT64_C(0xA1D94D43E2AFE2E2), UINT64_C(0x4EC2F82F61996161),
	UINT64_C(0x427B45F1B3F6B3B3), UINT64_C(0x3442A51521842121), UINT64_C(0x0825D6949C4A9C9C), UINT64_C(0xEE3C66F01E781E1E), UINT64_C(0x6186522243114343), UINT64_C(0xB193FC76C73BC7C7), UINT64_C(0x4FE52BB3FCD7FCFC), UINT64_C(0x2408142004100404),
	UINT64_C(0xE3A208B251595151), UINT64_C(0x252FC7BC995E9999), UINT64_C(0x22DAC44F6DA96D6D), UINT64_C(0x651A39680D340D0D), UINT64_C(0x79E93583FACFFAFA), UINT64_C(0x69A384B6DF5BDFDF), UINT64_C(0xA9FC9BD77EE57E7E), UINT64_C(0x1948B43D24902424),
	UINT64_C(0xFE76D7C53BEC3B3B), UINT64_C(0x9A4B3D31AB96ABAB), UINT64_C(0xF081D13ECE1FCECE), UINT64_C(0x9922558811441111), UINT64_C(0x8303890C8F068F8F), UINT64_C(0x049C6B4A4E254E4E), UINT64_C(0x667351D1B7E6B7B7), UINT64_C(0xE0CB600BEB8BEBEB),
	UINT64_C(0xC178CCFD3CF03C3C), UINT64_C(0xFD1FBF7C813E8181), UINT64_C(0x4035FED4946A9494), UINT64_C(0x1CF30CEBF7FBF7F7), UINT64_C(0x186F67A1B9DEB9B9), UINT64_C(0x8B265F98134C1313), UINT64_C(0x51589C7D2CB02C2C), UINT64_C(0x05BBB8D6D36BD3D3),
	UINT64_C(0x8CD35C6BE7BBE7E7), UINT64_C(0x39DCCB576EA56E6E), UINT64_C(0xAA95F36EC437C4C4), UINT64_C(0x1B060F18030C0303), UINT64_C(0xDCAC138A56455656), UINT64_C(0x5E88491A440D4444), UINT64_C(0xA0FE9EDF7FE17F7F), UINT64_C(0x884F3721A99EA9A9),
	UINT64_C(0x6754824D2AA82A2A), UINT64_C(0x0A6B6DB1BBD6BBBB), UINT64_C(0x879FE246C123C1C1), UINT64_C(0xF1A602A253515353), UINT64_C(0x72A58BAEDC57DCDC), UINT64_C(0x531627580B2C0B0B), UINT64_C(0x0127D39C9D4E9D9D), UINT64_C(0x2BD8C1476CAD6C6C),
	UINT64_C(0xA462F59531C43131), UINT64_C(0xF3E8B98774CD7474), UINT64_C(0x15F109E3F6FFF6F6), UINT64_C(0x4C8C430A46054646), UINT64_C(0xA5452609AC8AACAC), UINT64_C(0xB50F973C891E8989), UINT64_C(0xB42844A014501414), UINT64_C(0xBADF425BE1A3E1E1),
	UINT64_C(0xA62C4EB016581616), UINT64_C(0xF774D2CD3AE83A3A), UINT64_C(0x06D2D06F69B96969), UINT64_C(0x41122D4809240909), UINT64_C(0xD7E0ADA770DD7070), UINT64_C(0x6F7154D9B6E2B6B6), UINT64_C(0x1EBDB7CED067D0D0), UINT64_C(0xD6C77E3BED93EDED),
	UINT64_C(0xE285DB2ECC17CCCC), UINT64_C(0x6884572A42154242), UINT64_C(0x2C2DC2B4985A9898), UINT64_C(0xED550E49A4AAA4A4), UINT64_C(0x7550885D28A02828), UINT64_C(0x86B831DA5C6D5C5C), UINT64_C(0x6BED3F93F8C7F8F8), UINT64_C(0xC211A44486228686),
};


static void whirlpool_round(uint64_t block[8], const uint64_t key[8]) {
	uint64_t a = block[0];
	uint64_t b = block[1];
	uint64_t c = block[2];
	uint64_t d = block[3];
	uint64_t e = block[4];
	uint64_t f = block[5];
	uint64_t g = block[6];
	uint64_t h = block[7];

	uint64_t r;
#define ROTR64(x, n)  (((0U + (x)) << (64 - (n))) | ((x) >> (n)))  // Assumes that x is uint64_t and 0 < n < 64
#define DOROW(i, s, t, u, v, w, x, y, z)  \
		r = MAGIC_TABLE[(uint8_t)s];  r = ROTR64(r, 8);  \
		r ^= MAGIC_TABLE[(uint8_t)(t >>  8)];  r = ROTR64(r, 8);  \
		r ^= MAGIC_TABLE[(uint8_t)(u >> 16)];  r = ROTR64(r, 8);  \
		r ^= MAGIC_TABLE[(uint8_t)(v >> 24)];  r = ROTR64(r, 8);  \
		r ^= MAGIC_TABLE[(uint8_t)(w >> 32)];  r = ROTR64(r, 8);  \
		r ^= MAGIC_TABLE[(uint8_t)(x >> 40)];  r = ROTR64(r, 8);  \
		r ^= MAGIC_TABLE[(uint8_t)(y >> 48)];  r = ROTR64(r, 8);  \
		r ^= MAGIC_TABLE[(uint8_t)(z >> 56)];  r = ROTR64(r, 8);  \
		block[i] = r ^ key[i];

	DOROW(0, a, h, g, f, e, d, c, b)
		DOROW(1, b, a, h, g, f, e, d, c)
		DOROW(2, c, b, a, h, g, f, e, d)
		DOROW(3, d, c, b, a, h, g, f, e)
		DOROW(4, e, d, c, b, a, h, g, f)
		DOROW(5, f, e, d, c, b, a, h, g)
		DOROW(6, g, f, e, d, c, b, a, h)
		DOROW(7, h, g, f, e, d, c, b, a)
}

extern void whirlpool_hash(const uint8_t *message, uint32_t len, uint8_t *hash) {
	memset(hash, 0, 64);

	uint32_t i;
	for (i = 0; len - i >= 64; i += 64)
		whirlpool_compress(hash, message + i);

	uint8_t block[64];
	uint32_t rem = len - i;
	memcpy(block, message + i, rem);

	block[rem] = 0x80;
	rem++;
	if (64 - rem >= 32)
		memset(block + rem, 0, 56 - rem);
	else {
		memset(block + rem, 0, 64 - rem);
		whirlpool_compress(hash, block);
		memset(block, 0, 56);
	}

	uint64_t longLen = ((uint64_t)len) << 3;
	for (i = 0; i < 8; i++)
		block[64 - 1 - i] = (uint8_t)(longLen >> (i * 8));
	whirlpool_compress(hash, block);
}