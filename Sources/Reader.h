#ifndef READER_H_
#define READER_H_

#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

// #include <boost/algorithm/string.hpp>
// #include <boost/algorithm/string/trim.hpp>
// #include <boost/foreach.hpp>
// #include <boost/lexical_cast.hpp>
// #include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/xml_parser.hpp>

#include "Element.h"
#include "Prefices.h"
#include "Rule.h"

struct Reader {
  Reader(){};
  virtual ~Reader(){};
  static std::map<std::string, std::string> alias;
  static std::map<std::string, std::string> conv_alias;
  static std::map<int, Meaning> i_meaning_map;                                 // for evaluation of reconstruction
  static std::map<int, std::vector<int>> i_beat_map;                           // for making tree with graphs
  static std::unordered_map<std::size_t, std::vector<SymbolElement>> strings;  // to parse
  static std::map<int, std::string> labeling;                                  // file name
  static std::map<AMean, Conception> core_meaning;                             // semantics
  static std::vector<Rule> input_rules;                                        // record of existing score
  static std::map<std::string, int> conv_str;                                  // utility tool
  virtual void make_init_data(std::vector<std::string> &file_paths) = 0;
  static int index_count;
  static int variable_count;
  static int category_count;
  static int symbol_count;
};

// class XMLreader : Reader {
//  public:
//   static int index_count;

//   static int variable_count;

//   static int category_count;

//   static int symbol_count;

//   void make_init_data(std::vector<std::string> &file_paths) override;
//   void load(std::string, std::vector<Rule> &, int);
// };

// class XMLreaderMono : Reader {
//  public:
//   static int index_count;

//   static int variable_count;

//   static int category_count;

//   static int symbol_count;

//   void make_init_data(std::vector<std::string> &file_paths) override;
//   void load(std::string, std::vector<Rule> &, int);
// };

class ABCreader : Reader {
 public:
  void make_init_data(std::vector<std::string> &file_paths) override;
  void load(std::string, std::vector<Rule> &, std::size_t);

  std::string t_str;
  int l1;
  int l2;
  std::string q_str;
  int q1;
  int q2;
  int q3;
  int m1;
  int m2;
  std::string k_str;
  std::string opt_str;
  std::string name_str;
  std::string nl_str;
  int nl1;
  int nl2;
  std::string note_str;
  bool dot;
  Meaning flat_meaning;
  std::size_t file_no;
  std::vector<Rule> buf;

  std::string_view title(std::string_view str);
  std::string_view base_length(std::string_view str);
  std::string_view tempo(std::string_view str);
  std::string_view rhythm(std::string_view str);
  void k_(std::string_view str, Meaning &, std::vector<SymbolElement> &);
  std::string_view key(std::string_view str);
  std::string_view note(std::string_view str);
  std::string_view option(std::string_view str);
  std::string_view name(std::string_view str);
  std::string_view note_length(std::string_view str);
  std::string_view next_line(std::string_view str);
  std::string note_string(std::string_view opt_string, std::string_view note_string, int len1, int len2);
  std::string rhythm_string(int len1, int len2);
  static std::string_view range_substr(std::string_view &, std::size_t, std::size_t);
  static std::string range_substr(const std::string &, std::size_t, std::size_t);
  std::string_view quote_string(std::string_view);
};

#endif /* READER_H_ */
