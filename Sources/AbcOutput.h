#ifndef ABCOUTPUT_H_
#define ABCOUTPUT_H_

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Element.h"
#include "Knowledge.h"
#include "Prefices.h"
#include "Rule.h"
#include "Semantics.h"

class AbcOutputMusic {
 public:
  static Semantics<Conception> intention;
  static Knowledge kb;
  static std::string output(int no, std::vector<Rule> &ur,
                            std::map<AMean, Conception> cmap) {
    std::copy(std::begin(ur), std::end(ur),
              std::ostream_iterator<Rule>(std::cout, "\n"));

    std::ostringstream os;
    std::string l_s = "L:1/1", q_s = "", m_s = "", k_s = "",
                symbol_string = "",
                title = "music" + std::to_string(no);

    std::function<std::string(std::vector<Rule>::iterator &,
                              std::vector<Rule>::iterator)>
        func;
    std::function<std::string(const Symbol &)> symbolization =
        [](const Symbol &s) {
          return s.to_s().substr(1, s.to_s().size() - 2);
        };
    // std::function<std::string(Conception &)> l_selector =
    //     [](Conception &cc) {
    //       std::vector<std::string> pre =
    //           cc.extract_if([](const std::string &substr) {
    //             return substr.find_first_of("L:") == 0;
    //           });
    //       return pre[MT19937::irand(0, pre.size() - 1)];
    //     };
    std::function<std::string(Conception &)> q_selector =
        [](Conception &cc) {
          std::vector<std::string> pre =
              cc.extract_if([](const std::string &substr) {
                return substr.find_first_of("Q:") == 0;
              });
          return pre[MT19937::irand(0, pre.size() - 1)];
        };
    std::function<std::string(Conception &)> m_selector =
        [](Conception &cc) {
          std::vector<std::string> pre =
              cc.extract_if([](const std::string &substr) {
                return substr.find_first_of("M:") == 0;
              });
          return pre[MT19937::irand(0, pre.size() - 1)];
        };
    std::function<std::string(Conception &)> k_selector =
        [](Conception &cc) {
          std::vector<std::string> pre =
              cc.extract_if([](const std::string &substr) {
                return substr.find_first_of("K:") == 0;
              });
          return pre[MT19937::irand(0, pre.size() - 1)];
        };
    std::function<std::string(Conception &)> nl_selector =
        [](Conception &cc) {
          std::vector<std::string> pre =
              cc.extract_if([](const std::string &substr) {
                return substr.find_first_of("NL:") == 0;
              });
          return pre[MT19937::irand(0, pre.size() - 1)].substr(3);
        };
    func = [&func, &l_s, &q_s, &m_s, &k_s, &symbolization, &cmap,
            &q_selector, &m_selector, &k_selector,
            &nl_selector](std::vector<Rule>::iterator &trg_it,
                          std::vector<Rule>::iterator end_it) {
      //   return "C2E4C2|D2C2F4|]";
      std::cout << "processing: " << *trg_it << std::endl;
      std::string str = "";
      if (trg_it == end_it) {
        return str;
      }
      std::vector<SymbolElement> sel_vec = (*trg_it).get_external();
      Conception cc = cmap[(*trg_it).get_internal().get_base()];
      if (cc.include(Conception(Prefices::SLR))) {
        str += "(";
      }
      //   std::cout << "[" << cc << "]" << std::endl;
      //   if (cc.include([](const std::string &substr) {
      //         return substr.find_first_of("L:") == 0;
      //       })) {
      //     if (l_s == "") {
      //       l_s = l_selector(cc);
      //       std::cout << l_s << std::endl;
      //     } else {
      //       str += "[" + l_selector(cc) + "]";
      //     }
      //   }
      std::cout << "[" << cc << "]" << std::endl;
      if (cc.include([](const std::string &substr) {
            return substr.find_first_of("Q:") == 0;
          })) {
        if (q_s == "") {
          q_s = q_selector(cc);
          std::cout << q_s << std::endl;
        } else {
          str += "[" + q_selector(cc) + "]";
        }
      }
      std::cout << "[" << cc << "]" << std::endl;
      if (cc.include([](const std::string &substr) {
            return substr.find_first_of("M:") == 0;
          })) {
        if (m_s == "") {
          m_s = m_selector(cc);
          std::cout << m_s << std::endl;
        } else {
          str += "[" + m_selector(cc) + "]";
        }
      }
      std::cout << "[" << cc << "]" << std::endl;
      if (cc.include([](const std::string &substr) {
            return substr.find_first_of("K:") == 0;
          })) {
        if (k_s == "") {
          k_s = k_selector(cc);
          std::cout << k_s << std::endl;
        } else {
          str += "[" + k_selector(cc) + "]";
        }
      }
      for (auto sel : sel_vec) {
        if (sel.type() == ELEM_TYPE::SYM_TYPE) {
          str += symbolization(sel.get<Symbol>());
          if (cc.include([](const std::string &substr) {
                return substr.find_first_of("NL:") == 0;
              })) {
            str += nl_selector(cc);
          }
        } else {
          trg_it++;
          if (trg_it == end_it) {
            std::cerr
                << "Error: Reached the terminate of the rule iterator"
                << std::endl;
            exit(1);
          }
          str += func(trg_it, end_it);
        }
      }
      if (cc.include(Conception(Prefices::SLR))) {
        str += ")";
      }
      return str;
    };
    auto it = std::begin(ur);
    symbol_string = func(it, std::end(ur));

    os << "X:" << no << std::endl
       << "T:" << title << std::endl
       << l_s << std::endl
       << q_s << std::endl
       << m_s << std::endl
       << k_s << std::endl
       << " " << symbol_string << std::endl;
    return os.str();
  }
};

#endif /* LILYPOND_H_ */
