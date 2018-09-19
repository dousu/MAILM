#include "LilypondOutput.h"

std::map<std::string, std::string> XMLreader::alias;
std::map<std::string, std::string> XMLreader::conv_alias;
std::map<int, Meaning> XMLreader::i_meaning_map;
std::map<int, std::vector<int>> XMLreader::i_beat_map;
std::map<int, std::string> XMLreader::labeling;
std::map<AMean, Conception> XMLreader::core_meaning;
std::vector<Rule> XMLreader::input_rules;
std::map<std::string, int> XMLreader::conv_str;
std::unordered_map<int, std::vector<SymbolElement>> XMLreader::strings;

int XMLreader::index_count = -1;
int XMLreader::variable_count = -1;
int XMLreader::category_count = -1;
int XMLreader::symbol_count = 1;

void XMLreader::make_init_data(std::vector<std::string> &file_paths) {
  std::cout << "XMLreader3" << std::endl;
  {
    alias.clear();
    conv_alias.clear();
    i_meaning_map.clear();
    i_beat_map.clear();
    labeling.clear();
    core_meaning.clear();
    strings.clear();
  }

  std::sort(std::begin(file_paths), std::end(file_paths));
  int no = 1;
  std::for_each(std::begin(file_paths), std::end(file_paths), [&](std::string f_str) {
    std::vector<Rule> buf;
    load(f_str, buf, no++);
    std::copy(std::begin(buf), std::end(buf), std::back_inserter(input_rules));
  });
}

void XMLreader::load(std::string file_path, std::vector<Rule> &buf, int file_no) {
  // strings[file_no]
  boost::property_tree::ptree pt;
  boost::property_tree::read_xml(file_path.c_str(), pt);

  std::vector<std::string> lbuf1, lbuf2;
  boost::algorithm::split(lbuf1, file_path, boost::algorithm::is_any_of("."), boost::algorithm::token_compress_on);
  boost::algorithm::split(lbuf2, *(lbuf1.rbegin() + 1), boost::algorithm::is_any_of("/\\"), boost::algorithm::token_compress_on);
  labeling[file_no] = *(lbuf2.rbegin());

  //初期設定
  bool loop = true;
  std::string path1 = "score";
  int measure_num, met_num, p_num = 0, met_max;
  Meaning s_in, flat_meaning;
  std::vector<SymbolElement> s_ex;

  s_in = Meaning(AMean(index_count));
  core_meaning[AMean(index_count)] = Conception();
  core_meaning[AMean(index_count)].add(Prefices::SEN);

  // sentenceのインデックス記録
  flat_meaning = Meaning(AMean(index_count));

  index_count--;
  i_beat_map[file_no] = std::vector<int>();

  //楽曲の繰り返しもあるので全部検査するのを変化がなくなるまで行う
  while (loop) {
    //何巡目かカウント
    p_num++;
    //変化があればloopがtrueになる
    loop = false;
    // measure番号の初期化
    measure_num = 1;

    // score以下の木を順番に取っていく
    BOOST_FOREACH (const boost::property_tree::ptree::value_type &measure_t, pt.get_child(path1.c_str())) {
      // measureであり次に検査すべきmeasureか判定
      if (measure_t.first == "measure" &&
          (measure_t.second.get_optional<int>((std::string("<xmlattr>.number")).c_str())).get() == measure_num) {
        // p_flで処理すべきmetricか判定
        bool p_fl = false;
        //検査対象metric以下の木をすべて取ってくる
        BOOST_FOREACH (const boost::property_tree::ptree::value_type &period_t, measure_t.second.get_child("")) {
          // periodであり次に検査すべきmetricかを何巡目かで判定
          if (period_t.first == "period" && (period_t.second.get_optional<int>((std::string("<xmlattr>.number")).c_str())).get() == p_num) {
            //合致するperiodがあればこのmetricを検査
            loop = true;
            p_fl = true;
            break;
          }
        }

        if (p_fl) {
          std::vector<SymbolElement> sub_ex;
          // metric番号の初期化
          met_num = 1;
          //入っているべきmetricの数を取得
          met_max = (measure_t.second.get_optional<int>((std::string("<xmlattr>.metricals")).c_str())).get();
          i_beat_map[file_no].push_back(met_max);
          //検査対象measure以下の木をすべて取ってくる
          BOOST_FOREACH (const boost::property_tree::ptree::value_type &metric_t, measure_t.second.get_child("")) {
            // metricであり次に検査すべきmetricか判定
            if (metric_t.first == "metric" &&
                (metric_t.second.get_optional<int>((std::string("<xmlattr>.number")).c_str())).get() == met_num) {
              //ここで順番に処理していけば順序通りにとれる
              std::string str;
              //すべてのpitch classを取得
              BOOST_FOREACH (const boost::property_tree::ptree::value_type &class_t, metric_t.second.get_child("pitchclass")) {
                // pronが0以外の場合は記録する
                if ((class_t.second.get_optional<int>((std::string("<xmlattr>.pron")).c_str())).get() > 0) {
                  // pronの数は非表示
                  str += class_t.first + Prefices::DSP;
                }
              }
              str = boost::algorithm::trim_copy(str);
              if (str.size() == 0) {
                str = std::string("rest");
                core_meaning[AMean(index_count)] = Conception();
                core_meaning[AMean(index_count)].add("rest");
              }

              // strのaliasの作成とword_exへの保存
              std::string str2;
              if (alias.find(str) == alias.end()) {
                str2 = Prefices::SYN + std::to_string(symbol_count);
                alias.insert(std::map<std::string, std::string>::value_type(str, str2));
                conv_alias.insert(std::map<std::string, std::string>::value_type(str2, str));
                Dictionary::symbol.insert(std::map<int, std::string>::value_type(symbol_count, str2));
                Dictionary::conv_symbol.insert(std::map<std::string, int>::value_type(str2, symbol_count));
                conv_str[str] = symbol_count++;
              } else {
                str2 = alias[str];
              }
              sub_ex.push_back(Symbol(conv_str[str]));
              strings[file_no].push_back(Symbol(conv_str[str]));
              met_num++;
            }
            // met_numがmet_maxになると小節の終わりなのでルールとして掃き出し
            if (met_num == met_max + 1) {
              // wordに少しでもたまっていたらルールにする
              if (sub_ex.size() != 0) {
                Rule r(LeftNonterminal(Category(category_count), Meaning(AMean(index_count))), sub_ex);
                buf.push_back(r);
                if (core_meaning.find(AMean(index_count)) == core_meaning.end()) {
                  core_meaning[AMean(index_count)] = Conception();
                }
                core_meaning[AMean(index_count)].add(Prefices::MES);
                core_meaning[AMean(index_count)].add(Prefices::MEA);

                s_in.get_followings().push_back(Variable(variable_count));
                flat_meaning.get_followings().push_back(Meaning(AMean(index_count)));
                s_ex.push_back(RightNonterminal(Category(category_count), Variable(variable_count)));

                index_count--;
                variable_count--;
                category_count--;
              }
              break;
            }
          }
        }
        measure_num++;
      }
    }
  }
  Symbol::conv_symbol = XMLreader::conv_alias;
  Rule r_sent(LeftNonterminal(Category(category_count), s_in), s_ex);
  //文ルール追加
  buf.push_back(r_sent);
  // std::copy(std::begin(buf), std::end(buf), std::ostream_iterator<Rule>(std::cerr, "\n"));
  i_meaning_map[file_no] = flat_meaning;
  category_count--;
}

XMLreader XMLreader::copy(void) { return XMLreader(); }
