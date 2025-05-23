/*
Copyright 2019 VMware, Inc.

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

#ifndef FRONTENDS_P4_VALIDATEMATCHANNOTATIONS_H_
#define FRONTENDS_P4_VALIDATEMATCHANNOTATIONS_H_

#include "frontends/p4/typeMap.h"
#include "ir/ir.h"
#include "lib/error.h"

namespace P4 {

/**
 * Checks that match annotations only have 1 argument which is of type match_kind.
 */
class ValidateMatchAnnotations final : public Inspector {
    TypeMap *typeMap;

 public:
    explicit ValidateMatchAnnotations(TypeMap *typeMap) : typeMap(typeMap) {
        setName("ValidateMatchAnnotations");
    }
    void postorder(const IR::Annotation *annotation) override {
        if (annotation->name != IR::Annotation::matchAnnotation) return;
        if (!isInContext<IR::StructField>()) return;
        // FIXME: Check annotation kind
        const auto &expr = annotation->getExpr();
        if (expr.size() != 1)
            ::P4::error(ErrorType::ERR_INVALID, "%1%: annotation must have exactly 1 argument",
                        annotation);
        auto e0 = expr.at(0);
        auto type = typeMap->getType(e0, true);
        if (type == nullptr) return;
        if (!type->is<IR::Type_MatchKind>())
            ::P4::error(ErrorType::ERR_TYPE_ERROR, "%1%: value must be a match_kind", e0);
    }
};

}  // namespace P4

#endif /* FRONTENDS_P4_VALIDATEMATCHANNOTATIONS_H_ */
