#include "Element.h"

std::map<std::string, std::string> Symbol::conv_symbol;

std::ostream &operator<<(std::ostream &out, const AMean &obj)
{
	out << obj.to_s();
	return out;
}
std::ostream &operator<<(std::ostream &out, const Variable &obj)
{
	out << obj.to_s();
	return out;
}
std::ostream &operator<<(std::ostream &out, const Category &obj)
{
	out << obj.to_s();
	return out;
}
std::ostream &operator<<(std::ostream &out, const Symbol &obj)
{
	out << obj.to_s();
	return out;
}
std::ostream &operator<<(std::ostream &out, const MeaningElement &obj)
{
	out << obj.to_s();
	return out;
}
std::ostream &operator<<(std::ostream &out, const Meaning &obj)
{
	out << obj.to_s();
	return out;
}
std::ostream &operator<<(std::ostream &out, const LeftNonterminal &obj)
{
	out << obj.to_s();
	return out;
}
std::ostream &operator<<(std::ostream &out, const RightNonterminal &obj)
{
	out << obj.to_s();
	return out;
}
std::ostream &operator<<(std::ostream &out, const SymbolElement &obj)
{
	out << obj.to_s();
	return out;
}

Meaning Meaning::replaced(std::size_t n, std::size_t size, const MeaningElement &el) const
{
	if (size > means.size() || n < 0 || (n == 0 && (size != 1 || el.type() != MEANING_TYPE::MEANING_TYPE || el.get<Meaning>().get_followings().size() != 0)))
	{
		std::cerr << "Cannot remove" << std::endl;
		exit(1);
	}
	if (n == 0)
	{
		return Meaning(el.get<Meaning>().get_base(), means);
	}
	else
	{
		std::vector<MeaningElement> tmp = means;
		auto it = tmp.erase(std::next(std::begin(tmp), n - 1), std::next(std::begin(tmp), n - 1 + size));
		tmp.insert(it, el);
		return Meaning(base, tmp);
	}
}

Conception::Conception()
{
}

Conception::Conception(std::string str)
{
	factors.insert(str);
}

Conception::Conception(const Conception &dst) : factors(dst.factors)
{
}

/*
 * Operators
 */
bool Conception::operator!=(const Conception &dst) const
{
	return !(*this == dst);
}

//集合が空じゃなくて各要素が等しいならtrue
bool Conception::operator==(const Conception &dst) const
{
	return factors == dst.factors;
}

Conception &Conception::operator=(const Conception &dst)
{
	factors = dst.factors;
	return *this;
}

Conception Conception::operator+(Conception &dst)
{
	Conception trg;
	trg.factors = factors;
	trg.factors.insert(dst.factors.begin(), dst.factors.end());
	return trg;
}

Conception Conception::operator-(Conception &dst)
{
	Conception trg;
	std::set_difference(factors.begin(), factors.end(), dst.factors.begin(), dst.factors.end(), std::inserter(trg.factors, trg.factors.end()));
	return trg;
}

void Conception::diff(Conception &obj, Conception &res1, Conception &res2) const
{
	res1.clear();
	res2.clear();
	std::set_difference(factors.begin(), factors.end(), obj.factors.begin(), obj.factors.end(), std::inserter(res1.factors, res1.factors.end()));
	std::set_difference(obj.factors.begin(), obj.factors.end(), factors.begin(), factors.end(), std::inserter(res2.factors, res2.factors.end()));
}

void Conception::inter(Conception &obj, Conception &res) const
{
	res.clear();
	std::set_intersection(factors.begin(), factors.end(), obj.factors.begin(), obj.factors.end(), std::inserter(res.factors, res.factors.end()));
}

bool Conception::include(Conception &obj) const
{
	return std::includes(factors.begin(), factors.end(), obj.factors.begin(), obj.factors.end());
}

bool Conception::empty()
{
	return factors.size() == 0 || factors.count("") == factors.size();
}

void Conception::clear()
{
	factors.clear();
}

void Conception::add(std::string str)
{
	factors.insert(str);
}

std::string Conception::to_s() const
{
	std::string str;
	std::set<std::string>::iterator f_it = factors.begin();
	for (; f_it != factors.end(); f_it++)
	{
		str += (*f_it) + std::string(" ");
	}
	str.pop_back();
	return str;
}

std::ostream &operator<<(std::ostream &out, const Conception &obj)
{
	out << obj.to_s();
	return out;
}