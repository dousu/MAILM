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
	//!意味:internal
	MEAN_TYPE = 1,
	//!変数:internal
	VAR_TYPE = 2,
	//!シンボル:external
	SYM_TYPE = 3,
	//!カテゴリ付き変数:external
	CAT_TYPE = 4
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
		if (Dictionary::individual.find(obj) == Dictionary::individual.end())
		{
			return "*";
		}
		else
		{
			return Dictionary::individual[obj];
		}
	}
};

class Meaning;
class Category;
class Nonterminal;

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
			return Dictionary::symbol[obj];
		}
	}
};

class Meaning
{
	AMean base;
	std::vector<Meaning> means;
public:
	Meaning() : base(), means() {}
	Meaning(const Meaning &dst) : base(dst.base), means(dst.means) {}
	Meaning(std::initializer_list<AMean> list){
		base(AMean(*std::begin(list)));
		std::for_each(++std::begin(list), std::end(list), [&means](AMean &am){means.push_back(Meaning{{am}});});
	}
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
	Meaning & at(std::size_t i) const
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
}

class Nonterminal
{
	Category cat;
	Meaning means;

  public:
	Nonterminal(int cat_num, int obj_num) : cat(cat_num), obj(obj_num) {}
	Nonterminal(const Nonterminal &dst) : cat(dst.cat), obj(dst.obj) {}
	bool operator==(const Nonterminal &dst) const
	{
		return cat == dst.cat;
	}
	bool operator!=(const Nonterminal &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const Nonterminal &dst) const
	{
		return cat < dst.cat || (cat == dst.cat && obj < dst.obj);
	}
	int get_cat_id() const
	{
		return cat;
	}
	int get_obj_id() const
	{
		return obj;
	}
	std::string to_s() const
	{
		return Prefices::CAT + std::to_string(cat) + Prefices::DEL + Prefices::VAR + std::to_string(obj);
	}
};

class Element
{
	using ElementType = std::variant<std::monostate, Mean, Variable, Symbol, Nonterminal>;
	ElementType element;

  public:
	Element() : element() {}
	Element(const Element &other) : element(other.element) {}
	Element(const Mean &other) : element(other) {}
	Element(const Variable &other) : element(other) {}
	Element(const Symbol &other) : element(other) {}
	Element(const Nonterminal &other) : element(other) {}
	constexpr std::size_t type() const { return element.index(); }

	template <typename T>
	const T &get() const { return std::get<T>(element); }

	Element &operator=(const Element &dst)
	{
		element = dst.element;
		return *this;
	}
	Element &operator=(const Mean &dst)
	{
		element = dst;
		return *this;
	}
	Element &operator=(const Variable &dst)
	{
		element = dst;
		return *this;
	}
	Element &operator=(const Symbol &dst)
	{
		element = dst;
		return *this;
	}
	Element &operator=(const Nonterminal &dst)
	{
		element = dst;
		return *this;
	}
	bool operator==(const Element &dst) const
	{
		return type() == dst.type() && element == dst.element;
	}
	bool operator!=(const Element &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const Element &dst) const
	{
		return type() < dst.type() || (type() == dst.type() && element < dst.element);
	}
	std::string to_s() const
	{
		std::string str("");
		switch (type())
		{
		case ELEM_TYPE::MEAN_TYPE:
			str = Mean(std::get<ELEM_TYPE::MEAN_TYPE>(element)).to_s();
			break;
		case ELEM_TYPE::VAR_TYPE:
			str = Variable(std::get<ELEM_TYPE::VAR_TYPE>(element)).to_s();
			break;
		case ELEM_TYPE::SYM_TYPE:
			str = Symbol(std::get<ELEM_TYPE::SYM_TYPE>(element)).to_s();
			break;
		case ELEM_TYPE::CAT_TYPE:
			str = Nonterminal(std::get<ELEM_TYPE::CAT_TYPE>(element)).to_s();
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
	//コンストラクタ
	Conception();

	//デストラクタ
	//virtual ~Conception();


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
