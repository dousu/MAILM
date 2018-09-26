#ifndef LILYPOND_H_
#define LILYPOND_H_

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

class OutputMusic {
 public:
  static Semantics<Conception> intention;
  static Knowledge kb;
  static std::string output(UtteranceRules &ur) {
    std::cout << ur << std::endl;

    std::ostringstream os;
    std::string relative_s = "c'", key_s = "c", major_minor_s = "major", time_s = "4/4", symbol_string = "c4 e8 c4 d8 c8 c8";

    std::list<std::string> tmp_str;
    auto sel_vec = ur.string();
    std::transform(std::begin(sel_vec), std::end(sel_vec), std::back_inserter(tmp_str),
                   [](const SymbolElement &sel) { return sel.mono_to_s(); });
    symbol_string = std::accumulate(std::next(std::begin(tmp_str)), std::end(tmp_str), tmp_str.front(),
                                    [](std::string a, std::string b) { return a + " " + b; });

    os << "melody = "
       << "\\relative " << relative_s << " {" << std::endl
       << Prefices::DSP << "\\key " << key_s << " \\" << major_minor_s << std::endl
       << Prefices::DSP << "\\time " << time_s << std::endl
       << Prefices::DSP << symbol_string << std::endl
       << "}" << std::endl
       << "\\score {" << std::endl
       << Prefices::DSP << "\\new Staff \\melody" << std::endl
       << Prefices::DSP << "\\layout { }" << std::endl
       << Prefices::DSP << "\\midi { }" << std::endl
       << "}";
    return os.str();
  }
};

#endif /* LILYPOND_H_ */
