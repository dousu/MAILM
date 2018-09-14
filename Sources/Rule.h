#ifndef RULE_H_
#define RULE_H_

#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <variant>
#include <vector>

#include "Distance.hpp"
#include "Element.h"
#include "IndexFactory.h"
#include "Semantics.h"

template <class T>
constexpr T pow(T base, T exp) noexcept {
  return exp <= 0 ? 1 : exp == 1 ? base : base * pow(base, exp - 1);
}

class Rule;
template <>
struct std::hash<Rule> {
  std::size_t operator()(const Rule &) const noexcept;
};

class Rule {
  // member
  LeftNonterminal internal;             //内部
  std::vector<SymbolElement> external;  //外部言語列

 public:
  // constructor
  Rule() noexcept;
  Rule(const LeftNonterminal &lt, const std::vector<SymbolElement> &ex) noexcept;
  Rule(const Rule &o) noexcept : internal(o.internal), external(o.external) {}
  Rule(Rule &&o) noexcept : internal(std::move(o.internal)), external(std::move(o.external)) {}

  //! operator
  Rule &operator=(Rule &&o) noexcept;
  Rule &operator=(const Rule &o) noexcept;
  bool operator==(const Rule &dst) const;
  bool operator!=(Rule &dst) const;
  bool operator<(const Rule &dst) const;

  int size() const { return external.size(); };
  std::string to_s() const;
  bool is_noun(Semantics<Conception> &s) const;
  bool is_sentence(Semantics<Conception> &s) const;
  bool is_measure(Semantics<Conception> &s) const;
  void set_rule(LeftNonterminal &nt, std::vector<SymbolElement> &ex);
  LeftNonterminal &get_internal() { return internal; }
  const LeftNonterminal &get_internal() const { return internal; }
  std::vector<SymbolElement> &get_external() { return external; }
  const std::vector<SymbolElement> &get_external() const { return external; }
  friend std::ostream &operator<<(std::ostream &out, const Rule &obj);
  friend std::hash<Rule>;
};

class UtteranceRules {
  struct Node {
    Rule r;
    std::list<std::reference_wrapper<Node>> next;
    Node() : r(), next() {}
    Node(Rule &r) : r(r), next() {}
    Node(Rule &r, std::list<std::reference_wrapper<Node>> &n) : r(r), next(n) {}
    Node(const Node &dst) : r(dst.r), next(dst.next) {}
    Node &operator=(const Node &dst) {
      r = dst.r;
      next = dst.next;
      return *this;
    }
    friend std::hash<Node>;
  };
  Node top;
  std::list<Node> rules;
  UtteranceRules() : top(), rules() {}
  void list_rules(std::vector<Rule> &ret) {
    std::function<void(Node &)> func;
    func = [&ret, &func](Node &n) {
      ret.push_back(n.r);
      std::for_each(std::begin(n.next), std::end(n.next), func);
    };
    func(top);
  }
  void vector_rules(std::vector<Rule> &ret) {
    std::list<Rule> tmp;
    ret = std::vector<Rule>(std::begin(tmp), std::end(tmp));
  }
  void add(Node &n) {
    rules.push_back(n);
    n = rules.back();
  }
};

#endif /* RULE_H_ */
