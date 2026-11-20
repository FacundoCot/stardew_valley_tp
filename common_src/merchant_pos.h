#ifndef MERCHANT_POS_H
#define MERCHANT_POS_H

#include <cstdint>

enum class MerchantType : uint8_t {
    PIERRE = 0,
    ROBIN = 1,
    MARNIE = 2,
    CLINT = 3
};

struct MerchantPosition {
    MerchantType type;
    float x;
    float y;
};

#endif