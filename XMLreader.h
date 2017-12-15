/*
 * Dictionary.h
 *
 *  Created on: 2017/01/24
 *      Author: Hiroki Sudo
 */

#ifndef XMLREADER_H_
#define XMLREADER_H_
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/filesystem.hpp>
//#include <boost/filesystem/path.hpp>
//#include <boost/filesystem/fstream.hpp>
//#include <boost/iostreams/operations.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "Prefices.h"

struct XMLdata
{
	std::map<std::string, std::string> alias;
	std::map<std::string, std::string> conv_alias;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, unsigned int ver){
    	ar & alias;
    	ar & conv_alias;
    }
};

struct DicXMLdata
{
	std::map<std::string, std::string> dic_xml;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, unsigned int ver){
    	ar & dic_xml;
    }
};

// struct RulesXMLdata
// {
// 	std::map<int, std::vector<int> > meaning_rules;

// private:
//     friend class boost::serialization::access;
//     template<class Archive>
//     void serialize(Archive& ar, unsigned int ver){
//     	ar & meaning_rules;
//     }
// };

class XMLreader {
public:

	static std::map<std::string, std::string> alias;
    //コンマのあるリストからID number表示へ（(C,D,E) => C1）

	static std::map<std::string, std::string> conv_alias;

	static std::map<std::string, std::string> dic_xml;

	static std::vector<std::string> symbol_list;

	//serialization未設定
	static std::map<int, std::vector<std::string> > i_rules_str;
	static std::map<int, std::vector<int> > beat_map;
	static std::map<int, std::string> labeling;
	static std::map<int, std::vector<std::string> > core_meaning;

	// static std::map<int,std::vector<int> > meaning_rules;

	static int index_count;

	static int variable_count;

	static int category_count;

	static int symbol_count;

	//input.txt, data.dic,
    //read.dicが揃っていれば書き出しではなく，
    // read.dicの読み込みをする．処理時間の短縮
	//read.dicさえあれば，aliasとconv_aliasは埋まる．
	static void make_init_data(std::vector<std::string>& file_paths,
                        std::string& input_data_path,
                        std::string& read_dic_path,
                        std::string& alias_file_path,
                        std::string& dic_xml_path);//,
                        //std::string& meaning_rules_path);

	//XMLをパースしてルールで読み込める形式で返す．
	static void load(std::string,std::vector<std::string>&,int);

	static void get_sample(std::string&,
            std::vector<std::pair<std::string, std::string > >&);

	static void get_dic_xml(std::map<std::string, std::string>&);

	static void alias_load(std::string&);

	static void dic_xml_load(std::string&);

	// static void meaning_rules_load(std::string&);

	static XMLreader copy(void);
};

#endif /* XMLREADER_H_ */
