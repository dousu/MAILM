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

#include <variant>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <type_traits>

/*!
 * Elementクラスが取るタイプのインデックスを定義しています。
 */
namespace ELEM_TYPE
{
enum Type
{
	//!カテゴリ付き変数:external
	NT_TYPE = 0,
	//!シンボル:external
	SYM_TYPE
};
} // namespace ELEM_TYPE
namespace MEANING_TYPE
{
enum Type
{
	MEANING_TYPE = 0,
	VAR_TYPE
};
} // namespace MEANING_TYPE

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
	AMean() : obj(0) {}
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
	bool operator>(const AMean &dst) const
	{
		return obj > dst.obj;
	}
	bool operator<=(const AMean &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const AMean &dst) const
	{
		return !(*this < dst);
	}
	int get_obj_id() const
	{
		return obj;
	}
	std::string to_s() const
	{
		return Prefices::MES + std::to_string(obj);
	}
	friend std::ostream &operator<<(std::ostream &out, const AMean &obj);
};

class Variable
{
	int obj;

  public:
	Variable() : obj(0) {}
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
	bool operator>(const Variable &dst) const
	{
		return obj > dst.obj;
	}
	bool operator<=(const Variable &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const Variable &dst) const
	{
		return !(*this < dst);
	}
	int get_obj_id() const
	{
		return obj;
	}
	std::string to_s() const
	{
		return Prefices::VAR + std::to_string(obj);
	}
	friend std::ostream &operator<<(std::ostream &out, const Variable &obj);
};

class Category
{
	int obj;

  public:
	Category() : obj(0) {}
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
	bool operator>(const Category &dst) const
	{
		return obj > dst.obj;
	}
	bool operator<=(const Category &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const Category &dst) const
	{
		return !(*this < dst);
	}
	int get_obj_id() const
	{
		return obj;
	}
	std::string to_s() const
	{
		return Prefices::CAT + std::to_string(obj);
	}
	friend std::ostream &operator<<(std::ostream &out, const Category &obj);
};

class Symbol
{
	int obj;

  public:
	static std::map<std::string, std::string> conv_symbol;
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
	bool operator>(const Symbol &dst) const
	{
		return obj > dst.obj;
	}
	bool operator<=(const Symbol &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const Symbol &dst) const
	{
		return !(*this < dst);
	}
	std::string to_s() const
	{
		if (Dictionary::symbol.find(obj) == Dictionary::symbol.end())
		{
			return "*";
		}
		else
		{
			return "[" + conv_symbol[Dictionary::symbol[obj]] + "]";
		}
	}
	friend std::ostream &operator<<(std::ostream &out, const Symbol &obj);
};

class MeaningElement;

class Meaning
{
	AMean base;
	std::vector<MeaningElement> means;

  public:
	Meaning() : base(), means() {}
	Meaning(const Meaning &dst) : base(dst.base), means(dst.means) {}
	Meaning(const AMean &m) : base(m), means() {}
	Meaning(const AMean &m, const std::vector<MeaningElement> &dst) : base(m), means(dst) {}
	Meaning(const AMean &m, const std::vector<Meaning> &dst) : base(m), means()
	{
		std::for_each(std::begin(dst), std::end(dst), [&](auto &obj) { means.push_back(obj); });
	}
	Meaning(const AMean &m, const std::vector<Variable> &dst) : base(m), means()
	{
		std::for_each(std::begin(dst), std::end(dst), [&](auto &obj) { means.push_back(obj); });
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
	bool operator>(const Meaning &dst) const
	{
		return base > dst.base || (base == dst.base && means > dst.means);
	}
	bool operator<=(const Meaning &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const Meaning &dst) const
	{
		return !(*this < dst);
	}
	MeaningElement &at(std::size_t i)
	{
		if (i < 1)
		{
			std::cerr << "irregular index" << std::endl;
			exit(1);
		}
		return means.at(i - 1);
	}
	const MeaningElement &at(std::size_t i) const
	{
		if (i < 1)
		{
			std::cerr << "irregular index" << std::endl;
			exit(1);
		}
		return means.at(i - 1);
	}
	AMean &get_base()
	{
		return base;
	}
	const AMean &get_base() const
	{
		return base;
	}
	std::vector<MeaningElement> &get_followings()
	{
		return means;
	}
	const std::vector<MeaningElement> &get_followings() const
	{
		return means;
	}
	int get_size() const
	{
		return means.size() + 1;
	}
	Meaning removed(std::size_t n, std::size_t size) const
	{
		if (size > means.size() || n > 0)
		{
			std::cerr << "Cannot remove" << std::endl;
			exit(1);
		}
		std::vector<MeaningElement> tmp = means;
		tmp.erase(std::next(std::begin(tmp), n - 1), std::next(std::begin(tmp), n - 1 + size));
		return Meaning(base, tmp);
	}
	Meaning replaced(std::size_t n, std::size_t size, const MeaningElement &el) const;
	std::string to_s() const
	{
		std::string str{""};
		if (means.size() == 0)
		{
			str = base.to_s();
		}
		else
		{
			str += base.to_s();
			std::vector<std::string> buf;
			std::for_each(std::begin(means), std::end(means), [&](auto &m) {
				buf.push_back(m.to_s());
			});
			std::ostringstream os;
			std::copy(std::begin(buf), std::end(buf), std::ostream_iterator<std::string>(os, ","));
			str += Prefices::LPRN + os.str();
			str.erase(str.end() - 1);
			str += Prefices::RPRN;
		}
		return str;
	}
	friend std::ostream &operator<<(std::ostream &out, const Meaning &obj);
};

class MeaningElement
{
	using MeaningType = std::variant<Meaning, Variable>;
	MeaningType element;

  public:
	MeaningElement() : element() {}
	MeaningElement(const MeaningElement &other) : element(other.element) {}
	MeaningElement(const Meaning &other) : element(other) {}
	MeaningElement(const Variable &other) : element(other) {}
	constexpr std::size_t type() const { return element.index(); }

	template <typename T>
	T &get() { return std::get<T>(element); } //用途によりconstはつけない

	MeaningElement &operator=(const MeaningElement &dst)
	{
		element = dst.element;
		return *this;
	}
	MeaningElement &operator=(const Meaning &dst)
	{
		element = dst;
		return *this;
	}
	MeaningElement &operator=(const Variable &dst)
	{
		element = dst;
		return *this;
	}
	bool operator==(const MeaningElement &dst) const
	{
		return element == dst.element;
	}
	bool operator!=(const MeaningElement &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const MeaningElement &dst) const
	{
		return element < dst.element;
	}
	bool operator>(const MeaningElement &dst) const
	{
		return element > dst.element;
	}
	bool operator<=(const MeaningElement &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const MeaningElement &dst) const
	{
		return !(*this < dst);
	}
	std::string to_s() const
	{
		return std::visit([](auto &&arg) { return arg.to_s(); }, element);
	}
	friend std::ostream &operator<<(std::ostream &out, const MeaningElement &obj);
};

class LeftNonterminal
{
	Category cat;
	Meaning means;

  public:
	LeftNonterminal() : cat(), means() {}
	LeftNonterminal(const Category &c, Meaning m) : cat(c), means(m) {}
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
	bool operator>(const LeftNonterminal &dst) const
	{
		return cat > dst.cat || (cat == dst.cat && means > dst.means);
	}
	bool operator<=(const LeftNonterminal &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const LeftNonterminal &dst) const
	{
		return !(*this < dst);
	}
	const Category &get_cat() const
	{
		return cat;
	}
	Meaning &get_means()
	{
		return means;
	}
	const Meaning &get_means() const
	{
		return means;
	}
	AMean &get_base()
	{
		return means.get_base();
	}
	const AMean &get_base() const
	{
		return means.get_base();
	}
	std::vector<MeaningElement> &get_followings()
	{
		return means.get_followings();
	}
	const std::vector<MeaningElement> &get_followings() const
	{
		return means.get_followings();
	}
	std::string to_s() const
	{
		return cat.to_s() + Prefices::DEL + means.to_s();
	}
	friend std::ostream &operator<<(std::ostream &out, const LeftNonterminal &obj);
};
class RightNonterminal
{
	Category cat;
	Variable var;

  public:
	RightNonterminal() : cat(), var() {}
	RightNonterminal(const Category &c, const Variable &v) : cat(c), var(v) {}
	RightNonterminal(const RightNonterminal &dst) : cat(dst.cat), var(dst.var) {}
	bool operator==(const RightNonterminal &dst) const
	{
		return cat == dst.cat;
	}
	bool operator!=(const RightNonterminal &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const RightNonterminal &dst) const
	{
		return cat < dst.cat;
	}
	bool operator>(const RightNonterminal &dst) const
	{
		return cat > dst.cat;
	}
	bool operator<=(const RightNonterminal &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const RightNonterminal &dst) const
	{
		return !(*this < dst);
	}
	const Category &get_cat() const
	{
		return cat;
	}
	const Variable &get_var() const
	{
		return var;
	}
	std::string to_s() const
	{
		return cat.to_s() + Prefices::DEL + var.to_s();
	}
	friend std::ostream &operator<<(std::ostream &out, const RightNonterminal &obj);
};
class SymbolElement
{
	using ElementType = std::variant<RightNonterminal, Symbol>;
	ElementType element;

  public:
	SymbolElement() : element() {}
	SymbolElement(const SymbolElement &other) : element(other.element) {}
	SymbolElement(const Symbol &other) : element(other) {}
	SymbolElement(const RightNonterminal &other) : element(other) {}
	constexpr std::size_t type() const { return element.index(); }

	template <typename T>
	const T &get() const { return std::get<T>(element); }

	SymbolElement &operator=(const SymbolElement &dst)
	{
		element = dst.element;
		return *this;
	}
	SymbolElement &operator=(const Symbol &dst)
	{
		element = dst;
		return *this;
	}
	SymbolElement &operator=(const RightNonterminal &dst)
	{
		element = dst;
		return *this;
	}
	bool operator==(const SymbolElement &dst) const
	{
		return element == dst.element;
	}
	bool operator!=(const SymbolElement &dst) const
	{
		return !(*this == dst);
	}
	bool operator<(const SymbolElement &dst) const
	{
		return element < dst.element;
	}
	bool operator>(const SymbolElement &dst) const
	{
		return element > dst.element;
	}
	bool operator<=(const SymbolElement &dst) const
	{
		return !(*this > dst);
	}
	bool operator>=(const SymbolElement &dst) const
	{
		return !(*this < dst);
	}
	std::string to_s() const
	{
		return std::visit([](auto &&arg) { return arg.to_s(); }, element);
	}
	friend std::ostream &operator<<(std::ostream &out, const SymbolElement &obj);
};

class Conception
{
  public:
	std::set<std::string> factors;
	Conception();
	Conception(std::string);

	//operator
	//!等号。型が異なると偽を返します。
	// 型が等しい場合はインデックスが等しいか比べます。
	bool operator==(const Conception &dst) const;
	//!等号の否定です
	bool operator!=(const Conception &dst) const;
	Conception operator+(Conception &dst);
	Conception operator-(Conception &dst);
	//!代入
	Conception &operator=(const Conception &dst);
	void diff(Conception &obj, Conception &res1, Conception &res2) const;
	void inter(Conception &obj, Conception &res) const;
	bool include(Conception &obj) const;

	bool empty();
	void clear();
	void add(std::string str);
	std::string to_s() const;
	friend std::ostream &operator<<(std::ostream &out, const Conception &obj);
};

#endif /* Conception_H_ */
