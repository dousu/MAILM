#include "Reader.h"

std::map<std::string, std::string> Reader::alias;
std::map<std::string, std::string> Reader::conv_alias;
std::map<int, Meaning> Reader::i_meaning_map;
std::map<int, std::vector<int>> Reader::i_beat_map;
std::map<int, std::string> Reader::labeling;
std::map<AMean, Conception> Reader::core_meaning;
std::vector<Rule> Reader::input_rules;
std::map<std::string, int> Reader::conv_str;
std::unordered_map<std::size_t, std::vector<SymbolElement>>
    Reader::strings;

int Reader::index_count = -1;
int Reader::variable_count = -1;
int Reader::category_count = -1;
int Reader::symbol_count = 1;

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
  std::for_each(std::begin(file_paths), std::end(file_paths),
                [&](std::string f_str) {
                  buf.clear();
                  load(f_str, buf, no++);
                  std::copy(
                      std::begin(buf), std::end(buf),
                      std::ostream_iterator<Rule>(std::cout, "\n"));
                  std::copy(std::begin(buf), std::end(buf),
                            std::back_inserter(input_rules));
                });
}

void ABCreader::load(std::string file_path, std::vector<Rule> &buf,
                     std::size_t no) {
  // strings[file_no]
  file_no = no;
  std::fstream fs;
  fs.open(file_path, std::ios::in);
  std::ostringstream oss;
  for (std::string line; std::getline(fs, line);) {
    std::cout << line << std::endl;
    oss << line << std::endl;
  }

  Meaning s_in;
  std::vector<SymbolElement> s_ex;
  AMean sent_ind(index_count);
  s_in = Meaning(sent_ind);
  core_meaning[sent_ind] = Conception();
  // core_meaning[sent_ind].add(Prefices::SEN);

  // sentenceのインデックス記録
  flat_meaning = Meaning(AMean(index_count));

  index_count--;
  i_beat_map[file_no] = std::vector<int>();

  std::cout << "processing" << std::endl;
  std::string ref_str = oss.str();  // important
  std::string_view str = ref_str;
  // str = next_line(str);
  // std::cout << str << std::endl;
  // std::cout << "extract title" << std::endl;
  t_str = "";
  str = title(str);
  while (t_str == "") {
    str = next_line(str);
    str = title(str);
  }
  // std::cout << str << std::endl;
  std::cout << "Title: " << t_str << std::endl;

  l1 = l2 = 0;
  auto [lbool, lstr] = base_length(str);
  str = lstr;
  while (l1 == 0 && l2 == 0) {
    str = next_line(str);
    auto [lbool_, lstr_] = base_length(str);
    str = lstr_;
  }
  std::cout << "L1: " << l1 << std::endl;
  std::cout << "L2: " << l2 << std::endl;
  q1 = 1;
  q2 = 4;
  q3 = 100;
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  while (str.find("Q:") != 0 && str.size() != 0 &&
         str.find("M:") != 0) {
    str = next_line(str);
    str.remove_prefix(
        std::min(str.find_first_not_of(" "), str.size()));
  }
  auto [qbool, qstr] = tempo(str);
  str = qstr;
  std::cout << "Q1: " << q1 << std::endl;
  std::cout << "Q2: " << q2 << std::endl;
  std::cout << "Q3: " << q3 << std::endl;
  m1 = m2 = 0;
  auto [mbool, mstr] = rhythm(str);
  str = mstr;
  while (m1 == 0 && m2 == 0) {
    str = next_line(str);
    auto [mbool_, mstr_] = rhythm(str);
    str = mstr_;
  }
  std::cout << "M1: " << m1 << std::endl;
  std::cout << "M2: " << m2 << std::endl;
  note_str = "";
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  while (str.find("K:") != 0 && str.size() != 0) {
    str = next_line(str);
    str.remove_prefix(
        std::min(str.find_first_not_of(" "), str.size()));
  }
  Category sent_cat(category_count);
  category_count--;
  k_(str, sent_ind, s_in, s_ex);
  // std::cout << "Key: " << k_str << std::endl;
  // core_meaning[sent_ind].add(Prefices::SEN);
  Symbol::conv_symbol = conv_alias;
  Rule r_sent(LeftNonterminal(sent_cat, s_in), s_ex);
  //文ルール追加
  buf.push_back(r_sent);
  i_meaning_map[file_no] = flat_meaning;
  std::cout << "Flat:" << std::endl << flat_meaning << std::endl;
  std::cout << "Core Meanings" << std::endl;
  std::function<void(AMean &)> view_core_meaning = [](AMean &am) {
    std::cout << am.to_s() << "[" << core_meaning[am].to_s() << "]";
  };
  std::function<void(Meaning &)> confirm_meaning;
  confirm_meaning = [&confirm_meaning,
                     &view_core_meaning](Meaning &m) {
    view_core_meaning(m.get_base());
    std::cout << std::endl;
    for (auto &m_el : m.get_followings()) {
      confirm_meaning(m_el.get<Meaning>());
    }
  };
  confirm_meaning(flat_meaning);
  labeling[file_no] = t_str;
}

std::string_view ABCreader::next_line(std::string_view str) {
  // std::cout << str;
  str.remove_prefix(
      std::min(str.find_first_of("\n") + 1, str.size()));
  // std::cout << str;
  return str;
}

std::string_view ABCreader::title(std::string_view str) {
  // std::cout << str;
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  // std::cout << str;
  if (str.find("T:") != 0) {
    return str;
  }
  std::string_view title_str =
      range_substr(str, 2, str.find_first_of("\r\n"));
  title_str.remove_prefix(
      std::min(title_str.find_first_not_of(" "), title_str.size()));
  t_str = title_str;
  str.remove_prefix(
      std::min(str.find_first_of("\n") + 1, str.size()));
  return str;
}
std::pair<bool, std::string_view> ABCreader::base_length(
    std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("L:") != 0) {
    return {false, str};
  }

  std::string_view base_length_str =
      range_substr(str, 2, str.find_first_of("\r\n]"));
  // l1, l2
  base_length_str.remove_prefix(
      std::min(base_length_str.find_first_not_of(" "),
               base_length_str.size()));

  l1 = std::stoi(std::string(range_substr(
      base_length_str, 0, base_length_str.find_first_of("/"))));
  l2 = std::stoi(std::string(range_substr(
      base_length_str, base_length_str.find_first_of("/") + 1,
      std::min(base_length_str.find_first_of(" \r\n]"),
               base_length_str.size()))));
  str.remove_prefix(
      std::min(str.find_first_of("\n]") + 1, str.size()));
  return {true, str};
}
std::pair<bool, std::string_view> ABCreader::tempo(
    std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("Q:") != 0) {
    return {false, str};
  }
  std::string_view tempo_str =
      range_substr(str, 2, str.find_first_of("\r\n]"));
  // q_str, q1, q2, q3
  tempo_str.remove_prefix(
      std::min(tempo_str.find_first_not_of(" "), tempo_str.size()));
  q_str = quote_string(std::string(tempo_str));
  q1 = std::stoi(std::string(
      range_substr(tempo_str, 0, tempo_str.find_first_of("/"))));
  q2 = std::stoi(std::string(
      range_substr(tempo_str, tempo_str.find_first_of("/") + 1,
                   tempo_str.find_first_of("="))));
  q3 = std::stoi(std::string(range_substr(
      tempo_str, tempo_str.find_first_of("=") + 1,
      std::min(tempo_str.find_first_of(" "), tempo_str.size()))));
  str.remove_prefix(
      std::min(str.find_first_of("\n]") + 1, str.size()));
  return {true, str};
}
std::pair<bool, std::string_view> ABCreader::rhythm(
    std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("M:") != 0) {
    return {false, str};
  }
  std::string_view rhythm_str =
      range_substr(str, 2, str.find_first_of("\r\n]"));
  // m1, m2
  rhythm_str.remove_prefix(
      std::min(rhythm_str.find_first_not_of(" "), rhythm_str.size()));
  m1 = std::stoi(std::string(
      range_substr(rhythm_str, 0, rhythm_str.find_first_of("/"))));
  m2 = std::stoi(std::string(range_substr(
      rhythm_str, rhythm_str.find_first_of("/") + 1,
      std::min(rhythm_str.find_first_of(" "), rhythm_str.size()))));
  str.remove_prefix(
      std::min(str.find_first_of("\n]") + 1, str.size()));
  return {true, str};
}
void ABCreader::k_(std::string_view str, AMean &sent_ind,
                   Meaning &inter,
                   std::vector<SymbolElement> &exter) {
  //! [number](
  //! Cat -> Cat is prohibited
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));

  // std::cout << "in k_ " << std::endl;
  // std::cout << str << std::endl;

  if (str.find("K:") != 0) {
    std::cerr << "Key is not found." << std::endl;
    exit(-1);
  }
  auto [kbool, kstr] = key(str);
  std::cout << "K: " << k_str << std::endl;
  core_meaning[sent_ind].add("K:" + k_str);
  core_meaning[sent_ind].add("Q:" + std::to_string(q1) + "/" +
                             std::to_string(q2) + "=" +
                             std::to_string(q3));
  core_meaning[sent_ind].add("M:" + std::to_string(m1) + "/" +
                             std::to_string(m2));
  str = kstr;
  std::function<std::string_view(std::string_view, Meaning &,
                                 std::vector<SymbolElement> &,
                                 Meaning &)>
      func;
  bool rhythm_zone = false, tempo_zone = false, key_zone = false;
  func = [this, &func, &rhythm_zone, &tempo_zone, &key_zone](
             std::string_view str, Meaning &base_in,
             std::vector<SymbolElement> &base_ex,
             Meaning &flat) -> std::string_view {
    std::vector<SymbolElement> sub_ex;
    Meaning sub_in;
    Meaning sub_flat;
    std::cout << "(";
    bool less_than = false, greater_than = false, hyphen = false;
    do {
      std::string score_option_str;
      do {
        auto [a, b] = score_option(str);
        score_option_str = a;
        str = b;
        if (score_option_str == "|") {
          str.remove_prefix(
              std::min(str.find_first_of("|") + 1, str.size()));
          continue;
        }
        if (score_option_str == "[") {
          str = range_substr(str, str.find_first_of("[") + 1,
                             str.size());
          auto [mbool, mstr] = rhythm(str);
          str = mstr;
          if (mbool) {
            rhythm_zone = true;
            std::cout << "[" << rhythm_string(m1, m2) << "]";
            Category word_cat(category_count);
            Meaning word_in;
            std::vector<SymbolElement> word_ex;
            AMean word_ind(index_count);
            word_in = Meaning(word_ind);
            core_meaning[word_ind] = Conception();
            core_meaning[word_ind].add("M:" + std::to_string(m1) +
                                       "/" + std::to_string(m2));
            // core_meaning[word_ind].add(Prefices::SLR);

            // word
            base_in.get_followings().push_back(
                Variable(variable_count));
            base_ex.push_back(RightNonterminal(
                Category(category_count), Variable(variable_count)));

            Meaning flat_ = Meaning(AMean(index_count));

            index_count--;
            variable_count--;
            category_count--;
            str = func(str, word_in, word_ex, flat_);
            sub_ex = word_ex;
            sub_in = word_in;
            sub_flat = flat_;
            flat.get_followings().push_back(flat_);
            Rule r_word(LeftNonterminal(word_cat, word_in), word_ex);
            buf.push_back(r_word);
            std::cout << ")";
            continue;
          }
          auto [qbool, qstr] = tempo(str);
          str = qstr;
          if (qbool) {
            tempo_zone = true;
            std::cout << "["
                      << "Q:"
                      << std::to_string(q1) + "/" +
                             std::to_string(q2) + "=" +
                             std::to_string(q3)
                      << "]";
            Category word_cat(category_count);
            Meaning word_in;
            std::vector<SymbolElement> word_ex;
            AMean word_ind(index_count);
            word_in = Meaning(word_ind);
            core_meaning[word_ind] = Conception();
            core_meaning[word_ind].add("Q:" + std::to_string(q1) +
                                       "/" + std::to_string(q2) +
                                       "=" + std::to_string(q3));

            // word
            base_in.get_followings().push_back(
                Variable(variable_count));
            base_ex.push_back(RightNonterminal(
                Category(category_count), Variable(variable_count)));

            Meaning flat_ = Meaning(AMean(index_count));

            index_count--;
            variable_count--;
            category_count--;
            str = func(str, word_in, word_ex, flat_);
            sub_ex = word_ex;
            sub_in = word_in;
            sub_flat = flat_;
            flat.get_followings().push_back(flat_);
            std::cout << ")";
            Rule r_word(LeftNonterminal(word_cat, word_in), word_ex);
            buf.push_back(r_word);
            continue;
          }
          auto [kbool, kstr] = key(str);
          str = kstr;
          // std::cout << std::endl << std::string(str) << std::endl;
          if (kbool) {
            key_zone = true;
            std::cout << "["
                      << "K:" << k_str << "]";
            Category word_cat(category_count);
            Meaning word_in;
            std::vector<SymbolElement> word_ex;
            AMean word_ind(index_count);
            word_in = Meaning(word_ind);
            core_meaning[word_ind] = Conception();
            core_meaning[word_ind].add("K:" + k_str);
            // core_meaning[word_ind].add(Prefices::SLR);

            // word
            base_in.get_followings().push_back(
                Variable(variable_count));
            base_ex.push_back(RightNonterminal(
                Category(category_count), Variable(variable_count)));

            Meaning flat_ = Meaning(AMean(index_count));

            index_count--;
            variable_count--;
            category_count--;
            str = func(str, word_in, word_ex, flat_);
            sub_ex = word_ex;
            sub_in = word_in;
            sub_flat = flat_;
            flat.get_followings().push_back(flat_);
            std::cout << ")";
            Rule r_word(LeftNonterminal(word_cat, word_in), word_ex);
            buf.push_back(r_word);
            continue;
          }
        }
        if (score_option_str == "(") {
          str.remove_prefix(
              std::min(str.find_first_of("(") + 1, str.size()));
          Category word_cat(category_count);
          Meaning word_in;
          std::vector<SymbolElement> word_ex;
          AMean word_ind(index_count);
          word_in = Meaning(word_ind);
          core_meaning[word_ind] = Conception();
          core_meaning[word_ind].add(Prefices::SLR);

          // word
          base_in.get_followings().push_back(
              Variable(variable_count));
          base_ex.push_back(RightNonterminal(
              Category(category_count), Variable(variable_count)));

          Meaning flat_ = Meaning(AMean(index_count));

          index_count--;
          variable_count--;
          category_count--;
          str = func(str, word_in, word_ex, flat_);
          sub_ex = word_ex;
          sub_in = word_in;
          sub_flat = flat_;
          flat.get_followings().push_back(flat_);
          std::cout << ")";
          Rule r_word(LeftNonterminal(word_cat, word_in), word_ex);
          buf.push_back(r_word);
          continue;
        }
        if (score_option_str == ")") {
          if (rhythm_zone) {
            rhythm_zone = false;
            str.remove_prefix(
                std::min(str.find_first_of(")"), str.size()));
            if (base_ex.size() == 1 &&
                base_ex.front().type() == ELEM_TYPE::NT_TYPE) {
              flat = sub_flat;
              core_meaning[sub_in.get_base()].add(
                  core_meaning[base_in.get_base()]);
              core_meaning.erase(base_in.get_base());
              base_ex = sub_ex;
              base_in = sub_in;
              buf.erase(std::prev(std::end(buf)));
            }
            return str;
          }
          if (tempo_zone) {
            tempo_zone = false;
            str.remove_prefix(
                std::min(str.find_first_of(")"), str.size()));
            if (base_ex.size() == 1 &&
                base_ex.front().type() == ELEM_TYPE::NT_TYPE) {
              flat = sub_flat;
              core_meaning[sub_in.get_base()].add(
                  core_meaning[base_in.get_base()]);
              core_meaning.erase(base_in.get_base());
              base_ex = sub_ex;
              base_in = sub_in;
              buf.erase(std::prev(std::end(buf)));
            }
            return str;
          }
          if (key_zone) {
            key_zone = false;
            str.remove_prefix(
                std::min(str.find_first_of(")"), str.size()));
            if (base_ex.size() == 1 &&
                base_ex.front().type() == ELEM_TYPE::NT_TYPE) {
              flat = sub_flat;
              core_meaning[sub_in.get_base()].add(
                  core_meaning[base_in.get_base()]);
              core_meaning.erase(base_in.get_base());
              base_ex = sub_ex;
              base_in = sub_in;
              buf.erase(std::prev(std::end(buf)));
            }
            return str;
          }
          str.remove_prefix(
              std::min(str.find_first_of(")") + 1, str.size()));
          if (base_ex.size() == 1 &&
              base_ex.front().type() == ELEM_TYPE::NT_TYPE) {
            flat = sub_flat;
            core_meaning[sub_in.get_base()].add(
                core_meaning[base_in.get_base()]);
            core_meaning.erase(base_in.get_base());
            base_ex = sub_ex;
            base_in = sub_in;
            buf.erase(std::prev(std::end(buf)));
          }
          return str;
        }
        if (score_option_str == "!") {
          str.remove_prefix(std::min(std::size_t(1), str.size()));
          str.remove_prefix(
              std::min(str.find_first_of("!") + 1, str.size()));
          continue;
        }
        if (score_option_str == "\"") {
          // std::cout << std::endl << std::string(str) << std::endl;
          str.remove_prefix(std::min(std::size_t(1), str.size()));
          str.remove_prefix(
              std::min(str.find_first_of("\"") + 1, str.size()));
          // std::cout << std::string(str) << std::endl;
          continue;
        }
        // std::cout << score_option_str << std::endl;
      } while (score_option_str != "");

      str.remove_prefix(std::min(
          str.find_first_of("ABCDEFGZabcdefgz._=^"), str.size()));
      // std::cout << "before pocessing: " << str << std::endl;
      str = note(str);
      // std::cout << "after pocessing: " << str << std::endl;
      // note_str = note_string(opt_str, name_str, len1, len2);
      if (name_str == "") {
        if (base_ex.size() == 1 &&
            base_ex.front().type() == ELEM_TYPE::NT_TYPE) {
          flat = sub_flat;
          core_meaning[sub_in.get_base()].add(
              core_meaning[base_in.get_base()]);
          core_meaning.erase(base_in.get_base());
          base_ex = sub_ex;
          base_in = sub_in;
          buf.erase(std::prev(std::end(buf)));
        }
        return str;
      }
      // -, >, <
      if (greater_than) {
        nl2 *= 2;
      }
      if (less_than) {
        nl1 *= 3;
        nl2 *= 2;
      }

      hyphen = greater_than = less_than = false;
      if (str.find_first_of("-") == 0) {
        hyphen = true;
        note_str = note_string(opt_str, name_str, nl1, l2);
      } else if (str.find_first_of(">") == 0) {
        greater_than = true;
        note_str = note_string(opt_str, name_str, nl1 * 3, l2 * 2);
      } else if (str.find_first_of("<") == 0) {
        less_than = true;
        note_str = note_string(opt_str, name_str, nl1, nl2 * 2);
      } else {
        note_str = note_string(opt_str, name_str, nl1, nl2);
      }
      // std::cout << "Found: " << note_str << std::endl;
      std::cout << note_str << " ";

      if (true) {
        if (alias.find(opt_str + name_str) == alias.end()) {
          std::string str2;
          str2 = Prefices::SYN + std::to_string(symbol_count);
          alias.insert(std::map<std::string, std::string>::value_type(
              opt_str + name_str, str2));
          conv_alias.insert(
              std::map<std::string, std::string>::value_type(
                  str2, opt_str + name_str));
          Dictionary::symbol.insert(
              std::map<int, std::string>::value_type(symbol_count,
                                                     str2));
          Dictionary::conv_symbol.insert(
              std::map<std::string, int>::value_type(str2,
                                                     symbol_count));
          conv_str[opt_str + name_str] = symbol_count++;
        }
        Category note_cat(category_count);
        Meaning note_in;
        std::vector<SymbolElement> note_ex;
        AMean note_ind(index_count);
        note_in = Meaning(note_ind);
        core_meaning[note_ind] = Conception();
        core_meaning[note_ind].add("NL:" + std::to_string(nl1) + "/" +
                                   std::to_string(nl2));
        if (note_str.find_first_of("z") != note_str.npos) {
          core_meaning[note_ind].add("rest");
        }
        note_ex.push_back(Symbol(conv_str[opt_str + name_str]));
        strings[file_no].push_back(
            Symbol(conv_str[opt_str + name_str]));

        sub_ex = note_ex;
        sub_in = note_in;
        sub_flat = Meaning(AMean(index_count));

        // base_ex.push_back(Symbol(conv_str[note_str]));
        base_in.get_followings().push_back(Variable(variable_count));
        flat.get_followings().push_back(Meaning(AMean(index_count)));
        base_ex.push_back(RightNonterminal(Category(category_count),
                                           Variable(variable_count)));
        index_count--;
        variable_count--;
        category_count--;

        Rule r_note(LeftNonterminal(note_cat, note_in), note_ex);
        buf.push_back(r_note);
      } else {
        if (alias.find(note_str) == alias.end()) {
          std::string str2;
          str2 = Prefices::SYN + std::to_string(symbol_count);
          alias.insert(std::map<std::string, std::string>::value_type(
              note_str, str2));
          conv_alias.insert(
              std::map<std::string, std::string>::value_type(
                  str2, note_str));
          Dictionary::symbol.insert(
              std::map<int, std::string>::value_type(symbol_count,
                                                     str2));
          Dictionary::conv_symbol.insert(
              std::map<std::string, int>::value_type(str2,
                                                     symbol_count));
          conv_str[note_str] = symbol_count++;
        }
        base_ex.push_back(Symbol(conv_str[note_str]));
        strings[file_no].push_back(Symbol(conv_str[note_str]));
      }
    } while (note_str != "");
    if (base_ex.size() == 1 &&
        base_ex.front().type() == ELEM_TYPE::NT_TYPE) {
      flat = sub_flat;
      core_meaning[sub_in.get_base()].add(
          core_meaning[base_in.get_base()]);
      core_meaning.erase(base_in.get_base());
      base_ex = sub_ex;
      base_in = sub_in;
      buf.erase(std::prev(std::end(buf)));
    }
    return str;
  };
  str = func(str, inter, exter, flat_meaning);
  std::cout << ")" << std::endl;
}
std::pair<bool, std::string_view> ABCreader::key(
    std::string_view str) {
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
  if (str.find("K:") != 0) {
    return {false, str};
  }
  k_str = range_substr(str, 2, str.find_first_of("\r\n]"));
  // std::cout << "Key: " << k_str << std::endl;
  // std::string_view k_view = k_str;
  // k_view.remove_suffix(std::min(k_view.find_last_not_of(" "),
  // k_view.size())); k_str = k_view;
  str.remove_prefix(
      std::min(str.find_first_of("\n]") + 1, str.size()));
  return {true, str};
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
  std::string_view option_str;
  str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));

  option_str =
      range_substr(str, 0, str.find_first_of("ABCDEFGZabcdefgz"));

  if (option_str.find_first_of(".") != option_str.npos) {  // staccato
    dot = true;
    opt_str += ".";
  }
  if (option_str.find_first_of("^") != option_str.npos) {  // sharp
    opt_str += "^";
  } else if (option_str.find_first_of("=") !=
             option_str.npos) {  // natural
    opt_str += "=";
  } else if (option_str.find_first_of("_") !=
             option_str.npos) {  // flat
    opt_str += "_";
  }

  str.remove_prefix(
      std::min(str.find_first_of("ABCDEFGZabcdefgz"), str.size()));
  return str;
}
std::string_view ABCreader::name(std::string_view str) {
  name_str = "";
  if (str.find_first_of("ABCDEFGZabcdefgz") != 0) {
    return str;
  }
  std::string_view tmp = range_substr(str, 1, str.size());
  auto loc =
      std::min(tmp.find_first_of(
                   "ABCDEFGZabcdefgz<>|[)(-._=^/1234567890!\" \r\n"),
               tmp.size()) +
      1;
  name_str = range_substr(str, 0, loc);

  str.remove_prefix(loc);
  return str;
}
std::string_view ABCreader::note_length(std::string_view str) {
  nl_str = "";
  nl_str = range_substr(
      str, 0,
      std::min(
          str.find_first_of("ABCDEFGZabcdefgz<>|[)(-._=^!\" \r\nP"),
          str.size()));

  std::size_t bound = nl_str.find_first_of("/");
  if (bound != nl_str.npos) {
    if (bound != 0) {
      nl1 *= std::stoi(std::string(range_substr(nl_str, 0, bound)));
    }
    if (bound == nl_str.size() - 1) {
      nl2 *= 2;
    } else {
      nl2 *= std::stoi(std::string(
          range_substr(nl_str, bound + 1, nl_str.size())));
    }
  } else if (nl_str != "") {
    nl1 *= std::stoi(std::string(nl_str));
  }
  // if (dot) {
  //   nl1 *= 3;
  //   nl2 *= 2;
  // }
  str.remove_prefix(std::min(
      str.find_first_of("ABCDEFGZabcdefgz<>|[)(-._=^!\" \r\n"),
      str.size() - 1));
  return str;
}
std::string ABCreader::note_string(std::string_view opt_string,
                                   std::string_view note_string,
                                   int len1, int len2) {
  if (note_string == "") {
    return "";
  }
  auto val = std::gcd(len1, len2);
  return std::string(opt_string) + std::string(note_string) +
         std::to_string(len1 / val) + "/" +
         std::to_string(len2 / val);
}
std::string ABCreader::rhythm_string(int len1, int len2) {
  return "M:" + std::to_string(len1) + "/" + std::to_string(len2);
}

std::string_view ABCreader::range_substr(std::string_view &v,
                                         std::size_t p1,
                                         std::size_t p2) {
  return v.substr(p1, p2 - p1);
}
std::string ABCreader::range_substr(const std::string &v,
                                    std::size_t p1, std::size_t p2) {
  return v.substr(p1, p2 - p1);
}
std::string ABCreader::quote_string(const std::string &v) {
  auto pos = v.find_first_of("\"");
  if (pos == v.npos || pos == v.find_last_of("\"")) {
    return "";
  }
  return std::string(range_substr(v, v.find_first_of("\"") + 1,
                                  v.find_last_of("\"")));
}
std::string ABCreader::exclamation_string(const std::string &v) {
  auto pos = v.find_first_of("!");
  if (pos == v.npos || pos == v.find_last_of("!")) {
    return "";
  }
  return std::string(
      range_substr(v, v.find_first_of("!") + 1, v.find_last_of("!")));
}
std::pair<std::string, std::string_view> ABCreader::score_option(
    std::string_view str) {
  str.remove_prefix(std::min(
      str.find_first_of("ABCDEFGZabcdefgz|[)(._=^!\""), str.size()));
  if (str.find_first_of("|") == 0) {
    return {"|", str};
  }
  if (str.find_first_of("[") == 0) {
    return {"[", str};
  }
  if (str.find_first_of(")") == 0) {
    return {")", str};
  }
  if (str.find_first_of("(") == 0) {
    return {"(", str};
  }
  if (str.find_first_of("!") == 0) {
    return {"!", str};
  }
  if (str.find_first_of("\"") == 0) {
    return {"\"", str};
  }
  return {"", str};
}