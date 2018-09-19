#ifndef XMLREADER_H_
#define XMLREADER_H_

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Element.h"
#include "Prefices.h"
#include "Rule.h"

class XMLreaderMono;

class XMLreader {
 public:
  // ID numberからコンマのあるリスト表示へ（C1 => (C,D,E)）
  static std::map<std::string, std::string> alias;
  //コンマのあるリストからID number表示へ（(C,D,E) => C1）
  static std::map<std::string, std::string> conv_alias;

  static std::map<int, Meaning> i_meaning_map;        // meaning
  static std::map<int, std::vector<int>> i_beat_map;  // for making tree
  static std::unordered_map<int, std::vector<SymbolElement>> strings;

  static std::map<int, std::string> labeling;       // file name
  static std::map<AMean, Conception> core_meaning;  // semantics
  static std::vector<Rule> input_rules;
  static std::map<std::string, int> conv_str;  // utility tool

  // static std::map<int,std::vector<int> > meaning_rules;

  static int index_count;

  static int variable_count;

  static int category_count;

  static int symbol_count;

  static void make_init_data(std::vector<std::string> &file_paths);

  static void load(std::string, std::vector<Rule> &, int);

  static XMLreader copy(void);

  static XMLreader copy(const XMLreaderMono & obj);
};

class XMLreaderMono {
 public:
  // ID numberからコンマのあるリスト表示へ（C1 => (C,D,E)）
  static std::map<std::string, std::string> alias;
  //コンマのあるリストからID number表示へ（(C,D,E) => C1）
  static std::map<std::string, std::string> conv_alias;

  static std::map<int, Meaning> i_meaning_map;        // meaning
  static std::map<int, std::vector<int>> i_beat_map;  // for making tree
  static std::unordered_map<int, std::vector<SymbolElement>> strings;

  static std::map<int, std::string> labeling;       // file name
  static std::map<AMean, Conception> core_meaning;  // semantics
  static std::vector<Rule> input_rules;
  static std::map<std::string, int> conv_str;  // utility tool

  // static std::map<int,std::vector<int> > meaning_rules;

  static int index_count;

  static int variable_count;

  static int category_count;

  static int symbol_count;

  static void make_init_data(std::vector<std::string> &file_paths);

  static void load(std::string, std::vector<Rule> &, int);

  static XMLreaderMono copy(void);
};

#endif /* XMLREADER_H_ */

