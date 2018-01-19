/*
 * KnowledgeBase.h
 *
 *  Created on: 2016/11/16
 *      Author: Hiroki Sudo
 */

#ifndef KNOWLEDGEBASE_H_
#define KNOWLEDGEBASE_H_

 // #define DEBUG_KB

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include "Rule.h"
#include "Semantics.h"
#include "IndexFactory.h"
#include "MT19937.h"
#include "LogBox.h"

// #include "Distance.hpp"

class VectorSizeSort {
public:

	bool
		operator()(const std::vector<Rule> left,
			const std::vector<Rule> right) const {
		return left.size() > right.size();
	}
};

class ExternalSizeSort {
public:

	bool
		operator()(
			const Rule left,
			const Rule right
			) const {
		return
			left.external.size() < right.external.size();
	}
};

class RuleCompositionSort {
public:

	bool
		operator()(const Rule left, const Rule right) const {
		return left.composition() > right.composition();
	}
};

class KnowledgeBaseTypeDef {
public:
	typedef std::vector<Rule> RuleDBType;
	typedef std::map<int, std::multimap<int, Rule> > DicDBType;
	typedef std::multimap<Element, Rule> NormalDicType;

	typedef std::vector<Rule> PatternType;

};

/*!
 * 知識集合を表すクラスです
 */
class KnowledgeBase : public KnowledgeBaseTypeDef, public RuleTypeDef {
public:

	enum PATTERN_TYPE {
		ABSOLUTE, COMPLETE, SEMICOMPLETE, RANDOM, SORTED_ALL
	};

	IndexFactory cat_indexer;
	IndexFactory var_indexer;
	IndexFactory ind_indexer;
	// RuleDBType sbox_buffer;
	// RuleDBType sentence_box;
	// RuleDBType wbox_buffer;
	// RuleDBType word_box;
	RuleDBType box_buffer;
	RuleDBType input_box;
	DicDBType DB_dic;
	NormalDicType normal_DB_dic;

	RuleDBType sentenceDB;
	RuleDBType wordDB;

	Semantics<Conception> intention;

	typedef boost::unordered_map<std::vector<int>,
		boost::unordered_map<int, std::vector<Rule> > > IndexT;
	boost::shared_ptr<IndexT> fabricate_index;
	bool indexed;
	typedef std::pair<
		std::multimap<int, Rule>::iterator,
		std::multimap<int, Rule>::iterator
	> DictionaryRange;

	static bool LOGGING_FLAG;
	static int ABSENT_LIMIT;
	static uint32_t CONTROLS;
	static int buzz_length;
	static int RECURSIVE_LIMIT;
	// static int MEASURE_NO;
	static int VARIABLE_NO;

	static const uint32_t USE_OBLITERATION = 0x01;
	static const uint32_t USE_SEMICOMPLETE_FABRICATION = 0x02;
	static const uint32_t USE_ADDITION_OF_RANDOM_WORD = 0x03;
	static const uint32_t ANTECEDE_COMPOSITION = 0x04;

	KnowledgeBase();
	~KnowledgeBase();

	/*!
	 * 知識に対して、Chunk、Merge、Replaceを実行します。
	 * これはいずれのルールも
	 適用不可能になるまで実行されます。
	 */
	bool
		consolidate(void);

	/*!
	 * Chunkを全てのルールに対して実行します。
	 * ただし、Chunkが不可能になるまで
	 Chunkを繰り返すことを保証しません。
	 */
	bool
		chunk(void); //チャンク

		/*!
		 * Mergeを全てのルールに対して実行します。
		 * ただし、Mergeが不可能になるまで
		 Mergeを繰り返すことを保証しません。
		 */
	bool
		merge(void); //マージ

		/*!
		 * Replaceを全てのルールに対して実行します。
		 * ただし、Replaceが不可能になるまで
		 Replaceを繰り返すことを保証しません。
		 */
	bool
		replace(void); //リプレイス

		/*!
		 指定されたruleの組み合わせで
		 compositional ruleを生成しないようにします．
		 */
		 // void
		 // prohibited(KnowledgeBase::PatternType rules);

		 /*!
		  * 例外ルールである、単語削除を行います。
		  * これは、単語規則について、
		  内部言語が等しいものに対し、外部言語が最も短いものを残し、
		  * その他を削除するルールです。
		  */
		  // bool
		  // obliterate(void); //最短単語残す

		  /*!
		   * Ruleを受け取り、
		   その内部言語列に対応する外部言語列を生成し、
		   * その外部言語列をRuleに代入して返します。
		   なお生成ルールは以下のようになります。
		   * -# Ruleの内部言語列を構成可能で、
		   合成度の高いルールを使用する
		   * -# 合成度の高いルールで、
		   内部言語1要素だけが適合しない場合、
		   その1要素についてランダムの文字列を
		   当てて外部言語列を生成する
		   * -# 完全に内部言語が一致するルールを
		   使用する
		   * -# 上記に当てはまらない場合、
		   指定された長さ以下のランダムの文字列を生成する
		   * .
		   *
		   */
	Rule
		fabricate(Rule& src1);
	Rule
		pseudofabricate(Rule& src1);

	/*!
	 * 渡されたRuleの内部言語から
	 完全に外部言語列を構成可能かどうかを返す。
	 * 真なら構成可能、偽なら不可能
	 */
	bool
		acceptable(Rule& src);

	/*!
	 * Ruleを知識集合のメールボックスに送ります。
	 Ruleはそのまま知識集合に格納されません。
	 * それは、未処理のRuleだからです。
	 Chunk、Merge、Replaceなどの処理が終わって始めて
	 * 知識集合に格納されます。
	 */
	void
		send_box(Rule& mail);
	void
		send_box(std::vector<Rule>& mails);

	/*!
	 * Ruleを直接知識集合に追加します。
	 */
	void
		send_db(Rule& mail);
	void
		send_db(std::vector<Rule>& mails);

	/*!
	 * 実行速度を上げるために、
	 単語規則のハッシュを構成します。
	 * これはfabricateが始めて実行されるときに
	 自動的に呼び出されます。
	 */
	void
		build_word_index(void);
	void
		clear(void);

	/*!
	 * 知識集合の文字列表現を返します
	 */
	std::string
		to_s(void);

	/*!
	 * 実行ログを取るようにします。
	 */
	static void
		logging_on(void);

	/*!
	 * ログの取得を停止します。
	 */
	static void
		logging_off(void);

	KnowledgeBase&
		operator=(const KnowledgeBase& dst);
	static void
		set_control(uint32_t FLAGS);

	/*!
	 * Ruleの内部言語列に対応する外部言語列を
	 構成可能なとき、その構成に必要なRuleの集合を
	 * 構成可能パターンとして返します。
	 返値はMapクラスで、パターンのタイプをキーとして、
	 パターンの集合が入っています。
	 * パターンのタイプとは、
	 * - 1:完全一致（変数がないRule）
	 * - 2:完全構成（変数を含むRuleと単語Ruleの集合）
	 * - 3:不完全構成
	 （変数を含むRuleと、単語Rule、そしてランダム生成された単語規則）
	 */
	std::map<PATTERN_TYPE, std::vector<PatternType> >
		construct_grounding_patterns(Rule& src);
	bool
		lemma_construct_grounding_patterns(std::vector<Element>::iterator it,
			std::vector<Element>::iterator it_end,
			int location,
			int category,
			int& ungrounded_variable_num,
			std::vector<KnowledgeBase::PatternType>& patterns,
			bool& is_applied,
			bool& is_absolute,
			bool& is_complete,
			bool& is_semicomplete,
			bool& sent_search
		);

	void
		ground_with_pattern(Rule& src, PatternType& pattern);
	void
		ground_with_pattern_sub(
			int base_front,
			PatternType& pattern,
			Rule& src
		);
	std::map<PATTERN_TYPE, std::vector<PatternType> >
		construct_grounding_patterns_keep_id(Rule& src);
	bool
		lemma_construct_grounding_patterns_keep_id(std::vector<Element>::iterator it,
			std::vector<Element>::iterator it_end,
			int location,
			int category,
			int& ungrounded_variable_num,
			std::vector<KnowledgeBase::PatternType>& patterns,
			bool& is_applied,
			bool& is_absolute,
			bool& is_complete,
			bool& is_semicomplete,
			bool& sent_search
		);
	std::vector<Rule>
		grounded_rules(Rule src);
	void
		indexer(std::vector<Rule>& meanings);
	std::vector<Rule>
		rules(void);
	/*!
	 * Algorithm
	 * 1. 文ルールを一文ずつ回る．文ルール単位で分岐．文ルールをルールリストへ．
	 * 2. symbol列の分配の候補列挙（各ルールは一文字以上表現する）
	 と同時に文ルールの
	 *    symbol部分が一致するか．また，分配の候補で分岐
	 （文ルールとシンボル列をとって
	 *    std::vector< std::vector<Element> >を返す関数を使う）．
	 *    分配候補がない場合はパース不可→次の文ルールへ．
	 * 3. 各カテゴリ列単位で検査
	 * 4. 各カテゴリで与えられたシンボル列を
	 消費しながらルール集め．（parse_stepを利用）
	 *    消費の割り振りで分岐
	 （カテゴリとシンボル列をとって1つ以上のルールとシンボル列の
	 *    イテレータのstd::pairを返す関数を使う）．
	 同じ文字の割り振り間で直積を生成．
	 * 5. カテゴリ列単位での検査終了．
	 このとき，シンボル列イテレータがendを
	 *    指していないリストはパース不可．
	 symbol列の分配の候補ごとに直積を生成．
	 * 6. すべてのカテゴリ列検査終了．
	 symbol列の分配の候補ごとのルールリストを
	 ルールリストのリストへ．
	 * 7. 生成したルールリストのリストを回って，
	 各parse_tree生成（ルールリストをとってtreeを示す
	 std::string生成する関数を使う）．
	*/
	bool
		parse(
			Rule& src,
			std::vector<std::vector<Rule> >& r_list,
			std::vector<std::string>& t_list
		);
	void
		knowledge_tree(std::string&);

	bool
		explain(InType, std::vector<Rule>&);

	void
		define(Element a, Conception m);

	void
		init_semantics_rules(TransRules&);

	std::vector<Element>
		meaning_no(int obj);

	void
		dic_erase(DicDBType& dic, Rule r);

	void
		dic_add(DicDBType& dic, std::vector<Rule>& vec_r);

	void
		dic_add(DicDBType& dic, Rule r);

	void
		dic_change_cat(DicDBType& dic, int cat, int tocat);

	void
		dic_change_ind(DicDBType& dic, int ind, int toind);

	void
		dic_unify(DicDBType& dic);

	std::vector<Rule>
		generate_score(int beat_num, std::map<int, std::vector<std::string> >& core_meaning);

	bool
		create_measures(std::vector<Rule>& res, Element& cat_num, int beat_num);

	bool
		create_beats(std::vector<Rule>& res, std::vector<Element>& external, int beat_num);

	bool
		create_beat_eq(std::vector<Rule>& res, Element& elem, int space_num);

	std::string
		meaning_no_to_s(int obj);

private:
	std::vector<Rule>
		chunking(Rule& src, Rule& dst);
	bool
		chunking_loop(
			Rule& unchecked_sent,
			RuleDBType& checked_rules
		);

	bool
		merging(Rule& src);
	void
		collect_merge_cat(
			Rule&,
			RuleDBType&,
			std::map<int, bool>&,
			std::map<int, bool>&,
			std::map<int, bool>&
		);

	RuleDBType
		merge_sent_proc(int base_word_cat, RuleDBType& DB,
			std::map<int, bool>& unified_cat);
	RuleDBType
		merge_word_proc(int base_word_cat, RuleDBType& DB,
			std::map<int, bool>& unified_cat);
	RuleDBType
		merge_ind_proc(Element& base_word_el, RuleDBType& DB,
			std::map<int, bool>& unified_ind);

	bool
		replacing(Rule word, RuleDBType& checking_sents);

	KnowledgeBase::ExType
		construct_buzz_word(void);

	void
		unique(RuleDBType& DB);

	void
		unify(RuleDBType& DB);

	bool
		parse_step(
			ExType::iterator,
			ExType::iterator,
			int,
			std::vector<
			std::pair<std::vector<Rule>,
			ExType::iterator>
			>&,
			std::map<int, int>&
		);
	bool
		parse_option_symbols(
			Rule& base,
			ExType& ex,
			std::vector<std::vector<ExType> >& ex_lists
		);
	bool
		parse_option_symbols_no_end(
			Rule& base,
			ExType::iterator ex_it1,
			ExType::iterator ex_it2,
			std::vector<
			std::pair<
			std::vector<ExType>,
			ExType::iterator
			>
			>& options
		);

	int
		next_category(ExType::iterator, ExType::iterator);
	int
		next_symbol(ExType::iterator, ExType::iterator);

	std::pair<ExType::iterator, ExType::iterator>
		find_next_cat_seq(ExType::iterator begin_it, ExType::iterator end_it);

	bool
		unify_loop_sub(int, int, int, RuleDBType&, std::vector<int>&);

	std::vector<std::vector<int> >
		calculation_assignment(int,int);

	Element&
		return_cat(std::vector<Element>& external,int index);

private:
	friend class boost::serialization::access;

	template<class Archive>
	void
		serialize(Archive &ar, const unsigned int /* file_version */) {
		ar & BOOST_SERIALIZATION_NVP(cat_indexer);
		ar & BOOST_SERIALIZATION_NVP(var_indexer);
		ar & BOOST_SERIALIZATION_NVP(box_buffer);
		ar & BOOST_SERIALIZATION_NVP(input_box);
		ar & BOOST_SERIALIZATION_NVP(sentenceDB);
		ar & BOOST_SERIALIZATION_NVP(wordDB);

		/*
		 * Multimapはシリアライズできない（めんどくさい）ので
		 * DIC_BLDフラグを合わせて保存しないようにする
		 * そうすればFabricate時に自動で再構築される
		 bool DIC_BLD;
		 ar & BOOST_SERIALIZATION_NVP(DB_dic);
		 */

		 /*Staticをシリアライズすると何が起こるんだろうか*/
		ar & BOOST_SERIALIZATION_NVP(LOGGING_FLAG);
		ar & BOOST_SERIALIZATION_NVP(ABSENT_LIMIT);
		ar & BOOST_SERIALIZATION_NVP(CONTROLS);
		ar & BOOST_SERIALIZATION_NVP(buzz_length);
	}
};

#endif /* KNOWLEDGEBASE_H_ */
