//
// Created by apridgen on 6/5/21.
//
#include <cstdint>
#include "util.h"

namespace simple_buffer {
    void convert(char *src, char *dst, std::size_t sz) {
        if (sz == 2) {
            *(dst+0) = *(src+1);
            *(dst+1) = *(src+0);
        } else if (sz == 4) {
            *(dst+0) = *(src+3);
            *(dst+1) = *(src+2);
            *(dst+2) = *(src+1);
            *(dst+3) = *(src+0);
        } else if (sz == 8) {
            *(dst+0) = *(src+7);
            *(dst+1) = *(src+6);
            *(dst+2) = *(src+5);
            *(dst+3) = *(src+4);
            *(dst+4) = *(src+3);
            *(dst+5) = *(src+2);
            *(dst+6) = *(src+1);
            *(dst+7) = *(src+0);
        }
    }
}