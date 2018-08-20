#include "Knowledge.h"

bool Knowledge::LOGGING_FLAG = false;
int Knowledge::ABSENT_LIMIT = 1;
uint32_t Knowledge::CONTROLS = 0x00L;
int Knowledge::buzz_length = 3;
int Knowledge::EXPRESSION_LIMIT = 15;
int Knowledge::RECURSIVE_LIMIT = 3;

Knowledge::Knowledge() {
  cat_indexer.index_counter = 1;
  var_indexer.index_counter = 1;
  ind_indexer.index_counter = 1;
}

Knowledge::~Knowledge() {}

Knowledge &Knowledge::operator=(const Knowledge &dst) {
  cat_indexer = dst.cat_indexer;
  var_indexer = dst.var_indexer;
  ind_indexer = dst.ind_indexer;

  input_box = dst.input_box;
  ruleDB = dst.ruleDB;
  box_buffer = dst.box_buffer;

  DB_cat_amean_dic = dst.DB_cat_amean_dic;
  DB_amean_cat_dic = dst.DB_amean_cat_dic;
  LOGGING_FLAG = dst.LOGGING_FLAG;
  ABSENT_LIMIT = dst.ABSENT_LIMIT;
  CONTROLS = dst.CONTROLS;
  buzz_length = dst.buzz_length;

  intention = dst.intention;

  return *this;
}

void Knowledge::clear(void) {
  cat_indexer.index_counter = 1;
  var_indexer.index_counter = 1;
  ind_indexer.index_counter = 1;

  input_box.clear();
  ruleDB.clear();
  box_buffer.clear();

  DB_cat_amean_dic.clear();
  DB_amean_cat_dic.clear();
  intention.clear();
}

Rule Knowledge::at(std::size_t n) const {
  if (n >= ruleDB.size()) {
    std::cerr << "no exist : Knowledge::at(std::size_t)" << std::endl;
    exit(1);
  }
  return ruleDB[n];
}

std::size_t Knowledge::size() const { return ruleDB.size(); }

void Knowledge::send_box(Rule &mail) { input_box.push_back(mail); }

void Knowledge::send_box(RuleDBType &mails) {
  std::copy(std::begin(mails), std::end(mails), std::back_inserter(input_box));
  mails.clear();
}

void Knowledge::send_db(Rule &mail) { ruleDB.push_back(mail); }

void Knowledge::send_db(RuleDBType &mails) {
  std::copy(std::begin(mails), std::end(mails), std::back_inserter(ruleDB));
  mails.clear();
}

template <class FwdIt, typename Compare = std::less<>>
void quick_sort(FwdIt first, FwdIt last, Compare cmp = Compare()) {
  auto const N = std::distance(first, last);
  if (N <= 1) return;
  auto pivot = *std::next(first, N / 2);
  FwdIt middle1 = std::partition(first, last, [&pivot, &cmp](const auto &elem) { return cmp(elem, pivot); });
  FwdIt middle2 = std::partition(middle1, last, [&pivot, &cmp](const auto &elem) { return !cmp(pivot, elem); });
  quick_sort(first, middle1, cmp);  // assert(std::is_sorted(first, middle1, cmp));
  quick_sort(middle2, last, cmp);   // assert(std::is_sorted(middle2, last, cmp));
}

template <class BiDirIt, typename Compare = std::less<>>
void merge_sort(BiDirIt first, BiDirIt last, Compare cmp = Compare()) {
  auto const N = std::distance(first, last);
  if (N <= 1) return;
  auto const middle = std::next(first, N / 2);
  merge_sort(first, middle, cmp);                // assert(std::is_sorted(first, middle, cmp));
  merge_sort(middle, last, cmp);                 // assert(std::is_sorted(middle, last, cmp));
  std::inplace_merge(first, middle, last, cmp);  // assert(std::is_sorted(first, last, cmp));
}

template <typename T>
void Knowledge::unique(std::vector<T> &vec) {
  merge_sort(std::begin(vec), std::end(vec));
  vec.erase(std::unique(std::begin(vec), std::end(vec)), std::end(vec));
}

template <typename T>
void Knowledge::unique(std::list<T> &vec) {
  quick_sort(std::begin(vec), std::end(vec));
  vec.erase(std::unique(std::begin(vec), std::end(vec)), std::end(vec));
}

/*
 * ALGORITHM
 * Chunk
 * ・Chunkの結果はChunkを発生させる可能性がある
 * -新しい文規則が発生
 * ・Chunkの結果はMergeを発生させる可能性がある
 * --新しい単語規則が発生する
 * ・Chunkの結果はReplaceを発生させる可能性がある
 * --新しい単語規則が発生する
 * ・Sentenceカテゴリがwordに入る可能性はない
 *
 * Merge
 * ・Mergeの結果はChunkを発生させる可能性がある
 * --カテゴリが書き換えられた文規則が発生するため
 * ・Mergeの結果がMergeを発生させる可能性は無い
 * --単語や文規則の内部と外部が変わる事がないため
 * ・Mergeの結果がReplaceを発生させる可能性は無い
 * --単語や文の外部言語列が変化しないため
 * ・Sentenceカテゴリがwordに入る可能性はある
 *
 * Replace
 * ・Replaceの結果はChunkを発生させる可能性がある
 * --外部言語、内部言語が書き換えられた文や単語規則が発生する
 * ・Replaceの結果がMergeを発生させる可能性がある
 * --文規則の外部言語列が変化するため
 * ・Replaceの結果がReplaceを発生させる可能性がある
 * --文規則の外部言語列が変化するため
 * ・Sentenceカテゴリがwordに入る可能性はある
 */

bool Knowledge::consolidate(void) {
  // std::cout << intention.mapping_to_s() << std::endl;
  // std::cout << intention.rules_to_s() << std::endl;
  bool flag = true;
  std::array<int, CONSOLIDATE_TYPE::ALL_METHOD> ar, tmp;
  std::iota(std::begin(ar), std::end(ar), 0);
  if (LOGGING_FLAG) LogBox::push_log("\n\n!!CONSOLIDATE!!");
  while (flag) {
    flag = false;
    tmp = ar;
    std::shuffle(std::begin(tmp), std::end(tmp), MT19937::igen);
    std::for_each(std::begin(tmp), std::end(tmp), [&](int i) {
      switch (i) {
        case CONSOLIDATE_TYPE::CHUNK: {
          flag = chunk() || flag;
          break;
        }
        case CONSOLIDATE_TYPE::MERGE: {
          if ((flag = merge() || flag)) {
            unique(input_box);
          }
          break;
        }
        case CONSOLIDATE_TYPE::REPLACE: {
          flag = replace() || flag;
          break;
        }
        default:
          std::cerr << "Consolidate Error" << std::endl;
          exit(1);
      }
    });

    if (LOGGING_FLAG) {
      LogBox::refresh_log();
    }
  }

  if (LOGGING_FLAG) {
    LogBox::push_log("\n\n!!CONSOLIDATE FIN!!");
    LogBox::push_log("Knowledge Size: " + std::to_string(ruleDB.size() + box_buffer.size() + input_box.size()));
  }

  send_db(input_box);
  unique(ruleDB);

  if (box_buffer.size() != 0) {
    std::cerr << "Still remaining data(box_buffer): " << box_buffer.size() << std::endl;
    exit(1);
  }
  if (input_box.size() != 0) {
    std::cerr << "Still remaining data(input_box): " << input_box.size() << std::endl;
    exit(1);
  }
  build_word_index();

  return true;
}

bool Knowledge::chunk(void) {
  std::shuffle(std::begin(input_box), std::end(input_box), MT19937::igen);
  bool is_chunked;
  RuleDBType::iterator it;
  while ((it = std::begin(input_box)) != std::end(input_box)) {
    Rule r = *it;
    input_box.erase(it);
    is_chunked = chunking_loop(r, ruleDB);
    if (!is_chunked) is_chunked = is_chunked || chunking_loop(r, box_buffer);
    if (is_chunked) {
      break;
    } else {
      box_buffer.push_back(r);
    }
  }
  send_box(box_buffer);

  return is_chunked;
}

bool Knowledge::chunking_loop(Rule &unchecked_sent, RuleDBType &checked_rules) {
  RuleDBType buffer;
  bool is_chunked = false;
  auto it = std::begin(checked_rules);
  while (!is_chunked && it != std::end(checked_rules)) {
    Rule r = *it;
    buffer.clear();
    buffer = chunking(unchecked_sent, r);
    if (buffer.size() == 0) {
      it++;
    } else {
      if (LOGGING_FLAG) {
        LogBox::push_log("\n-->>CHUNK:");
        LogBox::push_log("**FROM");
        LogBox::push_log(unchecked_sent.to_s());
        LogBox::push_log(r.to_s());
        LogBox::push_log("**TO");
        std::for_each(std::begin(buffer), std::end(buffer), [&](Rule &temp) { LogBox::push_log(temp.to_s()); });
        LogBox::push_log("<<--CHUNK");
      }
      send_box(buffer);
      checked_rules.erase(it);
      is_chunked = true;
      break;
    }
  }
  return is_chunked;
}

/*
 # chunk!
 # Algorithm
 # //1．内部言語の差異の個数が1
 # //1.1 内部言語の差異は、(非変数, 変数)、
 または(非変数,非変数)の組み合わせのいずれか
 # 2．外部言語の前方一致部分、後方一致部分が
 いずれか空でない
 # 3．外部言語で一致部分を取り除いた時、
 差異部分がともに空でない
 # 4．外部言語の差異部分は、両方ともすべて文字か、
 または片方が変数1文字でもう片方がすべて文字
 #
 # 1．上記条件が満たされるとき以下
 # //1．内部言語の差異部分が変数であるとき
 # 1．外部言語列の差異部分のどちらか一方が
 変数であるとき
 # 2．変数でない方の規則について、
 外部言語の差異部分を使い単語規則を生成する。
 この単語規則のカテゴリは、
 差異部分の変数のカテゴリを使う．
 #    新奇な意味を作り出し，カテゴリと新奇な意味，
 外部言語列の差異で新しい規則を作る．
 #    さらにそのカテゴリから新奇な意味変数から
 元の意味変数の規則を導出するルールを作る．
 # 3. 新たな文規則を生成する．内部言語の対応する
 意味変数と外部言語列の差異部分の変数を新奇な
 意味に書き換えたものとする．
 #
 # 1．外部言語列の差異部分がともに非変数であるとき
 # 2．それぞれを表現する新奇な意味、
外部言語の差異部分を用いて
 #    単語規則を二つ生成する。
 この単語規則のカテゴリは新奇なカテゴリを
 #    生成したものを使用し、
 また二つの単語規則ともに同じものを使用する。
 # 3．元の文規則に対して、
 差異部分を変数で置き換える。
 この変数のカテゴリは2で使用したカテゴリを用いる．
 さらに，全体の意味を新奇なものにして
 #    単語規則で使った意味を追加する
 #
 # Chunkが発生した場合その瞬間に該当する規則は削除される
 #
 */
Knowledge::RuleDBType Knowledge::chunking(Rule &src, Rule &dst) {
  // 0: unchunkable
  // 1: chunk type 1
  // 2: chunk type 2

  //各チャンクタイプの動作
  // TYPE1: カテゴリの創出・意味変数の創出
  // TYPE2: カテゴリの引継・意味変数の引継

  enum CHUNK_TYPE { UNABLE = 0, TYPE1, TYPE2 };
  RuleDBType buf;

  bool multi_cat;
  if (src.get_internal().get_cat() != dst.get_internal().get_cat()) {
    if (intention.chunk_equal(src.get_internal().get_base(), dst.get_internal().get_base())) {
      multi_cat = true;
    } else {
      return buf;
    }
  }

  //外部言語検査
  //前方一致長の取得
  int fmatch_length = 0;
  for (auto src_it = src.get_external().begin(), dst_it = dst.get_external().begin();
       src_it != src.get_external().end() && dst_it != dst.get_external().end() && *src_it == *dst_it;
       fmatch_length++, src_it++, dst_it++) {
  }

  //後方一致長の取得
  int rmatch_length = 0;
  for (auto src_rit = src.get_external().rbegin(), dst_rit = dst.get_external().rbegin();
       src_rit != src.get_external().rend() && dst_rit != dst.get_external().rend() && *src_rit == *dst_rit;
       rmatch_length++, src_rit++, dst_rit++) {
  }

  //前・後方一致長が0でない
  //一致長和より外部言語列は長い
  if (fmatch_length + rmatch_length == 0 || fmatch_length + rmatch_length >= src.get_external().size() ||
      fmatch_length + rmatch_length >= dst.get_external().size())
    return buf;

  CHUNK_TYPE chunk_type = UNABLE;
  Rule base, targ;
  std::list<SymbolElement> noun1_ex, noun2_ex;
  std::copy(std::next(src.get_external().begin(), fmatch_length), std::prev(src.get_external().end(), rmatch_length),
            std::back_inserter(noun1_ex));
  std::copy(std::next(dst.get_external().begin(), fmatch_length), std::prev(dst.get_external().end(), rmatch_length),
            std::back_inserter(noun2_ex));
  if (noun1_ex.size() == 1 && noun1_ex.front().type() == ELEM_TYPE::NT_TYPE) {
    if (noun2_ex.size() == 1 && noun2_ex.front().type() == ELEM_TYPE::NT_TYPE) {
      return buf;  // type3
    } else {
      //基本はbase=src,targ=dst
      base = src;  // TYPE2の可能性あり
      targ = dst;
      chunk_type = TYPE2;
    }
  } else {
    if (noun2_ex.size() == 1 && noun2_ex.front().type() == ELEM_TYPE::NT_TYPE) {
      base = dst;
      targ = src;
      chunk_type = TYPE2;
      noun1_ex.swap(noun2_ex);
    } else {
      base = src;
      targ = dst;
      chunk_type = TYPE1;
    }
  }

  // for translation in intention
  int in_pos, d_size, e_size;
  in_pos = d_size = e_size = 0;
  // chunk条件チェック終了
  // CHUNK処理
  switch (chunk_type) {
    case TYPE1:  // type3も含む
    {
      int new_cat_id;        //汎化用category
      int new_var_id;        // sentのvariable
      int new_sent_ind_id1;  // sentのid
      int new_sent_ind_id2;  // sent2のid
      int new_ind_id1;       // noun1のid
      int new_ind_id2;       // noun2のid

      // generate
      new_cat_id = cat_indexer.generate();
      new_var_id = var_indexer.generate();
      new_sent_ind_id1 = ind_indexer.generate();
      new_sent_ind_id2 = ind_indexer.generate();
      new_ind_id1 = ind_indexer.generate();
      new_ind_id2 = ind_indexer.generate();

      std::list<MeaningElement> var_vector1, var_vector2;

      std::for_each(std::begin(noun1_ex), std::end(noun1_ex), [&](SymbolElement &se) {
        if (se.type() == ELEM_TYPE::NT_TYPE) {
          MeaningElement mel = se.get<RightNonterminal>().get_var();
          var_vector1.push_back(mel);
          d_size++;
        }
      });

      std::for_each(std::begin(noun2_ex), std::end(noun2_ex), [&](SymbolElement &se) {
        if (se.type() == ELEM_TYPE::NT_TYPE) {
          MeaningElement mel = se.get<RightNonterminal>().get_var();
          var_vector2.push_back(mel);
          e_size++;
        }
      });

      // noun1
      Rule noun1(LeftNonterminal(Category(new_cat_id), Meaning(AMean(new_ind_id1), var_vector1)), noun1_ex);
      // noun2
      Rule noun2(LeftNonterminal(Category(new_cat_id), Meaning(AMean(new_ind_id2), var_vector2)), noun2_ex);

      // insertポジション計算
      std::for_each(std::begin(targ.get_external()), std::next(std::begin(targ.get_external()), fmatch_length), [&](SymbolElement &sel) {
        if (sel.type() == ELEM_TYPE::NT_TYPE) {
          in_pos++;
        }
      });
      // Rule自体のindex分
      in_pos++;

      Meaning new_meaning = base.get_internal().get_means().replaced(in_pos, d_size, Variable(new_var_id));
      new_meaning = new_meaning.replaced(0, 1, Meaning(AMean(new_sent_ind_id1)));
      std::list<SymbolElement> vec_sel;
      std::copy_n(std::begin(base.get_external()), fmatch_length, std::back_inserter(vec_sel));
      vec_sel.push_back(RightNonterminal(Category(new_cat_id), Variable(new_var_id)));
      std::copy(std::prev(std::end(base.get_external()), rmatch_length), std::end(base.get_external()), std::back_inserter(vec_sel));

      Rule sent{LeftNonterminal{Category{base.get_internal().get_cat()}, new_meaning}, vec_sel};
      Rule sent2;
      if (multi_cat) {
        sent2 = Rule{LeftNonterminal{Category{targ.get_internal().get_cat()}, new_meaning.replaced(0, 1, Meaning(AMean(new_sent_ind_id2)))},
                     vec_sel};
      }
      buf.push_back(sent);
      if (multi_cat) {
        buf.push_back(sent2);
      }
      buf.push_back(noun1);
      buf.push_back(noun2);

      intention.chunk(base.get_internal().get_means().get_base(), targ.get_internal().get_means().get_base(),
                      sent.get_internal().get_means().get_base(), sent2.get_internal().get_means().get_base(),
                      noun1.get_internal().get_means().get_base(), noun2.get_internal().get_means().get_base(), in_pos, d_size, e_size,
                      chunk_type);

      break;
    }

    case TYPE2: {
      int new_ind_id_targ;  // nounのため
      int new_sent_ind_id1;
      int new_sent_ind_id2;

      // generate
      new_sent_ind_id1 = ind_indexer.generate();
      new_sent_ind_id2 = ind_indexer.generate();
      new_ind_id_targ = ind_indexer.generate();

      std::list<MeaningElement> var_vector;

      // var_vector
      std::for_each(std::begin(noun2_ex), std::end(noun2_ex), [&](SymbolElement &se) {
        if (se.type() == ELEM_TYPE::NT_TYPE) {
          var_vector.push_back(Variable(se.get<RightNonterminal>().get_var()));
          d_size++;
        }
      });

      // noun
      Rule noun(LeftNonterminal(Category{RightNonterminal(noun1_ex.front().get<RightNonterminal>()).get_cat()},
                                Meaning(AMean(new_ind_id_targ), var_vector)),
                noun2_ex);
      Rule sent{
          LeftNonterminal(Category{base.get_internal().get_cat()}, Meaning{AMean(new_sent_ind_id1), base.get_internal().get_followings()}),
          base.get_external()};
      Rule sent2;
      if (multi_cat) {
        sent2 = Rule{LeftNonterminal{Category{targ.get_internal().get_cat()},
                                     Meaning{AMean(new_sent_ind_id2), sent.get_internal().get_followings()}},
                     base.get_external()};
      }
      buf.push_back(sent);
      if (multi_cat) {
        buf.push_back(sent2);
      }
      buf.push_back(noun);

      // insertポジション計算
      std::for_each(std::begin(targ.get_external()), std::next(std::begin(targ.get_external()), fmatch_length), [&](SymbolElement &sel) {
        if (sel.type() == ELEM_TYPE::NT_TYPE) {
          in_pos++;
        }
      });
      // Rule自体のindex分
      in_pos++;

      intention.chunk(targ.get_internal().get_means().get_base(), base.get_internal().get_means().get_base(),
                      sent.get_internal().get_means().get_base(), sent2.get_internal().get_means().get_base(),
                      noun.get_internal().get_means().get_base(), AMean(), in_pos, d_size, e_size, chunk_type);

      break;
    }
    default:
      std::cerr << "CHUNK PROC ERROR" << std::endl;
      throw "CHUNK PROC ERROR";
  }

  return buf;
}

bool Knowledge::merge(void) {
  std::shuffle(std::begin(input_box), std::end(input_box), MT19937::igen);
  bool is_merged;
  RuleDBType::iterator it;
  while ((it = std::begin(input_box)) != std::end(input_box)) {
    Rule r = *it;
    input_box.erase(it);
    is_merged = merging(r);
    if (is_merged) {
      break;
    } else {
      box_buffer.push_back(r);
    }
  }
  send_box(box_buffer);

  return is_merged;
}

bool Knowledge::merging(Rule &src) {
  RuleDBType buf, sub_buf;

  // word 適合するか検索
  // Merge対象として適合したら、直接カテゴリを書き換えて、
  //書き換えられたカテゴリを収拾
  //※mergeで単語を書き換えた結果はReplaceに影響しない
  std::set<Category> unified_cat;
  std::set<AMean> unified_mean;

  //既存単語規則の被変更カテゴリの収拾
  collect_merge(src, input_box, unified_cat, unified_mean);

  //既存単語規則の被変更カテゴリの収拾
  collect_merge(src, ruleDB, unified_cat, unified_mean);

  //未検証単語規則の被変更カテゴリの収拾
  collect_merge(src, box_buffer, unified_cat, unified_mean);

  //被変更の数が0ならMergeは起こらない
  if (unified_cat.size() == 0 && unified_mean.size() == 0) {
    return false;
  }

  Category base_cat{cat_indexer.generate()};
  AMean base_mean{ind_indexer.generate()};
  unified_cat.insert(src.get_internal().get_cat());
  unified_mean.insert(src.get_internal().get_base());
  buf.push_back(src);

  if (LOGGING_FLAG) LogBox::push_log("MEAN base rule " + src.to_s());
  merge_mean_proc_buffer(base_mean, buf, unified_mean);

  if (LOGGING_FLAG) LogBox::push_log("MEAN input_box " + src.to_s());
  sub_buf = merge_mean_proc(base_mean, input_box, unified_mean);
  std::copy(std::begin(sub_buf), std::end(sub_buf), std::back_inserter(buf));
  sub_buf.clear();
  if (LOGGING_FLAG) LogBox::push_log("MEAN ruleDB " + src.to_s());
  sub_buf = merge_mean_proc(base_mean, ruleDB, unified_mean);
  std::copy(std::begin(sub_buf), std::end(sub_buf), std::back_inserter(buf));
  sub_buf.clear();
  if (LOGGING_FLAG) LogBox::push_log("MEAN box_buffer " + src.to_s());
  sub_buf = merge_mean_proc(base_mean, box_buffer, unified_mean);
  std::copy(std::begin(sub_buf), std::end(sub_buf), std::back_inserter(buf));
  sub_buf.clear();

  intention.merge(src.get_internal().get_base(), unified_mean, base_mean);

  if (LOGGING_FLAG) LogBox::push_log("CAT processed about MEAN " + src.to_s());
  merge_cat_proc_buffer(base_cat, buf, unified_cat);

  if (LOGGING_FLAG) LogBox::push_log("CAT input_box " + src.to_s());
  sub_buf = merge_cat_proc(base_cat, input_box, unified_cat);
  std::copy(std::begin(sub_buf), std::end(sub_buf), std::back_inserter(buf));
  sub_buf.clear();
  if (LOGGING_FLAG) LogBox::push_log("CAT ruleDB " + src.to_s());
  sub_buf = merge_cat_proc(base_cat, ruleDB, unified_cat);
  std::copy(std::begin(sub_buf), std::end(sub_buf), std::back_inserter(buf));
  sub_buf.clear();
  if (LOGGING_FLAG) LogBox::push_log("CAT box_buffer " + src.to_s());
  sub_buf = merge_cat_proc(base_cat, box_buffer, unified_cat);
  std::copy(std::begin(sub_buf), std::end(sub_buf), std::back_inserter(buf));
  sub_buf.clear();
  send_box(buf);

  if (LOGGING_FLAG) {
    LogBox::push_log("<<--MERGE");
  }

  return true;
}

void Knowledge::collect_merge(Rule &src, RuleDBType &rule_db, std::set<Category> &unified_cat, std::set<AMean> &unified_mean) {
  std::for_each(std::begin(rule_db), std::end(rule_db), [&](Rule &r) {
    if (src.get_external() == r.get_external() && intention.merge_equal(src.get_internal().get_base(), r.get_internal().get_base())) {
      if (src.get_internal().get_cat() != r.get_internal().get_cat()) {
        unified_cat.insert(Category{r.get_internal().get_cat()});
      }
      if (src.get_internal().get_base() != r.get_internal().get_base()) {
        unified_mean.insert(AMean{r.get_internal().get_base()});
      }
    }
  });
}

void Knowledge::merge_cat_proc_buffer(const Category &base_cat, RuleDBType &buffer, std::set<Category> &unified_cat) {
  std::for_each(std::begin(buffer), std::end(buffer), [&](Rule &r) {
    Rule tmp = r;
    bool is_modified = false;
    if (unified_cat.find(r.get_internal().get_cat()) != std::end(unified_cat)) {
      r.get_internal() = LeftNonterminal{Category{base_cat}, r.get_internal().get_means()};
      is_modified = is_modified || true;
    }
    std::for_each(std::begin(r.get_external()), std::end(r.get_external()), [&](SymbolElement &sel) {
      if (sel.type() == ELEM_TYPE::NT_TYPE && unified_cat.find(sel.get<RightNonterminal>().get_cat()) != std::end(unified_cat)) {
        sel = RightNonterminal{Category{base_cat}, sel.get<RightNonterminal>().get_var()};
        is_modified = true;
      }
    });
    if (is_modified && LOGGING_FLAG) {
      LogBox::push_log("CAT MERGE-> " + tmp.to_s());
      LogBox::push_log("CAT MERGE<- " + r.to_s());
    }
  });
}
Knowledge::RuleDBType Knowledge::merge_cat_proc(const Category &base_cat, RuleDBType &DB, std::set<Category> &unified_cat) {
  RuleDBType buf, swapDB;
  std::for_each(std::begin(DB), std::end(DB), [&](Rule &r) {
    Rule tmp = r;
    bool is_modified = false;
    if (unified_cat.find(r.get_internal().get_cat()) != std::end(unified_cat)) {
      r.get_internal() = LeftNonterminal{Category{base_cat}, r.get_internal().get_means()};
      is_modified = true;
    }
    std::for_each(std::begin(r.get_external()), std::end(r.get_external()), [&](SymbolElement &sel) {
      if (sel.type() == ELEM_TYPE::NT_TYPE && unified_cat.find(sel.get<RightNonterminal>().get_cat()) != std::end(unified_cat)) {
        sel = RightNonterminal{Category{base_cat}, sel.get<RightNonterminal>().get_var()};
        is_modified = true;
      }
    });
    if (is_modified) {
      if (LOGGING_FLAG) {
        LogBox::push_log("CAT MERGE-> " + tmp.to_s());
        LogBox::push_log("CAT MERGE<- " + r.to_s());
      }
      buf.push_back(r);
    } else {
      swapDB.push_back(r);
    }
  });
  DB.swap(swapDB);

  return buf;
}
void Knowledge::merge_mean_proc_buffer(const AMean &base_mean, RuleDBType &buffer, std::set<AMean> &unified_mean) {
  std::for_each(std::begin(buffer), std::end(buffer), [&](Rule &r) {
    Rule tmp = r;
    bool is_modified = false;
    if (unified_mean.find(r.get_internal().get_base()) != std::end(unified_mean)) {
      r.get_internal() = LeftNonterminal{r.get_internal().get_cat(), Meaning{AMean{base_mean}, r.get_internal().get_followings()}};
      is_modified = true;
    }
    if (is_modified && LOGGING_FLAG) {
      LogBox::push_log("MEAN MERGE-> " + tmp.to_s());
      LogBox::push_log("MEAN MERGE<- " + r.to_s());
    }
  });
}

Knowledge::RuleDBType Knowledge::merge_mean_proc(const AMean &base_mean, RuleDBType &DB, std::set<AMean> &unified_mean) {
  RuleDBType buf, swapDB;
  std::for_each(std::begin(DB), std::end(DB), [&](Rule &r) {
    Rule tmp = r;
    bool is_modified = false;
    if (unified_mean.find(r.get_internal().get_base()) != std::end(unified_mean)) {
      r.get_internal() = LeftNonterminal{r.get_internal().get_cat(), Meaning{AMean{base_mean}, r.get_internal().get_followings()}};
      is_modified = true;
    }
    if (is_modified) {
      if (LOGGING_FLAG) {
        LogBox::push_log("MEAN MERGE-> " + tmp.to_s());
        LogBox::push_log("MEAN MERGE<- " + r.to_s());
      }
      buf.push_back(r);
    } else {
      swapDB.push_back(r);
    }
  });
  DB.swap(swapDB);

  return buf;
}

bool Knowledge::replace(void) {
  std::shuffle(std::begin(input_box), std::end(input_box), MT19937::igen);
  bool is_replaced;
  RuleDBType::iterator it;
  while ((it = std::begin(input_box)) != std::end(input_box)) {
    if (LOGGING_FLAG) {
      LogBox::push_log("\n-->>REPLACE");
    }
    Rule r = *it;
    input_box.erase(it);
    is_replaced = replacing(r, input_box);
    is_replaced = is_replaced || replacing(r, box_buffer);
    is_replaced = is_replaced || replacing(r, ruleDB);
    if (is_replaced) {
      if (LOGGING_FLAG) {
        LogBox::push_log("USED WORD:");
        LogBox::push_log(r.to_s());
        LogBox::push_log("<<--REPLACE");
      }
      send_box(r);
      break;
    } else {
      if (LOGGING_FLAG) LogBox::pop_log(1);
      box_buffer.push_back(r);
    }
  }
  send_box(box_buffer);

  return is_replaced;
}

bool Knowledge::replacing(Rule &word, RuleDBType &checking_sents) {
  bool is_replaced = false;
  RuleDBType buf, swapDB;

  std::for_each(std::begin(checking_sents), std::end(checking_sents), [&](Rule &r) {
    if (r.get_external().size() > word.get_external().size() &&
        intention.replace_equal(r.get_internal().get_base(), word.get_internal().get_base())) {
      std::list<SymbolElement> el_vec = r.get_external();
      auto it = std::search(std::begin(el_vec), std::end(el_vec), std::begin(word.get_external()), std::end(word.get_external()));
      if (it != std::end(el_vec)) {
        if (LOGGING_FLAG) {
          LogBox::push_log("REPLACE-> " + r.to_s());
        }
        int new_var_id, new_mean_id;
        int b_pos, b_size;
        b_pos = b_size = 0;

        new_var_id = var_indexer.generate();
        new_mean_id = ind_indexer.generate();
        std::for_each(std::begin(word.get_external()), std::end(word.get_external()), [&](SymbolElement &se) {
          if (se.type() == ELEM_TYPE::NT_TYPE) {
            b_size++;
          }
        });
        // insertポジション計算
        std::for_each(std::begin(el_vec), it, [&](SymbolElement &sel) {
          if (sel.type() == ELEM_TYPE::NT_TYPE) {
            b_pos++;
          }
        });
        // Rule自体のindex分
        b_pos++;
        Meaning new_meaning = r.get_internal().get_means().replaced(b_pos, b_size, Variable(new_var_id));
        new_meaning = new_meaning.replaced(0, 1, Meaning(AMean(new_mean_id)));

        it = el_vec.erase(it, std::next(it, word.get_external().size()));
        el_vec.insert(it, RightNonterminal(word.get_internal().get_cat(), Variable(new_var_id)));

        Rule sent{LeftNonterminal{Category{r.get_internal().get_cat()}, new_meaning}, el_vec};
        buf.push_back(sent);
        intention.replace(r.get_internal().get_base(), word.get_internal().get_base(), sent.get_internal().get_base(), b_pos, b_size);
        if (LOGGING_FLAG) {
          LogBox::push_log("REPLACE<- " + sent.to_s());
        }
        is_replaced = true;
      } else {
        swapDB.push_back(r);
      }
    } else {
      swapDB.push_back(r);
    }
  });
  checking_sents.swap(swapDB);
  send_box(buf);

  return is_replaced;
}

std::string Knowledge::to_s() {
  std::ostringstream os;
  os << "Learning buffer" << std::endl << std::endl;
  std::copy(std::begin(box_buffer), std::end(box_buffer), std::ostream_iterator<Rule>(os, "\n"));
  os << "Input box" << std::endl << std::endl;
  std::copy(std::begin(input_box), std::end(input_box), std::ostream_iterator<Rule>(os, "\n"));
  os << "Rule DB" << std::endl << std::endl;
  std::copy(std::begin(ruleDB), std::end(ruleDB), std::ostream_iterator<Rule>(os, "\n"));

  os << intention.mapping_to_s();

  return os.str();
}

void Knowledge::logging_on(void) { LOGGING_FLAG = true; }

void Knowledge::logging_off(void) { LOGGING_FLAG = false; }

void Knowledge::set_control(uint32_t FLAGS) { CONTROLS |= FLAGS; }

Knowledge::RuleDBType Knowledge::rules(void) {
  RuleDBType kb_all;
  RuleDBType::iterator ruleDB_it;
  ruleDB_it = ruleDB.begin();
  for (; ruleDB_it != ruleDB.end(); ruleDB_it++) {
    kb_all.push_back(*ruleDB_it);
  }

  return kb_all;
}

void Knowledge::define(const AMean &a, const Conception &m) { intention.store(a, m); }

void Knowledge::init_semantics_rules(TransRules &obj) { intention.init_rules(obj); }

Meaning Knowledge::meaning_no(int obj) { return intention.trans(obj); }

// DBの検索を高速化するため
void Knowledge::build_word_index(void) {
  DB_cat_amean_dic.clear();
  DB_amean_cat_dic.clear();
  dic_add(ruleDB);
}

void Knowledge::dic_add(RuleDBType &vec_r) {
  std::for_each(std::begin(vec_r), std::end(vec_r), [&](Rule &r) { dic_add(r); });
}

void Knowledge::dic_add(Rule &r) {
  AMean m(r.get_internal().get_base());
  Category c(r.get_internal().get_cat());
  {
    std::multimap<AMean, Rule>::value_type pair({m, r});
    DB_cat_amean_dic[c].insert(pair);
  }
  {
    std::multimap<Category, Rule>::value_type pair({c, r});
    DB_amean_cat_dic[m].insert(pair);
  }
}

std::string Knowledge::meaning_no_to_s(int obj) { return "[" + intention[obj].to_s() + "]"; }

std::string Knowledge::dic_to_s() {
  return "CAT->AMEAN******************************************\n" + dic_cat_to_s() +
         "\nAMEAN->CAT******************************************\n" + dic_amean_to_s() + "\n";
}

std::string Knowledge::dic_cat_to_s() {
  // std::map<Category, std::multimap<AMean, Rule>> DB_cat_amean_dic
  std::ostringstream os;
  std::for_each(std::begin(DB_cat_amean_dic), std::end(DB_cat_amean_dic),
                [&](std::map<Category, std::multimap<AMean, Rule>>::value_type &mp) {
                  std::for_each(std::begin(mp.second), std::end(mp.second), [&](std::multimap<AMean, Rule>::value_type &mmp) {
                    os << mp.first << " " << mmp.first << " " << mmp.second << std::endl;
                  });
                });
  return os.str();
}

std::string Knowledge::dic_amean_to_s() {
  // std::map<AMean, std::multimap<Category, Rule>> DB_amean_cat_dic
  std::ostringstream os;
  std::for_each(std::begin(DB_amean_cat_dic), std::end(DB_amean_cat_dic),
                [&](std::map<AMean, std::multimap<Category, Rule>>::value_type &mp) {
                  std::for_each(std::begin(mp.second), std::end(mp.second), [&](std::multimap<Category, Rule>::value_type &mmp) {
                    os << mp.first << " " << mmp.first << " " << mmp.second << std::endl;
                  });
                });
  return os.str();
}

bool Knowledge::construct_grounding_rules(const Category &c, Meaning m, std::function<void(RuleDBType &)> f) {
  return construct_grounding_rules(c, m, f, [](Rule &r) -> bool { return true; });
}

bool Knowledge::construct_grounding_rules(const Category &c, Meaning m, std::function<void(RuleDBType &)> f1,
                                          std::function<bool(Rule &)> f2) {
  bool is_constructable = false;
  if (DB_cat_amean_dic.find(c) != std::end(DB_cat_amean_dic) && DB_cat_amean_dic[c].find(m.get_base()) != DB_cat_amean_dic[c].end()) {
    std::vector<RuleDBType> prod;
    std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> range_pair = dic_range(c, m.get_base());
    std::for_each(range_pair.first, range_pair.second, [&](std::multimap<AMean, Rule>::value_type &p) {
      if (f2(p.second)) {
        bool subconst = true;
        std::vector<RuleDBType> sub_prod{{{p.second}}};
        std::function<void(RuleDBType &)> func = [&sub_prod](RuleDBType &rules) {
          std::for_each(std::begin(sub_prod), std::end(sub_prod), [&rules](RuleDBType &prod_rules) {
            std::copy(std::begin(rules), std::end(rules), std::back_inserter(prod_rules));
          });
        };
        int num = 1;
        std::for_each(std::begin(p.second.get_external()), std::end(p.second.get_external()), [&](SymbolElement &sel) {
          if (sel.type() == ELEM_TYPE::NT_TYPE) {
            subconst = subconst && construct_grounding_rules(RightNonterminal(sel.get<RightNonterminal>()).get_cat(),
                                                             m.at(num++).get<Meaning>(), func, f2);
          }
        });
        if (subconst) std::copy(std::begin(sub_prod), std::end(sub_prod), std::back_inserter(prod));
        is_constructable = is_constructable || subconst;
      }
    });
    std::for_each(std::begin(prod), std::end(prod), f1);
  }
  return is_constructable;
}

bool Knowledge::all_construct_grounding_rules(const Category &c, std::function<bool(std::vector<RuleDBType> &)> f0,
                                              std::function<void(RuleDBType &)> f1, std::function<bool(Rule &)> f2) {
  bool is_constructable = false;
  if (DB_cat_amean_dic.find(c) != std::end(DB_cat_amean_dic)) {
    bool first = true;
    product_loop = true;
    std::vector<RuleDBType> prod;
    std::vector<std::pair<AMean, Rule>> pairs{std::begin(DB_cat_amean_dic[c]), std::end(DB_cat_amean_dic[c])};
    std::shuffle(std::begin(pairs), std::end(pairs), MT19937::igen);
    std::for_each(std::begin(pairs), std::end(pairs), [&](auto &p) {
      if (f2(p.second)) {
        bool subconst = true;
        std::vector<RuleDBType> sub_prod{{{p.second}}};
        std::function<void(RuleDBType &)> func = [&sub_prod, &f0](RuleDBType &rules) {
          std::for_each(std::begin(sub_prod), std::end(sub_prod), [&rules](RuleDBType &prod_rules) {
            std::copy(std::begin(rules), std::end(rules), std::back_inserter(prod_rules));
          });
        };
        std::for_each(std::begin(p.second.get_external()), std::end(p.second.get_external()), [&](SymbolElement &sel) {
          if (sel.type() == ELEM_TYPE::NT_TYPE) {
            subconst = subconst && all_construct_grounding_rules(RightNonterminal(sel.get<RightNonterminal>()).get_cat(), f0, func, f2);
          }
        });
        if (subconst) {
          std::copy(std::begin(sub_prod), std::end(sub_prod), std::back_inserter(prod));
          product_loop = false;
          first = first || false;
        } else {
          if (first)
            product_loop = true;
          else
            product_loop = false;
        }
        is_constructable = is_constructable || subconst;
      }
    });
    std::for_each(std::begin(prod), std::end(prod), f1);
    was_constructable = is_constructable = f0(prod);
  } else {
    was_constructable = is_constructable;
  }
  return is_constructable;
}

// std::function<std::vector<SymbolElement> &(Meaning &)> Knowledge::rule_function(Rule & r)
// {
// }

std::list<SymbolElement> Knowledge::construct_buzz_word() {
  int length;
  int sym_id;
  std::list<SymbolElement> buzz;

  length = MT19937::irand(1, buzz_length);
  for (int i = 0; i < length; i++) {
    sym_id = MT19937::irand(0, Dictionary::symbol.size() - 1);
    buzz.push_back(Symbol(sym_id));
  }

  if (buzz.size() == 0) {
    std::cerr << "Failed making random" << std::endl;
    throw "make random external error";
  }
  return buzz;
}

bool Knowledge::explain(Meaning ref, RuleDBType &res) {
  std::vector<RuleDBType> pattern_list;
  auto range = dic_amean_range(ref.get_base());
  std::for_each(range.first, range.second, [&](auto &p) {
    // Please check it ;; mem_fun_t. this is a converter from member function to  object.
    construct_grounding_rules(p.first, ref, [&](RuleDBType &list) -> void { pattern_list.push_back(list); });
  });
  if (pattern_list.size() > 0) {
    res = pattern_list[MT19937::irand(0, pattern_list.size() - 1)];
    return true;
  } else {
    return false;
  }
}

// leftmost(upmost, index-zero-most) derivation
void Knowledge::ground_with_pattern(Rule &src, RuleDBType &pattern) {
  // std::advanced(std::begin(basestring), length)
  // if the length is equal to a size of basestring, finished this process.
  std::list<SymbolElement> vec_sel = pattern.front().get_external();
  auto p_it = std::next(std::begin(pattern));
  std::size_t num = 0;
  while (p_it != std::end(pattern) && num != vec_sel.size()) {
    auto sel_it = std::begin(vec_sel);
    std::advance(sel_it, num);
    if ((*sel_it).type() == ELEM_TYPE::NT_TYPE) {
      sel_it = vec_sel.erase(sel_it);
      vec_sel.insert(sel_it, std::begin((*p_it).get_external()), std::begin((*p_it).get_external()));
      p_it++;
    } else {
      num++;
    }
  }
  if (p_it != std::end(pattern)) {
    std::cerr << "Error in ground with pattern" << std::endl;
    exit(1);
  }
  src.get_external() = vec_sel;
}

Knowledge::RuleDBType Knowledge::grounded_rules(Meaning ref) {
  RuleDBType grounded_rules;
  auto range = dic_amean_range(ref.get_base());
  std::for_each(range.first, range.second, [&](std::pair<Category, Rule> p) {
    construct_grounding_rules(p.first, ref, [&](RuleDBType &list) {
      Rule r(LeftNonterminal(Category(0), ref), std::list<SymbolElement>());
      ground_with_pattern(r, list);
      grounded_rules.push_back(r);
    });
  });

  return grounded_rules;
}

std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> Knowledge::dic_cat_range(const Category &c) {
  if (DB_cat_amean_dic.find(c) != std::end(DB_cat_amean_dic)) {
    std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> p = {std::begin(DB_cat_amean_dic[c]),
                                                                                               std::end(DB_cat_amean_dic[c])};
    return p;
  } else {
    std::cerr << "Don't exist : Knowledge::dic_cat_range" << std::endl;
    exit(1);
  }
}
std::pair<std::multimap<Category, Rule>::iterator, std::multimap<Category, Rule>::iterator> Knowledge::dic_amean_range(const AMean &m) {
  if (DB_amean_cat_dic.find(m) != std::end(DB_amean_cat_dic)) {
    std::pair<std::multimap<Category, Rule>::iterator, std::multimap<Category, Rule>::iterator> p = {std::begin(DB_amean_cat_dic[m]),
                                                                                                     std::end(DB_amean_cat_dic[m])};
    return p;
  } else {
    std::cerr << "Don't exist : Knowledge::dic_amean_range" << std::endl;
    exit(1);
  }
}
std::pair<std::multimap<AMean, Rule>::iterator, std::multimap<AMean, Rule>::iterator> Knowledge::dic_range(const Category &c,
                                                                                                           const AMean &m) {
  if (DB_cat_amean_dic.find(c) != std::end(DB_cat_amean_dic)) {
    std::vector<Rule> res;
    return DB_cat_amean_dic[c].equal_range(m);
  } else {
    std::cerr << "Don't exist : Knowledge::dic_cat_amean_range" << std::endl;
    exit(1);
  }
}

// //3つの流れ（invent based on conditions, remap meaning for music score, make concepts for transfer）
// //XMLreader::index_count,XMLreader::category_countを使って意味とカテゴリのobjを変更する．
std::vector<Rule> Knowledge::generate_score(std::map<AMean, Conception> &core_meaning) {
  std::vector<RuleDBType> res;
  bool sentence;
  std::function<bool(std::vector<RuleDBType> &)> f0 = [&](std::vector<RuleDBType> &rules) {
    // std::for_each(std::begin(rules), std::end(rules), [&](RuleDBType &list) {
    // 	std::for_each(std::begin(list), std::end(list), [&](Rule &r) { sentence = sentence || r.is_sentence(intention); });
    // });
    return true;
  };
  std::function<void(RuleDBType &)> f1 = [&](RuleDBType rules) {
    bool has_sentence = false;
    std::for_each(std::begin(rules), std::end(rules), [&](Rule &r) { has_sentence = has_sentence || r.is_sentence(intention); });
    if (has_sentence) {
      res.push_back(rules);
    }
  };
  std::function<bool(Rule &)> f2 = [&](Rule &r) {
    if (!sentence) {
      sentence = r.is_sentence(intention);
      // return product_loop || !sentence;
      return sentence;
    } else {
      return product_loop;
    }
    // return product_loop || !sentence; //return product_loop
  };
  std::for_each(std::begin(DB_cat_amean_dic), std::end(DB_cat_amean_dic), [&](auto &p) {
    sentence = false;
    all_construct_grounding_rules(p.first, f0, f1, f2);
  });
  std::cout << "Number of generated score: " << res.size() << std::endl;
  RuleDBType rdb0 = res[MT19937::irand(0, res.size() - 1)], rdb;

  int index = 0;
  std::function<void(Rule &, int)> func1;
  func1 = [&](Rule &r0, int cat) {
    // std::cout << "Rule: " << r0 << std::endl;
    // std::copy(std::begin(rdb0), std::end(rdb0), std::ostream_iterator<Rule>(std::cout, "\n"));
    AMean am{ut_index--};
    Category ca{cat};
    core_meaning[am] = intention.get(r0.get_internal().get_base());
    rdb.push_back(Rule{LeftNonterminal{ca, Meaning{am, r0.get_internal().get_followings()}}, r0.get_external()});
    // std::cerr << "RDB size: " << rdb.size() << " index: " << index << std::endl;
    Rule &r = rdb[index++];
    std::list<SymbolElement> sel_vec;
    std::for_each(std::begin(r.get_external()), std::end(r.get_external()), [&](SymbolElement &sel) {
      if (sel.type() == ELEM_TYPE::NT_TYPE) {
        auto it = std::find_if(std::begin(rdb0), std::end(rdb0),
                               [&](Rule &r1) { return r1.get_internal().get_cat() == sel.template get<RightNonterminal>().get_cat(); });
        if (it == std::end(rdb0)) {
          std::cerr << "Error Rule: " << r0 << " changed: " << r << std::endl;
          std::copy(std::begin(rdb0), std::end(rdb0), std::ostream_iterator<Rule>(std::cerr, "\n"));
          std::cerr << "Irregular rule set" << std::endl;
          exit(1);
        }
        sel_vec.push_back(RightNonterminal{Category(ut_category--), sel.template get<RightNonterminal>().get_var()});
        auto r2 = *it;
        rdb0.erase(it);
        func1(r2, ut_category--);
      } else {
        sel_vec.push_back(sel);
      }
    });
    r = Rule{r.get_internal(), sel_vec};
  };
  // std::cout << "base" << std::endl;
  // std::copy(std::begin(rdb0), std::end(rdb0), std::ostream_iterator<Rule>(std::cout, "\n"));
  auto r0 = rdb0.front();
  rdb0.erase(std::begin(rdb0));
  func1(r0, ut_category--);
  return rdb;
}
Rule Knowledge::fabricate(Rule &src1) {
  // std::vector<RuleDBType> groundable_patterns;
  // std::map<PATTERN_TYPE, std::vector<RuleDBType>> all_patterns;
  Rule src;

  // //pattern作成
  // all_patterns = construct_grounding_patterns(src);

  // if (LOGGING_FLAG)
  // {
  // 	LogBox::push_log("\n-->>FABRICATE1:");
  // }

  // if (all_patterns[COMPLETE].size() != 0)
  // {
  // 	RuleDBType target_pattern = all_patterns[COMPLETE][MT19937::irand(0, all_patterns[COMPLETE].size() - 1)];

  // 	if (LOGGING_FLAG)
  // 	{
  // 		std::ostringstream os;

  // 		LogBox::push_log("**CONSTRUCT");
  // 		LogBox::push_log("***->>USED_RULES");
  // 		std::copy(std::begin(target_pattern), std::end(target_pattern), std::ostream_iterator<Rule>(os, " "));
  // 		LogBox::push_log(os.str());
  // 		LogBox::push_log("***<<-USED_RULES");
  // 	}

  // 	ground_with_pattern(src, target_pattern);
  // }
  // else if (all_patterns[ABSOLUTE].size() != 0)
  // {
  // 	if (LOGGING_FLAG)
  // 	{
  // 		LogBox::push_log("**ABSOLUTE");
  // 	}

  // 	src = all_patterns[ABSOLUTE][MT19937::irand(0, all_patterns[ABSOLUTE].size() - 1)].front();
  // }
  // else if (all_patterns[SEMICOMPLETE].size() != 0)
  // {
  // 	RuleDBType target_pattern = all_patterns[SEMICOMPLETE][MT19937::irand(0, all_patterns[SEMICOMPLETE].size() - 1)];
  // 	if (LOGGING_FLAG)
  // 	{
  // 		std::ostringstream os;

  // 		LogBox::push_log("**SEMI CONSTRUCT");
  // 		LogBox::push_log("***->>USED_RULES");
  // 		std::copy(std::begin(use_pattern), std::end(use_pattern), std::ostream_iterator<Rule>(os, " "));
  // 		LogBox::push_log(os.str());
  // 		LogBox::push_log("***<<-USED_RULES");
  // 	}
  // 	std::for_each(std::begin(target_pattern), std::end(target_pattern), [&](Rule &r) {
  // 		if (r.is_noun(intention) && r.get_external().size())
  // 		{
  // 			std::vector<SymbolElement> buzz = construct_buzz_word();
  // 			r = Rule(r.get_internal(), buzz);
  // 			if (CONTROLS & USE_ADDITION_OF_RANDOM_WORD)
  // 			{
  // 				Rule keep_word;
  // 				keep_word = r;
  // 				send_db(keep_word);
  // 				dic_add(DB_dic, keep_word);
  // 				if (LOGGING_FLAG)
  // 				{
  // 					LogBox::push_log("***->>KEPT THE COMP_RULE");
  // 					LogBox::push_log(keep_word.to_s());
  // 					LogBox::push_log("***<<-KEPT THE COMP_RULE");
  // 				}
  // 			}
  // 			if (LOGGING_FLAG)
  // 			{
  // 				LogBox::push_log("***->>COMP_RULE");
  // 				LogBox::push_log(r.to_s());
  // 				LogBox::push_log("***<<-COMP_RULE");
  // 			}
  // 		}
  // 	});

  // 	ground_with_pattern(src, target_pattern);
  // }
  // else
  // {
  // 	if (LOGGING_FLAG)
  // 	{
  // 		LogBox::push_log("**RANDOM");
  // 	}

  // 	std::vector<SymbolElement> ex = construct_buzz_word();
  // 	src.get_external().swap(ex);

  // 	if (CONTROLS & USE_ADDITION_OF_RANDOM_WORD)
  // 	{
  // 		send_db(src);
  // 		if (LOGGING_FLAG)
  // 		{
  // 			LogBox::push_log("**KEPT THE RULE");
  // 		}
  // 	}
  // }
  // if (LOGGING_FLAG)
  // {
  // 	LogBox::push_log("**OUTPUT");
  // 	LogBox::push_log(src.to_s());
  // 	LogBox::push_log("<<--FABRICATE");
  // }
  return src;
}