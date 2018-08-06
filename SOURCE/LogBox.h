#ifndef LOGBOX_H_
#define LOGBOX_H_

#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>

/*!
  * ロギングデータを保持するクラス
  * 特に意識する必要なし。
  */
class LogBox
{
public:
	static std::vector<std::string> log;
	static std::string log_file;
	static int log_size;
	static int threads;

	LogBox();
	virtual ~LogBox();

	static void push_log(std::string);
	static void pop_log(int nth = 1);
	static void refresh_log(void);

	static void set_filepath(std::string);
};

#endif /* LOGBOX_H_ */
