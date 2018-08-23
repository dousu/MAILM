#include "Rule.h"

Rule::Rule() noexcept
{
	internal = LeftNonterminal();
	external.clear();
}

Rule::Rule(const LeftNonterminal &lt, const std::list<SymbolElement> &ex) noexcept : internal(lt), external(ex)
{
}

/*
 *
 * Operators
 *
 */
Rule &Rule::operator=(Rule &&o) noexcept
{
	internal = std::move(o.internal);
	external = std::move(o.external);
	return *this;
}
Rule &Rule::operator=(const Rule &o) noexcept
{
	internal = o.internal;
	external = o.external;
	return *this;
}
bool Rule::operator!=(Rule &dst) const
{
	return !(*this == dst);
}

bool Rule::operator==(const Rule &dst) const
{
	return internal.get_cat() == dst.internal.get_cat() && internal.get_base() == dst.internal.get_base() && external == dst.external;
}

bool Rule::operator<(const Rule &dst) const
{
	return internal.get_cat() < dst.internal.get_cat() || (internal.get_cat() == dst.internal.get_cat() && (internal.get_base() < dst.internal.get_base() || (internal.get_base() == dst.internal.get_base() && external < dst.external)));
}

bool Rule::is_sentence(Semantics<Conception> &s) const
{
	Conception c(Prefices::SEN);
	return s.get(internal.get_base()).include(c);
}

bool Rule::is_noun(Semantics<Conception> &s) const
{
	Conception c(Prefices::CAT);
	return s.get(internal.get_base()).include(c);
}

bool Rule::is_measure(Semantics<Conception> &s) const
{
	Conception c(Prefices::MES);
	return s.get(internal.get_base()).include(c);
}

std::string Rule::to_s() const
{
	std::string str;
	std::ostringstream os;
	std::copy(std::begin(external), std::end(external), std::ostream_iterator<SymbolElement>(os, " "));
	str = os.str();
	str.pop_back();
	return internal.to_s() + Prefices::ARW + str;
}

void Rule::set_rule(LeftNonterminal &nt, std::list<SymbolElement> &ex)
{
	internal = nt;
	external = ex;
}
std::ostream &operator<<(std::ostream &out, const Rule &obj)
{
	out << obj.to_s();
	return out;
}