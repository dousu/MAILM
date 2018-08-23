#ifndef MAILM_MAIN_H
#define MAILM_MAIN_H

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
#include <filesystem>
#include <queue>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "Rule.h"
#include "Element.h"
#include "MT19937.h"
#include "Dictionary.h"
#include "XMLreader.h"
#include "LogBox.h"
#include "Knowledge.h"
#include "Prefices.h"
#include "MAILMParameters.h"
#include "Agent.h"

void output_data(std::string, std::string);

#endif /* MAILM_MAIN_H */
