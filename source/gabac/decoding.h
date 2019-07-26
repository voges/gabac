#ifndef GABAC_DECODING_H_
#define GABAC_DECODING_H_

namespace gabac {

struct EncodingConfiguration;
struct IOConfiguration;

void decode(const IOConfiguration& ioConf, const EncodingConfiguration& enConf);

}  // namespace gabac

#endif  // GABAC_DECODING_H_
