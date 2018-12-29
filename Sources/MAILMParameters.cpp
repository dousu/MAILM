#include "MAILMParameters.h"

MAILMParameters::MAILMParameters() {
  // experiment parameters
  MAX_GENERATIONS = 10;
  LIMIT_UTTERANCE = 100;
  RANDOM_SEED = 101010;  //

  CONTROLS = 0x0;
  buzz_length = 3;

  // Execution Values
  UTTERANCES = 25;
  Generation_Counter = 0;  //

  MONO = false;
  LILYPOND = false;

  // system parameters
  LOGGING = false;
  // PROGRESS = false;
  ANALYZE = false;

  // file parameters
  // file prefix
  FILE_PREFIX = "MAILM";
  time_t now = std::time(nullptr);
  struct tm *stm = localtime(&now);
  char s[100];
  strftime(s, 100, "%Yy%mm%dd%Hh%Mm%Ss", stm);
  DATE_STR = std::string(s);

  // file extentions
  RESULT_EXT = "_result.rst";
  LOG_EXT = "_log.log";

  // path
  BASE_PATH = "./";
  RESULT_PATH = BASE_PATH + "Result/";

  ProgramOption spo;

  // file
  INPUT_FILE = BASE_PATH + "Sources/input.txt";
  ALIAS_FILE = BASE_PATH + "Sources/alias.data";
  DIC_XML_FILE = BASE_PATH + "Sources/dic_xml.data";
  XML_DIR = BASE_PATH + "XML";
  MONOXML_DIR = BASE_PATH + "monoXML";
  ABC_DIR = BASE_PATH + "abc";
  XML_EXT = ".xml";
  ABC_EXT = ".abc";
  FILE_PREFIX = "MAILM_";

  RESULT_FILE = RESULT_PATH + FILE_PREFIX + DATE_STR + RESULT_EXT;
  LOG_FILE = RESULT_PATH + FILE_PREFIX + DATE_STR + LOG_EXT;
  DICTIONARY_FILE = BASE_PATH + "Sources/data.dic";
}

MAILMParameters::~MAILMParameters() {
  // TODO Auto-generated destructor stub
}

void MAILMParameters::set_option(ProgramOption &po) {
  spo = po;

  if (po.count("prefix")) {
    FILE_PREFIX = po.get<std::string>("prefix");
  }

  if (po.count("path")) {
    BASE_PATH = (po.get<std::string>("path"));
  }

  if (po.count("random-seed")) {
    RANDOM_SEED = po.get<int>("random-seed");
  }

  if (po.count("generations")) {
    MAX_GENERATIONS = po.get<int>("generations");
  }

  if (po.count("utterances")) {
    UTTERANCES = po.get<int>("utterances");
  }

  if (po.count("utterancelimit")) {
    LIMIT_UTTERANCE = po.get<int>("utterancelimit");
  }

  if (po.count("mono")) {
    MONO = true;
    XML_DIR = MONOXML_DIR;
  }

  if (po.count("abc")) {
    ABC = true;
    MONO = true;
    XML_DIR = ABC_DIR;
    XML_EXT = ABC_EXT;
  }

  if (po.count("lilypond")) {
    LILYPOND = true;
  }

  if (po.count("analyze")) {
    ANALYZE = po.get<bool>("analyze");
  }

  if (po.count("dictionary")) {
    DICTIONARY_FILE = po.get<std::string>("dictionary");
  }

  if (po.count("keep-random-rule")) {
    CONTROLS |= Knowledge::USE_ADDITION_OF_RANDOM_WORD;
  }

  if (po.count("logging")) {
    LOGGING = po.get<bool>("logging");
  }
  RESULT_PATH = BASE_PATH + "Result/";
  if (ANALYZE && po.count("prefix"))
    RESULT_FILE = RESULT_PATH + Prefices::DEL + FILE_PREFIX +
                  DATE_STR + RESULT_EXT;
  if (LOGGING && po.count("prefix"))
    LOG_FILE = RESULT_PATH + Prefices::DEL + FILE_PREFIX + DATE_STR +
               LOG_EXT;
  if (po.count("dictionary") || po.count("prefix"))
    DICTIONARY_FILE = BASE_PATH + Prefices::DEL + DICTIONARY_FILE;
}

std::string MAILMParameters::to_s() {
  std::vector<std::string> bag;
  if (spo.count("prefix")) {
    bag.push_back("--prefix");
    bag.push_back(spo.get<std::string>("prefix"));
  }

  if (spo.count("path")) {
    bag.push_back("--path");
    bag.push_back(spo.get<std::string>("path"));
  }

  if (spo.count("random-seed")) {
    bag.push_back("--random-seed");
    bag.push_back(std::to_string(spo.get<int>("random-seed")));
  }

  if (spo.count("generations")) {
    bag.push_back("--generations");
    bag.push_back(std::to_string(spo.get<int>("generations")));
  }

  if (spo.count("utterances")) {
    bag.push_back("--utterances");
    bag.push_back(std::to_string(spo.get<int>("utteranes")));
  }

  if (spo.count("utterancelimit")) {
    bag.push_back("--utterancelimit");
    bag.push_back(std::to_string(spo.get<int>("utteranelimit")));
  }

  if (spo.count("mono")) {
    bag.push_back("--mono");
    bag.push_back(std::to_string(spo.get<bool>("mono")));
  }

  if (spo.count("abc")) {
    bag.push_back("--abc");
    bag.push_back(std::to_string(spo.get<bool>("abc")));
  }

  if (spo.count("lilypond")) {
    bag.push_back("--lilypond");
    bag.push_back(std::to_string(spo.get<bool>("lilypond")));
  }

  if (spo.count("analyze")) {
    bag.push_back("--analyze");
    bag.push_back(std::to_string(spo.get<bool>("analyze")));
  }

  if (spo.count("dictionary")) {
    bag.push_back("--dictionary");
    bag.push_back(spo.get<std::string>("dictionary"));
  }

  if (spo.count("keep-random-rule")) {
    bag.push_back("--keep-random-rule");
    bag.push_back(std::to_string(spo.get<bool>("keep-random-rule")));
  }

  if (spo.count("logging")) {
    bag.push_back("--logging");
    bag.push_back(std::to_string(spo.get<bool>("logging")));
  }
  std::ostringstream os;
  std::copy(std::begin(bag), std::end(bag),
            std::ostream_iterator<std::string>(os, " "));
  return os.str();
}

std::string MAILMParameters::to_all_s(void) {
  std::ostringstream ss;
  //  int MAX_GENERATIONS;
  ss << "MAX_GENERATIONS = " << MAX_GENERATIONS << std::endl;
  //  double PER_UTTERANCES; //
  ss << "PER_UTTERANCES = " << PER_UTTERANCES << std::endl;
  //  int RANDOM_SEED;       //
  ss << "RANDOM_SEED = " << RANDOM_SEED << std::endl;
  //  uint32_t CONTROLS;
  ss << "CONTROLS = " << CONTROLS << std::endl;
  //  int buzz_length;
  ss << "buzz_length = " << buzz_length << std::endl;
  //  int UTTERANCES;
  ss << "UTTERANCES = " << UTTERANCES << std::endl;
  //  uint32_t Generation_Counter; //
  ss << "Generation_Counter = " << Generation_Counter << std::endl;
  //  bool LOGGING;
  ss << "LOGGING = " << std::boolalpha << LOGGING << std::noboolalpha
     << std::endl;
  //  bool PROGRESS;
  // ss << "PROGRESS = " << PROGRESS << std::endl;
  //  bool MONO;
  ss << "MONO = " << std::boolalpha << MONO << std::noboolalpha
     << std::endl;
  //  bool ABC;
  ss << "ABC = " << std::boolalpha << ABC << std::noboolalpha
     << std::endl;
  //  bool LILYPOND;
  ss << "LILYPOND = " << std::boolalpha << LILYPOND
     << std::noboolalpha << std::endl;
  //  bool ANALYZE;
  ss << "ANALYZE = " << std::boolalpha << ANALYZE << std::noboolalpha
     << std::endl;
  //  std::string DICTIONARY_FILE;
  ss << "DICTIONARY_FILE = " << DICTIONARY_FILE << std::endl;
  //  std::string FILE_PREFIX;
  ss << "FILE_PREFIX = " << FILE_PREFIX << std::endl;
  //  std::string DATE_STR;
  ss << "DATE_STR = " << DATE_STR << std::endl;
  //  std::string RESULT_EXT;
  ss << "RESULT_EXT = " << RESULT_EXT << std::endl;
  //  std::string LOG_EXT;
  ss << "LOG_EXT = " << LOG_EXT << std::endl;
  //  std::string BASE_PATH;
  ss << "BASE_PATH = " << BASE_PATH << std::endl;
  //  std::string LOG_FILE;
  ss << "LOG_FILE = " << LOG_FILE << std::endl;
  //  std::string RESULT_FILE;
  ss << "RESULT_FILE = " << RESULT_FILE << std::endl;
  // std::string INPUT_FILE;
  ss << "INPUT_FILE = " << INPUT_FILE << std::endl;
  // std::string ALIAS_FILE;
  ss << "ALIAS_FILE = " << ALIAS_FILE << std::endl;
  // std::string DIC_XML_FILE;
  ss << "DIC_XML_FILE = " << DIC_XML_FILE << std::endl;
  // std::string XML_DIR;
  ss << "XML_DIR = " << XML_DIR << std::endl;
  // std::string XML_EXT;
  ss << "XML_EXT = " << XML_EXT << std::endl;

  return ss.str();
}