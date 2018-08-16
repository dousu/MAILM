#include "Dictionary.h"

std::map<int, std::string> Dictionary::symbol;
std::map<std::string, int> Dictionary::conv_symbol;

Dictionary
Dictionary::copy(void)
{
	return Dictionary();
}