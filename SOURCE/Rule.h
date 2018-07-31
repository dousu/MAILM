#ifndef RULE_H_
#define RULE_H_

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <variant>

#include "Distance.hpp"
#include "Element.h"
#include "IndexFactory.h"
#include "Semantics.h"

class Rule
{
	//member
	LeftNonterminal internal;			 //内部
	std::vector<SymbolElement> external; //外部言語列

  public:
	//constructor
	Rule();
	// Rule(char *cstr);
	// Rule(std::string str);
	Rule(const LeftNonterminal &lt, const std::vector<SymbolElement> &ex);

	//!operator
	bool operator==(const Rule &dst) const;
	bool operator!=(Rule &dst) const;
	Rule &operator=(const Rule &dst);
	bool operator<(const Rule &dst) const;

	int size() const { return external.size(); };
	std::string to_s();
	bool is_noun(Semantics<Conception> &s) const;
	bool is_sentence(Semantics<Conception> &s) const;
	bool is_measure(Semantics<Conception> &s) const;
	void set_rule(LeftNonterminal &nt, std::vector<SymbolElement> &ex);
	LeftNonterminal &get_internal() { return internal; }
	const LeftNonterminal &get_internal() const { return internal; }
	std::vector<SymbolElement> &get_external() { return external; }
	const std::vector<SymbolElement> &get_external() const { return external; }
	friend std::ostream &operator<<(std::ostream &out, const Rule &obj);
};

#endif /* RULE_H_ */
