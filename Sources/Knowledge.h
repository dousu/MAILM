#ifndef Knowledge_H_
#define Knowledge_H_

#include <algorithm>
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

  IndexFactory cat_indexer;
  IndexFactory var_indexer;
  IndexFactory ind_indexer;
  RuleDBType input_box;
  RuleDBType box_buffer;
  DicDBType_cat DB_cat_amean_dic;
  DicDBType_amean DB_amean_cat_dic;

  RuleDBType ruleDB;

  Semantics<Conception> intention;

  int ut_index;
  int ut_category;

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
  bool construct_grounding_rules(const Category &c, Meaning m, std::function<void(RuleDBType &)> f);
  bool construct_grounding_rules(const Category &c, Meaning m, std::function<void(RuleDBType &)> f1, std::function<bool(Rule &)> f2);
  bool product_loop;
  bool all_construct_grounding_rules(const Category &c, std::function<bool(std::vector<RuleDBType> &)> f0,
                                     std::function<void(RuleDBType &)> f1, std::function<bool(Rule &)> f2);
  std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> dic_cat_range(const Category &c);
  std::pair<std::multimap<Category, Rule>::iterator, std::multimap<Category, Rule>::iterator> dic_amean_range(const AMean &c);
  std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> dic_range(const Category &c, const AMean &m);
  std::string dic_cat_to_s();
  std::string dic_amean_to_s();
};

template <class F, class... A>
struct lazy {
  F f;
  std::tuple<A...> a;
  lazy(F f, A... a) : f(f), a(a...) {}
  auto operator()() const { return std::apply(f, a); }
};

#endif /* Knowledge_H_ */
