#include "MAILM_main.h"

void output_data_trunc(std::string file_path, std::string data) {
  //存在しない場合はすぐreturn
  const std::filesystem::path path(file_path.c_str());
  std::cout << "Output file(trunc mdoe): " << path << std::endl;
  std::ofstream ofs(path, std::ios_base::out | std::ios_base::trunc);
  ofs << data;
}
void output_data_app(std::string file_path, std::string data) {
  //存在しない場合はすぐreturn
  const std::filesystem::path path(file_path.c_str());
  std::cout << "Output file(app mode): " << path << std::endl;
  std::ofstream ofs(path, std::ios_base::in | std::ios_base::out |
                              std::ios_base::app);
  ofs << data;
}
std::string make_tree_str_for_dot(std::vector<Rule> &r_list) {
  std::set<std::string> syms;
  std::ostringstream os;
  std::list<Rule> r_list_1;
  std::copy(std::begin(r_list), std::end(r_list),
            std::back_inserter(r_list_1));
  std::map<std::variant<AMean, Symbol>, int> index_table;
  std::multimap<int, std::string> rank_table;
  std::function<void(int, const Rule &)> make_tree_str_for_dot_2;
  make_tree_str_for_dot_2 = [&](int level, const auto &rule) {
    std::list<std::string> o_list;
    int j = index_table[rule.get_internal().get_base()]++;
    rank_table.insert(
        std::make_pair(level, rule.get_internal().get_base().to_s() +
                                  "(" + std::to_string(j) + ")"));
    auto make_tree_str_for_dot_3 = [&](const auto &sel) {
      switch (sel.type()) {
        case ELEM_TYPE::NT_TYPE: {
          auto it = std::find_if(
              std::begin(r_list_1), std::end(r_list_1),
              [&sel](const auto &a) {
                return sel.template get<RightNonterminal>()
                           .get_cat() == a.get_internal().get_cat();
              });
          if (it == std::end(r_list_1)) {
            std::cerr << "Irregular rule set in making tree"
                      << std::endl;
            exit(1);
          }
          int i = index_table[it->get_internal().get_base()];
          o_list.push_back(it->get_internal().get_base().to_s() +
                           "(" + std::to_string(i) + ")");
          auto rule_1 = *it;
          r_list_1.erase(it);
          make_tree_str_for_dot_2(level + 1, rule_1);
        } break;
        case ELEM_TYPE::SYM_TYPE: {
          int i = index_table[sel.template get<Symbol>()]++;
          syms.insert(sel.template get<Symbol>().to_s() + "(" +
                      std::to_string(i) + ")");
          o_list.push_back(sel.template get<Symbol>().to_s() + "(" +
                           std::to_string(i) + ")");
        } break;
      }
    };
    std::for_each(std::begin(rule.get_external()),
                  std::end(rule.get_external()),
                  make_tree_str_for_dot_3);
    std::for_each(std::begin(o_list), std::end(o_list), [&](auto p) {
      os << std::quoted(rule.get_internal().get_base().to_s() + "(" +
                        std::to_string(j) + ")")
         << Prefices::DOTARW << std::quoted(p) << std::endl;
    });
  };
  auto item = r_list_1.front();
  r_list_1.erase(std::begin(r_list_1));
  make_tree_str_for_dot_2(0, item);
  os << "{rank = min; "
     << std::quoted(item.get_internal().get_base().to_s() + "(0)")
     << "}" << std::endl;

  int max = 0;
  std::multimap<int, std::string>::iterator rank_it;
  do {
    max++;
    os << "{rank = same;";
    auto r = rank_table.equal_range(max);
    std::for_each(r.first, r.second, [&](auto &p) {
      os << " " << std::quoted(p.second) << ";";
    });
    os << "}" << std::endl;
  } while ((rank_it = rank_table.upper_bound(max)) !=
           std::end(rank_table));

  os << "{rank = max;";
  std::for_each(std::begin(syms), std::end(syms),
                [&](auto &s) { os << " " << std::quoted(s) << ";"; });
  os << "}" << std::endl;
  return "digraph sample{\n" + os.str() + "}";
}

void static_assessment(Agent &ma, std::string out) {
  std::cout << "static assessment file: " << out << std::endl;
  std::ostringstream os;
  os << "Generation(" << ma.generation_index << ")" << std::endl
     << "Size = " << ma.kb.size() << std::endl;
  int no;
  for (no = 1; no <= Reader::labeling.size(); no++) {
    std::string name = Reader::labeling[no];
    std::cout << "Parse step " << name << ".xml" << std::endl;
    std::copy(std::begin(Reader::strings[no]),
              std::end(Reader::strings[no]),
              std::ostream_iterator<SymbolElement>(std::cout, " "));
    std::cout << std::endl;
    auto ret = ma.kb.parse_string(Reader::strings[no]);
    if (ret.size() != 0) {
      std::copy(std::begin(ret), std::end(ret),
                std::ostream_iterator<Rule>(std::cout, "\n"));
      std::cout << "finished to parse" << std::endl;
      os << "Parsable[" << name << ".xml] = true" << std::endl;
    } else {
      std::cout << "Can't parse" << std::endl;
      os << "Parsable[" << name << ".xml] = false" << std::endl;
    }
  }
  output_data_app(out, os.str());
}

void tree_assessment(Agent &ma, std::string out) {
  std::vector<std::string> t_list;
  std::vector<std::string>::iterator t_it;

  std::vector<Rule> r_list;
  std::string tree_str;
  int no;
  std::vector<int> beat_nums;
  for (int i = 1; i <= Reader::labeling.size(); i++) {
    no = i;
    beat_nums = Reader::i_beat_map[no];
    std::string name = Reader::labeling[no];
    r_list.clear();
    std::cout << "Construct " << name << ".xml" << std::endl;
    if (ma.kb.explain(ma.kb.meaning_no(no), r_list)) {
      std::cout << "explaination: true" << std::endl;
      tree_str = make_tree_str_for_dot(r_list);
      std::cout << "made tree graph as a dot file" << std::endl;

      std::filesystem::path p(out + "generation" +
                              std::to_string(ma.generation_index) +
                              ".d/");
      if (!std::filesystem::exists(p)) {
        std::filesystem::create_directory(p);
      }

      output_data_trunc(p.string() + name + ".dot", tree_str);

      std::cout << "output fin." << std::endl;
    } else {
      std::cout << "Can't construct" << std::endl;
    }
  }
}

void evaluate_knowledge(Agent &ma, MAILMParameters &param) {
  static_assessment(ma, param.RESULT_PATH + "static/" +
                            param.FILE_PREFIX + param.DATE_STR +
                            ".d/" + "static" + param.RESULT_EXT);
  tree_assessment(ma, param.RESULT_PATH + "dot/" + param.FILE_PREFIX +
                          param.DATE_STR + ".d/");
}

int main(int argc, char *argv[]) {
  LogBox log;  // for destructor
  MAILMParameters param;

  /**************************************************
   *
   * OPTION PROCESSING
   *
   **************************************************/
  ProgramOption opt;
  opt.add_option()("help,h", "Description")
      /*ランダムシード*/
      ("random-seed", ProgramOption::value<int>(),
       "Random seed (101010)")
      /*実験世代数*/
      ("generations", ProgramOption::value<int>(),
       "Max generation number (100)")
      /*発話回数*/
      ("utterances", ProgramOption::value<int>(),
       "Uttering ratio for meaning space (25)")
      // handling monophony
      ("mono", "mode for monophony")
      // using abc notation
      ("abc", "abc notation for input and output")
      // using lilypond output
      ("lilypond", "output by lilypond style")
      /*ロギング*/
      ("logging", "Logging")
      /*分析*/
      ("analyze",
       "Analyze each agent for expression and a number of rules")
      /*辞書ファイル*/
      ("dictionary", ProgramOption::value<std::string>(),
       "Dictionary file name for meaning space(\"./data.dic\")")
      /*生成規則再利用*/
      ("keep-random-rule",
       "Keep created rules with a random word into parent "
       "knowledge-base")
      /*FILE PREFIX*/
      ("prefix", ProgramOption::value<std::string>(),
       "Set file prefix (\"MSILM\")")
      /*BASE PATH*/
      ("path", ProgramOption::value<std::string>(),
       "Set folder for output files (\"../Result/\")")
      ///*プログレスバー*/
      //("progress,p", "Show progress bar")
      ;
  opt.parse(argc, argv);
  param.set_option(opt);

  MT19937::set_seed(param.RANDOM_SEED);
  if (param.LOGGING) {
    Knowledge::logging_on();
    LogBox::set_filepath(param.LOG_FILE);
  }
  std::vector<std::string> file_list;

  const std::filesystem::path path(param.XML_DIR);
  std::for_each(std::filesystem::directory_iterator(path),
                std::filesystem::directory_iterator(),
                [&](const std::filesystem::directory_entry &p) {
                  if (!p.is_directory() &&
                      (p.path().extension()).generic_string() ==
                          param.XML_EXT) {
                    file_list.push_back(p.path().generic_string());
                  }
                });
  if (param.ABC) {
    ABCreader abcr;
    abcr.make_init_data(file_list);
  }  // } else if (param.MONO) {
  //   XMLreaderMono xmlr;
  //   xmlr.make_init_data(file_list);
  // } else {
  //   XMLreader xmlr;
  //   xmlr.make_init_data(file_list);
  // }

  std::cout << "Read" << std::endl;
  Agent parent;
  parent.init_semantics(Reader::i_meaning_map);
  std::cout << "Initialize" << std::endl;
  parent.hear(Reader::input_rules, Reader::core_meaning);
  std::cout << "Heared" << std::endl;
  if (param.LOGGING) LogBox::push_log(parent.kb.to_s());
  // parent.learn();
  // std::cout << "Learned" << std::endl;
  parent.grow();
  std::cout << parent.kb.to_s() << std::endl;
  if (param.ANALYZE) {
    {
      std::filesystem::path p(param.RESULT_PATH + "static/" +
                              param.FILE_PREFIX + param.DATE_STR +
                              ".d/");
      if (!std::filesystem::exists(p)) {
        std::filesystem::create_directory(p);
      }
    }

    evaluate_knowledge(parent, param);
  }
  // for utterance data
  {
    std::filesystem::path p(param.RESULT_PATH + "dot/" +
                            param.FILE_PREFIX + param.DATE_STR +
                            ".d/");
    if (!std::filesystem::exists(p)) {
      std::filesystem::create_directory(p);
    }
  }
  {
    std::filesystem::path p(param.RESULT_PATH + "ly/" +
                            param.FILE_PREFIX + param.DATE_STR +
                            ".d/");
    if (!std::filesystem::exists(p)) {
      std::filesystem::create_directory(p);
    }
  }
  {
    std::filesystem::path p(param.RESULT_PATH + "abc/" +
                            param.FILE_PREFIX + param.DATE_STR +
                            ".d/");
    if (!std::filesystem::exists(p)) {
      std::filesystem::create_directory(p);
    }
  }
  if (param.LOGGING) {
    log.push_log("******************PARENT");
    log.push_log(parent.kb.to_s());
    log.push_log(parent.kb.dic_to_s());
  }

  for (int g = 0; g < param.MAX_GENERATIONS; g++) {
    std::cout << std::endl << "Generation " << g + 1 << std::endl;
    std::vector<Rule> inputs = Reader::input_rules;
    std::map<AMean, Conception> cmap = Reader::core_meaning;
    std::filesystem::path dot_dir, ly_dir, abc_dir;
    {
      dot_dir = std::filesystem::path(
          param.RESULT_PATH + "dot/" + param.FILE_PREFIX +
          param.DATE_STR + ".d/" + "generation" +
          std::to_string(parent.generation_index) + ".d/");
      if (!std::filesystem::exists(dot_dir)) {
        std::filesystem::create_directory(dot_dir);
      }
    }
    if (param.LILYPOND) {
      ly_dir = std::filesystem::path(
          param.RESULT_PATH + "ly/" + param.FILE_PREFIX +
          param.DATE_STR + ".d/" + "generation" +
          std::to_string(parent.generation_index) + ".d/");
      if (!std::filesystem::exists(ly_dir)) {
        std::filesystem::create_directory(ly_dir);
      }
    }
    if (param.ABC) {
      abc_dir = std::filesystem::path(
          param.RESULT_PATH + "abc/" + param.FILE_PREFIX +
          param.DATE_STR + ".d/" + "generation" +
          std::to_string(parent.generation_index) + ".d/");
      if (!std::filesystem::exists(abc_dir)) {
        std::filesystem::create_directory(abc_dir);
      }
    }
    for (int u = 0; u < param.UTTERANCES; u++) {
      std::cout << std::endl << "Utterance " << u + 1 << std::endl;
      std::map<AMean, Conception> cmap_say;
      std::vector<Rule> base;
      std::vector<Rule> utter = parent.say(cmap_say, base);
      std::copy(std::begin(utter), std::end(utter),
                std::ostream_iterator<Rule>(std::cout, "\n"));
      std::cout << std::endl << "Core Meanings" << std::endl;
      std::function<void(AMean &)> view_core_meaning =
          [&cmap_say](AMean &am) {
            std::cout << am.to_s() << "[" << cmap_say[am].to_s()
                      << "]" << std::endl;
          };
      for (auto &u_rule : utter) {
        view_core_meaning(u_rule.get_internal().get_base());
      }
      std::cout << std::endl;
      if (utter.size() != 0) {
        output_data_trunc(dot_dir.string() + "utter" +
                              std::to_string(u + 1) + ".dot",
                          make_tree_str_for_dot(base));
        if (param.LILYPOND) {
          std::string output_str =
              LyOutputMusic::output(utter, cmap_say);
          std::cout << output_str << std::endl;
          output_data_trunc(ly_dir.string() + "utter" +
                                std::to_string(u + 1) + ".ly",
                            output_str);
        }
        if (param.ABC) {
          std::string output_str =
              AbcOutputMusic::output(u + 1, utter, cmap_say);
          std::cout << output_str << std::endl;
          output_data_trunc(abc_dir.string() + "utter" +
                                std::to_string(u + 1) + ".abc",
                            output_str);
        }
        std::cout << "output fin." << std::endl;
      } else {
        std::cout << "no utterance" << std::endl;
      }
      std::cout << std::endl;
      cmap.merge(cmap_say);
      inputs.insert(std::end(inputs), std::begin(utter),
                    std::end(utter));
    }
    Agent ma = parent.make_child();
    ma.init_semantics(Reader::i_meaning_map);
    ma.hear(inputs, cmap);
    std::cout << "\n%%% initial state" << std::endl;
    std::cout << ma.to_s() << std::endl;
    std::cout << "start learning" << std::endl;
    ma.learn();
    std::cout << "finish learning" << std::endl;
    std::cout << "learned" << std::endl;
    std::cout << "\n%%% after learning" << std::endl;
    std::cout << ma.to_s() << std::endl;

    if (param.LOGGING) {
      log.push_log("******************AGENT " +
                   std::to_string(ma.generation_index));
      log.push_log(ma.kb.to_s());
      log.push_log(ma.kb.dic_to_s());
    }

    log.refresh_log();

    ma.grow();
    if (param.ANALYZE) evaluate_knowledge(ma, param);
    parent = ma;
  }

  log.refresh_log();

  return 0;
}
