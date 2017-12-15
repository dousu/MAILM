/* 
 * File:   MAILM_main.h
 * Author: hiroki
 *
 * Created on October 28, 2013, 6:14 PM
 */

#ifndef MAILM_MAIN_H
#define	MAILM_MAIN_H

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <climits>
#include <cfloat>

#include <stdio.h>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/progress.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/filesystem.hpp>
//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/io.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

#include "Rule.h"
#include "Element.h"
#include "MT19937.h"
#include "Dictionary.h"
#include "XMLreader.h"
#include "LogBox.h"
#include "KnowledgeBase.h"
#include "Prefices.h"

void
load_input_data(std::vector<std::string>&, std::string&);

void
output_data(std::string, std::string);

#endif	/* MAILM_MAIN_H */

