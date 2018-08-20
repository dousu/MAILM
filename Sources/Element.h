#ifndef ELEMENT_H_
#define ELEMENT_H_

#include <iostream>
#include <map>
#include <string>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <variant>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <type_traits>
#include <list>

#include "Dictionary.h"
#include "Prefices.h"

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
	AMean() noexcept : obj(0) {}
	AMean(int num) noexcept : obj(num) {}
	AMean(const AMean &dst) noexcept : obj(dst.obj) {}
	AMean(AMean &&o) noexcept : obj(std::move(o.obj)) {}
	AMean &operator=(AMean &&o) noexcept
	{
		obj = std::move(o.obj);
		return *this;
	}
	AMean &operator=(const AMean &o) noexcept
	{
		obj = o.obj;
		return *this;
	}
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
	std::string to_s() const
	{
		return Prefices::MEA + std::to_string(obj);
	}
	friend std::ostream &operator<<(std::ostream &out, const AMean &obj);
};

class Variable
{
	int obj;

  public:
	Variable() noexcept : obj(0) {}
	Variable(int var_num) noexcept : obj(var_num) {}
	Variable(const Variable &dst) noexcept : obj(dst.obj) {}
	Variable(Variable &&o) noexcept : obj(std::move(o.obj)) {}
	Variable &operator=(Variable &&o) noexcept
	{
		obj = std::move(o.obj);
		return *this;
	}
	Variable &operator=(const Variable &o) noexcept
	{
		obj = o.obj;
		return *this;
	}
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
	Category() noexcept : obj(0) {}
	Category(int cat_num) noexcept : obj(cat_num) {}
	Category(const Category &dst) noexcept : obj(dst.obj) {}
	Category(Category &&o) noexcept : obj(std::move(o.obj)) {}
	Category &operator=(Category &&o) noexcept
	{
		obj = std::move(o.obj);
		return *this;
	}
	Category &operator=(const Category &o) noexcept
	{
		obj = o.obj;
		return *this;
	}
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
	Symbol(int num) noexcept : obj(num) {}
	Symbol(const Symbol &dst) noexcept : obj(dst.obj) {}
	Symbol(Symbol &&o) noexcept : obj(std::move(o.obj)) {}
	Symbol &operator=(Symbol &&o) noexcept
	{
		obj = std::move(o.obj);
		return *this;
	}
	Symbol &operator=(const Symbol &o) noexcept
	{
		obj = o.obj;
		return *this;
	}
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
	std::list<MeaningElement> means;

  public:
	Meaning() noexcept : base(), means() {}
	Meaning(const Meaning &dst) noexcept : base(dst.base), means(dst.means) {}
	Meaning(const AMean &m) noexcept : base(m), means() {}
	Meaning(const AMean &m, const std::list<MeaningElement> &dst) noexcept : base(m), means(dst) {}
	Meaning(const AMean &m, const std::list<Meaning> &dst) noexcept : base(m), means()
	{
		std::for_each(std::begin(dst), std::end(dst), [&](auto &obj) { means.push_back(obj); });
	}
	Meaning(const AMean &m, const std::list<Variable> &dst) noexcept : base(m), means()
	{
		std::for_each(std::begin(dst), std::end(dst), [&](auto &obj) { means.push_back(obj); });
	}
	Meaning(Meaning &&o) noexcept : base(std::move(o.base)), means(std::move(o.means)) {}
	Meaning &operator=(Meaning &&o) noexcept
	{
		base = std::move(o.base);
		means = std::move(o.means);
		return *this;
	}
	Meaning &operator=(const Meaning &o) noexcept
	{
		base = o.base;
		means = o.means;
		return *this;
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
		// return means.at(i - 1);
		return *std::next(std::begin(means), i - 1);
	}
	const MeaningElement &at(std::size_t i) const
	{
		if (i < 1)
		{
			std::cerr << "irregular index" << std::endl;
			exit(1);
		}
		// return means.at(i - 1);
		return *std::next(std::begin(means), i - 1);
	}
	AMean &get_base() noexcept
	{
		return base;
	}
	const AMean &get_base() const noexcept
	{
		return base;
	}
	std::list<MeaningElement> &get_followings() noexcept
	{
		return means;
	}
	const std::list<MeaningElement> &get_followings() const noexcept
	{
		return means;
	}
	int get_size() const
	{
		return means.size() + 1;
	}
	Meaning removed(std::size_t n, std::size_t size) const
	{
		if (size > means.size() || n == 0)
		{
			std::cerr << "Cannot remove" << std::endl;
			exit(1);
		}
		std::list<MeaningElement> tmp = means;
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
			std::list<std::string> buf;
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
	MeaningElement() noexcept : element() {}
	MeaningElement(const MeaningElement &other) noexcept : element(other.element) {}
	MeaningElement(const Meaning &other) noexcept : element(other) {}
	MeaningElement(const Variable &other) noexcept : element(other) {}
	MeaningElement(MeaningElement &&o) noexcept : element(std::move(o.element)) {}
	MeaningElement &operator=(MeaningElement &&o) noexcept
	{
		element = std::move(o.element);
		return *this;
	}
	MeaningElement &operator=(const MeaningElement &dst) noexcept
	{
		element = dst.element;
		return *this;
	}

	constexpr std::size_t type() const { return element.index(); }

	template <typename T>
	T &get() { return std::get<T>(element); } //用途によりconstはつけない

	template <typename T>
	const T &get() const { return std::get<T>(element); }

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
	LeftNonterminal() noexcept : cat(), means() {}
	LeftNonterminal(const Category &c, Meaning m) noexcept : cat(c), means(m) {}
	LeftNonterminal(const LeftNonterminal &dst) noexcept : cat(dst.cat), means(dst.means) {}
	LeftNonterminal(LeftNonterminal &&o) noexcept : cat(std::move(o.cat)), means(std::move(o.means)) {}
	LeftNonterminal &operator=(LeftNonterminal &&o) noexcept
	{
		cat = std::move(o.cat);
		means = std::move(o.means);
		return *this;
	}
	LeftNonterminal &operator=(const LeftNonterminal &o) noexcept
	{
		cat = o.cat;
		means = o.means;
		return *this;
	}
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
	std::list<MeaningElement> &get_followings()
	{
		return means.get_followings();
	}
	const std::list<MeaningElement> &get_followings() const
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
	RightNonterminal() noexcept : cat(), var() {}
	RightNonterminal(const Category &c, const Variable &v) noexcept : cat(c), var(v) {}
	RightNonterminal(const RightNonterminal &dst) noexcept : cat(dst.cat), var(dst.var) {}
	RightNonterminal(RightNonterminal &&o) noexcept : cat(std::move(o.cat)), var(std::move(o.var)) {}
	RightNonterminal &operator=(RightNonterminal &&o) noexcept
	{
		cat = std::move(o.cat);
		var = std::move(o.var);
		return *this;
	}
	RightNonterminal &operator=(const RightNonterminal &o) noexcept
	{
		cat = o.cat;
		var = o.var;
		return *this;
	}
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
	SymbolElement() noexcept : element() {}
	SymbolElement(const SymbolElement &other) noexcept : element(other.element) {}
	SymbolElement(const Symbol &other) noexcept : element(other) {}
	SymbolElement(const RightNonterminal &other) noexcept : element(other) {}
	SymbolElement(SymbolElement &&o) noexcept : element(std::move(o.element)) {}
	constexpr std::size_t type() const { return element.index(); }

	template <typename T>
	const T &get() const { return std::get<T>(element); }

	SymbolElement &operator=(const SymbolElement &&dst)
	{
		element = std::move(dst.element);
		return *this;
	}
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
	Conception() noexcept {}
	Conception(std::string str) noexcept : factors{{str}} {}
	Conception(const Conception &dst) noexcept : factors(dst.factors) {}
	Conception(Conception &&dst) noexcept : factors(std::move(dst.factors)) {}

	//operator
	//!等号。型が異なると偽を返します。
	// 型が等しい場合はインデックスが等しいか比べます。
	bool operator==(const Conception &dst) const
	{
		return factors == dst.factors;
	}
	//!等号の否定です
	bool operator!=(const Conception &dst) const
	{
		return !(*this == dst);
	}
	Conception operator+(const Conception &dst) const
	{
		Conception trg;
		trg.factors = factors;
		trg.factors.insert(std::begin(dst.factors), std::end(dst.factors));
		return trg;
	}
	Conception operator-(const Conception &dst) const
	{
		Conception trg;
		std::set_difference(std::begin(factors), std::end(factors), std::begin(dst.factors), std::end(dst.factors), std::inserter(trg.factors, std::begin(trg.factors)));
		return trg;
	}
	//!代入
	Conception &operator=(const Conception &dst) noexcept
	{
		factors = dst.factors;
		return *this;
	}
	Conception &operator=(Conception &&dst) noexcept
	{
		factors = std::move(dst.factors);
		return *this;
	}
	void diff(const Conception &obj, Conception &res1, Conception &res2) const
	{
		res1.clear();
		res2.clear();
		std::set_difference(std::begin(factors), std::end(factors), std::begin(obj.factors), std::end(obj.factors), std::inserter(res1.factors, std::begin(res1.factors)));
		std::set_difference(std::begin(obj.factors), std::end(obj.factors), std::begin(factors), std::end(factors), std::inserter(res2.factors, std::begin(res2.factors)));
	}
	void inter(const Conception &obj, Conception &res) const
	{
		res.clear();
		std::set_intersection(std::begin(factors), std::end(factors), std::begin(obj.factors), std::end(obj.factors), std::inserter(res.factors, std::begin(res.factors)));
	}
	bool include(const Conception &obj) const
	{
		return std::includes(std::begin(factors), std::end(factors), std::begin(obj.factors), std::end(obj.factors));
	}
	bool empty() const noexcept
	{
		return factors.size() == 0 || factors.count("") == factors.size();
	}
	void clear()
	{
		factors.clear();
	}
	void add(std::string str)
	{
		factors.insert(str);
	}
	std::string to_s() const
	{
		if (!empty())
		{
			std::string str;
			std::ostringstream os;
			std::copy(std::begin(factors), std::end(factors), std::ostream_iterator<std::string>(os, " "));
			str = os.str();
			str.pop_back();
			return str;
		}
		else
		{
			return "";
		}
	}
	friend std::ostream &operator<<(std::ostream &out, const Conception &obj);
};

#endif /* ELEMENT_H_ */
