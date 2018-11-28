#include "gabac/release.h"

#include <cassert>

#include "gabac/return_codes.h"


int gabac_release(
        void *chunk
){
    free(chunk);
    return GABAC_SUCCESS;
}
