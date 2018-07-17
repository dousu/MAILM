#include "Element.h"

std::ostream & operator<<(std::ostream & out, AMean & obj){
	out << obj.to_s();
	return out;
}
std::ostream & operator<<(std::ostream & out, Variable & obj){
	out << obj.to_s();
	return out;
}
std::ostream & operator<<(std::ostream & out, Category & obj){
	out << obj.to_s();
	return out;
}
std::ostream & operator<<(std::ostream & out, Symbol & obj){
	out << obj.to_s();
	return out;
}
std::ostream & operator<<(std::ostream & out, Meaning & obj){
	out << obj.to_s();
	return out;
}
std::ostream & operator<<(std::ostream & out, LeftNonterminal & obj){
	out << obj.to_s();
	return out;
}
std::ostream & operator<<(std::ostream & out, RightNonterminal & obj){
	out << obj.to_s();
	return out;
}
std::ostream & operator<<(std::ostream & out, Element & obj){
	out << obj.to_s();
	return out;
}

Conception::Conception() {
}

/*
 * Operators
 */
bool Conception::operator != (const Conception& dst) const {
	return !(*this == dst);
}

//集合が空じゃなくて各要素が等しいならtrue
bool Conception::operator == (const Conception& dst) const {
	return factors == dst.factors;
}


Conception& Conception::operator = (const Conception& dst) {
	factors = dst.factors;
	return *this;
}

Conception Conception::operator + (Conception& dst) {
	Conception trg;
	trg.factors = factors;
	trg.factors.insert(dst.factors.begin(), dst.factors.end());
	return trg;
}

Conception Conception::operator - (Conception& dst) {
	Conception trg;
	std::set_difference(
		factors.begin(), factors.end(),
		dst.factors.begin(), dst.factors.end(),
		std::inserter(trg.factors, trg.factors.end())
	);
	return trg;
}

void Conception::diff(Conception& obj, Conception& res1, Conception& res2) {
	res1.clear();
	res2.clear();
	std::set_difference(
		factors.begin(), factors.end(),
		obj.factors.begin(), obj.factors.end(),
		std::inserter(res1.factors, res1.factors.end())
	);
	std::set_difference(
		obj.factors.begin(), obj.factors.end(),
		factors.begin(), factors.end(),
		std::inserter(res2.factors, res2.factors.end())
	);
}

void Conception::inter(Conception& obj, Conception& res) {
	res.clear();
	std::set_intersection(
		factors.begin(), factors.end(),
		obj.factors.begin(), obj.factors.end(),
		std::inserter(res.factors, res.factors.end())
	);
}

bool Conception::include(Conception& obj) {
	return std::includes(factors.begin(), factors.end(), obj.factors.begin(), obj.factors.end());
}

bool Conception::empty() {
	return factors.size() == 0 || factors.count("") == factors.size();
}

void Conception::clear() {
	factors.clear();
}

void Conception::add(std::string str) {
	factors.insert(str);
}

std::string
Conception::to_s() {
	std::string str;
	std::set<std::string>::iterator f_it = factors.begin();
	for (; f_it != factors.end(); f_it++) {
		str += (*f_it) + std::string(" ");
	}
	str.pop_back();
	return str;
}