#ifndef _DUMMY_RAMDOM_H_
#define _DUMMY_RAMDOM_H_

#include "mbed.h"

extern "C" {

void arm_random_module_init(void)
{
}

uint32_t arm_random_seed_get(void)
{
    uint32_t result = 0;
    return result;
}

}

#endif  // _DUMMY_RAMDOM_H_
