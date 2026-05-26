/**
 * @file SHA256.h
 * @brief SHA-256 mock for native unit testing.
 *
 * Implements a real SHA-256 so that password-hashing tests produce correct,
 * deterministic results without requiring the Crypto library.
 */
#pragma once

#include <cstdint>
#include <cstring>

class SHA256 {
public:
    SHA256() { reset(); }

    void reset() {
        _state[0] = 0x6a09e667;
        _state[1] = 0xbb67ae85;
        _state[2] = 0x3c6ef372;
        _state[3] = 0xa54ff53a;
        _state[4] = 0x510e527f;
        _state[5] = 0x9b05688c;
        _state[6] = 0x1f83d9ab;
        _state[7] = 0x5be0cd19;
        _length = 0;
        _bufLen = 0;
    }

    void update(const void* data, size_t len) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
        _length += len;
        while (len > 0) {
            size_t space = 64 - _bufLen;
            size_t copy  = len < space ? len : space;
            memcpy(_buf + _bufLen, ptr, copy);
            _bufLen += copy;
            ptr += copy;
            len -= copy;
            if (_bufLen == 64) {
                processBlock(_buf);
                _bufLen = 0;
            }
        }
    }

    void finalize(void* digest, size_t) {
        uint8_t tmp[64];
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, _buf, _bufLen);
        tmp[_bufLen] = 0x80;
        if (_bufLen >= 56) {
            processBlock(tmp);
            memset(tmp, 0, sizeof(tmp));
        }
        uint64_t bitLen = _length * 8;
        for (int i = 0; i < 8; i++)
            tmp[63 - i] = (uint8_t)(bitLen >> (i * 8));
        processBlock(tmp);

        uint8_t* out = reinterpret_cast<uint8_t*>(digest);
        for (int i = 0; i < 8; i++) {
            out[i * 4]     = (_state[i] >> 24) & 0xFF;
            out[i * 4 + 1] = (_state[i] >> 16) & 0xFF;
            out[i * 4 + 2] = (_state[i] >>  8) & 0xFF;
            out[i * 4 + 3] =  _state[i]        & 0xFF;
        }
    }

private:
    uint32_t _state[8];
    uint8_t  _buf[64];
    size_t   _bufLen;
    uint64_t _length;

    static uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
    static uint32_t ch(uint32_t e, uint32_t f, uint32_t g)  { return (e & f) ^ (~e & g); }
    static uint32_t maj(uint32_t a, uint32_t b, uint32_t c) { return (a & b) ^ (a & c) ^ (b & c); }
    static uint32_t ep0(uint32_t a) { return rotr(a,2)^rotr(a,13)^rotr(a,22); }
    static uint32_t ep1(uint32_t e) { return rotr(e,6)^rotr(e,11)^rotr(e,25); }
    static uint32_t sig0(uint32_t x){ return rotr(x,7)^rotr(x,18)^(x>>3); }
    static uint32_t sig1(uint32_t x){ return rotr(x,17)^rotr(x,19)^(x>>10); }

    static const uint32_t K[64];

    void processBlock(const uint8_t* block) {
        uint32_t w[64];
        for (int i = 0; i < 16; i++) {
            w[i] = ((uint32_t)block[i*4] << 24) |
                   ((uint32_t)block[i*4+1] << 16) |
                   ((uint32_t)block[i*4+2] << 8) |
                    (uint32_t)block[i*4+3];
        }
        for (int i = 16; i < 64; i++)
            w[i] = sig1(w[i-2]) + w[i-7] + sig0(w[i-15]) + w[i-16];

        uint32_t a=_state[0], b=_state[1], c=_state[2], d=_state[3];
        uint32_t e=_state[4], f=_state[5], g=_state[6], h=_state[7];

        for (int i = 0; i < 64; i++) {
            uint32_t t1 = h + ep1(e) + ch(e,f,g) + K[i] + w[i];
            uint32_t t2 = ep0(a) + maj(a,b,c);
            h=g; g=f; f=e; e=d+t1;
            d=c; c=b; b=a; a=t1+t2;
        }
        _state[0]+=a; _state[1]+=b; _state[2]+=c; _state[3]+=d;
        _state[4]+=e; _state[5]+=f; _state[6]+=g; _state[7]+=h;
    }
};

inline const uint32_t SHA256::K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,
    0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,
    0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,
    0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,
    0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,
    0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};