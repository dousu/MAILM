/*
 * LogBox.cpp
 *
 *  Created on: 2016/12/20
 *      Author: Hiroki Sudo
 */

#include "LogBox.h"

/* statics */

std::vector<std::string> LogBox::log;
std::string LogBox::log_file("");
int LogBox::log_size = 500;
int LogBox::threads = 1;

/*Constructor & destructor*/
LogBox::LogBox() {

}

LogBox::~LogBox() {
  refresh_log();
  //writer->stop();
}

void
LogBox::push_log(std::string msg) {
  //if(log.max_size() > log.size()){
  if (log_size > log.size()) {
    log.push_back(msg);
  }
  else {
    refresh_log();
    log.push_back(msg);
  }
}

void
LogBox::pop_log(int nth) {
  if (log.size() < nth) {
    std::string str;
    str = "**** CANCELED UPPER MESSAGE ****";
    log.push_back(str);
    return;
  }
  else {
    for (int i = 0; i < nth; i++)
      log.pop_back();
  }
}

void
LogBox::set_filepath(std::string file) {
  log_file = file;
  //writer = boost::shared_ptr<LogWriter>(new LogWriter(file));
  //boost::thread th(writer.get());
  //th.detach();
}

void
LogBox::refresh_log(void) {
  if (log.size() > 0) {
    std::ofstream ofs(log_file.c_str(), std::ios::app);
    for (int i = 0; i < log.size(); i++) {
      ofs << log[i] << std::endl;
    }
    log.clear();
    std::vector<std::string>(log).swap(log);
  }
}

