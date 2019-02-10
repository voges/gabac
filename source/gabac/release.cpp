#include "gabac/release.h"
#include "constants.h"

#include <cassert>



int gabac_release(
        void *chunk
){
    free(chunk);
    return GABAC_SUCCESS;
}
