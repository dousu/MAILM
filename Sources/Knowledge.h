#ifndef Knowledge_H_
#define Knowledge_H_

#include <algorithm>
#include <any>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "IndexFactory.h"
#include "LogBox.h"
#include "MT19937.h"
#include "Rule.h"
#include "Semantics.h"
#include "XMLreader.h"

class ParseLink {
  struct ParseNode {
    Rule r;
    std::list<std::reference_wrapper<SymbolElement>> str;
    std::vector<std::reference_wrapper<ParseNode>> next;
    ParseNode() : r(), str(), next() {}
    ParseNode(Rule r) : r(r), str(), next() {}
  };
  std::list<ParseNode> dic;
  std::list<std::reference_wrapper<ParseNode>> search_dic;
  std::list<std::reference_wrapper<ParseNode>>::iterator search_it;

 public:
  ParseLink() : dic() {}
  void expansion(std::vector<Rule> &rules, ParseNode &node) {
    rules.push_back(node.r);
    if (node.next.size() == 0) {
      return;
    } else {
      std::for_each(std::begin(node.next), std::end(node.next), [this, &rules](ParseNode &p) { expansion(rules, p); });
    }
  }
  bool search_init(std::list<SymbolElement> sel_vec) {
    search_dic.clear();
    // search_dic holds parse-nodes which has forward matching str with sel_vec

    search_it = std::begin(search_dic);
    return true;
  }
  void search() { search_it = std::begin(search_dic); }
  bool search_next(std::list<std::reference_wrapper<ParseNode>>::iterator &it) {
    if (search_it != std::end(search_dic)) {
      it = search_it++;
      return true;
    } else {
      return false;
    }
  }
  void add(std::list<std::reference_wrapper<Rule>> rules, std::set<SymbolElement> &ref, std::size_t limit) {
    dic.clear();
    bool b = false;
    std::list<std::reference_wrapper<Rule>> rules_sym;
    std::list<std::reference_wrapper<Rule>> rules_nt;
    std::for_each(std::begin(rules), std::end(rules), [this, &rules_sym, &rules_nt](Rule &r) {
      if (std::find_if(std::begin(r.get_external()), std::end(r.get_external()),
                       [](SymbolElement &sel) { return sel.type() == ELEM_TYPE::NT_TYPE; }) == std::end(r.get_external())) {
        rules_sym.push_back(r);
      } else {
        rules_nt.push_back(r);
      }
    });
    std::cout << "sym loop" << std::endl;
    std::for_each(std::begin(rules_sym), std::end(rules_sym), [this, &ref, &limit, &b](Rule &r) { b = b || add(r, ref, limit); });
    while (b) {
      std::cout << "nt loop" << std::endl;
      b = false;
      std::for_each(std::begin(rules_nt), std::end(rules_nt), [this, &ref, &limit, &b](Rule &r) { b = b || add(r, ref, limit); });
    }
  }

 private:
  bool add(Rule &r, std::set<SymbolElement> &ref, std::size_t limit) {
    std::list<ParseNode> box{{ParseNode(r)}};
    bool b;
    std::set<SymbolElement> set_r;
    std::copy_if(std::begin(r.get_external()), std::end(r.get_external()), std::inserter(set_r, std::begin(set_r)),
                 [](SymbolElement &sel) { return sel.type() == ELEM_TYPE::SYM_TYPE; });
    b = std::includes(std::begin(ref), std::end(ref), std::begin(set_r), std::end(set_r));
    if (!b) return b;
    std::for_each(std::begin(r.get_external()), std::end(r.get_external()), [this, &r, &limit, &box, &b](SymbolElement &sel) {
      switch (sel.type()) {
        case ELEM_TYPE::NT_TYPE: {
          if (b) {
            std::list<ParseNode> subbox;
            std::for_each(std::begin(dic), std::end(dic), [&r, &limit, &subbox, &sel, &box, &b](ParseNode &p) {
              if (r.get_internal().get_base() != p.r.get_internal().get_base() &&
                  sel.template get<RightNonterminal>().get_cat() == p.r.get_internal().get_cat()) {
                std::for_each(std::begin(box), std::end(box), [&limit, &subbox, &p](ParseNode box_p) {
                  if (p.str.size() + (box_p.str.size()) <= limit) {
                    box_p.next.push_back(p);
                    box_p.str.insert(std::end(box_p.str), std::begin(p.str), std::end(p.str));
                    std::copy(std::begin(p.str), std::end(p.str), std::back_inserter(box_p.str));
                    subbox.push_back(box_p);
                  }
                });
              }
            });
            if (subbox.size() != 0) {
              b = false;
              box.swap(subbox);
            }
          }
        } break;
        case ELEM_TYPE::SYM_TYPE: {
          if (b) {
            std::list<ParseNode> subbox;
            std::for_each(std::begin(box), std::end(box), [&limit, &subbox, &sel, &b](ParseNode &box_p) {
              if (box_p.str.size() + 1 <= limit) {
                box_p.str.push_back(sel);
                subbox.push_back(box_p);
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
    if (b) std::copy(std::begin(box), std::end(box), std::back_inserter(dic));
    return b;
  }
};

class KnowledgeTypeDef {
 public:
  typedef std::vector<Rule> RuleDBType;
  typedef std::map<Category, std::multimap<AMean, Rule>> DicDBType_cat;
  typedef std::map<AMean, std::multimap<Category, Rule>> DicDBType_amean;
};

/*!
 * 知識集合を表すクラスです
 */
class Knowledge : public KnowledgeTypeDef {
 public:
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
  std::map<std::list<SymbolElement>, RuleDBType> strDic;
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

  std::list<SymbolElement> construct_buzz_word();
  bool construct_groundable_rules(const Category &c, Meaning m, std::function<void(RuleDBType &)> &f);
  bool construct_groundable_rules(const Category &c, Meaning m, std::function<void(RuleDBType &)> &f1, std::function<bool(Rule &)> &f2);
  bool product_loop;
  bool construct_groundable_rules(const Category &c, std::function<bool(std::vector<RuleDBType> &)> &f0,
                                  std::function<void(RuleDBType &)> &f1, std::function<bool(Rule &)> &f2);
  bool construct_groundable_rules_1(Rule &base, std::vector<RuleDBType> &prod,
                                    std::function<bool(const Category &, const std::any &)> &func);
  bool construct_parsed_rules(std::list<SymbolElement> &str);

 public:
  void bottom_up_construction(std::list<SymbolElement> &str, ParseLink &pl);

 private:
  std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> dic_cat_range(const Category &c);
  std::pair<std::multimap<Category, Rule>::iterator, std::multimap<Category, Rule>::iterator> dic_amean_range(const AMean &c);
  std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> dic_range(const Category &c, const AMean &m);
  std::string dic_cat_to_s();
  std::string dic_amean_to_s();
};

#endif /* Knowledge_H_ */
