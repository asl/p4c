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

#ifndef IR_NODE_H_
#define IR_NODE_H_

#include <iosfwd>
#include <typeinfo>

#include <boost/container/small_vector.hpp>
#include <boost/range/iterator_range_core.hpp>

#include "ir-tree-macros.h"
#include "ir/gen-tree-macro.h"
#include "lib/castable.h"
#include "lib/cstring.h"
#include "lib/exceptions.h"
#include "lib/source_file.h"

class Visitor;
struct Visitor_Context;
class Inspector_base;
class Inspector;
template <class T> class InspectorCRTP;
class Modifier_base;
class Modifier;
template <class T> class ModifierCRTP;
class Transform_base;
class Transform;
template <class T> class TransformCRTP;
class JSONGenerator;
class JSONLoader;

namespace Util {
class JsonObject;
}  // namespace Util

namespace IR {

class Node;
class Annotation;  // IWYU pragma: keep
template <class T>
class Vector;  // IWYU pragma: keep
template <class T>
class IndexedVector;  // IWYU pragma: keep

/// SFINAE helper to check if given class has a `static_type_name`
/// method. Definite node classes have them while interfaces do not
template <class, class = void>
struct has_static_type_name : std::false_type {};

template <class T>
struct has_static_type_name<T, std::void_t<decltype(T::static_type_name())>> : std::true_type {};

template <class T>
inline constexpr bool has_static_type_name_v = has_static_type_name<T>::value;

// node interface
class INode : public Util::IHasSourceInfo, public IHasDbPrint, public ICastable {
 public:
    virtual ~INode() {}
    virtual const Node *getNode() const = 0;
    virtual Node *getNode() = 0;
    virtual void toJSON(JSONGenerator &) const = 0;
    virtual cstring node_type_name() const = 0;
    virtual void validate() const {}
    virtual const Annotation *getAnnotation(cstring) const { return nullptr; }
    

    // default checkedTo implementation for nodes: just fallback to generic ICastable method
    template <typename T>
    std::enable_if_t<!has_static_type_name_v<T>, const T *> checkedTo() const {
        return ICastable::checkedTo<T>();
    }

    // alternative checkedTo implementation that produces slightly better error message
    // due to node_type_name() / static_type_name() being available
    template <typename T>
    std::enable_if_t<has_static_type_name_v<T>, const T *> checkedTo() const {
        const auto *result = to<T>();
        BUG_CHECK(result, "Cast failed: %1% with type %2% is not a %3%.", this, node_type_name(),
                  T::static_type_name());
        return result;
    }

    DECLARE_TYPEINFO_WITH_TYPEID(INode, NodeKind::INode);
};

class NodeChildren;
class ReplacementNodeChildren;

class Node : public virtual INode {
 public:
    virtual void fill_children(NodeChildren &) const { }
    virtual void update_children(ReplacementNodeChildren &) { }
 public:
    virtual bool apply_visitor_preorder(Modifier &v);
    virtual void apply_visitor_postorder(Modifier &v);
    virtual void apply_visitor_revisit(Modifier &v, const Node *n) const;
    virtual void apply_visitor_loop_revisit(Modifier &v) const;
    virtual bool apply_visitor_preorder(Inspector &v) const;
    virtual void apply_visitor_postorder(Inspector &v) const;
    virtual void apply_visitor_revisit(Inspector &v) const;
    virtual void apply_visitor_loop_revisit(Inspector &v) const;
    virtual const Node *apply_visitor_preorder(Transform &v);
    virtual const Node *apply_visitor_postorder(Transform &v);
    virtual void apply_visitor_revisit(Transform &v, const Node *n) const;
    virtual void apply_visitor_loop_revisit(Transform &v) const;
    Node &operator=(const Node &) = default;
    Node &operator=(Node &&) = default;

 protected:
    static int currentId;
    void traceVisit(const char *visitor) const;
    virtual void visit_children(Visitor &) {}
    virtual void visit_children(Visitor &) const {}
    friend class ::Visitor;
    friend class ::Inspector_base;
    friend class ::Modifier_base;
    friend class ::Modifier;
    template <class T> friend class ::ModifierCRTP;
    friend class ::Transform_base;
    friend class ::Transform;
    template <class T> friend class ::TransformCRTP;
    cstring prepareSourceInfoForJSON(Util::SourceInfo &si, unsigned *lineNumber,
                                     unsigned *columnNumber) const;

 public:
    Util::SourceInfo srcInfo;
    int id;        // unique id for each node
    int clone_id;  // unique id this node was cloned from (recursively)
    void traceCreation() const;
    Node() : id(currentId++), clone_id(id) { traceCreation(); }
    explicit Node(Util::SourceInfo si) : srcInfo(si), id(currentId++), clone_id(id) {
        traceCreation();
    }
    Node(const Node &other) : srcInfo(other.srcInfo), id(currentId++), clone_id(other.clone_id) {
        traceCreation();
    }
    virtual ~Node() {}
    const Node *apply(Visitor &v, const Visitor_Context *ctxt = nullptr) const;
    const Node *apply(Visitor &&v, const Visitor_Context *ctxt = nullptr) const {
        return apply(v, ctxt);
    }
    virtual Node *clone() const = 0;
    void dbprint(std::ostream &out) const override;
    virtual void dump_fields(std::ostream &) const {}
    const Node *getNode() const final { return this; }
    Node *getNode() final { return this; }
    Util::SourceInfo getSourceInfo() const override { return srcInfo; }
    cstring node_type_name() const override { return "Node"; }
    static cstring static_type_name() { return "Node"; }
    virtual int num_children() { return 0; }
    explicit Node(JSONLoader &json);
    cstring toString() const override { return node_type_name(); }
    void toJSON(JSONGenerator &json) const override;
    void sourceInfoToJSON(JSONGenerator &json) const;
    Util::JsonObject *sourceInfoJsonObj() const;
    /* operator== does a 'shallow' comparison, comparing two Node subclass objects for equality,
     * and comparing pointers in the Node directly for equality */
    virtual bool operator==(const Node &a) const { return this->typeId() == a.typeId(); }
    /* 'equiv' does a deep-equals comparison, comparing all non-pointer fields and recursing
     * though all Node subclass pointers to compare them with 'equiv' as well. */
    virtual bool equiv(const Node &a) const { return this->typeId() == a.typeId(); }
#define DEFINE_OPEQ_FUNC(CLASS, BASE) \
    virtual bool operator==(const CLASS &) const { return false; }
    IRNODE_ALL_SUBCLASSES(DEFINE_OPEQ_FUNC)
#undef DEFINE_OPEQ_FUNC

    bool operator!=(const Node &n) const { return !operator==(n); }

    DECLARE_TYPEINFO_WITH_TYPEID(Node, NodeKind::Node, INode);
};

// simple version of dbprint
cstring dbp(const INode *node);

inline bool equal(const Node *a, const Node *b) { return a == b || (a && b && *a == *b); }
inline bool equal(const INode *a, const INode *b) {
    return a == b || (a && b && *a->getNode() == *b->getNode());
}
inline bool equiv(const Node *a, const Node *b) { return a == b || (a && b && a->equiv(*b)); }
inline bool equiv(const INode *a, const INode *b) {
    return a == b || (a && b && a->getNode()->equiv(*b->getNode()));
}
// NOLINTBEGIN(bugprone-macro-parentheses)
/* common things that ALL Node subclasses must define */
#define IRNODE_SUBCLASS(T)                             \
 public:                                               \
    T *clone() const override { return new T(*this); } \
    IRNODE_COMMON_SUBCLASS(T)
#define IRNODE_ABSTRACT_SUBCLASS(T) \
 public:                            \
    T *clone() const override = 0;  \
    IRNODE_COMMON_SUBCLASS(T)

// NOLINTEND(bugprone-macro-parentheses)
#define IRNODE_COMMON_SUBCLASS(T)                                           \
 public:                                                                    \
    using Node::operator==;                                                 \
    bool apply_visitor_preorder(Modifier &v) override;                      \
    void apply_visitor_postorder(Modifier &v) override;                     \
    void apply_visitor_revisit(Modifier &v, const Node *n) const override;  \
    void apply_visitor_loop_revisit(Modifier &v) const override;            \
    bool apply_visitor_preorder(Inspector &v) const override;               \
    void apply_visitor_postorder(Inspector &v) const override;              \
    void apply_visitor_revisit(Inspector &v) const override;                \
    void apply_visitor_loop_revisit(Inspector &v) const override;           \
    const Node *apply_visitor_preorder(Transform &v) override;              \
    const Node *apply_visitor_postorder(Transform &v) override;             \
    void apply_visitor_revisit(Transform &v, const Node *n) const override; \
    void apply_visitor_loop_revisit(Transform &v) const override;

/* only define 'apply' for a limited number of classes (those we want to call
 * visitors directly on), as defining it and making it virtual would mean that
 * NO Transform could transform the class into a sibling class */
#define IRNODE_DECLARE_APPLY_OVERLOAD(T)                                       \
    const T *apply(Visitor &v, const Visitor_Context *ctxt = nullptr) const;   \
    const T *apply(Visitor &&v, const Visitor_Context *ctxt = nullptr) const { \
        return apply(v, ctxt);                                                 \
    }
#define IRNODE_DEFINE_APPLY_OVERLOAD(CLASS, TEMPLATE, TT)                                    \
    TEMPLATE                                                                                 \
    const IR::CLASS TT *IR::CLASS TT::apply(Visitor &v, const Visitor_Context *ctxt) const { \
        const CLASS *tmp = this;                                                             \
        auto prof = v.init_apply(tmp, ctxt);                                                 \
        v.visit(tmp);                                                                        \
        v.end_apply(tmp);                                                                    \
        return tmp;                                                                          \
    }

enum class GroupTraversalKind {
    Sequential,
    SplitFlow,
    Conditional,
};

class NodeChildren {
    using ChildInfo = std::pair<const Node *, const char *>;
    using GroupInfo = std::pair<GroupTraversalKind, size_t>;
    using Children = boost::container::small_vector<ChildInfo, 16>;
    Children children;
    boost::container::small_vector<GroupInfo, 4> groups;
    void add_child(const Node *n, const char *name) { children.emplace_back(n, name); }
    void finish_group(GroupTraversalKind kind) { groups.emplace_back(kind, children.size()); }
    void add_next_child(const Node *n) {
        add_child(n, nullptr);
    }
    void add_next_child(const Node *n, const char *name) {
        add_child(n, name);
    }
    template <class... T>
    void add_next_child(const Node *n, const Node *next_node, const T *...rest) {
        add_child(n, nullptr);
        add_next_child(next_node, rest...);
    }
    template <class... T>
    void add_next_child(const Node *n, const char *name, const T *...rest) {
        add_child(n, name);
        add_next_child(rest...);
    }
 public:
    using ChildrenRange = boost::iterator_range<Children::const_iterator>;

    bool empty() const { return children.empty(); }
    size_t children_count() const { return children.size(); }
    size_t group_count() const { return groups.size(); }
    GroupTraversalKind group_kind(size_t i) const {
        BUG_CHECK(i < groups.size(), "group index out of bounds");
        return groups[i].first;
    }
    ChildrenRange group_children(size_t i) const {
        BUG_CHECK(i < groups.size(), "group index out of bounds");
        size_t from = (i > 0) ? groups[i - 1].second : 0;
        size_t to = groups[i].second;
        return boost::make_iterator_range(children.begin() + from, children.begin() + to);
    }

    template <GroupTraversalKind Kind = GroupTraversalKind::Sequential, class... T>
    void add_children(const T *...n) {
        children.reserve(children.size() + sizeof...(n));
        add_next_child(n...);
        finish_group(Kind);
    }
    template <GroupTraversalKind Kind = GroupTraversalKind::Sequential, class It>
    void add_children_range(It from, It to, const char *name = nullptr) {
        children.reserve(children.size() + std::distance(from, to));
        for (auto it = from; it != to; ++it)
            add_child(*it, name);
        finish_group(Kind);
    }
    template <GroupTraversalKind Kind = GroupTraversalKind::Sequential, class It>
    void add_named_children_range(It from, It to) {
        children.reserve(children.size() + std::distance(from, to));
        for (auto it = from; it != to; ++it)
            add_child(it->first, it->second);
        finish_group(Kind);
    }
};

template <class T>
static void update_node_field(const T *&n, const Node *t) {
    if (!t)
        n = nullptr;
    else if (t->is<T>())
        n = static_cast<const T *>(t);
    else
        BUG("visitor returned a node of invalid type: %1%", t);
}

class ReplacementNodeChildren {
    using Children = boost::container::small_vector<const Node *, 16>;
    Children children;
    boost::container::small_vector<size_t, 4> groups;
    size_t current_group = 0;
    size_t group_start = 0;

    template <size_t I>
    void update_next_node_field() {
        BUG_CHECK(group_start + I == groups[current_group],
                  "unexpected child count in the current group");
    }
    template <size_t I, class T, class... U>
    void update_next_node_field(const T *&n, const U *&...rest) {
        update_node_field(n, children[group_start + I]);
        update_next_node_field<I + 1>(rest...);
    }

 protected:
    void add_child(const Node *n) { children.emplace_back(n); }
    void clear_unfinished_group() {
        if (groups.empty())
            children.clear();
        else
            children.resize(groups.back());
    }
    void finish_group() { groups.emplace_back(children.size()); }
    ReplacementNodeChildren(const NodeChildren &nch) {
        children.reserve(nch.children_count());
        groups.reserve(nch.group_count());
    }

 public:
    using ChildrenRange = boost::iterator_range<Children::const_iterator>;

    template <class... T>
    void update_node_fields(const T *&...n) {
        BUG_CHECK(current_group < groups.size(), "no more groups");
        BUG_CHECK(sizeof...(T) == groups[current_group] - group_start,
                  "unexpected number of children");
        update_next_node_field<0>(n...);
        group_start = groups[current_group++];
    }
    ChildrenRange get_next_group() {
        BUG_CHECK(current_group < groups.size(), "no more groups");
        auto from = children.begin() + group_start;
        auto to = children.begin() + groups[current_group];
        group_start = groups[current_group++];
        return boost::make_iterator_range(from, to);
    }
};

class ReplacementNodeChildrenFill : public ReplacementNodeChildren {
 public:
    ReplacementNodeChildrenFill(const NodeChildren &nch) : ReplacementNodeChildren(nch) {}
    using ReplacementNodeChildren::add_child;
    using ReplacementNodeChildren::clear_unfinished_group;
    using ReplacementNodeChildren::finish_group;
};

}  // namespace IR

#endif /* IR_NODE_H_ */
