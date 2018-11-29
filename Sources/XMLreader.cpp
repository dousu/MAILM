#include "XMLreader.h"

std::map<std::string, std::string> Reader::alias;
std::map<std::string, std::string> Reader::conv_alias;
std::map<int, Meaning> Reader::i_meaning_map;
std::map<int, std::vector<int>> Reader::i_beat_map;
std::map<int, std::string> Reader::labeling;
std::map<AMean, Conception> Reader::core_meaning;
std::vector<Rule> Reader::input_rules;
std::map<std::string, int> Reader::conv_str;
std::unordered_map<int, std::vector<SymbolElement>> Reader::strings;

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
  Symbol::conv_symbol = conv_alias;
  Rule r_sent(LeftNonterminal(Category(category_count), s_in), s_ex);
  //文ルール追加
  buf.push_back(r_sent);
  i_meaning_map[file_no] = flat_meaning;
  category_count--;
}

int XMLreaderMono::index_count = -1;
int XMLreaderMono::variable_count = -1;
int XMLreaderMono::category_count = -1;
int XMLreaderMono::symbol_count = 1;

void XMLreaderMono::make_init_data(std::vector<std::string> &file_paths) {
  std::cout << "XMLreaderMono" << std::endl;
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

void XMLreaderMono::load(std::string file_path, std::vector<Rule> &buf, int file_no) {
  // strings[file_no]
  boost::property_tree::ptree pt;
  boost::property_tree::read_xml(file_path.c_str(), pt);

  std::vector<std::string> lbuf1, lbuf2;
  boost::algorithm::split(lbuf1, file_path, boost::algorithm::is_any_of("."), boost::algorithm::token_compress_on);
  boost::algorithm::split(lbuf2, *(lbuf1.rbegin() + 1), boost::algorithm::is_any_of("/\\"), boost::algorithm::token_compress_on);
  labeling[file_no] = *(lbuf2.rbegin());

  int time_type;
  int time_beats;

  int key_fifth;
  std::string key_mode;

  //初期設定
  std::string path1 = "score-partwise";
  Meaning s_in, flat_meaning;
  std::vector<SymbolElement> s_ex;

  s_in = Meaning(AMean(index_count));
  core_meaning[AMean(index_count)] = Conception();
  core_meaning[AMean(index_count)].add(Prefices::SEN);

  // sentenceのインデックス記録
  flat_meaning = Meaning(AMean(index_count));

  index_count--;
  i_beat_map[file_no] = std::vector<int>();

  BOOST_FOREACH (const boost::property_tree::ptree::value_type &part_t, pt.get_child(path1.c_str())) {
    if (part_t.first == "part") {
      BOOST_FOREACH (const boost::property_tree::ptree::value_type &measure_t, part_t.second.get_child("")) {
        if (measure_t.first == "measure") {
          std::vector<SymbolElement> sub_ex;
          //入っているべきmetricの数を取得
          // met_max = (measure_t.second.get_optional<int>((std::string("<xmlattr>.metricals")).c_str())).get();
          // i_beat_map[file_no].push_back(met_max);
          //検査対象measure以下の木をすべて取ってくる
          BOOST_FOREACH (const boost::property_tree::ptree::value_type &note_t, measure_t.second.get_child("")) {
            if (note_t.first == "attributes") {
              BOOST_FOREACH (const boost::property_tree::ptree::value_type &time_t, note_t.second.get_child("")) {
                if (time_t.first == "time") {
                  BOOST_FOREACH (const boost::property_tree::ptree::value_type &beat_t, time_t.second.get_child("")) {
                    boost::optional<std::string> val;
                    if (beat_t.first == "beats" && (val = beat_t.second.get_optional<std::string>(""))) {
                      time_beats = std::stoi(val.get());
                    }
                    if (beat_t.first == "beat-type" && (val = beat_t.second.get_optional<std::string>(""))) {
                      time_type = std::stoi(val.get());
                    }
                  }
                }
                if (time_t.first == "key") {
                  BOOST_FOREACH (const boost::property_tree::ptree::value_type &key_t, time_t.second.get_child("")) {
                    boost::optional<std::string> val;
                    if (key_t.first == "fifths" && (val = key_t.second.get_optional<std::string>(""))) {
                      key_fifth = std::stoi(val.get());
                    }
                    if (key_t.first == "mode" && (val = key_t.second.get_optional<std::string>(""))) {
                      key_mode = val.get();
                    }
                  }
                }
              }
            }
            if (note_t.first == "note") {
              //ここで順番に処理していけば順序通りにとれる
              std::string str;
              //すべてのpitchを取得
              BOOST_FOREACH (const boost::property_tree::ptree::value_type &pitch_t, note_t.second.get_child("")) {
                if (pitch_t.first == "pitch") {
                  BOOST_FOREACH (const boost::property_tree::ptree::value_type &step_t, pitch_t.second.get_child("")) {
                    boost::optional<std::string> val;
                    if (step_t.first == "step" && (val = pitch_t.second.get_optional<std::string>("step"))) {
                      std::string data = val.get();
                      std::transform(std::begin(data), std::end(data), std::begin(data), ::tolower);
                      str += data;
                    }
                  }
                }
                if (str.size() == 0) {
                  str = std::string("r");
                  core_meaning[AMean(index_count)] = Conception();
                  core_meaning[AMean(index_count)].add("rest");
                }

                if (pitch_t.first == "type") {
                  if (boost::optional<std::string> val = note_t.second.get_optional<std::string>("type")) {
                    std::string span = val.get();
                    if (span == "whole") {
                      str += "1";
                    } else if (span == "half") {
                      str += "2";
                    } else if (span == "quarter") {
                      str += "4";
                    } else if (span == "eighth") {
                      str += "8";
                    } else {
                      std::regex num("[1-9][0-9]*");
                      std::sregex_token_iterator it(std::begin(span), std::end(span), num, 0);
                      str += *it;
                    }
                  }
                }
                if (pitch_t.first == "dot") {
                  str += ".";
                }
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
            }
          }
          // word rule
          Rule r(LeftNonterminal(Category(category_count), Meaning(AMean(index_count))), sub_ex);
          buf.push_back(r);
          if (core_meaning.find(AMean(index_count)) == core_meaning.end()) {
            core_meaning[AMean(index_count)] = Conception();
          }
          core_meaning[AMean(index_count)].add(Prefices::MES);
          core_meaning[AMean(index_count)].add(Prefices::MEA);
          core_meaning[AMean(index_count)].add("time" + std::to_string(time_beats) + "/" + std::to_string(time_type));
          core_meaning[AMean(index_count)].add("keys" + std::to_string(key_fifth) + "/" + key_mode);
          // core_meaning[AMean(index_count)].add("mode" + key_mode);

          s_in.get_followings().push_back(Variable(variable_count));
          flat_meaning.get_followings().push_back(Meaning(AMean(index_count)));
          s_ex.push_back(RightNonterminal(Category(category_count), Variable(variable_count)));

          index_count--;
          variable_count--;
          category_count--;
        }
      }
    }
  }

  Symbol::conv_symbol = conv_alias;
  Rule r_sent(LeftNonterminal(Category(category_count), s_in), s_ex);
  //文ルール追加
  buf.push_back(r_sent);
  i_meaning_map[file_no] = flat_meaning;
  category_count--;
}

int ABCreader::index_count = -1;
int ABCreader::variable_count = -1;
int ABCreader::category_count = -1;
int ABCreader::symbol_count = 1;

void ABCreader::make_init_data(std::vector<std::string> &file_paths) {
  std::cout << "ABCreader" << std::endl;
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

void ABCreader::load(std::string file_path, std::vector<Rule> &buf, int file_no) {
  // strings[file_no]
  boost::property_tree::ptree pt;
  boost::property_tree::read_xml(file_path.c_str(), pt);

  std::vector<std::string> lbuf1, lbuf2;
  boost::algorithm::split(lbuf1, file_path, boost::algorithm::is_any_of("."), boost::algorithm::token_compress_on);
  boost::algorithm::split(lbuf2, *(lbuf1.rbegin() + 1), boost::algorithm::is_any_of("/\\"), boost::algorithm::token_compress_on);
  labeling[file_no] = *(lbuf2.rbegin());

  int time_type;
  int time_beats;

  int key_fifth;
  std::string key_mode;

  //初期設定
  std::string path1 = "score-partwise";
  Meaning s_in, flat_meaning;
  std::vector<SymbolElement> s_ex;

  s_in = Meaning(AMean(index_count));
  core_meaning[AMean(index_count)] = Conception();
  core_meaning[AMean(index_count)].add(Prefices::SEN);

  // sentenceのインデックス記録
  flat_meaning = Meaning(AMean(index_count));

  index_count--;
  i_beat_map[file_no] = std::vector<int>();

  BOOST_FOREACH (const boost::property_tree::ptree::value_type &part_t, pt.get_child(path1.c_str())) {
    if (part_t.first == "part") {
      BOOST_FOREACH (const boost::property_tree::ptree::value_type &measure_t, part_t.second.get_child("")) {
        if (measure_t.first == "measure") {
          std::vector<SymbolElement> sub_ex;
          //入っているべきmetricの数を取得
          // met_max = (measure_t.second.get_optional<int>((std::string("<xmlattr>.metricals")).c_str())).get();
          // i_beat_map[file_no].push_back(met_max);
          //検査対象measure以下の木をすべて取ってくる
          BOOST_FOREACH (const boost::property_tree::ptree::value_type &note_t, measure_t.second.get_child("")) {
            if (note_t.first == "attributes") {
              BOOST_FOREACH (const boost::property_tree::ptree::value_type &time_t, note_t.second.get_child("")) {
                if (time_t.first == "time") {
                  BOOST_FOREACH (const boost::property_tree::ptree::value_type &beat_t, time_t.second.get_child("")) {
                    boost::optional<std::string> val;
                    if (beat_t.first == "beats" && (val = beat_t.second.get_optional<std::string>(""))) {
                      time_beats = std::stoi(val.get());
                    }
                    if (beat_t.first == "beat-type" && (val = beat_t.second.get_optional<std::string>(""))) {
                      time_type = std::stoi(val.get());
                    }
                  }
                }
                if (time_t.first == "key") {
                  BOOST_FOREACH (const boost::property_tree::ptree::value_type &key_t, time_t.second.get_child("")) {
                    boost::optional<std::string> val;
                    if (key_t.first == "fifths" && (val = key_t.second.get_optional<std::string>(""))) {
                      key_fifth = std::stoi(val.get());
                    }
                    if (key_t.first == "mode" && (val = key_t.second.get_optional<std::string>(""))) {
                      key_mode = val.get();
                    }
                  }
                }
              }
            }
            if (note_t.first == "note") {
              //ここで順番に処理していけば順序通りにとれる
              std::string str;
              //すべてのpitchを取得
              BOOST_FOREACH (const boost::property_tree::ptree::value_type &pitch_t, note_t.second.get_child("")) {
                if (pitch_t.first == "pitch") {
                  BOOST_FOREACH (const boost::property_tree::ptree::value_type &step_t, pitch_t.second.get_child("")) {
                    boost::optional<std::string> val;
                    if (step_t.first == "step" && (val = pitch_t.second.get_optional<std::string>("step"))) {
                      std::string data = val.get();
                      std::transform(std::begin(data), std::end(data), std::begin(data), ::tolower);
                      str += data;
                    }
                  }
                }
                if (str.size() == 0) {
                  str = std::string("r");
                  core_meaning[AMean(index_count)] = Conception();
                  core_meaning[AMean(index_count)].add("rest");
                }

                if (pitch_t.first == "type") {
                  if (boost::optional<std::string> val = note_t.second.get_optional<std::string>("type")) {
                    std::string span = val.get();
                    if (span == "whole") {
                      str += "1";
                    } else if (span == "half") {
                      str += "2";
                    } else if (span == "quarter") {
                      str += "4";
                    } else if (span == "eighth") {
                      str += "8";
                    } else {
                      std::regex num("[1-9][0-9]*");
                      std::sregex_token_iterator it(std::begin(span), std::end(span), num, 0);
                      str += *it;
                    }
                  }
                }
                if (pitch_t.first == "dot") {
                  str += ".";
                }
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
            }
          }
          // word rule
          Rule r(LeftNonterminal(Category(category_count), Meaning(AMean(index_count))), sub_ex);
          buf.push_back(r);
          if (core_meaning.find(AMean(index_count)) == core_meaning.end()) {
            core_meaning[AMean(index_count)] = Conception();
          }
          core_meaning[AMean(index_count)].add(Prefices::MES);
          core_meaning[AMean(index_count)].add(Prefices::MEA);
          core_meaning[AMean(index_count)].add("time" + std::to_string(time_beats) + "/" + std::to_string(time_type));
          core_meaning[AMean(index_count)].add("keys" + std::to_string(key_fifth) + "/" + key_mode);
          // core_meaning[AMean(index_count)].add("mode" + key_mode);

          s_in.get_followings().push_back(Variable(variable_count));
          flat_meaning.get_followings().push_back(Meaning(AMean(index_count)));
          s_ex.push_back(RightNonterminal(Category(category_count), Variable(variable_count)));

          index_count--;
          variable_count--;
          category_count--;
        }
      }
    }
  }

  Symbol::conv_symbol = conv_alias;
  Rule r_sent(LeftNonterminal(Category(category_count), s_in), s_ex);
  //文ルール追加
  buf.push_back(r_sent);
  i_meaning_map[file_no] = flat_meaning;
  category_count--;
}

std::string_view ABCreader::next_line(std::string_view str) {
  str.remove_prefix(std::min(str.find_first_of("\n") + 1, str.size()));
  return str;
}

std::string_view ABCreader::title(std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("T:") != 0) {
    std::cerr << "Title is not found" << std::endl;
    exit(-1);
  }
  std::string_view title_str = str.substr(2, str.find_first_of("\r\n"));
  title_str.remove_prefix(std::min(title_str.find_first_not_of(" "), title_str.size()));
  // auto trim_pos = title_str.find(" ");
  // if(trim_pos != title_str.npos)
  //   title_str.remove_suffix(title_str.size() - trim_pos);
  t_str = title_str;
  str.remove_prefix(std::min(str.find_first_of("\n") + 1, str.size()));
  return str;
}
std::string_view ABCreader::base_length(std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("L:") != 0) {
    return str;
  }
  std::string_view base_length_str = str.substr(2, str.find_first_of("\r\n]"));
  // l1, l2
  base_length_str.remove_suffix(std::min(base_length_str.find_last_not_of(" "), base_length_str.size()));
  l1 = std::stoi(std::string(base_length_str.substr(0, base_length_str.find_first_of("/"))));
  l2 = std::stoi(std::string(base_length_str.substr(base_length_str.find_last_of("/") + 1,
                                                    std::min(base_length_str.find_first_of(" \r\n]"), base_length_str.size()))));
  str.remove_prefix(std::min(str.find_first_of("\n]") + 1, str.size()));
  return str;
}
std::string_view ABCreader::tempo(std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("Q:") != 0) {
    return str;
  }
  std::string_view tempo_str = str.substr(2, str.find_first_of("\r\n]"));
  // q_str, q1, q2, q3
  tempo_str.remove_suffix(std::min(tempo_str.find_last_not_of(" "), tempo_str.size()));
  q1 = std::stoi(std::string(tempo_str.substr(0, tempo_str.find_first_of("/"))));
  q2 = std::stoi(std::string(tempo_str.substr(tempo_str.find_last_of("/") + 1, tempo_str.find_last_of("="))));
  q3 = std::stoi(std::string(tempo_str.substr(tempo_str.find_last_of("=") + 1, std::min(tempo_str.find_first_of(" "), tempo_str.size()))));
  str.remove_prefix(std::min(str.find_first_of("\n]") + 1, str.size()));
  return str;
}
std::string_view ABCreader::rhythm(std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("M:") != 0) {
    return str;
  }
  std::string_view rhythm_str = str.substr(2, str.find_first_of("\r\n]"));
  // m1, m2
  rhythm_str.remove_suffix(std::min(rhythm_str.find_last_not_of(" "), rhythm_str.size()));
  m1 = std::stoi(std::string(rhythm_str.substr(0, rhythm_str.find_first_of("/"))));
  m2 = std::stoi(
      std::string(rhythm_str.substr(rhythm_str.find_last_of("/") + 1, std::min(rhythm_str.find_first_of(" "), rhythm_str.size()))));
  str.remove_prefix(std::min(str.find_first_of("\n]") + 1, str.size()));
  return str;
}
void ABCreader::k_(std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("K:") != 0) {
    std::cerr << "Key is not found." << std::endl;
    exit(-1);
  }
  str = key(str);
  std::function<void(std::string_view)> func;
  func = [this, &func](std::string_view str) {
    bool less_than, greater_than, hyphen;
    do {
      // parenthesis ()[]
      std::size_t loc = std::min(str.find_first_of("ABCDEFGabcdefg._=^"), str.size());
      std::string_view strview = str.substr(0, loc);
      if (strview.find_first_of("(") != strview.npos) {
        func(str);
      }
      if (strview.find_first_of(")") != strview.npos) {
        return;
      }
      if (strview.find_first_of("[") != strview.npos) {
        std::string_view v = strview.substr(strview.find_first_of("[") + 1);
        v = base_length(v);
        v = rhythm(v);
        v = tempo(v);
      }
      if (strview.find_first_of("|") != strview.npos) {
      }
      str.remove_prefix(str.find_first_of("ABCDEFGabcdefg._=^"));
      str = note(str);
      // note_str = note_string(opt_str, name_str, len1, len2);
      // -, >, <
      if (greater_than) {
        l2 *= 2;
      }
      if (less_than) {
        l1 *= 3;
        l2 *= 2;
      }

      hyphen = greater_than = less_than = false;
      if (str.find_first_of("-") == 0) {
        hyphen = true;
      } else if (str.find_first_of(">") == 0) {
        greater_than = true;
        note_str = note_string(opt_str, name_str, l1 * 3, l2 * 2);
      } else if (str.find_first_of("<") == 0) {
        less_than = true;
        note_str = note_string(opt_str, name_str, l1, l2 * 2);
      } else {
        note_str = note_string(opt_str, name_str, l1, l2);
      }
    } while (note_str != "");
  };
  func(str);
}
std::string_view ABCreader::key(std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("K:") != 0) {
    return str;
  }
  k_str = str.substr(2, str.find_first_of("\r\n]"));
  str.remove_prefix(std::min(str.find_first_of("\n]") + 1, str.size()));
  return str;
}
std::string_view ABCreader::note(std::string_view str) {
  nl1 = l1;
  nl2 = l2;
  note_str = "";
  std::string_view rest = str;
  rest = option(rest);
  rest = name(rest);
  rest = note_length(rest);
  return rest;
}
std::string_view ABCreader::option(std::string_view str) {
  opt_str = "";
  dot = false;
  std::string_view option_str = str.substr(0, str.find_first_of("ABCDEFGabcdefg"));
  if (option_str.find_last_of("^") != option_str.npos) {
    opt_str = "^";
  } else if (option_str.find_last_of("=") != option_str.npos) {
    opt_str = "=";
  } else if (option_str.find_last_of("_") != option_str.npos) {
    opt_str = "_";
  }
  if (option_str.find_last_of(".") != option_str.npos) {
    dot = true;
  }
  str.remove_prefix(std::min(str.find_first_of("ABCDEFGabcdefg"), str.size()));
  return str;
}
std::string_view ABCreader::name(std::string_view str) {
  name_str = "";
  name_str = str.substr(0, std::min(std::size_t(1), str.size()));
  str.remove_prefix(std::min(std::size_t(1), str.size()));
  return str;
}
std::string_view ABCreader::note_length(std::string_view str) {
  nl_str = "";
  nl_str = str.substr(0, std::min(str.find_first_of("ABCDEFGabcdefg<>|[)(-. \r\n"), str.size()));
  nl1 = nl2 = 1;
  std::size_t bound = nl_str.find_first_of("/");
  if (bound != nl_str.npos) {
    std::string v1 = nl_str.substr(0, bound);
    nl1 = std::stoi(v1);
  }
  if (bound != nl_str.npos) {
    if (bound == nl_str.size() - 1) {
      nl2 = 2;
    } else {
      nl2 = std::stoi(std::string(nl_str.substr(bound + 1, nl_str.size())));
    }
  }
  if (dot) {
    nl1 *= 3;
    nl2 *= 2;
  }
  str.remove_prefix(std::min(str.find_first_of("ABCDEFGabcdefg<>|[)(-. \r\n"), str.size()));
  return str;
}
std::string ABCreader::note_string(std::string_view opt_string, std::string_view note_string, int len1, int len2) {
  return std::string(opt_string) + std::string(note_string) + std::to_string(len1) + "/" + std::to_string(len2);
}