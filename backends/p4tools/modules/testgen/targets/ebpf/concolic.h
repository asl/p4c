#ifndef BACKENDS_P4TOOLS_MODULES_TESTGEN_TARGETS_EBPF_CONCOLIC_H_
#define BACKENDS_P4TOOLS_MODULES_TESTGEN_TARGETS_EBPF_CONCOLIC_H_

#include "backends/p4tools/modules/testgen/lib/concolic.h"

namespace P4::P4Tools::P4Testgen::EBPF {

class EBPFConcolic : public Concolic {
 private:
    /// This is the list of concolic functions that are implemented in this class.
    static const ConcolicMethodImpls::ImplList EBPF_CONCOLIC_METHOD_IMPLS;

 public:
    /// @returns the concolic  functions that are implemented for this particular target.
    static const ConcolicMethodImpls::ImplList *getEBPFConcolicMethodImpls();
};

}  // namespace P4::P4Tools::P4Testgen::EBPF

#endif /* BACKENDS_P4TOOLS_MODULES_TESTGEN_TARGETS_EBPF_CONCOLIC_H_ */
