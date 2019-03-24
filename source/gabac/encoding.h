#ifndef GABAC_ENCODING_H_
#define GABAC_ENCODING_H_

namespace gabac {

class IOConfiguration;
class EncodingConfiguration;

void encode(
        const IOConfiguration& ioConf,
        const EncodingConfiguration& enConf
);

}  // namespace gabac

#endif  /* GABAC_ENCODING_H_ */
