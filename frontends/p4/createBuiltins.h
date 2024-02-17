/*
Copyright 2013-present Barefoot Networks, Inc.

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

#ifndef P4_CREATEBUILTINS_H_
#define P4_CREATEBUILTINS_H_

#include "ir/ir.h"
#include "ir/visitor.h"

/*
 * Creates accept and reject states.
 * Adds parentheses to action invocations in tables:
    e.g., actions = { a; } becomes actions = { a(); }
 * Parser states without selects will transition to reject.
 */
namespace P4 {
class CreateBuiltins final : public TransformCRTP<CreateBuiltins> {
    using Base = TransformCRTP<CreateBuiltins>;
    bool addNoAction = false;
    /// toplevel declaration of NoAction.
    /// Checked only if it is referred.
    const IR::IDeclaration *globalNoAction;

    void checkGlobalAction();

 public:
    CreateBuiltins() {
        setName("CreateBuiltins");
        globalNoAction = nullptr;
    }
    using Base::preorder;
    using Base::postorder;
    const IR::Node *postorder(IR::ParserState *state);
    const IR::Node *postorder(IR::P4Parser *parser);
    const IR::Node *postorder(IR::ActionListElement *element);
    const IR::Node *postorder(IR::ExpressionValue *property);
    const IR::Node *postorder(IR::Entry *property);
    const IR::Node *preorder(IR::P4Table *table);
    const IR::Node *postorder(IR::ActionList *actions);
    const IR::Node *postorder(IR::TableProperties *properties);
    const IR::Node *postorder(IR::Property *property);
    const IR::Node *preorder(IR::P4Program *program);
};
}  // namespace P4

#endif /* P4_CREATEBUILTINS_H_ */
