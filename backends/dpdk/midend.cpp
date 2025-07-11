/*
Copyright 2020 Intel Corp.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "midend.h"

#include "dpdkArch.h"
#include "frontends/common/constantFolding.h"
#include "frontends/common/resolveReferences/resolveReferences.h"
#include "frontends/p4-14/fromv1.0/v1model.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/p4/moveDeclarations.h"
#include "frontends/p4/simplify.h"
#include "frontends/p4/simplifyParsers.h"
#include "frontends/p4/simplifySwitch.h"
#include "frontends/p4/strengthReduction.h"
#include "frontends/p4/typeChecking/typeChecker.h"
#include "frontends/p4/typeMap.h"
#include "frontends/p4/uniqueNames.h"
#include "frontends/p4/unusedDeclarations.h"
#include "lib/cstring.h"
#include "midend/actionSynthesis.h"
#include "midend/compileTimeOps.h"
#include "midend/complexComparison.h"
#include "midend/convertEnums.h"
#include "midend/convertErrors.h"
#include "midend/copyStructures.h"
#include "midend/eliminateInvalidHeaders.h"
#include "midend/eliminateNewtype.h"
#include "midend/eliminateSerEnums.h"
#include "midend/eliminateSwitch.h"
#include "midend/eliminateTuples.h"
#include "midend/eliminateTypedefs.h"
#include "midend/expandEmit.h"
#include "midend/expandLookahead.h"
#include "midend/fillEnumMap.h"
#include "midend/flattenHeaders.h"
#include "midend/flattenInterfaceStructs.h"
#include "midend/flattenUnions.h"
#include "midend/hsIndexSimplify.h"
#include "midend/local_copyprop.h"
#include "midend/midEndLast.h"
#include "midend/nestedStructs.h"
#include "midend/noMatch.h"
#include "midend/orderArguments.h"
#include "midend/parserUnroll.h"
#include "midend/predication.h"
#include "midend/removeAssertAssume.h"
#include "midend/removeExits.h"
#include "midend/removeLeftSlices.h"
#include "midend/removeMiss.h"
#include "midend/removeSelectBooleans.h"
#include "midend/removeUnusedParameters.h"
#include "midend/replaceSelectRange.h"
#include "midend/simplifyKey.h"
#include "midend/simplifySelectCases.h"
#include "midend/simplifySelectList.h"
#include "midend/tableHit.h"
#include "midend/validateProperties.h"
#include "options.h"

namespace P4::DPDK {

/// This class implements a policy suitable for the ConvertEnums pass.
/// The policy is: convert all enums to bit<32>
class EnumOn32Bits : public P4::ChooseEnumRepresentation {
    bool convert(const IR::Type_Enum *) const override { return true; }

    /// This function assigns DPDK target compatible values to the enums.
    unsigned encoding(const IR::Type_Enum *type, unsigned n) const override {
        if (type->name == "PSA_MeterColor_t") {
            // DPDK target assumes the following values for Meter colors
            // (Green: 0, Yellow: 1, Red: 2)
            // For PSA, the default values are  RED: 0, Green: 1, Yellow: 2
            return (n + 2) % 3;
        }
        return n;
    }

    unsigned enumSize(unsigned) const override { return 32; }

 public:
    EnumOn32Bits() {}
};

/// This class implements a policy suitable for the ConvertErrors pass.
/// The policy is: convert all errors to specified width.
class ErrorWidth : public P4::ChooseErrorRepresentation {
    unsigned width;
    bool convert(const IR::Type_Error *) const override { return true; }

    unsigned errorSize(unsigned) const override { return width; }

 public:
    explicit ErrorWidth(unsigned width) : width(width) {}
};

DpdkMidEnd::DpdkMidEnd(CompilerOptions &options, std::ostream *outStream) {
    auto convertEnums = new P4::ConvertEnums(&typeMap, new EnumOn32Bits());
    auto convertErrors = new P4::ConvertErrors(&typeMap, new ErrorWidth(16));
    auto evaluator = new P4::EvaluatorPass(&refMap, &typeMap);
    P4::LocalCopyPropPolicyCallbackFn policy = [=](const Context *ctx, const IR::Expression *,
                                                   const DeclarationLookup *refMap) -> bool {
        if (auto mce = findContext<IR::MethodCallExpression>(ctx)) {
            auto mi = P4::MethodInstance::resolve(mce, refMap, &typeMap);
            if (auto em = mi->to<P4::ExternMethod>()) {
                cstring externType = em->originalExternType->getName().name;
                cstring externMethod = em->method->getName().name;

                std::vector<std::pair<cstring, cstring>> doNotCopyPropList = {
                    {"Checksum"_cs, "update"_cs},
                    {"Hash"_cs, "get_hash"_cs},
                    {"InternetChecksum"_cs, "add"_cs},
                    {"InternetChecksum"_cs, "subtract"_cs},
                    {"InternetChecksum"_cs, "set_state"_cs},
                    {"Register"_cs, "read"_cs},
                    {"Register"_cs, "write"_cs},
                    {"Counter"_cs, "count"_cs},
                    {"Meter"_cs, "execute"_cs},
                    {"Meter"_cs, "dpdk_execute"_cs},
                    {"Digest"_cs, "pack"_cs},
                };
                for (auto f : doNotCopyPropList) {
                    if (externType == f.first && externMethod == f.second) {
                        return false;
                    }
                }
            } else if (auto ef = mi->to<P4::ExternFunction>()) {
                cstring externFuncName = ef->method->getName().name;
                std::vector<cstring> doNotCopyPropList = {
                    "verify"_cs,
                };
                for (auto f : doNotCopyPropList) {
                    if (externFuncName == f) return false;
                }
            }
        }
        return true;
    };

    std::function<Inspector *(cstring)> validateTableProperties = [=](cstring arch) -> Inspector * {
        if (arch == "pna") {
            return new P4::ValidateTableProperties(
                {"pna_implementation"_cs, "pna_direct_counter"_cs, "pna_direct_meter"_cs,
                 "pna_idle_timeout"_cs, "size"_cs, "add_on_miss"_cs,
                 "idle_timeout_with_auto_delete"_cs,
                 "default_idle_timeout_for_data_plane_added_entries"_cs});
        } else if (arch == "psa") {
            return new P4::ValidateTableProperties({"psa_implementation"_cs,
                                                    "psa_direct_counter"_cs, "psa_direct_meter"_cs,
                                                    "psa_idle_timeout"_cs, "size"_cs});
        } else {
            return nullptr;
        }
    };

    if (!DPDK::DpdkContext::get().options().loadIRFromJson) {
        addPasses({
            options.ndebug ? new P4::RemoveAssertAssume(&typeMap) : nullptr,
            new P4::RemoveMiss(&typeMap),
            new P4::EliminateNewtype(&typeMap),
            new P4::EliminateSerEnums(&typeMap),
            new P4::EliminateInvalidHeaders(&typeMap),
            convertEnums,
            new VisitFunctor([this, convertEnums]() { enumMap = convertEnums->getEnumMapping(); }),
            new P4::OrderArguments(&typeMap),
            new P4::TypeChecking(&refMap, &typeMap),
            new P4::SimplifyKey(
                &typeMap, new P4::OrPolicy(new P4::IsValid(&typeMap), new P4::IsLikeLeftValue())),
            new P4::RemoveExits(&typeMap),
            new P4::ConstantFolding(&typeMap),
            new P4::StrengthReduction(&typeMap),
            new P4::SimplifySelectCases(&typeMap, true),
            // The lookahead implementation in DPDK target supports only a header instance as
            // an operand, we do not expand headers.
            // Structures expanded here are then processed as base bit type in ConvertLookahead
            // pass in backend.
            new P4::ExpandLookahead(&typeMap, nullptr, false),
            new P4::ExpandEmit(&typeMap),
            new P4::HandleNoMatch(),
            new P4::SimplifyParsers(),
            new P4::StrengthReduction(&typeMap),
            new P4::EliminateTuples(&typeMap),
            new P4::SimplifyComparisons(&typeMap),
            new P4::CopyStructures(&typeMap, false /* errorOnMethodCall */),
            new P4::NestedStructs(&typeMap),
            new P4::SimplifySelectList(&typeMap),
            new P4::RemoveSelectBooleans(&typeMap),
            new P4::FlattenHeaders(&typeMap),
            new P4::FlattenInterfaceStructs(&typeMap),
            new P4::EliminateTypedef(&typeMap),
            new P4::HSIndexSimplifier(&typeMap),
            new P4::ParsersUnroll(true, &refMap, &typeMap),
            new P4::FlattenHeaderUnion(&refMap, &typeMap),
            new P4::SimplifyControlFlow(&typeMap, true),
            new P4::ReplaceSelectRange(),
            new P4::MoveDeclarations(),  // more may have been introduced
            new P4::ConstantFolding(&typeMap),
            new P4::LocalCopyPropagation(&typeMap, nullptr, policy),
            new PassRepeated({
                new P4::ConstantFolding(&typeMap),
                new P4::StrengthReduction(&typeMap),
            }),
            new P4::MoveDeclarations(),
            validateTableProperties(options.arch),
            new P4::SimplifyControlFlow(&typeMap, true),
            new P4::SimplifySwitch(&typeMap),
            new P4::CompileTimeOperations(),
            new P4::TableHit(&typeMap),
            new P4::RemoveLeftSlices(&typeMap),
            new P4::TypeChecking(&refMap, &typeMap),
            convertErrors,
            new P4::EliminateSerEnums(&typeMap),
            new P4::MidEndLast(),
            evaluator,
            new VisitFunctor([this, evaluator]() { toplevel = evaluator->getToplevelBlock(); }),
        });
        if (options.listMidendPasses) {
            listPasses(*outStream, cstring::newline);
            *outStream << std::endl;
            return;
        }
        if (options.excludeMidendPasses) {
            removePasses(options.passesToExcludeMidend);
        }
    } else {
        auto fillEnumMap = new P4::FillEnumMap(new EnumOn32Bits(), &typeMap);
        addPasses({
            new P4::ResolveReferences(&refMap),
            new P4::TypeChecking(&refMap, &typeMap),
            fillEnumMap,
            new VisitFunctor([this, fillEnumMap]() { enumMap = fillEnumMap->repr; }),
            evaluator,
            new VisitFunctor([this, evaluator]() { toplevel = evaluator->getToplevelBlock(); }),
        });
    }
}

}  // namespace P4::DPDK
