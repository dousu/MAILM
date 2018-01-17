/*
 * Element.cpp
 *
 *  Created on: 2016/11/22
 *      Author: Hiroki Sudo
 */

#include "Element.h"


//static initialize
Dictionary Element::dictionary = Dictionary::copy();
XMLreader Element::reader = XMLreader::copy();


Element::Element(){
}

Element::Element(std::string str){
	//[C|V|I|S]:[obj]:[cat]:[ch.front()]
	std::vector<std::string> buf;

	// std::cout << "Element Constructor(str) :" << str << std::endl;

  	boost::algorithm::split(buf, str,
      	boost::algorithm::is_any_of(Prefices::CLN.c_str()),
  		boost::algorithm::token_compress_on);
  	if(buf[0] == "C"){
  		set_cat(std::stoi(buf[1]),std::stoi(buf[2]));
  	}else if(buf[0] == "V"){
  		set_var(std::stoi(buf[1]),std::stoi(buf[2]));
  	}else if(buf[0] == "I"){
  		set_ind(std::stoi(buf[1]));
  		set_ch(std::stoi(buf[3]));
  	}else if(buf[0] == "S"){
  		set_sym(std::stoi(buf[1]));
  	}else{
  		std::cerr << "Constructor Element(std::string) Error" << std::endl;
  		throw "Constructor Element(std::string) Error";
  	}
}

/*
 * Operators
 */
bool Element::operator!=(const Element& dst) const{
	return !(*this == dst);
}


bool Element::operator==(const Element& dst) const{
	//タイプ検査
	if(type != dst.type) return false;

	//同タイプでの比較
	switch(type){
	case ELEM_TYPE::CAT_TYPE://constant of meaning
		if(cat == dst.cat) return true;
		break;
	case ELEM_TYPE::VAR_TYPE://発話組み立て時注意
		if(cat == dst.cat) return true;
		break;
	case ELEM_TYPE::IND_TYPE:
		if(obj == dst.obj) return true;
		break;
	case ELEM_TYPE::SYM_TYPE:
		if(obj == dst.obj) return true;
		break;
	default:
		std::cout << type << ":"
		 << "unknown type" << std::endl;
		throw "unknown type";
		break;
	}

	return false;
}

bool Element::operator<(const Element& dst) const{
	if(type < dst.type)
		return true;
	else if(type > dst.type)
		return false;

	//同タイプでの比較
	switch(type){
	case ELEM_TYPE::CAT_TYPE:
		if(cat < dst.cat)
			return true;
		else if(cat > dst.cat)
			return false;

		if(obj < dst.obj)
			return true;
		else if(obj > dst.obj)
			return false;

		break;
	case ELEM_TYPE::VAR_TYPE:
		if(obj < dst.obj)
			return true;
		else if(obj > dst.obj)
			return false;

		break;
	case ELEM_TYPE::IND_TYPE:
		if(obj < dst.obj)
			return true;
		else if(obj > dst.obj)
			return false;

		break;
	case ELEM_TYPE::SYM_TYPE:
		if(obj < dst.obj)
			return true;
		else if(obj > dst.obj)
			return false;

		break;
	default:
		std::cout << type << ":"
		 << "unknown type" << std::endl;
		throw "unknown type";
		break;
	}

	return false;

}

Element& Element::operator=(const Element& dst){
	type = dst.type;
	obj = dst.obj;
	cat = dst.cat;
	ch = dst.ch;
	sent_type = dst.sent_type;

	return *this;
}

std::string Element::to_s(void){
	switch(type){

	case ELEM_TYPE::CAT_TYPE :
		if(sent_type){
			return Prefices::SEN +// Prefices::CLN +
					boost::lexical_cast<std::string>(cat) +
					Prefices::DEL +
					Prefices::VAR +// Prefices::CLN +
					boost::lexical_cast<std::string>(obj);
		}else{
			return Prefices::CAT +// Prefices::CLN +
					boost::lexical_cast<std::string>(cat) +
					Prefices::DEL +
					Prefices::VAR +// Prefices::CLN +
					boost::lexical_cast<std::string>(obj);
		}
		break;

	case ELEM_TYPE::IND_TYPE :
		return Prefices::IND +// Prefices::CLN +
				boost::lexical_cast<std::string>(obj);
		break;

	case ELEM_TYPE::SYM_TYPE :
		return "[" +
			reader.conv_alias[dictionary.symbol[obj]] +
			"]";
		// return dictionary.symbol[obj];
		break;

	case ELEM_TYPE::VAR_TYPE :
		// if(sent_type){
		// 	return Prefices::SEN + Prefices::CLN +
		// 			boost::lexical_cast<std::string>(cat) +
		// 			Prefices::DEL +
		// 			Prefices::VAR + Prefices::CLN +
		// 			boost::lexical_cast<std::string>(obj);
		// }else{
		// 	return Prefices::CAT + Prefices::CLN +
		// 			boost::lexical_cast<std::string>(cat) +
		// 			Prefices::DEL +
		// 			Prefices::VAR + Prefices::CLN +
		// 			boost::lexical_cast<std::string>(obj);
		// }
		return Prefices::VAR +// Prefices::CLN +
		 		boost::lexical_cast<std::string>(obj);
		break;

	default:
		std::cerr <<
			"unknown type: Element::to_s()" <<
			std::endl;
		throw "unknown type of Element";
	}
}

Element& Element::set_cat(int var, int cat){
	return set(ELEM_TYPE::CAT_TYPE, var, cat, false);
}

Element& Element::set_cat(int var, int cat,
	bool sent){
	return set(ELEM_TYPE::CAT_TYPE, var, cat, sent);
}

Element& Element::set_var(int var, int cat){
	return set(ELEM_TYPE::VAR_TYPE, var, cat, false);
}

Element& Element::set_ind(int id){
	return set(ELEM_TYPE::IND_TYPE, id, 0, false);
}

Element& Element::set_sym(int id){
	if(
		dictionary.symbol.find(id)
		==
		dictionary.symbol.end())
		throw "range over for symbol";
	return set(ELEM_TYPE::SYM_TYPE, id, 0, false);
}

void Element::set_ch(int chunk){
	if(ch.front() == 1){
		ch.front() = chunk;
	}else{
		ch.insert(ch.begin(),chunk);
	}
}

Element& Element::set(int dtype, int dobj, int dcat,
	bool sent){
	type = dtype;
	obj  = dobj;
	cat  = dcat;
	ch.push_back(1);
	sent_type = sent;
	return *this;
}

bool Element::is_var(void) const {
	if(type == ELEM_TYPE::VAR_TYPE) return true;
	else return false;
}
bool Element::is_cat(void) const {
	if(type == ELEM_TYPE::CAT_TYPE) return true;
	else return false;
}
bool Element::is_ind(void) const {
	if(type == ELEM_TYPE::IND_TYPE) return true;
	else return false;
}
bool Element::is_sym(void) const {
	if(type == ELEM_TYPE::SYM_TYPE) return true;
	else return false;
}

Conception::Conception(){
}

/*
 * Operators
 */
bool Conception::operator != (const Conception& dst) const{
	return !(*this == dst);
}

//集合が空じゃなくて各要素が等しいならtrue
bool Conception::operator == (const Conception& dst) const{
	return factors == dst.factors;
}


Conception& Conception::operator = (const Conception& dst){
	factors = dst.factors;
	return *this;
}

Conception Conception::operator + (Conception& dst){
	Conception trg;
	trg.factors = factors;
	trg.factors.insert(dst.factors.begin(),dst.factors.end());
	return trg;
}

Conception Conception::operator - (Conception& dst){
	Conception trg;
	std::set_difference(
		factors.begin(),factors.end(),
		dst.factors.begin(),dst.factors.end(),
		std::inserter(trg.factors,trg.factors.end())
	);
	return trg;
}

void Conception::diff(Conception& obj, Conception& res1, Conception& res2){
	res1.clear();
	res2.clear();
	std::set_difference(
		factors.begin(),factors.end(),
		obj.factors.begin(),obj.factors.end(),
		std::inserter(res1.factors,res1.factors.end())
	);
	std::set_difference(
		obj.factors.begin(),obj.factors.end(),
		factors.begin(),factors.end(),
		std::inserter(res2.factors,res2.factors.end())
	);
}

void Conception::inter(Conception& obj, Conception& res){
	res.clear();
	std::set_intersection(
		factors.begin(),factors.end(),
		obj.factors.begin(),obj.factors.end(),
		std::inserter(res.factors,res.factors.end())
	);
}

bool Conception::include(Conception& obj){
	return std::includes(factors.begin(),factors.end(),obj.factors.begin(),obj.factors.end());
}

bool Conception::empty(){
	return factors.size() == 0 || factors.count("") == factors.size();
}

void Conception::clear(){
	factors.clear();
}

void Conception::add(std::string str){
	factors.insert(str);
}

std::string
Conception::to_s(){
	std::string str;
	std::set<std::string>::iterator f_it = factors.begin();
	for(;f_it != factors.end();f_it++){
		str += (*f_it) + std::string(" ");
	}
	str.pop_back();
	return str;
}