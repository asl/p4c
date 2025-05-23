/*
Copyright 2018-present Barefoot Networks, Inc.

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

#ifndef CONTROL_PLANE_P4RUNTIMEARCHSTANDARD_H_
#define CONTROL_PLANE_P4RUNTIMEARCHSTANDARD_H_

#include "frontends/common/resolveReferences/referenceMap.h"
#include "frontends/p4-14/fromv1.0/v1model.h"
#include "frontends/p4/typeMap.h"
#include "ir/ir.h"
#include "p4RuntimeArchHandler.h"

namespace p4configv1 = ::p4::config::v1;

using ::P4::ControlPlaneAPI::Helpers::getExternInstanceFromProperty;
using ::P4::ControlPlaneAPI::Helpers::setPreamble;

namespace P4 {

/** \addtogroup control_plane
 *  @{
 */
namespace ControlPlaneAPI {

namespace Standard {

/// We re-use as much code as possible between PSA and v1model. The two
/// architectures have some differences though, in particular regarding naming
/// (of table properties, extern types, parameter names). We define some
/// "traits" for each extern type, templatized by the architecture name (using
/// the Arch enum class defined below), as a convenient way to access
/// architecture-specific names in the unified code.
/// V1MODEL2020 is v1model with a version >= 20200408.
enum class Arch { V1MODEL, PSA, PNA, V1MODEL2020 };

template <Arch arch>
struct CounterExtern {};
template <Arch arch>
struct MeterExtern {};

}  // namespace Standard

namespace Helpers {

// According to the C++11 standard: An explicit specialization shall be declared
// in a namespace enclosing the specialized template. An explicit specialization
// whose declarator-id is not qualified shall be declared in the nearest
// enclosing namespace of the template, or, if the namespace is inline (7.3.1),
// any namespace from its enclosing namespace set. Such a declaration may also
// be a definition. If the declaration is not a definition, the specialization
// may be defined later (7.3.1.2).
//
// GCC reports an error when trying so specialize CounterlikeTraits<> for
// Standard::CounterExtern & Standard::MeterExtern outside of the Helpers
// namespace, even when qualifying CounterlikeTraits<> with Helpers::. It seems
// to be related to this bug:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56480.

/// @ref CounterlikeTraits<> specialization for @ref CounterExtern for v1model
template <>
struct CounterlikeTraits<Standard::CounterExtern<Standard::Arch::V1MODEL>> {
    static const cstring name() { return "counter"_cs; }
    static const cstring directPropertyName() {
        return P4V1::V1Model::instance.tableAttributes.counters.name;
    }
    static const cstring typeName() { return P4V1::V1Model::instance.counter.name; }
    static const cstring directTypeName() { return P4V1::V1Model::instance.directCounter.name; }
    static const cstring sizeParamName() { return "size"_cs; }
    static p4configv1::CounterSpec::Unit mapUnitName(const cstring name) {
        using p4configv1::CounterSpec;
        if (name == "packets")
            return CounterSpec::PACKETS;
        else if (name == "bytes")
            return CounterSpec::BYTES;
        else if (name == "packets_and_bytes")
            return CounterSpec::BOTH;
        return CounterSpec::UNSPECIFIED;
    }
    static std::optional<size_t> indexTypeParamIdx() { return std::nullopt; }
};

template <>
struct CounterlikeTraits<Standard::CounterExtern<Standard::Arch::V1MODEL2020>> {
    static const cstring name() { return "counter"_cs; }
    static const cstring directPropertyName() {
        return P4V1::V1Model::instance.tableAttributes.counters.name;
    }
    static const cstring typeName() { return P4V1::V1Model::instance.counter.name; }
    static const cstring directTypeName() { return P4V1::V1Model::instance.directCounter.name; }
    static const cstring sizeParamName() { return "size"_cs; }
    static p4configv1::CounterSpec::Unit mapUnitName(const cstring name) {
        using p4configv1::CounterSpec;
        if (name == "packets")
            return CounterSpec::PACKETS;
        else if (name == "bytes")
            return CounterSpec::BYTES;
        else if (name == "packets_and_bytes")
            return CounterSpec::BOTH;
        return CounterSpec::UNSPECIFIED;
    }
    static std::optional<size_t> indexTypeParamIdx() { return 0; }
};

/// @ref CounterlikeTraits<> specialization for @ref CounterExtern for PSA
template <>
struct CounterlikeTraits<Standard::CounterExtern<Standard::Arch::PSA>> {
    static const cstring name() { return "counter"_cs; }
    static const cstring directPropertyName() { return "psa_direct_counter"_cs; }
    static const cstring typeName() { return "Counter"_cs; }
    static const cstring directTypeName() { return "DirectCounter"_cs; }
    static const cstring sizeParamName() { return "n_counters"_cs; }
    static p4configv1::CounterSpec::Unit mapUnitName(const cstring name) {
        using p4configv1::CounterSpec;
        if (name == "PACKETS")
            return CounterSpec::PACKETS;
        else if (name == "BYTES")
            return CounterSpec::BYTES;
        else if (name == "PACKETS_AND_BYTES")
            return CounterSpec::BOTH;
        return CounterSpec::UNSPECIFIED;
    }
    // the index of the type parameter for the counter index, in the type
    // parameter list of the extern type declaration.
    static std::optional<size_t> indexTypeParamIdx() { return 1; }
};

/// @ref CounterlikeTraits<> specialization for @ref CounterExtern for PNA
template <>
struct CounterlikeTraits<Standard::CounterExtern<Standard::Arch::PNA>> {
    static const cstring name() { return "counter"_cs; }
    static const cstring directPropertyName() { return "pna_direct_counter"_cs; }
    static const cstring typeName() { return "Counter"_cs; }
    static const cstring directTypeName() { return "DirectCounter"_cs; }
    static const cstring sizeParamName() { return "n_counters"_cs; }
    static p4configv1::CounterSpec::Unit mapUnitName(const cstring name) {
        using p4configv1::CounterSpec;
        if (name == "PACKETS")
            return CounterSpec::PACKETS;
        else if (name == "BYTES")
            return CounterSpec::BYTES;
        else if (name == "PACKETS_AND_BYTES")
            return CounterSpec::BOTH;
        return CounterSpec::UNSPECIFIED;
    }
    // the index of the type parameter for the counter index, in the type
    // parameter list of the extern type declaration.
    static std::optional<size_t> indexTypeParamIdx() { return 1; }
};

/// @ref CounterlikeTraits<> specialization for @ref MeterExtern for v1model
template <>
struct CounterlikeTraits<Standard::MeterExtern<Standard::Arch::V1MODEL>> {
    static const cstring name() { return "meter"_cs; }
    static const cstring directPropertyName() {
        return P4V1::V1Model::instance.tableAttributes.meters.name;
    }
    static const cstring typeName() { return P4V1::V1Model::instance.meter.name; }
    static const cstring directTypeName() { return P4V1::V1Model::instance.directMeter.name; }
    static const cstring sizeParamName() { return "size"_cs; }
    static p4configv1::MeterSpec::Unit mapUnitName(const cstring name) {
        using p4configv1::MeterSpec;
        if (name == "packets")
            return MeterSpec::PACKETS;
        else if (name == "bytes")
            return MeterSpec::BYTES;
        return MeterSpec::UNSPECIFIED;
    }
    static std::optional<size_t> indexTypeParamIdx() { return std::nullopt; }
};

template <>
struct CounterlikeTraits<Standard::MeterExtern<Standard::Arch::V1MODEL2020>> {
    static const cstring name() { return "meter"_cs; }
    static const cstring directPropertyName() {
        return P4V1::V1Model::instance.tableAttributes.meters.name;
    }
    static const cstring typeName() { return P4V1::V1Model::instance.meter.name; }
    static const cstring directTypeName() { return P4V1::V1Model::instance.directMeter.name; }
    static const cstring sizeParamName() { return "size"_cs; }
    static p4configv1::MeterSpec::Unit mapUnitName(const cstring name) {
        using p4configv1::MeterSpec;
        if (name == "packets")
            return MeterSpec::PACKETS;
        else if (name == "bytes")
            return MeterSpec::BYTES;
        return MeterSpec::UNSPECIFIED;
    }
    static std::optional<size_t> indexTypeParamIdx() { return 0; }
};

/// @ref CounterlikeTraits<> specialization for @ref MeterExtern for PSA
template <>
struct CounterlikeTraits<Standard::MeterExtern<Standard::Arch::PSA>> {
    static const cstring name() { return "meter"_cs; }
    static const cstring directPropertyName() { return "psa_direct_meter"_cs; }
    static const cstring typeName() { return "Meter"_cs; }
    static const cstring directTypeName() { return "DirectMeter"_cs; }
    static const cstring sizeParamName() { return "n_meters"_cs; }
    static p4configv1::MeterSpec::Unit mapUnitName(const cstring name) {
        using p4configv1::MeterSpec;
        if (name == "PACKETS")
            return MeterSpec::PACKETS;
        else if (name == "BYTES")
            return MeterSpec::BYTES;
        return MeterSpec::UNSPECIFIED;
    }
    // the index of the type parameter for the meter index, in the type
    // parameter list of the extern type declaration.
    static std::optional<size_t> indexTypeParamIdx() { return 0; }
};

/// @ref CounterlikeTraits<> specialization for @ref MeterExtern for PNA
template <>
struct CounterlikeTraits<Standard::MeterExtern<Standard::Arch::PNA>> {
    static const cstring name() { return "meter"_cs; }
    static const cstring directPropertyName() { return "pna_direct_meter"_cs; }
    static const cstring typeName() { return "Meter"_cs; }
    static const cstring directTypeName() { return "DirectMeter"_cs; }
    static const cstring sizeParamName() { return "n_meters"_cs; }
    static p4configv1::MeterSpec::Unit mapUnitName(const cstring name) {
        using p4configv1::MeterSpec;
        if (name == "PACKETS")
            return MeterSpec::PACKETS;
        else if (name == "BYTES")
            return MeterSpec::BYTES;
        return MeterSpec::UNSPECIFIED;
    }
    // the index of the type parameter for the meter index, in the type
    // parameter list of the extern type declaration.
    static std::optional<size_t> indexTypeParamIdx() { return 0; }
};

}  // namespace Helpers

/// Declarations specific to standard architectures (v1model & PSA).
namespace Standard {

/// The architecture handler builder implementation for v1model.
struct V1ModelArchHandlerBuilder : public P4RuntimeArchHandlerBuilderIface {
    P4RuntimeArchHandlerIface *operator()(ReferenceMap *refMap, TypeMap *typeMap,
                                          const IR::ToplevelBlock *evaluatedProgram) const override;
};

/// The architecture handler builder implementation for PSA.
struct PSAArchHandlerBuilder : public P4RuntimeArchHandlerBuilderIface {
    P4RuntimeArchHandlerIface *operator()(ReferenceMap *refMap, TypeMap *typeMap,
                                          const IR::ToplevelBlock *evaluatedProgram) const override;
};

/// The architecture handler builder implementation for PNA.
struct PNAArchHandlerBuilder : public P4RuntimeArchHandlerBuilderIface {
    P4RuntimeArchHandlerIface *operator()(ReferenceMap *refMap, TypeMap *typeMap,
                                          const IR::ToplevelBlock *evaluatedProgram) const override;
};

/// The architecture handler builder implementation for UBPF.
struct UBPFArchHandlerBuilder : public P4RuntimeArchHandlerBuilderIface {
    P4RuntimeArchHandlerIface *operator()(ReferenceMap *refMap, TypeMap *typeMap,
                                          const IR::ToplevelBlock *evaluatedProgram) const override;
};

/// Extends @ref P4RuntimeSymbolType for the standard (v1model, PSA & PNA) extern
/// types.
class SymbolType : public P4RuntimeSymbolType {
 public:
    SymbolType() = delete;

    static P4RuntimeSymbolType P4RT_ACTION_PROFILE() {
        return P4RuntimeSymbolType::make(p4configv1::P4Ids::ACTION_PROFILE);
    }
    static P4RuntimeSymbolType P4RT_COUNTER() {
        return P4RuntimeSymbolType::make(p4configv1::P4Ids::COUNTER);
    }
    static P4RuntimeSymbolType P4RT_DIGEST() {
        return P4RuntimeSymbolType::make(p4configv1::P4Ids::DIGEST);
    }
    static P4RuntimeSymbolType P4RT_DIRECT_COUNTER() {
        return P4RuntimeSymbolType::make(p4configv1::P4Ids::DIRECT_COUNTER);
    }
    static P4RuntimeSymbolType P4RT_DIRECT_METER() {
        return P4RuntimeSymbolType::make(p4configv1::P4Ids::DIRECT_METER);
    }
    static P4RuntimeSymbolType P4RT_METER() {
        return P4RuntimeSymbolType::make(p4configv1::P4Ids::METER);
    }
    static P4RuntimeSymbolType P4RT_REGISTER() {
        return P4RuntimeSymbolType::make(p4configv1::P4Ids::REGISTER);
    }
};

/// Traits for the action profile extern, must be specialized for v1model and
/// PSA.
template <Arch arch>
struct ActionProfileTraits;

template <>
struct ActionProfileTraits<Arch::V1MODEL> {
    static const cstring name() { return "action profile"_cs; }
    static const cstring propertyName() {
        return P4V1::V1Model::instance.tableAttributes.tableImplementation.name;
    }
    static const cstring typeName() { return P4V1::V1Model::instance.action_profile.name; }
    static const cstring sizeParamName() { return "size"_cs; }
};

template <>
struct ActionProfileTraits<Arch::V1MODEL2020> {
    static const cstring name() { return "action profile"_cs; }
    static const cstring propertyName() {
        return P4V1::V1Model::instance.tableAttributes.tableImplementation.name;
    }
    static const cstring typeName() { return P4V1::V1Model::instance.action_profile.name; }
    static const cstring sizeParamName() { return "size"_cs; }
};

template <>
struct ActionProfileTraits<Arch::PSA> {
    static const cstring name() { return "action profile"_cs; }
    static const cstring propertyName() { return "psa_implementation"_cs; }
    static const cstring typeName() { return "ActionProfile"_cs; }
    static const cstring sizeParamName() { return "size"_cs; }
};

template <>
struct ActionProfileTraits<Arch::PNA> {
    static const cstring name() { return "action profile"_cs; }
    static const cstring propertyName() { return "pna_implementation"_cs; }
    static const cstring typeName() { return "ActionProfile"_cs; }
    static const cstring sizeParamName() { return "size"_cs; }
};

/// Traits for the action selector extern, must be specialized for v1model and
/// PSA. Inherits from ActionProfileTraits because of their similarities.
template <Arch arch>
struct ActionSelectorTraits;

template <>
struct ActionSelectorTraits<Arch::V1MODEL> : public ActionProfileTraits<Arch::V1MODEL> {
    static const cstring name() { return "action selector"_cs; }
    static const cstring typeName() { return P4V1::V1Model::instance.action_selector.name; }
};

template <>
struct ActionSelectorTraits<Arch::V1MODEL2020> : public ActionProfileTraits<Arch::V1MODEL2020> {
    static const cstring name() { return "action selector"_cs; }
    static const cstring typeName() { return P4V1::V1Model::instance.action_selector.name; }
};

template <>
struct ActionSelectorTraits<Arch::PSA> : public ActionProfileTraits<Arch::PSA> {
    static const cstring name() { return "action selector"_cs; }
    static const cstring typeName() { return "ActionSelector"_cs; }
};

template <>
struct ActionSelectorTraits<Arch::PNA> : public ActionProfileTraits<Arch::PNA> {
    static const cstring name() { return "action selector"_cs; }
    static const cstring typeName() { return "ActionSelector"_cs; }
};

/// Traits for the register extern, must be specialized for v1model and PSA.
template <Arch arch>
struct RegisterTraits;

template <>
struct RegisterTraits<Arch::V1MODEL> {
    static const cstring name() { return "register"_cs; }
    static const cstring typeName() { return P4V1::V1Model::instance.registers.name; }
    static const cstring sizeParamName() { return "size"_cs; }
    // the index of the type parameter for the data stored in the register, in
    // the type parameter list of the extern type declaration
    static size_t dataTypeParamIdx() { return 0; }
    static std::optional<size_t> indexTypeParamIdx() { return std::nullopt; }
};

template <>
struct RegisterTraits<Arch::V1MODEL2020> {
    static const cstring name() { return "register"_cs; }
    static const cstring typeName() { return P4V1::V1Model::instance.registers.name; }
    static const cstring sizeParamName() { return "size"_cs; }
    // the index of the type parameter for the data stored in the register, in
    // the type parameter list of the extern type declaration
    static size_t dataTypeParamIdx() { return 0; }
    static std::optional<size_t> indexTypeParamIdx() { return 1; }
};

template <>
struct RegisterTraits<Arch::PSA> {
    static const cstring name() { return "register"_cs; }
    static const cstring typeName() { return "Register"_cs; }
    static const cstring sizeParamName() { return "size"_cs; }
    static size_t dataTypeParamIdx() { return 0; }
    // the index of the type parameter for the register index, in the type
    // parameter list of the extern type declaration.
    static std::optional<size_t> indexTypeParamIdx() { return 1; }
};

template <>
struct RegisterTraits<Arch::PNA> {
    static const cstring name() { return "register"_cs; }
    static const cstring typeName() { return "Register"_cs; }
    static const cstring sizeParamName() { return "size"_cs; }
    static size_t dataTypeParamIdx() { return 0; }
    // the index of the type parameter for the register index, in the type
    // parameter list of the extern type declaration.
    static std::optional<size_t> indexTypeParamIdx() { return 1; }
};

//// The information about a digest call which is needed to serialize it.
struct Digest {
    const cstring name;                          // The fully qualified external name of the digest
                                                 // *data* - in P4-14, the field list name, or in
                                                 // P4-16, the type of the 'data' parameter.
    const p4configv1::P4DataTypeSpec *typeSpec;  // The format of the packed data.
    const IR::IAnnotated *annotations;  // If non-null, any annotations applied to this digest
                                        // declaration.
};

struct Register {
    const cstring name;                 // The fully qualified external name of this register.
    const IR::IAnnotated *annotations;  // If non-null, any annotations applied
                                        // to this field.
    const int64_t size;
    const p4configv1::P4DataTypeSpec *typeSpec;  // The format of the stored data.
    // If the type of the index is a user-defined type, this is the name of the type. Otherwise it
    // is nullptr.
    const cstring index_type_name;

    /// @return the information required to serialize an @instance of register
    /// or std::nullopt in case of error.
    template <Arch arch>
    static std::optional<Register> from(const IR::ExternBlock *instance, const ReferenceMap *refMap,
                                        P4::TypeMap *typeMap,
                                        p4configv1::P4TypeInfo *p4RtTypeInfo) {
        CHECK_NULL(instance);
        auto declaration = instance->node->to<IR::Declaration_Instance>();

        auto size = instance->getParameterValue("size"_cs)->to<IR::Constant>();
        if (!size->is<IR::Constant>()) {
            ::P4::error(ErrorType::ERR_UNSUPPORTED, "Register '%1%' has a non-constant size: %2%",
                        declaration, size);
            return std::nullopt;
        }
        if (!size->to<IR::Constant>()->fitsInt()) {
            ::P4::error(ErrorType::ERR_UNSUPPORTED,
                        "Register '%1%' has a size that doesn't fit in an integer: %2%",
                        declaration, size);
            return std::nullopt;
        }

        // retrieve type parameter for the register instance and convert it to P4DataTypeSpec
        BUG_CHECK(declaration->type->is<IR::Type_Specialized>(), "%1%: expected Type_Specialized",
                  declaration->type);
        auto type = declaration->type->to<IR::Type_Specialized>();
        auto typeParamIdx = RegisterTraits<arch>::dataTypeParamIdx();
        BUG_CHECK(type->arguments->size() > typeParamIdx,
                  "%1%: expected at least %2% type arguments", instance, typeParamIdx + 1);
        auto typeArg = type->arguments->at(typeParamIdx);
        auto typeSpec = TypeSpecConverter::convert(refMap, typeMap, typeArg, p4RtTypeInfo);
        CHECK_NULL(typeSpec);

        cstring index_type_name = nullptr;
        auto indexTypeParamIdx = RegisterTraits<arch>::indexTypeParamIdx();
        // In v1model, the index is a bit<32>, in PSA it is determined by a type parameter.
        if (indexTypeParamIdx != std::nullopt) {
            // retrieve type parameter for the index.
            BUG_CHECK(declaration->type->is<IR::Type_Specialized>(),
                      "%1%: expected Type_Specialized", declaration->type);
            auto type = declaration->type->to<IR::Type_Specialized>();
            BUG_CHECK(type->arguments->size() > *indexTypeParamIdx,
                      "%1%: expected at least %2% type arguments", instance,
                      *indexTypeParamIdx + 1);
            auto typeArg = type->arguments->at(*indexTypeParamIdx);
            // We ignore the return type on purpose, but the call is required to update p4RtTypeInfo
            // if the index has a user-defined type.
            TypeSpecConverter::convert(refMap, typeMap, typeArg, p4RtTypeInfo);
            index_type_name = getTypeName(typeArg, typeMap);
        }

        return Register{declaration->controlPlaneName(), declaration->to<IR::IAnnotated>(),
                        int(size->value), typeSpec, index_type_name};
    }
};

/// The types of action profiles available in v1model & PSA.
enum class ActionProfileType { INDIRECT, INDIRECT_WITH_SELECTOR };

/// The information about an action profile which is necessary to generate its
/// serialized representation.
struct ActionProfile {
    const cstring name;  // The fully qualified external name of this action profile.
    const ActionProfileType type;
    const int64_t size;
    const IR::IAnnotated *annotations;  // If non-null, any annotations applied to this action
                                        // profile declaration.

    bool operator<(const ActionProfile &other) const {
        if (name != other.name) return name < other.name;
        if (type != other.type) return type < other.type;
        return size < other.size;
    }
};

/// Parent class for P4RuntimeArchHandlerV1Model and P4RuntimeArchHandlerPSA; it
/// includes all the common code between the two architectures (which is only
/// dependent on the @arch template parameter. The major difference at the
/// moment is handling of digest, which is an extern function in v1model and an
/// extern type in PSA.
template <Arch arch>
class P4RuntimeArchHandlerCommon : public P4RuntimeArchHandlerIface {
 protected:
    using ArchCounterExtern = CounterExtern<arch>;
    using CounterTraits = Helpers::CounterlikeTraits<ArchCounterExtern>;
    using ArchMeterExtern = MeterExtern<arch>;
    using MeterTraits = Helpers::CounterlikeTraits<ArchMeterExtern>;

    using Counter = p4configv1::Counter;
    using Meter = p4configv1::Meter;
    using CounterSpec = p4configv1::CounterSpec;
    using MeterSpec = p4configv1::MeterSpec;

    P4RuntimeArchHandlerCommon(ReferenceMap *refMap, TypeMap *typeMap,
                               const IR::ToplevelBlock *evaluatedProgram)
        : refMap(refMap), typeMap(typeMap), evaluatedProgram(evaluatedProgram) {
        jsonPrintOptions.add_whitespace = true;
    }

    void collectTableProperties(P4RuntimeSymbolTableIface *symbols,
                                const IR::TableBlock *tableBlock) override {
        CHECK_NULL(tableBlock);
        auto table = tableBlock->container;
        bool isConstructedInPlace = false;

        {
            auto instance =
                getExternInstanceFromProperty(table, ActionProfileTraits<arch>::propertyName(),
                                              refMap, typeMap, &isConstructedInPlace);
            if (instance) {
                if (instance->type->name != ActionProfileTraits<arch>::typeName() &&
                    instance->type->name != ActionSelectorTraits<arch>::typeName()) {
                    ::P4::error(ErrorType::ERR_EXPECTED,
                                "Expected an action profile or action selector: %1%",
                                instance->expression);
                } else if (isConstructedInPlace) {
                    symbols->add(SymbolType::P4RT_ACTION_PROFILE(), *instance->name);
                }
            }
        }
        {
            auto instance = getExternInstanceFromProperty(
                table, CounterTraits::directPropertyName(), refMap, typeMap, &isConstructedInPlace);
            if (instance) {
                if (instance->type->name != CounterTraits::directTypeName()) {
                    ::P4::error(ErrorType::ERR_EXPECTED, "Expected a direct counter: %1%",
                                instance->expression);
                } else if (isConstructedInPlace) {
                    symbols->add(SymbolType::P4RT_DIRECT_COUNTER(), *instance->name);
                }
            }
        }
        {
            auto instance = getExternInstanceFromProperty(table, MeterTraits::directPropertyName(),
                                                          refMap, typeMap, &isConstructedInPlace);
            if (instance) {
                if (instance->type->name != MeterTraits::directTypeName()) {
                    ::P4::error(ErrorType::ERR_EXPECTED, "Expected a direct meter: %1%",
                                instance->expression);
                } else if (isConstructedInPlace) {
                    symbols->add(SymbolType::P4RT_DIRECT_METER(), *instance->name);
                }
            }
        }
    }

    void collectAssignmentStatement(P4RuntimeSymbolTableIface *,
                                    const IR::BaseAssignmentStatement *) override {}

    void collectExternMethod(P4RuntimeSymbolTableIface *, const P4::ExternMethod *) override {}

    void collectExternInstance(P4RuntimeSymbolTableIface *symbols,
                               const IR::ExternBlock *externBlock) override {
        CHECK_NULL(externBlock);

        auto decl = externBlock->node->to<IR::IDeclaration>();
        // Skip externs instantiated inside table declarations (as properties);
        // that should only apply to action profiles / selectors since direct
        // resources cannot be constructed in place for PSA.
        if (decl == nullptr) return;

        if (externBlock->type->name == CounterTraits::typeName()) {
            symbols->add(SymbolType::P4RT_COUNTER(), decl);
        } else if (externBlock->type->name == CounterTraits::directTypeName()) {
            symbols->add(SymbolType::P4RT_DIRECT_COUNTER(), decl);
        } else if (externBlock->type->name == MeterTraits::typeName()) {
            symbols->add(SymbolType::P4RT_METER(), decl);
        } else if (externBlock->type->name == MeterTraits::directTypeName()) {
            symbols->add(SymbolType::P4RT_DIRECT_METER(), decl);
        } else if (externBlock->type->name == ActionProfileTraits<arch>::typeName() ||
                   externBlock->type->name == ActionSelectorTraits<arch>::typeName()) {
            symbols->add(SymbolType::P4RT_ACTION_PROFILE(), decl);
        } else if (externBlock->type->name == RegisterTraits<arch>::typeName()) {
            symbols->add(SymbolType::P4RT_REGISTER(), decl);
        }
    }

    void collectExternFunction(P4RuntimeSymbolTableIface *symbols,
                               const P4::ExternFunction *externFunction) override {
        // no common task
        (void)symbols;
        (void)externFunction;
    }

    void collectExtra(P4RuntimeSymbolTableIface *symbols) override {
        // nothing to do for standard architectures
        (void)symbols;
    }

    void postCollect(const P4RuntimeSymbolTableIface &symbols) override {
        (void)symbols;
        // analyze action profiles and build a mapping from action profile name
        // to the set of tables referencing them
        Helpers::forAllEvaluatedBlocks(evaluatedProgram, [&](const IR::Block *block) {
            if (!block->is<IR::TableBlock>()) return;
            auto table = block->to<IR::TableBlock>()->container;
            auto implementation = getTableImplementationName(table, refMap);
            if (implementation)
                actionProfilesRefs[*implementation].insert(table->controlPlaneName());
        });
    }

    void addTableProperties(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4info,
                            p4configv1::Table *table, const IR::TableBlock *tableBlock) override {
        CHECK_NULL(tableBlock);
        auto tableDeclaration = tableBlock->container;

        using Helpers::isExternPropertyConstructedInPlace;

        auto implementation = getActionProfile(tableDeclaration, refMap, typeMap);
        auto directCounter =
            Helpers::getDirectCounterlike<ArchCounterExtern>(tableDeclaration, refMap, typeMap);
        auto directMeter =
            Helpers::getDirectCounterlike<ArchMeterExtern>(tableDeclaration, refMap, typeMap);

        if (implementation) {
            auto id = symbols.getId(SymbolType::P4RT_ACTION_PROFILE(), implementation->name);
            table->set_implementation_id(id);
            auto propertyName = ActionProfileTraits<arch>::propertyName();
            if (isExternPropertyConstructedInPlace(tableDeclaration, propertyName))
                addActionProfile(symbols, p4info, *implementation);
        }

        if (directCounter) {
            auto id = symbols.getId(SymbolType::P4RT_DIRECT_COUNTER(), directCounter->name);
            table->add_direct_resource_ids(id);
            // no risk to add twice because direct counters cannot be shared
            addCounter(symbols, p4info, *directCounter);
        }

        if (directMeter) {
            auto id = symbols.getId(SymbolType::P4RT_DIRECT_METER(), directMeter->name);
            table->add_direct_resource_ids(id);
            // no risk to add twice because direct meters cannot be shared
            addMeter(symbols, p4info, *directMeter);
        }
    }

    void addExternInstance(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4info,
                           const IR::ExternBlock *externBlock) override {
        auto decl = externBlock->node->to<IR::Declaration_Instance>();
        if (decl == nullptr) return;

        auto p4RtTypeInfo = p4info->mutable_type_info();
        if (externBlock->type->name == CounterTraits::typeName()) {
            auto counter = Helpers::Counterlike<ArchCounterExtern>::from(externBlock, refMap,
                                                                         typeMap, p4RtTypeInfo);
            if (counter) addCounter(symbols, p4info, *counter);
        } else if (externBlock->type->name == MeterTraits::typeName()) {
            auto meter = Helpers::Counterlike<ArchMeterExtern>::from(externBlock, refMap, typeMap,
                                                                     p4RtTypeInfo);
            if (meter) addMeter(symbols, p4info, *meter);
        } else if (externBlock->type->name == RegisterTraits<arch>::typeName()) {
            auto register_ = Register::from<arch>(externBlock, refMap, typeMap, p4RtTypeInfo);
            if (register_) addRegister(symbols, p4info, *register_);
        } else if (externBlock->type->name == ActionProfileTraits<arch>::typeName() ||
                   externBlock->type->name == ActionSelectorTraits<arch>::typeName()) {
            auto actionProfile = getActionProfile(externBlock);
            if (actionProfile) addActionProfile(symbols, p4info, *actionProfile);
        }
    }

    void addExternFunction(const P4RuntimeSymbolTableIface &, p4configv1::P4Info *,
                           const P4::ExternFunction *) override {}

    void postAdd(const P4RuntimeSymbolTableIface &, ::p4::config::v1::P4Info *) override {}

    void addExternEntries(const p4::v1::WriteRequest *, const P4RuntimeSymbolTableIface &,
                          const IR::ExternBlock *) override {}
    bool filterAnnotations(cstring) override { return false; }

    google::protobuf::util::JsonPrintOptions getJsonPrintOptions() override {
        return jsonPrintOptions;
    }

    static std::optional<ActionProfile> getActionProfile(cstring name, const IR::Type_Extern *type,
                                                         int64_t size,
                                                         const IR::IAnnotated *annotations) {
        ActionProfileType actionProfileType;
        if (type->name == ActionSelectorTraits<arch>::typeName()) {
            actionProfileType = ActionProfileType::INDIRECT_WITH_SELECTOR;
        } else if (type->name == ActionProfileTraits<arch>::typeName()) {
            actionProfileType = ActionProfileType::INDIRECT;
        } else {
            return std::nullopt;
        }

        return ActionProfile{name, actionProfileType, size, annotations};
    }

    /// @return the action profile referenced in @table's implementation property,
    /// if it has one, or std::nullopt otherwise.
    static std::optional<ActionProfile> getActionProfile(const IR::P4Table *table,
                                                         ReferenceMap *refMap, TypeMap *typeMap) {
        auto propertyName = ActionProfileTraits<arch>::propertyName();
        auto instance = getExternInstanceFromProperty(table, propertyName, refMap, typeMap);
        if (!instance) return std::nullopt;
        auto size = instance->substitution.lookupByName(ActionProfileTraits<arch>::sizeParamName())
                        ->expression;
        if (!size->template is<IR::Constant>()) {
            ::P4::error(ErrorType::ERR_INVALID, "Action profile '%1%' has non-constant size '%2%'",
                        *instance->name, size);
            return std::nullopt;
        }
        return getActionProfile(*instance->name, instance->type,
                                size->template to<IR::Constant>()->asInt(),
                                getTableImplementationAnnotations(table, refMap));
    }

    /// @return the action profile declared with @decl
    static std::optional<ActionProfile> getActionProfile(const IR::ExternBlock *instance) {
        auto decl = instance->node->to<IR::IDeclaration>();
        auto size = instance->getParameterValue(ActionProfileTraits<arch>::sizeParamName());
        if (!size->template is<IR::Constant>()) {
            ::P4::error(ErrorType::ERR_INVALID, "Action profile '%1%' has non-constant size '%2%'",
                        decl->controlPlaneName(), size);
            return std::nullopt;
        }
        return getActionProfile(decl->controlPlaneName(), instance->type,
                                size->template to<IR::Constant>()->asInt(),
                                decl->to<IR::IAnnotated>());
    }

    void addActionProfile(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4Info,
                          const ActionProfile &actionProfile) {
        auto profile = p4Info->add_action_profiles();
        auto id = symbols.getId(SymbolType::P4RT_ACTION_PROFILE(), actionProfile.name);
        setPreamble(profile->mutable_preamble(), id, actionProfile.name,
                    symbols.getAlias(actionProfile.name), actionProfile.annotations,
                    // exclude @max_group_size, @selector_size_semantics, and
                    // @max_member_weight if present
                    [](cstring name) {
                        return name == "max_group_size" || name == "selector_size_semantics" ||
                               name == "max_member_weight";
                    });
        profile->set_with_selector(actionProfile.type == ActionProfileType::INDIRECT_WITH_SELECTOR);
        profile->set_size(actionProfile.size);
        auto maxGroupSizeAnnotation = actionProfile.annotations->getAnnotation("max_group_size"_cs);
        if (maxGroupSizeAnnotation) {
            if (actionProfile.type == ActionProfileType::INDIRECT_WITH_SELECTOR) {
                auto maxGroupSizeConstant =
                    maxGroupSizeAnnotation->getExpr(0)->checkedTo<IR::Constant>();
                CHECK_NULL(maxGroupSizeConstant);
                profile->set_max_group_size(maxGroupSizeConstant->asInt());
            } else {
                ::P4::warning(ErrorType::WARN_IGNORE,
                              "Ignoring annotation @max_group_size on action profile '%1%', "
                              "which does not have a selector",
                              actionProfile.annotations);
            }
        }

        // By default, an action profile uses the SumOfWeights semantics.
        auto selectorSizeSemanticsAnnotation =
            actionProfile.annotations->getAnnotation("selector_size_semantics"_cs);
        if (selectorSizeSemanticsAnnotation) {
            if (actionProfile.type == ActionProfileType::INDIRECT_WITH_SELECTOR) {
                auto selectorSizeSemantics =
                    selectorSizeSemanticsAnnotation->getExpr(0)->checkedTo<IR::StringLiteral>();
                CHECK_NULL(selectorSizeSemantics);
                // The expression may only contain 'sum_of_weights' or 'sum_of_members'
                // in any case.
                if (selectorSizeSemantics->value.toUpper() == "SUM_OF_WEIGHTS") {
                    profile->mutable_sum_of_weights();
                } else if (selectorSizeSemantics->value.toUpper() == "SUM_OF_MEMBERS") {
                    profile->mutable_sum_of_members();
                } else {
                    ::P4::error(ErrorType::ERR_INVALID,
                                "Expected selector_size_semantics value \"sum_of_weights\" or "
                                "\"sum_of_members\", but got '%1%'",
                                selectorSizeSemantics);
                }
            } else {
                ::P4::warning(ErrorType::WARN_IGNORE,
                              "Ignoring annotation @selector_size_semantics on action "
                              "profile '%1%', which does not have a selector ",
                              actionProfile.annotations);
            }
        }

        // By default, an action profile uses the SumOfWeights semantics.
        auto maxMemberWeightAnnotation =
            actionProfile.annotations->getAnnotation("max_member_weight"_cs);
        if (maxMemberWeightAnnotation) {
            if (actionProfile.type == ActionProfileType::INDIRECT_WITH_SELECTOR &&
                profile->has_sum_of_members()) {
                auto maxMemberWeightConstant =
                    maxMemberWeightAnnotation->getExpr(0)->checkedTo<IR::Constant>();
                CHECK_NULL(maxMemberWeightConstant);
                profile->mutable_sum_of_members()->set_max_member_weight(
                    maxMemberWeightConstant->asInt());
            } else if (actionProfile.type != ActionProfileType::INDIRECT_WITH_SELECTOR) {
                ::P4::warning(ErrorType::WARN_IGNORE,
                              "Ignoring annotation @max_member_weight on action profile "
                              "'%1%', which does not have a selector",
                              actionProfile.annotations);
            } else {
                ::P4::warning(ErrorType::WARN_IGNORE,
                              "Ignoring annotation @max_member_weight on action profile '%1%', "
                              "which does not use 'sum_of_members' as its SelectorSizeSemantics",
                              actionProfile.annotations);
            }
        }

        auto tablesIt = actionProfilesRefs.find(actionProfile.name);
        if (tablesIt != actionProfilesRefs.end()) {
            for (const auto &table : tablesIt->second)
                profile->add_table_ids(symbols.getId(P4RuntimeSymbolType::P4RT_TABLE(), table));
        }
    }

    /// Set common fields between Counter and DirectCounter.
    template <typename Kind>
    void setCounterCommon(const P4RuntimeSymbolTableIface &symbols, Kind *counter, p4rt_id_t id,
                          const Helpers::Counterlike<ArchCounterExtern> &counterInstance) {
        setPreamble(counter->mutable_preamble(), id, counterInstance.name,
                    symbols.getAlias(counterInstance.name), counterInstance.annotations);
        auto counter_spec = counter->mutable_spec();
        counter_spec->set_unit(CounterTraits::mapUnitName(counterInstance.unit));
    }

    void addCounter(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4Info,
                    const Helpers::Counterlike<ArchCounterExtern> &counterInstance) {
        if (counterInstance.table) {
            auto counter = p4Info->add_direct_counters();
            auto id = symbols.getId(SymbolType::P4RT_DIRECT_COUNTER(), counterInstance.name);
            setCounterCommon(symbols, counter, id, counterInstance);
            auto tableId = symbols.getId(P4RuntimeSymbolType::P4RT_TABLE(), *counterInstance.table);
            counter->set_direct_table_id(tableId);
        } else {
            auto counter = p4Info->add_counters();
            auto id = symbols.getId(SymbolType::P4RT_COUNTER(), counterInstance.name);
            setCounterCommon(symbols, counter, id, counterInstance);
            counter->set_size(counterInstance.size);
            if (counterInstance.index_type_name) {
                counter->mutable_index_type_name()->set_name(counterInstance.index_type_name);
            }
        }
    }

    /// Set common fields between Meter and DirectMeter.
    template <typename Kind>
    void setMeterCommon(const P4RuntimeSymbolTableIface &symbols, Kind *meter, p4rt_id_t id,
                        const Helpers::Counterlike<ArchMeterExtern> &meterInstance) {
        setPreamble(meter->mutable_preamble(), id, meterInstance.name,
                    symbols.getAlias(meterInstance.name), meterInstance.annotations);
        auto meter_spec = meter->mutable_spec();
        meter_spec->set_unit(MeterTraits::mapUnitName(meterInstance.unit));
    }

    void addMeter(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4Info,
                  const Helpers::Counterlike<ArchMeterExtern> &meterInstance) {
        if (meterInstance.table) {
            auto meter = p4Info->add_direct_meters();
            auto id = symbols.getId(SymbolType::P4RT_DIRECT_METER(), meterInstance.name);
            setMeterCommon(symbols, meter, id, meterInstance);
            auto tableId = symbols.getId(P4RuntimeSymbolType::P4RT_TABLE(), *meterInstance.table);
            meter->set_direct_table_id(tableId);
        } else {
            auto meter = p4Info->add_meters();
            auto id = symbols.getId(SymbolType::P4RT_METER(), meterInstance.name);
            setMeterCommon(symbols, meter, id, meterInstance);
            meter->set_size(meterInstance.size);
            if (meterInstance.index_type_name) {
                meter->mutable_index_type_name()->set_name(meterInstance.index_type_name);
            }
        }
    }

    void addRegister(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4Info,
                     const Register &registerInstance) {
        auto register_ = p4Info->add_registers();
        auto id = symbols.getId(SymbolType::P4RT_REGISTER(), registerInstance.name);
        setPreamble(register_->mutable_preamble(), id, registerInstance.name,
                    symbols.getAlias(registerInstance.name), registerInstance.annotations);
        register_->set_size(registerInstance.size);
        register_->mutable_type_spec()->CopyFrom(*registerInstance.typeSpec);
        if (registerInstance.index_type_name) {
            register_->mutable_index_type_name()->set_name(registerInstance.index_type_name);
        }
    }

    void addDigest(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4Info,
                   const Digest &digest) {
        // Each call to digest() creates a new digest entry in the P4Info.
        // Right now we only take the type of data included in the digest
        // (encoded in its name) into account, but it may be that we should also
        // consider the receiver.
        auto id = symbols.getId(SymbolType::P4RT_DIGEST(), digest.name);
        if (serializedInstances.find(id) != serializedInstances.end()) return;
        serializedInstances.insert(id);

        auto *digestInstance = p4Info->add_digests();
        setPreamble(digestInstance->mutable_preamble(), id, digest.name,
                    symbols.getAlias(digest.name), digest.annotations);
        digestInstance->mutable_type_spec()->CopyFrom(*digest.typeSpec);
    }

    /// @return the table implementation property, or nullptr if the table has no
    /// such property.
    static const IR::Property *getTableImplementationProperty(const IR::P4Table *table) {
        return table->properties->getProperty(ActionProfileTraits<arch>::propertyName());
    }

    static const IR::IAnnotated *getTableImplementationAnnotations(const IR::P4Table *table,
                                                                   ReferenceMap *refMap) {
        // Cannot use auto here, otherwise the compiler seems to think that the
        // type of impl is dependent on the template parameter and we run into
        // this issue: https://stackoverflow.com/a/15572442/4538702
        const IR::Property *impl = getTableImplementationProperty(table);
        if (impl == nullptr) return nullptr;
        if (!impl->value->is<IR::ExpressionValue>()) return nullptr;
        auto expr = impl->value->to<IR::ExpressionValue>()->expression;
        if (expr->is<IR::ConstructorCallExpression>()) return impl->to<IR::IAnnotated>();
        if (expr->is<IR::PathExpression>()) {
            auto decl = refMap->getDeclaration(expr->to<IR::PathExpression>()->path, true);
            return decl->to<IR::IAnnotated>();
        }
        return nullptr;
    }

    static std::optional<cstring> getTableImplementationName(const IR::P4Table *table,
                                                             ReferenceMap *refMap) {
        const IR::Property *impl = getTableImplementationProperty(table);
        if (impl == nullptr) return std::nullopt;
        if (!impl->value->is<IR::ExpressionValue>()) {
            ::P4::error(
                ErrorType::ERR_EXPECTED,
                "Expected implementation property value for table %1% to be an expression: %2%",
                table->controlPlaneName(), impl);
            return std::nullopt;
        }
        auto expr = impl->value->to<IR::ExpressionValue>()->expression;
        if (expr->is<IR::ConstructorCallExpression>()) return impl->controlPlaneName();
        if (expr->is<IR::PathExpression>()) {
            auto decl = refMap->getDeclaration(expr->to<IR::PathExpression>()->path, true);
            return decl->controlPlaneName();
        }
        return std::nullopt;
    }

    ReferenceMap *refMap;
    TypeMap *typeMap;
    const IR::ToplevelBlock *evaluatedProgram;

    std::unordered_map<cstring, std::set<cstring>> actionProfilesRefs;

    /// The extern instances we've serialized so far. Used for deduplication.
    std::set<p4rt_id_t> serializedInstances;

    // JSON printing options for serialization
    google::protobuf::util::JsonPrintOptions jsonPrintOptions;
};

/// Implements  a common @ref P4RuntimeArchHandlerIface for the PSA and PNA architecture. The
/// overridden methods will be called by the @P4RuntimeSerializer to collect and
/// serialize PSA and PNA specific symbols which are exposed to the control-plane.
template <Arch arch>
class P4RuntimeArchHandlerPSAPNA : public P4RuntimeArchHandlerCommon<arch> {
 public:
    P4RuntimeArchHandlerPSAPNA(ReferenceMap *refMap, TypeMap *typeMap,
                               const IR::ToplevelBlock *evaluatedProgram)
        : P4RuntimeArchHandlerCommon<arch>(refMap, typeMap, evaluatedProgram) {}

    void collectExternInstance(P4RuntimeSymbolTableIface *symbols,
                               const IR::ExternBlock *externBlock) override {
        P4RuntimeArchHandlerCommon<arch>::collectExternInstance(symbols, externBlock);

        auto decl = externBlock->node->to<IR::IDeclaration>();
        if (decl == nullptr) return;
        if (externBlock->type->name == "Digest") {
            symbols->add(SymbolType::P4RT_DIGEST(), decl);
        }
    }

    void addTableProperties(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4info,
                            p4configv1::Table *table, const IR::TableBlock *tableBlock) override {
        P4RuntimeArchHandlerCommon<arch>::addTableProperties(symbols, p4info, table, tableBlock);

        auto tableDeclaration = tableBlock->container;
        bool supportsTimeout = getSupportsTimeout(tableDeclaration);
        if (supportsTimeout) {
            table->set_idle_timeout_behavior(p4configv1::Table::NOTIFY_CONTROL);
        } else {
            table->set_idle_timeout_behavior(p4configv1::Table::NO_TIMEOUT);
        }
    }

    void addExternInstance(const P4RuntimeSymbolTableIface &symbols, p4configv1::P4Info *p4info,
                           const IR::ExternBlock *externBlock) override {
        P4RuntimeArchHandlerCommon<arch>::addExternInstance(symbols, p4info, externBlock);

        auto decl = externBlock->node->to<IR::Declaration_Instance>();
        if (decl == nullptr) return;
        auto p4RtTypeInfo = p4info->mutable_type_info();
        if (externBlock->type->name == "Digest") {
            auto digest = getDigest(decl, p4RtTypeInfo);
            if (digest) this->addDigest(symbols, p4info, *digest);
        }
    }

    /// @return serialization information for the Digest extern instacne @decl
    std::optional<Digest> getDigest(const IR::Declaration_Instance *decl,
                                    p4configv1::P4TypeInfo *p4RtTypeInfo) {
        BUG_CHECK(decl->type->is<IR::Type_Specialized>(), "%1%: expected Type_Specialized",
                  decl->type);
        auto type = decl->type->to<IR::Type_Specialized>();
        BUG_CHECK(type->arguments->size() == 1, "%1%: expected one type argument", decl);
        auto typeArg = type->arguments->at(0);
        auto typeSpec =
            TypeSpecConverter::convert(this->refMap, this->typeMap, typeArg, p4RtTypeInfo);
        BUG_CHECK(typeSpec != nullptr,
                  "P4 type %1% could not be converted to P4Info P4DataTypeSpec");

        return Digest{decl->controlPlaneName(), typeSpec, decl->to<IR::IAnnotated>()};
    }

    /// @return true if @table's 'psa_idle_timeout' property exists and is true. This
    /// indicates that @table supports entry ageing.
    static bool getSupportsTimeout(const IR::P4Table *table) {
        auto timeout = table->properties->getProperty("psa_idle_timeout");

        if (timeout == nullptr) return false;

        if (auto exprValue = timeout->value->to<IR::ExpressionValue>()) {
            if (auto expr = exprValue->expression) {
                if (auto member = expr->to<IR::Member>()) {
                    if (member->member == "NOTIFY_CONTROL") {
                        return true;
                    } else if (member->member == "NO_TIMEOUT") {
                        return false;
                    }
                } else if (expr->is<IR::PathExpression>()) {
                    ::P4::error(ErrorType::ERR_UNEXPECTED,
                                "Unresolved value %1% for psa_idle_timeout "
                                "property on table %2%. Must be a constant and one of "
                                "{ NOTIFY_CONTROL, NO_TIMEOUT }",
                                timeout, table);
                    return false;
                }
            }
        }

        ::P4::error(ErrorType::ERR_UNEXPECTED,
                    "Unexpected value %1% for psa_idle_timeout "
                    "property on table %2%. Supported values are "
                    "{ NOTIFY_CONTROL, NO_TIMEOUT }",
                    timeout, table);
        return false;
    }
};

/// Implements @ref P4RuntimeArchHandlerIface for the UBPF architecture.
/// We re-use PSA to handle externs.
/// Rationale: The only configurable extern object in ubpf_model.p4 is Register.
/// The Register is defined exactly the same as for PSA. Therefore, we can re-use PSA.
class P4RuntimeArchHandlerUBPF final : public P4RuntimeArchHandlerCommon<Arch::PSA> {
 public:
    P4RuntimeArchHandlerUBPF(ReferenceMap *refMap, TypeMap *typeMap,
                             const IR::ToplevelBlock *evaluatedProgram)
        : P4RuntimeArchHandlerCommon<Arch::PSA>(refMap, typeMap, evaluatedProgram) {}
};

class P4RuntimeArchHandlerPSA final : public P4RuntimeArchHandlerPSAPNA<Arch::PSA> {
 public:
    P4RuntimeArchHandlerPSA(ReferenceMap *refMap, TypeMap *typeMap,
                            const IR::ToplevelBlock *evaluatedProgram)
        : P4RuntimeArchHandlerPSAPNA(refMap, typeMap, evaluatedProgram) {}
};

class P4RuntimeArchHandlerPNA final : public P4RuntimeArchHandlerPSAPNA<Arch::PNA> {
 public:
    P4RuntimeArchHandlerPNA(ReferenceMap *refMap, TypeMap *typeMap,
                            const IR::ToplevelBlock *evaluatedProgram)
        : P4RuntimeArchHandlerPSAPNA(refMap, typeMap, evaluatedProgram) {}
};

}  // namespace Standard

}  // namespace ControlPlaneAPI

/** @} */ /* end group control_plane */
}  // namespace P4

#endif /* CONTROL_PLANE_P4RUNTIMEARCHSTANDARD_H_ */
