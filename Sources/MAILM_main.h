#ifndef MAILM_MAIN_H
#define MAILM_MAIN_H

#include <cassert>
#include <cfloat>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

// #include <boost/algorithm/string.hpp>
// #include <boost/algorithm/string/trim.hpp>

#include "Agent.h"
#include "Dictionary.h"
#include "Element.h"
#include "Knowledge.h"
#include "LilypondOutput.h"
#include "LogBox.h"
#include "MAILMParameters.h"
#include "MT19937.h"
#include "Prefices.h"
#include "Reader.h"
#include "Rule.h"

void output_data(std::string, std::string);

#endif /* MAILM_MAIN_H */
