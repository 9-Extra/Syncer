/* MD5
 converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 for bzflag (http://www.bzflag.org)

   based on:

   md5.h and md5.c
   reference implementation of RFC 1321

   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/

#ifndef COMMON_SOURCE_MD5_H
#define COMMON_SOURCE_MD5_H

#include <string>
#include <vector>
#include <assert.h>

namespace md5 {

// a small class for calculating MD5 hashes of strings or byte arrays
// it is not meant to be fast or secure
//
// usage: 1) feed it blocks of uchars with update()
//      2) finalize()
//      3) get hexdigest() string
//      or
//      MD5(std::string).hexdigest()
//
// assumes that char is 8 bit and int is 32 bit
class MD5 {
public:
    typedef unsigned int size_type; // must be 32bit

    MD5();
    MD5(const std::string &input);
    void update(const unsigned char *buf, size_type length);
    void update(const char *buf, size_type length);
    MD5 &finalize();
    std::string hexdigest() const;
    std::vector<unsigned char> raw_digest() const{
        assert(finalized);
        std::vector<unsigned char> v(16);
        for(size_t i = 0;i < 16;i++){
            v[i] = digestRaw[i];
        }
        return v;
    }
    friend std::string digestString(const void *input, unsigned int length);
    friend std::string digest(const void *input, unsigned int length);

private:
    MD5(const void *input, size_type length);

    void init();
    typedef unsigned char uint1; //  8bit
    typedef unsigned int uint4;  // 32bit
    enum { blocksize = 64 };     // VC6 won't eat a const static int here

    void transform(const uint1 block[blocksize]);
    static void decode(uint4 output[], const uint1 input[], size_type len);
    static void encode(uint1 output[], const uint4 input[], size_type len);

    bool finalized;
    uint1 buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
    uint4 count[2];          // 64bit counter for number of bits (lo, hi)
    uint4 state[4];          // digestRaw so far
    uint1 digestRaw[16];     // the result

    // low level logic operations
    static inline uint4 F(uint4 x, uint4 y, uint4 z);
    static inline uint4 G(uint4 x, uint4 y, uint4 z);
    static inline uint4 H(uint4 x, uint4 y, uint4 z);
    static inline uint4 I(uint4 x, uint4 y, uint4 z);
    static inline uint4 rotate_left(uint4 x, int n);
    static inline void FF(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
    static inline void GG(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
    static inline void HH(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
    static inline void II(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
};

std::string digestString(const std::string &input);
/**
 * 生成指定数据的MD5并转为HEX字符串返回,为null时返回空对象
 */
std::string digestString(const void *input, unsigned int length);
std::string digest(const std::string &input);
/**
 * 生成指定数据的MD5,为null时返回空对象
 */
std::string digest(const void *input, unsigned int length);

} /* namespace md5 */
#endif /* COMMON_SOURCE_MD5_H */