/*
 * File:   MAILMAgent.cpp
 * Author: hiroki
 *
 * Created on October 25, 2013, 4:08 PM
 */


#include "MAILMAgent.h"


MAILMAgent::MAILMAgent() {
}

MAILMAgent::~MAILMAgent() {
}

MAILMAgent&
MAILMAgent::operator=(const MAILMAgent& dst) {
	kb = dst.kb;
	generation_index = dst.generation_index;
	LOGGING_FLAG = dst.LOGGING_FLAG;
	return *this;
}

MAILMAgent
MAILMAgent::make_child(void) {
	MAILMAgent child;
	child.generation_index = generation_index + 1;
	return child;
}

MAILMAgent&
MAILMAgent::grow(std::vector<Rule> meanings) {

	kb.consolidate();
	kb.gen_cat = XMLreader::index_count;
	kb.gen_ind = XMLreader::category_count;

	return *this;
}

std::vector<Rule>
MAILMAgent::say(int beat_num, std::map<int, std::vector<std::string> >& mapping) {
	try {
		std::vector<Rule> res = kb.generate_score(beat_num, mapping);
		return res;
	}
	catch (...) {
		LogBox::refresh_log();
		throw;
	}
}

void
MAILMAgent::hear(std::vector<Rule>& rules, std::map<int, std::vector<std::string> >& mapping) {

	registration(mapping);
	kb.send_box(rules);
}

void MAILMAgent::registration(std::map<int, std::vector<std::string> >& core_meaning) {
	for (auto& p_i_str_v : core_meaning) {
		Element ind;
		ind.set_ind(p_i_str_v.first);
		Conception ss;
		for (auto& str : p_i_str_v.second) {
			ss.add(str);
		}
		kb.define(ind, ss);
	}
}

//utility 

std::string
MAILMAgent::tr_vector_Rule_to_string(std::vector<Rule> vector) {
	if (vector.size() != 0) {
		std::string res = "(";
		std::vector<Rule>::iterator rule_it = vector.begin();

		res = res + (*rule_it).to_s();
		rule_it++;
		for (; rule_it != vector.end(); rule_it++) {
			res = res + "," + (*rule_it).to_s();
		}
		return (res + ")");
	}
	else {
		return "no rules";
	}
}

std::string
MAILMAgent::tr_vector_Rule_to_string(std::vector<std::vector<Rule> > vector) {
	if (vector.size() != 0) {
		std::string res = "(";
		std::vector<std::vector<Rule> >::iterator rule_it = vector.begin();

		res = res + tr_vector_Rule_to_string(*rule_it);
		rule_it++;
		for (; rule_it != vector.end(); rule_it++) {
			res = res + "," + tr_vector_Rule_to_string(*rule_it);
		}
		return (res + ")");
	}
	else {
		return "no rules";
	}
}