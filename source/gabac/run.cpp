#include "gabac/run.h"

#include "encoding.h"
#include "decoding.h"

namespace gabac {
void run(
        const IOConfiguration& conf,
        const EncodingConfiguration& enConf,
        bool decode
) {
    if(decode) {
        gabac::decode(conf, enConf);
    } else {
        gabac::encode(conf, enConf);
    }
}

}  // namespace gabac
