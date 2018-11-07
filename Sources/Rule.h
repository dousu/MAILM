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

namespace MAILM {
template <class T>
constexpr T pow(T base, T exp) noexcept {
  return exp <= 0 ? 1 : exp == 1 ? base : base * pow(base, exp - 1);
}
constexpr double sqrt(std::size_t base) noexcept {
  return 2.2360679775;  // 5
}
}  // namespace MAILM

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
  auto has_key(Semantics<Conception> &s) const;
  auto has_time(Semantics<Conception> &s) const;
  void set_rule(LeftNonterminal &nt, std::vector<SymbolElement> &ex);
  LeftNonterminal &get_internal() { return internal; }
  const LeftNonterminal &get_internal() const { return internal; }
  std::vector<SymbolElement> &get_external() { return external; }
  const std::vector<SymbolElement> &get_external() const { return external; }
  friend std::ostream &operator<<(std::ostream &out, const Rule &obj);
  friend std::hash<Rule>;
};

struct UtteranceRules {
  struct Node {
    Rule r;
    std::list<std::reference_wrapper<Node>> next;
    std::string property;
    Node() : r(), next() {}
    Node(const Rule &r) : r(r), next() {}
    Node(const Rule &r, const std::list<std::reference_wrapper<Node>> &n) : r(r), next(n) {}
    Node(const Node &dst) : r(dst.r), next(dst.next) {}
    Node &operator=(const Node &dst) {
      r = dst.r;
      next = dst.next;
      return *this;
    }
    bool operator==(const Node &obj) const { return r == obj.r; }
    friend std::hash<Node>;
  };

  Node top;

 private:
  std::list<Node> rules;
  void expansion(Node &loc, std::function<void(const Node &)> &func) {
    func(loc);
    std::for_each(std::begin(loc.next), std::end(loc.next), [this, &func](Node &node) { expansion(node, func); });
  }

 public:
  static Node empty_node;
  static void expansion(UtteranceRules &ur, std::function<void(const Node &)> &func) {
    func(ur.top);
    std::for_each(std::begin(ur.top.next), std::end(ur.top.next), [&ur, &func](Node &node) { ur.expansion(node, func); });
  }
  UtteranceRules() : top(), rules() {}
  void list_rules(std::list<Rule> &ret) const {
    ret.clear();
    std::function<void(const Node &)> func;
    func = [&ret, &func](const Node &n) {
      ret.push_back(n.r);
      std::for_each(std::begin(n.next), std::end(n.next), func);
    };
    func(top);
  }
  void vector_rules(std::vector<Rule> &ret) const {
    ret.clear();
    std::list<Rule> tmp;
    list_rules(tmp);
    std::copy(std::begin(tmp), std::end(tmp), std::back_inserter(ret));
  }
  void clear() {
    rules.clear();
    top = Node();
  }
  Node &add(const Node &n) {
    rules.push_back(n);
    return rules.back();
  }
  Node &add_top(const Node &n) {
    top = n;
    return top;
  }
  void sub_rules(Node &n, UtteranceRules &ur) {
    std::function<Node &(Node &)> func;
    func = [&ur, &func](Node &n) -> Node & {
      Node &node = ur.add(n);
      std::for_each(std::begin(n.next), std::end(n.next), [&ur, &node, &func](Node &node) { node.next.push_back(func(node)); });
      return node;
    };
    Node &top = ur.add_top(n);
    std::for_each(std::begin(n.next), std::end(n.next), [&ur, &top, &func](Node &node) { top.next.push_back(func(node)); });
  }
  std::vector<SymbolElement> string() {
    std::vector<SymbolElement> ret;
    std::function<void(Node &)> func;
    func = [&ret, &func](Node &n) {
      auto it = std::begin(n.next);
      std::for_each(std::begin(n.r.get_external()), std::end(n.r.get_external()), [&func, &ret, &n, &it](const SymbolElement &sel) {
        if (sel.type() == ELEM_TYPE::NT_TYPE) {
          if (it == std::end(n.next)) {
            std::cerr << "Error: UtteranceRules::string()" << std::endl;
            exit(1);
          }
          func(*(it++));
        } else {
          ret.push_back(sel);
        }
      });
    };
    auto it = std::begin(top.next);
    std::for_each(std::begin(top.r.get_external()), std::end(top.r.get_external()), [this, &func, &ret, &it](const SymbolElement &sel) {
      if (sel.type() == ELEM_TYPE::NT_TYPE) {
        if (it == std::end(top.next)) {
          std::cerr << "Error: UtteranceRules::string()" << std::endl;
          exit(1);
        }
        func(*(it++));
      } else {
        ret.push_back(sel);
      }
    });
    return ret;
  }
  std::string to_s() const {
    std::ostringstream os;
    os << "UtteranceRules" << std::endl;
    std::list<Rule> rlist;
    list_rules(rlist);
    std::copy(std::begin(rlist), std::end(rlist), std::ostream_iterator<Rule>(os, "\n"));
    return os.str();
  }
  template <typename T>
  void construct(const T &list) {
    clear();
    Node &node = add_top(list.front());
    std::function<std::pair<typename T::const_iterator, Node &>(typename T::const_iterator, const typename T::const_iterator)> func;
    func =
        [ this, &func ](typename T::const_iterator s_it, typename T::const_iterator e_it) -> std::pair<typename T::const_iterator, Node &> {
      Node &node = add(*s_it);
      typename T::const_iterator it = std::next(s_it);
      std::for_each(std::cbegin((*s_it).get_external()), std::cend((*s_it).get_external()),
                    [&node, &func, &it, &e_it](const SymbolElement &sel) {
                      if (sel.type() == ELEM_TYPE::NT_TYPE) {
                        auto ret = func(it, e_it);
                        it = ret.first;
                        node.next.push_back(ret.second);
                      }
                    });
      return {it, node};
    };
    typename T::const_iterator it = std::next(std::begin(list));
    std::for_each(std::cbegin(list.front().get_external()), std::cend(list.front().get_external()),
                  [this, &node, &list, &func, &it](const SymbolElement &sel) {
                    if (sel.type() == ELEM_TYPE::NT_TYPE) {
                      auto ret = func(it, std::end(list));
                      it = ret.first;
                      node.next.push_back(ret.second);
                    }
                  });
  }
  void insert(Node &n, UtteranceRules &ur) {
    if (std::count_if(std::begin(n.r.get_external()), std::end(n.r.get_external()),
                      [](const SymbolElement &sel) { return sel.type() == ELEM_TYPE::NT_TYPE; }) == n.next.size()) {
      std::cerr << "Error: UtteranceRules::insert()" << std::endl;
      exit(1);
    }
    std::vector<SymbolElement>::iterator it;
    std::size_t s = 0;
    while (s == n.next.size()) {
      it = std::find_if(std::begin(n.r.get_external()), std::end(n.r.get_external()),
                        [](const SymbolElement &sel) { return sel.type() == ELEM_TYPE::NT_TYPE; });
      s++;
    }
    if ((*it).get<RightNonterminal>().get_cat() != ur.top.r.get_internal().get_cat()) {
      std::cerr << "Error: UtteranceRules::insert()" << std::endl;
      exit(1);
    }
    std::function<Node &(Node &)> func;
    func = [this, &func](Node &n) -> Node & {
      Node &node = add(n.r);
      std::for_each(std::begin(n.next), std::end(n.next), [&func, &node](Node &nn) { node.next.push_back(func(nn)); });
      return node;
    };
    n.next.push_back(func(ur.top));
  }
  friend std::ostream &operator<<(std::ostream &out, const UtteranceRules &obj);
};

#endif /* RULE_H_ */
