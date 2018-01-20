/*
 * MAILMParameters.cpp
 *
 *  Created on: 2017/10/25
 *      Author: hiroki
 */

#include "MAILMParameters.h"

MAILMParameters::MAILMParameters() {
	INPUT_FILE = "./input.txt";
	ALIAS_FILE = "./alias.data";
	DIC_XML_FILE = "./dic_xml.data";
	XML_DIR = "../XML";
	XML_EXT = ".xml";
	FILE_PREFIX = "MAILM_";

	SAVE_FILE = (FILE_PREFIX + DATE_STR + STATE_EXT);
	RESULT_FILE = (FILE_PREFIX + DATE_STR + RESULT_EXT);
	RESUME_FILE = (FILE_PREFIX + DATE_STR + STATE_EXT);
	LOG_FILE = (FILE_PREFIX + DATE_STR + LOG_EXT);
	DICTIONARY_FILE = "./data.dic";
	BASE_PATH = "../RESULT/";
}

MAILMParameters::~MAILMParameters() {
	// TODO Auto-generated destructor stub
}

void
MAILMParameters::set_option(boost::program_options::variables_map& vm) {
	Parameters::set_option(vm);

	//if (vm.count("linked-matrix")) {
	//  LINKED_MSTRIX = true;
	//  LINKED_MSTRIX_PATH = vm["linked-matrix"].as<std::string>();
	//}
	if (vm.count("input-file")) {
		INPUT_FILE = vm["input-file"].as<std::string>();
	}

	if (vm.count("alias-file")) {
		ALIAS_FILE = vm["alias-file"].as<std::string>();
	}

	if (vm.count("dic-xml-file")) {
		DIC_XML_FILE = vm["dic-xml-file"].as<std::string>();
	}

	if (vm.count("xml-dir")) {
		XML_DIR = vm["xml-dir"].as<std::string>();
	}

	if (vm.count("xml-ext")) {
		XML_EXT = vm["xml-ext"].as<std::string>();
	}

	//必ずprefixの変更後に行うこと
	SAVE_FILE = (FILE_PREFIX + DATE_STR + "_" + boost::lexical_cast<std::string>(RANDOM_SEED) + STATE_EXT);
	RESULT_FILE = (FILE_PREFIX + DATE_STR + "_" + boost::lexical_cast<std::string>(RANDOM_SEED) + RESULT_EXT);
	RESUME_FILE = (FILE_PREFIX + DATE_STR + "_" + boost::lexical_cast<std::string>(RANDOM_SEED) + STATE_EXT);
	LOG_FILE = (FILE_PREFIX + DATE_STR + "_" + boost::lexical_cast<std::string>(RANDOM_SEED) + LOG_EXT);
}

std::string
MAILMParameters::to_s(void) {
	std::string param1, param2;
	std::vector<std::string> bag;
	param1 = Parameters::to_s();
	bag.push_back(param1);

	//if (svm.count("contact")) {
	//  bag.push_back("--contact");
	//  bag.push_back(
	//      boost::lexical_cast<std::string>(svm["contact"].as<double>()));
	//}

	//if (svm.count("neighbor")) {
	//  bag.push_back("--contact");
	//  bag.push_back(
	//      boost::lexical_cast<std::string>(svm["neighbor"].as<double>()));
	//}

	//if (svm.count("convert")) {
	//  bag.push_back("--convert");
	//  std::vector<std::string> buf;
	//  buf = svm["convert"].as<std::vector<std::string> >();

	//  bag.push_back(buf[0]);
	//  bag.push_back(buf[1]);
	//}

	//if (svm.count("igraph")) {
	//  bag.push_back("--igraph");
	//  bag.push_back(svm["igraph"].as<std::string>());
	//}

	//if (svm.count("agents")) {
	//  bag.push_back("--agents");
	//  bag.push_back(boost::lexical_cast<std::string>(svm["agents"].as<int>()));
	//}

	//if (svm.count("linked-matrix")) {
	//  bag.push_back("--linked-matrix");
	//  bag.push_back(svm["linked-matrix"].as<std::string>());
	//}
	if (svm.count("delete-rule-length")) {
		bag.push_back("--delete-rule-length");
		bag.push_back(
			boost::lexical_cast<std::string>(svm["delete-rule-length"].as<int>()));
	}

	//if (svm.count("threads")) {
	//  bag.push_back(boost::lexical_cast<std::string>(svm["threads"].as<int>()));
	//}

	if (svm.count("minimum-utter")) {
		bag.push_back("--minimum-utter");
	}
	if (svm.count("index")) {
		bag.push_back("--index");
	}

	if (svm.count("interspace-analysis")) {
		bag.push_back("--interspace-analysis " + boost::lexical_cast<std::string>(svm["interspace-analysis"].as<int>()));
	}
	if (svm.count("interspace-logging")) {
		bag.push_back("--interspace-logging " + boost::lexical_cast<std::string>(svm["interspace-logging"].as<int>()));
	}

	if (svm.count("max-listening-length")) {
		bag.push_back("--max-listening-length " + boost::lexical_cast<std::string>(svm["max-listening-length"].as<int>()));
	}

	if (svm.count("multiple-meanings")) {
		bag.push_back("--multiple-meanings " + boost::lexical_cast<std::string>(svm["multiple-meanings"].as<int>()));
	}

	if (svm.count("term")) {
		bag.push_back("--term " + boost::lexical_cast<std::string>(svm["term"].as<double>()));
	}

	if (svm.count("window")) {
		bag.push_back("--window " + boost::lexical_cast<std::string>(svm["window"].as<int>()));
	}

	if (svm.count("symmetry")) {
		bag.push_back("--symmetry ");
	}

	if (svm.count("mutual-exclusivity")) {
		bag.push_back("--mutual-exclusivity ");
	}

	if (svm.count("exception")) {
		bag.push_back("--exception ");
	}

	if (svm.count("omission-A")) {
		bag.push_back("--omission-A ");
	}

	if (svm.count("omission-B")) {
		bag.push_back("--omission-B ");
	}

	if (svm.count("omission-C")) {
		bag.push_back("--omission-C ");
	}

	if (svm.count("omission-D")) {
		bag.push_back("--omission-D ");
	}

	if (svm.count("accuracy-meaning")) {
		bag.push_back("--accuracy-meaning ");
	}

	//if (svm.count("once-parent-test")) {
	//  bag.push_back("--once-parent-test");
	//}


	return boost::algorithm::join(bag, " ");
}
