/*
 * Element.h
 *
 *  Created on: 2016/11/22
 *      Author: Hiroki Sudo
 */

#ifndef ELEMENT_H_
#define ELEMENT_H_
#include <iostream>
#include <map>
#include <string>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <iterator>

#include "Dictionary.h"
#include "Prefices.h"
#include "XMLreader.h"

#include <variant>
#include <cstddef>


/*!
 * Elementクラスが取るタイプのインデックスを定義しています。
 */
namespace ELEM_TYPE
{
enum Type
{
	//!シンボル:external
	SYM_TYPE = 1,
	//!カテゴリ付き変数:external
	NT_TYPE = 2
};
}

//型
/*!
 * 内部言語と外部言語の要素を表すクラスです。
 * メンバ変数は全て数値で扱われ、インデックスで表現されています。
 * このインデックスは静的メンバ変数のDictionaryクラスインスタンスに依存します。
 */

class AMean
{
	int obj;

  public:
	AMean(int num) : obj(num) {}
	AMean(const AMean &dst) : obj(dst.obj) {}
	bool operator==(const AMean &dst) const
	{
		return obj == dst.obj;
	}
	bool operator!=(const AMean &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const AMean &dst) const
	{
		return obj < dst.obj;
	}
	int get_obj_id() const
	{
		return obj;
	}
	std::string to_s() const
	{
		if (Dictionary::symbol.find(obj) == Dictionary::symbol.end())
		{
			return "*";
		}
		else
		{
			return Dictionary::symbol[obj];
		}
	}
};

class Variable
{
	int obj;

  public:
	Variable(int var_num) : obj(var_num) {}
	Variable(const Variable &dst) : obj(dst.obj) {}
	bool operator==(const Variable &dst) const
	{
		return obj == dst.obj;
	}
	bool operator!=(const Variable &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const Variable &dst) const
	{
		return obj < dst.obj;
	}
	int get_obj_id() const
	{
		return obj;
	}
	std::string to_s() const
	{
		return Prefices::VAR + std::to_string(obj);
	}
};

class Category
{
	int obj;

  public:
	Category(int cat_num) : obj(cat_num) {}
	Category(const Category &dst) : obj(dst.obj) {}
	bool operator==(const Category &dst) const
	{
		return obj == dst.obj;
	}
	bool operator!=(const Category &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const Category &dst) const
	{
		return obj < dst.obj;
	}
	int get_obj_id() const
	{
		return obj;
	}
	std::string to_s() const
	{
		return Prefices::CAT + std::to_string(obj);
	}
};

class Symbol
{
	int obj;

  public:
	Symbol(int num) : obj(num) {}
	Symbol(const Symbol &dst) : obj(dst.obj) {}
	bool operator==(const Symbol &dst) const
	{
		return obj == dst.obj;
	}
	bool operator!=(const Symbol &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const Symbol &dst) const
	{
		return obj < dst.obj;
	}
	std::string to_s() const
	{
		if (Dictionary::symbol.find(obj) == Dictionary::symbol.end())
		{
			return "*";
		}
		else
		{
			return "[" + XMLreader::conv_alias[Dictionary::symbol[obj]] + "]";
		}
	}
};

class Meaning
{
	using MeaningType = std::variant<std::monostate, Meaning, Variable>;
	AMean base;
	std::vector<MeaningType> means;
public:
	Meaning(const Meaning &dst) : base(dst.base), means(dst.means) {}
	Meaning(const AMean &m) : base(m), means() {}
	Meaning(const AMean &m, const std::vector<MeaningType> &dst) : base(m), means(dst) {}
	bool operator==(const Meaning &dst) const
	{
		return base == dst.base && means == dst.means;
	}
	bool operator!=(const Meaning &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const Meaning &dst) const
	{
		return base < dst.base || (base == dst.base && means < dst.means);
	}
	const MeaningType & at(std::size_t i) const
	{
		if(i == 0){
			std::cerr << "irregular index" << std::endl;
			exit(1);
		}else{
			return means.at(i-1);
		}
	}
	const AMean & get_base() const
	{
		return base;
	}
	const std::vector<MeaningType> & get_vec() const
	{
		return means;
	} 
	std::string to_s() const
	{
		std::string str{""};
		if(means.size() == 0){
			return str;
		}else{
			if(means.size() == 1){
				return means.first.to_s();
			}else{
				str += means.first.to_s();
				means.erase(means.begin());
			}
			std::vector<std::string> buf;
			std::for_each(std::begin(means), std::end(means), [&buf](AMean m){buf.push_back(m.to_s());});
			std::ostringstream os;
			std::copy(std::begin(buf), std::end(buf), std::ostream_iterator<std::string>(os, ","));
			str += Prefices::LPRN + os.str();
			str.erase(str.end() - 1);
			str += Prefices::RPRN;
		}
		return str;
	}
};

class LeftNonterminal
{
	Category cat;
	Meaning means;

  public:
	LeftNonterminal(Category & c, Meaning & m) : cat(c), means(m) {}
	LeftNonterminal(const LeftNonterminal &dst) : cat(dst.cat), means(dst.means) {}
	bool operator==(const LeftNonterminal &dst) const
	{
		return cat == dst.cat && means == dst.means;
	}
	bool operator!=(const LeftNonterminal &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const LeftNonterminal &dst) const
	{
		return cat < dst.cat || (cat == dst.cat && means < dst.means);
	}
	const Category & get_cat() const
	{
		return cat;
	}
	const std::vector<MeaningType> & get_means() const
	{
		return means;
	}
	std::string to_s() const
	{
		return cat.to_s() + Prefices::DEL + means.to_s();
	}
};
class RightNonterminal
{
	Category cat;
	Variable var;

  public:
	bool operator==(const RightNonterminal &dst) const
	{
		return cat == dst.cat && var == dst.var;
	}
	bool operator!=(const RightNonterminal &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const RightNonterminal &dst) const
	{
		return cat < dst.cat || (cat == dst.cat && var < dst.var);
	}
	const Category & get_cat() const
	{
		return cat;
	}
	const Variable & get_var() const
	{
		return var;
	}
	std::string to_s() const
	{
		return cat.to_s() + Prefices::DEL + means.to_s();
	}
};
class Element
{
	using ElementType = std::variant<std::monostate, Symbol, RightNonterminal>;
	ElementType element;

  public:
	Element() : element() {}
	Element(const Element &other) : element(other.element) {}
	Element(const Symbol &other) : element(other) {}
	Element(const RightNonterminal &other) : element(other) {}
	constexpr std::size_t type() const { return element.index(); }

	template <typename T>
	const T &get() const { return std::get<T>(element); }

	Element &operator=(const Element &dst)
	{
		element = dst.element;
		return *this;
	}
	Element &operator=(const Symbol &dst)
	{
		element = dst;
		return *this;
	}
	Element &operator=(const RightNonterminal &dst)
	{
		element = dst;
		return *this;
	}
	bool operator==(const Element &dst) const
	{
		return element == dst.element;
	}
	bool operator!=(const Element &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const Element &dst) const
	{
		return element < dst.element;
	}
	std::string to_s() const
	{
		std::string str("");
		switch (type())
		{
		case ELEM_TYPE::SYM_TYPE:
			str = Symbol(std::get<ELEM_TYPE::SYM_TYPE>(element)).to_s();
			break;
		case ELEM_TYPE::NT_TYPE:
			str = RightNonterminal(std::get<ELEM_TYPE::NT_TYPE>(element)).to_s();
			break;
		default:
			str = "*";
		}
		return str;
	}
};

class Conception {
public:
	std::set<std::string> factors;
	Conception();

	//operator
	//!等号。型が異なると偽を返します。
	// 型が等しい場合はインデックスが等しいか比べます。
	bool operator ==(const Conception& dst) const;
	//!等号の否定です
	bool operator !=(const Conception& dst) const;
	Conception operator + (Conception& dst);
	Conception operator - (Conception& dst);
	//!代入
	Conception& operator =(const Conception& dst);
	void diff(Conception& obj, Conception& res1, Conception& res2);
	void inter(Conception& obj, Conception& res);
	bool include(Conception& obj);

	bool empty();
	void clear();
	void add(std::string str);
	std::string to_s();
};

#endif /* Conception_H_ */
