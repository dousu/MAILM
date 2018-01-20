/*
 * MAILMParameters.h
 *
 *  Created on: 2017/10/25
 *      Author: hiroki
 */

#ifndef MAILMPARAMETERS_H_
#define MAILMARAMETERS_H_

#include <vector>
#include <string>
#include <fstream>
#include <boost/program_options.hpp>
 //#include <boost/system/config.hpp>
 //#include <boost/filesystem.hpp>
 //#include <boost/filesystem/path.hpp>
 //#include <boost/filesystem/fstream.hpp>

 //#include <boost/shared_ptr.hpp>

#include "Parameters.h"
#include "LogBox.h"



/*!
 * 実行時引数を解釈して、保持するクラス
 */
class MAILMParameters : public Parameters {
public:
	std::string INPUT_FILE;
	std::string ALIAS_FILE;
	std::string DIC_XML_FILE;
	std::string XML_DIR;
	std::string XML_EXT;

	MAILMParameters();
	virtual ~MAILMParameters();

	void set_option(boost::program_options::variables_map& vm);
	std::string to_s(void);
};

#endif /* MAILMPARAMETERS_H_ */
