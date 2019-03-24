#ifndef GABACIFY_ANALYSIS_H_
#define GABACIFY_ANALYSIS_H_

namespace gabac {

struct AnalysisConfiguration;
struct IOConfiguration;

const AnalysisConfiguration& getCandidateConfig();

void analyze(const IOConfiguration& ioconf,
               const AnalysisConfiguration& aconf
);

}


#endif  // GABACIFY_ANALYSIS_H_
