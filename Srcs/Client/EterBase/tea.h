#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* TEA is a 64-bit symmetric block cipher with a 128-bit key, developed
       by David J. Wheeler and Roger M. Needham, and described in their
       paper at <URL:http://www.cl.cam.ac.uk/ftp/users/djw3/tea.ps>.

       This implementation is based on their code in
       <URL:http://www.cl.cam.ac.uk/ftp/users/djw3/xtea.ps> */

    #define TEA_KEY_LENGTH 16

    int tea_encrypt(uint32_t *dest, const uint32_t *src, const uint32_t *key, int size);
    int tea_decrypt(uint32_t *dest, const uint32_t *src, const uint32_t *key, int size);

#ifdef __cplusplus
}

// Overload for callers that explicitly cast to (unsigned long*) such as PythonApplicationModule.cpp
static inline int tea_encrypt(unsigned long *dest, const unsigned long *src, const unsigned long *key, int size) {
    return tea_encrypt(reinterpret_cast<uint32_t*>(dest), reinterpret_cast<const uint32_t*>(src), reinterpret_cast<const uint32_t*>(key), size);
}

static inline int tea_decrypt(unsigned long *dest, const unsigned long *src, const unsigned long *key, int size) {
    return tea_decrypt(reinterpret_cast<uint32_t*>(dest), reinterpret_cast<const uint32_t*>(src), reinterpret_cast<const uint32_t*>(key), size);
}
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075