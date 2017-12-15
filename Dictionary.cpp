/*
 * Dictionary.cpp
 *
 *  Created on: 2017/01/26
 *      Author: Hiroki Sudo
 */

#include "Dictionary.h"

std::map<int, std::string> Dictionary::symbol;
std::map<std::string, int> Dictionary::conv_symbol;

//Dictionary::Dictionary() {
//	// TODO Auto-generated constructor stub
//
//}
//
//Dictionary::~Dictionary() {
//	// TODO Auto-generated destructor stub
//}

void
Dictionary::load(std::string& file_path) {
  std::string line;
  std::ifstream source(file_path.c_str());

  //read test
  if (!source.good())
    throw "not found dictionary file";

  //read items
  std::vector<std::string> symbol_buffer;
  while (std::getline(source, line)) {
    std::string::size_type p = line.find("=");
    std::string key = line.substr(0, p);
    std::string value = line.substr(p + 1);

    boost::algorithm::trim_if(key,
      boost::algorithm::is_any_of("\r\n "));
    boost::algorithm::trim_if(value,
      boost::algorithm::is_any_of("\r\n "));

    if (key == "SYM") {
      boost::algorithm::split(symbol_buffer, value,
          boost::algorithm::is_any_of(","),
          boost::algorithm::token_compress_on);
    }
    else {
//			throw "unknown key";
    }
  }

  //store items
  int index;
  std::vector<std::string>::iterator it;

  it = symbol_buffer.begin();
  index = 0;
  while (it != symbol_buffer.end()) {
    if (symbol.find(index) == symbol.end()
        && conv_symbol.find(*it) == conv_symbol.end()
    ) {
      symbol.insert(
        std::map<int, std::string>::value_type(index, *it)
      );
      conv_symbol.insert(
        std::map<std::string, int>::value_type(*it, index)
      );
    }
    if (index + 1 <= index)
      throw "range over";

    index++;
    it++;
  }
}

Dictionary
Dictionary::copy(void) {
  return Dictionary();
}

