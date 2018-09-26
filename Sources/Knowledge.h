#ifndef Knowledge_H_
#define Knowledge_H_

#include <algorithm>
#include <any>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "IndexFactory.h"
#include "LogBox.h"
#include "MT19937.h"
#include "Rule.h"
#include "Semantics.h"
#include "XMLreader.h"

#include <functional>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

template <typename T>
struct LengthGreater {
  bool operator()(const T &lhs, const T &rhs) const { return lhs.size() > rhs.size() || (lhs.size() == rhs.size() && lhs < rhs); }
};
struct HashSymbolVector {
  std::size_t operator()(const std::vector<SymbolElement> &dst) const {
    size_t seed = 0;
    constexpr size_t value = pow<std::size_t>(2, 32) / (1 + std::sqrt(5)) * 2;
    for (const SymbolElement &val : dst) {
      seed ^= std::hash<SymbolElement>()(val) + value + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

class ParseLink {
  friend class Knowledge;

  struct ParseNode {
    friend std::hash<ParseLink::ParseNode>;
    Rule r;
    std::vector<SymbolElement> str;
    std::unordered_set<AMean> record;
    std::vector<std::reference_wrapper<ParseNode>> next;
    ParseNode() : r(), str(), record(), next() {}
    ParseNode(const Rule &r) : r(r), str(), record(), next() {}
    ParseNode(const Rule &r, const AMean &m) : r(r), str(), record({m}), next() {}
    ParseNode(const ParseNode &dst) : r(dst.r), str(dst.str), record(dst.record), next(dst.next) {}
    ParseNode &operator=(const ParseNode &dst) {
      r = dst.r;
      str = dst.str;
      next = dst.next;
      return *this;
    }
    bool operator==(const ParseNode &dst) const { return r == dst.r && str == dst.str; }
  };

  std::unordered_map<Category, std::map<std::vector<SymbolElement>, ParseNode, LengthGreater<std::vector<SymbolElement>>>> dic;
  std::map<std::vector<SymbolElement>, std::reference_wrapper<ParseNode>, LengthGreater<std::vector<SymbolElement>>> str_dic;
  std::map<std::vector<SymbolElement>, std::reference_wrapper<ParseNode>, LengthGreater<std::vector<SymbolElement>>>::iterator
      bottom_up_search_it;

 public:
  std::unordered_set<SymbolElement> symbol_set;
  std::unordered_set<SymbolElement> symbol_set_1;

 public:
  static ParseNode empty_node;
  ParseLink() : dic() {}
  std::size_t get_dic_size() const { return dic.size(); }
  void build_str_dic() {
    std::for_each(std::begin(dic), std::end(dic), [this](auto &dic_map) {
      std::for_each(std::begin(dic_map.second), std::end(dic_map.second),
                    [this](auto &dic_pair) { str_dic.try_emplace(dic_pair.first, dic_pair.second); });
    });
  }
  auto end_it() { return std::end(str_dic); }
  bool empty() { return dic.size() == 0; }
  std::string to_s() const {
    std::ostringstream os;
    os << "Dic size: " << get_dic_size() << std::endl;
    int num = 1;
    std::for_each(std::begin(dic), std::end(dic), [&os, &num](auto &cat_string_map) {
      os << "Category: " << cat_string_map.first << std::endl;
      std::for_each(std::begin(cat_string_map.second), std::end(cat_string_map.second), [&os, &num](auto &string_pn) {
        os << num++ << ": " << string_pn.second.r << " // "
           << "(" << string_pn.first.size() << ")";
        std::copy(std::begin(string_pn.first), std::end(string_pn.first), std::ostream_iterator<SymbolElement>(os, " "));
        os << std::endl;
      });
    });
    return os.str();
  }
  auto bottom_up_search_init(void) { return bottom_up_search_it = std::begin(str_dic); }
  void bottom_up_search_init(
      std::map<std::vector<SymbolElement>, std::reference_wrapper<ParseNode>, LengthGreater<std::vector<SymbolElement>>>::iterator it) {
    bottom_up_search_it = it;
  }
  std::optional<std::reference_wrapper<ParseNode>> bottom_up_search_next(
      const std::vector<SymbolElement> &sel_vec,
      std::map<std::vector<SymbolElement>, std::reference_wrapper<ParseNode>, LengthGreater<std::vector<SymbolElement>>>::iterator &it) {
    for (; bottom_up_search_it != std::end(str_dic);) {
      ParseNode &p = (*(bottom_up_search_it++)).second;
      if (p.str.size() <= sel_vec.size() && std::equal(std::begin(p.str), std::end(p.str), std::begin(sel_vec))) {
        it = bottom_up_search_it;
        return p;
      }
    }
    return std::nullopt;
  }
  bool parse_init(const std::list<std::reference_wrapper<Rule>> rules, const std::vector<SymbolElement> &ref) {
    dic.clear();
    symbol_set_1 = symbol_set = std::unordered_set<SymbolElement>{std::begin(ref), std::end(ref)};
    bool b = false;
    std::list<std::reference_wrapper<Rule>> rules_sym;
    std::list<std::reference_wrapper<Rule>> rules_nt;
    std::unordered_map<std::vector<SymbolElement>, bool, HashSymbolVector> searched_str;
    std::for_each(std::begin(rules), std::end(rules), [this, &rules_sym, &rules_nt](Rule &r) {
      if (std::find_if(std::begin(r.get_external()), std::end(r.get_external()),
                       [](const SymbolElement &sel) { return sel.type() == ELEM_TYPE::NT_TYPE; }) == std::end(r.get_external())) {
        rules_sym.push_back(r);
      } else {
        rules_nt.push_back(r);
      }
    });
    if (std::find_if(std::begin(ref), std::end(ref), [](const SymbolElement &sel) { return sel.type() == ELEM_TYPE::SYM_TYPE; }) !=
        std::end(ref)) {
      std::for_each(std::begin(rules_sym), std::end(rules_sym),
                    [this, &ref, &b, &searched_str](const Rule &r) { b = add(r, ref, searched_str) || b; });
      while (b) {
        b = false;
        std::for_each(std::begin(rules_nt), std::end(rules_nt),
                      [this, &ref, &b, &searched_str](const Rule &r) { b = add(r, ref, searched_str) || b; });
        if (!symbol_set_1.empty()) return false;
      }
    } else {
      b = true;
      while (b) {
        b = false;
        std::for_each(std::begin(rules_nt), std::end(rules_nt),
                      [this, &ref, &b, &searched_str](const Rule &r) { b = add(r, ref, searched_str) || b; });
        if (!symbol_set_1.empty()) return false;
      }
    }

    return symbol_set.empty();
  }
  bool parse_add(const std::list<std::reference_wrapper<Rule>> rules, const std::vector<SymbolElement> &ref) {
    symbol_set_1 = symbol_set = std::unordered_set<SymbolElement>{std::begin(ref), std::end(ref)};
    bool b = false;
    std::list<std::reference_wrapper<Rule>> rules_sym;
    std::list<std::reference_wrapper<Rule>> rules_nt;
    std::unordered_map<std::vector<SymbolElement>, bool, HashSymbolVector> searched_str;
    std::for_each(std::begin(rules), std::end(rules), [this, &rules_sym, &rules_nt](Rule &r) {
      if (std::find_if(std::begin(r.get_external()), std::end(r.get_external()),
                       [](const SymbolElement &sel) { return sel.type() == ELEM_TYPE::NT_TYPE; }) == std::end(r.get_external())) {
        rules_sym.push_back(r);
      } else {
        rules_nt.push_back(r);
      }
    });
    if (std::find_if(std::begin(ref), std::end(ref), [](const SymbolElement &sel) { return sel.type() == ELEM_TYPE::SYM_TYPE; }) !=
        std::end(ref)) {
      std::for_each(std::begin(rules_sym), std::end(rules_sym),
                    [this, &ref, &b, &searched_str](const Rule &r) { b = add(r, ref, searched_str) || b; });
      while (b) {
        b = false;
        std::for_each(std::begin(rules_nt), std::end(rules_nt),
                      [this, &ref, &b, &searched_str](const Rule &r) { b = add(r, ref, searched_str) || b; });
        if (!symbol_set_1.empty()) return false;
      }
    } else {
      b = true;
      while (b) {
        b = false;
        std::for_each(std::begin(rules_nt), std::end(rules_nt),
                      [this, &ref, &b, &searched_str](const Rule &r) { b = add(r, ref, searched_str) || b; });
        if (!symbol_set_1.empty()) return false;
      }
    }

    return symbol_set.empty();
  }

 private:
  bool add(const Rule &r, const std::vector<SymbolElement> &ref,
           std::unordered_map<std::vector<SymbolElement>, bool, HashSymbolVector> &searched_str) {
    // std::unordered_map<std::vector<SymbolElement>, ParseNode, HashSymbolVector> box{
    //     {std::vector<SymbolElement>(), ParseNode(r, r.get_internal().get_base())}};
    std::unordered_map<std::vector<SymbolElement>, ParseNode, HashSymbolVector> box{{std::vector<SymbolElement>(), ParseNode(r)}};
    bool b = true;
    std::list<std::reference_wrapper<ParseNode>> used;
    std::for_each(
        std::begin(r.get_external()), std::end(r.get_external()),
        [this, &r, &ref, &box, &b, &used, &searched_str](const SymbolElement &sel) {
          switch (sel.type()) {
            case ELEM_TYPE::NT_TYPE: {
              if (b) {
                std::unordered_map<std::vector<SymbolElement>, ParseNode, HashSymbolVector> subbox;
                const Category &cat = sel.template get<RightNonterminal>().get_cat();

                // embedding pattern
                if (dic.count(cat) == 1) {
                  std::for_each(std::begin(dic[cat]), std::end(dic[cat]), [this, &r, &ref, &subbox, &box, &used, &searched_str](auto &pa) {
                    if (pa.second.record.find(r.get_internal().get_base()) == std::end(pa.second.record)) {
                      std::for_each(std::begin(box), std::end(box), [this, &r, &ref, &subbox, &pa, &used, &searched_str](auto &box_pair) {
                        ParseNode box_p = box_pair.second;
                        std::copy(std::begin(pa.second.str), std::end(pa.second.str), std::back_inserter(box_p.str));
                        const std::vector<SymbolElement> &key = box_p.str;
                        if (subbox.count(key) == 0 &&
                            (searched_str.count(key) == 0 || (searched_str.count(key) != 0 && searched_str[key]))) {
                          if (std::search(std::begin(ref), std::end(ref), std::begin(key), std::end(key)) != std::end(ref)) {
                            searched_str[key] = true;
                            box_p.next.push_back(pa.second);
                            subbox[key] = box_p;
                            used.push_back(pa.second);
                          } else {
                            searched_str[key] = false;
                          }
                        }
                      });
                    }
                  });
                }

                // not embedding pattern
                std::for_each(std::begin(box), std::end(box), [this, &r, &ref, &subbox, &sel, &searched_str](auto &box_pair) {
                  ParseNode box_p = box_pair.second;
                  box_p.str.push_back(sel);
                  const std::vector<SymbolElement> &key = box_p.str;
                  if (subbox.count(key) == 0 && (searched_str.count(key) == 0 || (searched_str.count(key) != 0 && searched_str[key]))) {
                    if (std::search(std::begin(ref), std::end(ref), std::begin(key), std::end(key)) != std::end(ref)) {
                      searched_str[key] = true;
                      box_p.next.push_back(std::reference_wrapper(empty_node));
                      subbox[key] = box_p;
                    } else {
                      searched_str[key] = false;
                    }
                  }
                });

                if (subbox.size() != 0)
                  box.swap(subbox);
                else
                  b = false;
              }
            } break;
            case ELEM_TYPE::SYM_TYPE: {
              if (b) {
                std::unordered_map<std::vector<SymbolElement>, ParseNode, HashSymbolVector> subbox;
                std::for_each(std::begin(box), std::end(box), [this, &r, &ref, &subbox, &sel, &searched_str](auto &box_pair) {
                  box_pair.second.str.push_back(sel);
                  const std::vector<SymbolElement> &key = box_pair.second.str;
                  if (subbox.count(key) == 0 && (searched_str.count(key) == 0 || (searched_str.count(key) != 0 && searched_str[key]))) {
                    if (std::search(std::begin(ref), std::end(ref), std::begin(key), std::end(key)) != std::end(ref)) {
                      searched_str[key] = true;
                      subbox[key] = box_pair.second;
                    } else {
                      searched_str[key] = false;
                    }
                  }
                });
                if (subbox.size() != 0)
                  box.swap(subbox);
                else
                  b = false;
              }
            } break;
          }
        });
    if (b) {
      b = false;
      std::for_each(std::begin(box), std::end(box), [this, &r, &b](auto &pair) {
        auto res_p = dic[r.get_internal().get_cat()].insert(pair);
        b = b || res_p.second;
      });
      if (b) {
        std::for_each(std::begin(used), std::end(used), [&r](ParseNode &p) { p.record.insert(r.get_internal().get_base()); });
        if (!symbol_set.empty())
          std::for_each(std::begin(r.get_external()), std::end(r.get_external()),
                        [this](const SymbolElement &sel) { symbol_set.erase(sel); });
      }
    }
    if (!symbol_set_1.empty())
      std::for_each(std::begin(r.get_external()), std::end(r.get_external()),
                    [this](const SymbolElement &sel) { symbol_set_1.erase(sel); });
    return b;
  }
  friend std::ostream &operator<<(std::ostream &out, const ParseLink &obj);
};

/*!
 * 知識集合を表すクラスです
 */
class Knowledge {
 public:
  typedef std::vector<Rule> RuleDBType;  // for std::shuffle()
  typedef std::map<Category, std::multimap<AMean, Rule>> DicDBType_cat;
  typedef std::map<AMean, std::multimap<Category, Rule>> DicDBType_amean;

  enum PATTERN_TYPE { ABSOLUTE, COMPLETE, SEMICOMPLETE, RANDOM, NUM_SORT };
  enum CONSOLIDATE_TYPE { CHUNK = 0, MERGE, REPLACE, ALL_METHOD };

  int ut_index;
  int ut_category;
  RuleDBType input_box;
  Semantics<Conception> intention;

  static bool LOGGING_FLAG;
  static int ABSENT_LIMIT;
  static uint32_t CONTROLS;
  static int buzz_length;
  static int EXPRESSION_LIMIT;
  static int RECURSIVE_LIMIT;

  static const uint32_t USE_OBLITERATION = 0x01;
  static const uint32_t USE_SEMICOMPLETE_FABRICATION = 0x02;
  static const uint32_t USE_ADDITION_OF_RANDOM_WORD = 0x03;
  static const uint32_t ANTECEDE_COMPOSITION = 0x04;

  Knowledge();
  ~Knowledge();
  Knowledge &operator=(const Knowledge &dst);

  /*!
   * 知識に対して、Chunk、Merge、Replaceを実行します。
   * これはいずれのルールも適用不可能になるまで実行されます。
   */
  bool consolidate(void);

  /*!
   * Chunkを全てのルールに対して実行します。
   * ただし、Chunkが不可能になるまで
   Chunkを繰り返すことを保証しません。
   */
  bool chunk(void);  //チャンク

  /*!
           * Mergeを全てのルールに対して実行します。
           * ただし、Mergeが不可能になるまで
           Mergeを繰り返すことを保証しません。
           */
  bool merge(void);  //マージ

  /*!
           * Replaceを全てのルールに対して実行します。
           * ただし、Replaceが不可能になるまで
           Replaceを繰り返すことを保証しません。
           */
  bool replace(void);  //リプレイス

  /*!
   * 渡されたRuleの内部言語から
   * 完全に外部言語列を構成可能かどうかを返す。
   * 真なら構成可能、偽なら不可能
   */
  bool acceptable(Rule &src);

  RuleDBType grounded_rules(Meaning ref);

  bool explain(Meaning, RuleDBType &);

  /*!
   * Ruleを知識集合のメールボックスに送ります。
   * Ruleはそのまま知識集合に格納されません。
   * それは、未処理のRuleだからです。
   * Chunk、Merge、Replaceなどの処理が終わって始めて
   * 知識集合に格納されます。
   */
  void send_box(Rule &mail);
  void send_box(RuleDBType &mails);

  /*!
   * Ruleを直接知識集合に追加します。
   */
  void send_db(Rule &mail);
  void send_db(RuleDBType &mails);

  /*!
   * 実行速度を上げるために、
   * 単語規則のハッシュを構成します。
   * これはfabricateが始めて実行されるときに
   * 自動的に呼び出されます。
   */
  void build_word_index(void);
  void clear(void);

  /*!
   * 知識集合の文字列表現を返します
   */
  std::string to_s(void);

  /*!
   * 実行ログを取るようにします。
   */
  static void logging_on(void);

  /*!
   * ログの取得を停止します。
   */
  static void logging_off(void);

  static void set_control(uint32_t FLAGS);

  RuleDBType rules(void);

  void define(const AMean &n, const Conception &m);

  void init_semantics_rules(TransRules &);

  Meaning meaning_no(int obj);

  void ground_with_pattern(Rule &src, RuleDBType &pattern);

  void dic_add(RuleDBType &vec_r);

  void dic_add(Rule &r);

  RuleDBType generate_score(std::map<AMean, Conception> &core_meaning, RuleDBType &base);

  void generate_score_mono(std::map<AMean, Conception> &core_meaning, UtteranceRules &base, UtteranceRules &ret);

  RuleDBType parse_string(const std::vector<SymbolElement> &str);

  std::string meaning_no_to_s(int obj);

  std::string dic_to_s();

  Rule at(std::size_t) const;

  std::size_t size() const;

 private:
  IndexFactory cat_indexer;
  IndexFactory var_indexer;
  IndexFactory ind_indexer;
  RuleDBType box_buffer;
  DicDBType_cat DB_cat_amean_dic;
  DicDBType_amean DB_amean_cat_dic;
  RuleDBType ruleDB;
  std::map<std::vector<SymbolElement>, RuleDBType> strDic;
  ParseLink parse_info;
  RuleDBType chunking(Rule &src, Rule &dst);
  bool chunking_loop(Rule &unchecked_sent, RuleDBType &checked_rules);
  bool merging(Rule &src);
  void collect_merge(Rule &, RuleDBType &, std::set<Category> &, std::set<AMean> &);
  RuleDBType merge_cat_proc(const Category &, RuleDBType &, std::set<Category> &);
  void merge_cat_proc_buffer(const Category &, RuleDBType &, std::set<Category> &);
  RuleDBType merge_mean_proc(const AMean &, RuleDBType &, std::set<AMean> &);
  void merge_mean_proc_buffer(const AMean &, RuleDBType &, std::set<AMean> &);
  bool replacing(Rule &word, RuleDBType &dst);

  template <typename T>
  void unique(std::vector<T> &);

  template <typename T>
  void unique(std::list<T> &);
  void unify(RuleDBType &DB);
  std::vector<SymbolElement> construct_buzz_word();
  bool construct_groundable_rules(const Category &c, Meaning m, std::function<void(RuleDBType &)> &f);
  bool construct_groundable_rules(const Category &c, Meaning m, std::function<void(RuleDBType &)> &f1, std::function<bool(Rule &)> &f2);
  bool product_loop;
  bool construct_groundable_rules(const Category &c, std::function<bool(std::vector<RuleDBType> &)> &f0,
                                  std::function<void(RuleDBType &)> &f1, std::function<bool(Rule &)> &f2);
  bool construct_groundable_rules_1(Rule &base, std::vector<RuleDBType> &prod,
                                    std::function<bool(const Category &, const std::any &)> &func);
  bool construct_groundable_rules_mono(const Category &c, std::function<bool(std::vector<RuleDBType> &)> &f0,
                                       std::function<void(RuleDBType &)> &f1, std::function<bool(Rule &)> &f2);
  bool construct_groundable_rules_mono_1(Rule &base, std::vector<RuleDBType> &prod,
                                         std::function<bool(const Category &, const std::any &)> &func);
  bool construct_parsed_rules(const std::vector<SymbolElement> &str, UtteranceRules &ur);
  bool construct_parsed_rules2(const std::vector<SymbolElement> &str, UtteranceRules &ur);
  std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> dic_cat_range(const Category &c);
  std::pair<std::multimap<Category, Rule>::iterator, std::multimap<Category, Rule>::iterator> dic_amean_range(const AMean &c);
  std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> dic_range(const Category &c, const AMean &m);
  std::string dic_cat_to_s();
  std::string dic_amean_to_s();
};

class ParseLink2 {
  friend class Knowledge;

  struct ParseNode {
    friend std::hash<ParseLink2::ParseNode>;
    Rule r;
    std::vector<SymbolElement> str;
    std::vector<std::reference_wrapper<ParseNode>> next;
    ParseNode() : r(), str(), next() {}
    ParseNode(const Rule &r) : r(r), str(), next() {}
    ParseNode(const ParseNode &dst) : r(dst.r), str(dst.str), next(dst.next) {}
    ParseNode &operator=(const ParseNode &dst) {
      r = dst.r;
      str = dst.str;
      next = dst.next;
      return *this;
    }
    bool operator==(const ParseNode &dst) const { return r == dst.r && str == dst.str; }
  };

  std::unordered_map<Category, std::map<std::vector<SymbolElement>, ParseNode, LengthGreater<std::vector<SymbolElement>>>> dic;
  std::map<std::vector<SymbolElement>, std::reference_wrapper<ParseNode>, LengthGreater<std::vector<SymbolElement>>> str_dic;
  std::map<std::vector<SymbolElement>, std::reference_wrapper<ParseNode>, LengthGreater<std::vector<SymbolElement>>>::iterator
      bottom_up_search_it;
  std::unordered_set<SymbolElement> symbol_set;

 public:
  static ParseNode empty_node;
  ParseLink2() : dic() {}
  std::size_t get_dic_size() const { return dic.size(); }
  void build_str_dic() {
    std::for_each(std::begin(dic), std::end(dic), [this](auto &dic_map) {
      std::for_each(std::begin(dic_map.second), std::end(dic_map.second),
                    [this](auto &dic_pair) { str_dic.try_emplace(dic_pair.first, dic_pair.second); });
    });
  }
  auto end_it() { return std::end(str_dic); }
  bool empty() { return dic.size() == 0; }
  std::string to_s() const {
    std::ostringstream os;
    os << "Dic size: " << get_dic_size() << std::endl;
    int num = 1;
    std::for_each(std::begin(dic), std::end(dic), [&os, &num](auto &cat_string_map) {
      os << "Category: " << cat_string_map.first << std::endl;
      std::for_each(std::begin(cat_string_map.second), std::end(cat_string_map.second), [&os, &num](auto &string_pn) {
        os << num++ << ": " << string_pn.second.r << " // "
           << "(" << string_pn.first.size() << ")";
        std::copy(std::begin(string_pn.first), std::end(string_pn.first), std::ostream_iterator<SymbolElement>(os, " "));
        os << std::endl;
      });
    });
    return os.str();
  }
  auto bottom_up_search_init(void) { return bottom_up_search_it = std::begin(str_dic); }
  void bottom_up_search_init(
      std::map<std::vector<SymbolElement>, std::reference_wrapper<ParseNode>, LengthGreater<std::vector<SymbolElement>>>::iterator it) {
    bottom_up_search_it = it;
  }
  std::optional<std::reference_wrapper<ParseNode>> bottom_up_search_next(
      const std::vector<SymbolElement> &sel_vec,
      std::map<std::vector<SymbolElement>, std::reference_wrapper<ParseNode>, LengthGreater<std::vector<SymbolElement>>>::iterator &it) {
    for (; bottom_up_search_it != std::end(str_dic);) {
      ParseNode &p = (*(bottom_up_search_it++)).second;
      if (p.str.size() <= sel_vec.size() && std::equal(std::begin(p.str), std::end(p.str), std::begin(sel_vec))) {
        it = bottom_up_search_it;
        return p;
      }
    }
    return std::nullopt;
  }
  bool parse_init(std::list<std::reference_wrapper<Rule>> rules, const std::vector<SymbolElement> &ref) {
    dic.clear();
    symbol_set = std::unordered_set<SymbolElement>{std::begin(ref), std::end(ref)};
    bool b = false;
    std::list<std::reference_wrapper<Rule>> rules_sym;
    std::list<std::reference_wrapper<Rule>> rules_nt;
    std::unordered_map<std::vector<SymbolElement>, bool, HashSymbolVector> searched_str;
    std::for_each(std::begin(rules), std::end(rules), [this, &rules_sym, &rules_nt](Rule &r) {
      if (std::find_if(std::begin(r.get_external()), std::end(r.get_external()),
                       [](SymbolElement &sel) { return sel.type() == ELEM_TYPE::NT_TYPE; }) == std::end(r.get_external())) {
        rules_sym.push_back(r);
      } else {
        rules_nt.push_back(r);
      }
    });
    if (std::find_if(std::begin(ref), std::end(ref), [](const SymbolElement &sel) { return sel.type() == ELEM_TYPE::SYM_TYPE; }) !=
        std::end(ref)) {
      std::for_each(std::begin(rules_sym), std::end(rules_sym),
                    [this, &ref, &b, &searched_str](Rule &r) { b = add(r, ref, searched_str) || b; });
      while (b) {
        b = false;
        std::for_each(std::begin(rules_nt), std::end(rules_nt),
                      [this, &ref, &b, &searched_str](Rule &r) { b = add(r, ref, searched_str) || b; });
      }
    } else {
      b = true;
      while (b) {
        b = false;
        std::for_each(std::begin(rules_nt), std::end(rules_nt),
                      [this, &ref, &b, &searched_str](Rule &r) { b = add(r, ref, searched_str) || b; });
      }
    }

    return symbol_set.empty();
  }
  bool parse_add(std::list<std::reference_wrapper<Rule>> rules, const std::vector<SymbolElement> &ref) {
    symbol_set = std::unordered_set<SymbolElement>{std::begin(ref), std::end(ref)};
    bool b = false;
    std::list<std::reference_wrapper<Rule>> rules_sym;
    std::list<std::reference_wrapper<Rule>> rules_nt;
    std::unordered_map<std::vector<SymbolElement>, bool, HashSymbolVector> searched_str;
    std::for_each(std::begin(rules), std::end(rules), [this, &rules_sym, &rules_nt](Rule &r) {
      if (std::find_if(std::begin(r.get_external()), std::end(r.get_external()),
                       [](SymbolElement &sel) { return sel.type() == ELEM_TYPE::NT_TYPE; }) == std::end(r.get_external())) {
        rules_sym.push_back(r);
      } else {
        rules_nt.push_back(r);
      }
    });
    if (std::find_if(std::begin(ref), std::end(ref), [](const SymbolElement &sel) { return sel.type() == ELEM_TYPE::SYM_TYPE; }) !=
        std::end(ref)) {
      std::for_each(std::begin(rules_sym), std::end(rules_sym),
                    [this, &ref, &b, &searched_str](Rule &r) { b = add(r, ref, searched_str) || b; });
      while (b) {
        b = false;
        std::for_each(std::begin(rules_nt), std::end(rules_nt),
                      [this, &ref, &b, &searched_str](Rule &r) { b = add(r, ref, searched_str) || b; });
      }
    } else {
      b = true;
      while (b) {
        b = false;
        std::for_each(std::begin(rules_nt), std::end(rules_nt),
                      [this, &ref, &b, &searched_str](Rule &r) { b = add(r, ref, searched_str) || b; });
      }
    }

    return symbol_set.empty();
  }

 private:
  bool add(Rule &r, const std::vector<SymbolElement> &ref,
           std::unordered_map<std::vector<SymbolElement>, bool, HashSymbolVector> &searched_str) {
    std::unordered_map<std::vector<SymbolElement>, ParseNode, HashSymbolVector> box{{std::vector<SymbolElement>(), ParseNode(r)}};
    bool b = true;
    std::for_each(std::begin(r.get_external()), std::end(r.get_external()), [this, &r, &ref, &box, &b, &searched_str](SymbolElement &sel) {
      switch (sel.type()) {
        case ELEM_TYPE::NT_TYPE: {
          if (b) {
            std::unordered_map<std::vector<SymbolElement>, ParseNode, HashSymbolVector> subbox;
            const Category &cat = sel.template get<RightNonterminal>().get_cat();

            // embedding pattern
            if (dic.count(cat) == 1) {
              std::for_each(std::begin(dic[cat]), std::end(dic[cat]), [this, &r, &ref, &subbox, &box, &searched_str](auto &pa) {
                std::for_each(std::begin(box), std::end(box), [this, &r, &ref, &subbox, &pa, &searched_str](auto &box_pair) {
                  ParseNode box_p = box_pair.second;
                  std::copy(std::begin(pa.second.str), std::end(pa.second.str), std::back_inserter(box_p.str));
                  const std::vector<SymbolElement> &key = box_p.str;
                  if (subbox.count(key) == 0 && (searched_str.count(key) == 0 || (searched_str.count(key) != 0 && searched_str[key]))) {
                    if (std::search(std::begin(ref), std::end(ref), std::begin(key), std::end(key)) != std::end(ref)) {
                      searched_str[key] = true;
                      box_p.next.push_back(pa.second);
                      subbox[key] = box_p;
                    } else {
                      searched_str[key] = false;
                    }
                  }
                });
              });
            }

            // not embedding pattern
            std::for_each(std::begin(box), std::end(box), [this, &r, &ref, &subbox, &sel, &searched_str](auto &box_pair) {
              ParseNode box_p = box_pair.second;
              box_p.str.push_back(sel);
              const std::vector<SymbolElement> &key = box_p.str;
              if (subbox.count(key) == 0 && (searched_str.count(key) == 0 || (searched_str.count(key) != 0 && searched_str[key]))) {
                if (std::search(std::begin(ref), std::end(ref), std::begin(key), std::end(key)) != std::end(ref)) {
                  searched_str[key] = true;
                  box_p.next.push_back(std::reference_wrapper(empty_node));
                  subbox[key] = box_p;
                } else {
                  searched_str[key] = false;
                }
              }
            });

            if (subbox.size() != 0)
              box.swap(subbox);
            else
              b = false;
          }
        } break;
        case ELEM_TYPE::SYM_TYPE: {
          if (b) {
            std::unordered_map<std::vector<SymbolElement>, ParseNode, HashSymbolVector> subbox;
            std::for_each(std::begin(box), std::end(box), [this, &r, &ref, &subbox, &sel, &searched_str](auto &box_pair) {
              box_pair.second.str.push_back(sel);
              const std::vector<SymbolElement> &key = box_pair.second.str;
              if (subbox.count(key) == 0 && (searched_str.count(key) == 0 || (searched_str.count(key) != 0 && searched_str[key]))) {
                if (std::search(std::begin(ref), std::end(ref), std::begin(key), std::end(key)) != std::end(ref)) {
                  searched_str[key] = true;
                  subbox[key] = box_pair.second;
                } else {
                  searched_str[key] = false;
                }
              }
            });
            if (subbox.size() != 0)
              box.swap(subbox);
            else
              b = false;
          }
        } break;
      }
    });
    if (b) {
      b = false;
      std::for_each(std::begin(box), std::end(box), [this, &r, &b](auto &pair) {
        auto res_p = dic[r.get_internal().get_cat()].insert(pair);
        b = b || res_p.second;
      });
      if (b) {
        if (symbol_set.size() != 0)
          std::for_each(std::begin(r.get_external()), std::end(r.get_external()),
                        [this](const SymbolElement &sel) { symbol_set.erase(sel); });
      }
    }
    return b;
  }
  friend std::ostream &operator<<(std::ostream &out, const ParseLink2 &obj);
};

#endif /* Knowledge_H_ */
