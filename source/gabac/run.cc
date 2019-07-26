#include "run.h"

#include "decoding.h"
#include "encoding.h"

namespace gabac {

void run(const IOConfiguration& conf, const EncodingConfiguration& enConf,
         bool decode) {
    if (decode) {
        gabac::decode(conf, enConf);
    } else {
        gabac::encode(conf, enConf);
    }
}

}  // namespace gabac
