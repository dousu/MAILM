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

std::string Rule::to_s()
{
	std::ostringstream os;
	std::copy(std::begin(external), std::end(external), std::ostream_iterator<SymbolElement>(os, " "));
	return internal.to_s() + Prefices::ARW + os.str();
}

void Rule::set_rule(LeftNonterminal &nt, std::list<SymbolElement> &ex)
{
	internal = nt;
	external = ex;
}
std::ostream &operator<<(std::ostream &out, const Rule &obj)
{
	std::ostringstream os;
	std::list<SymbolElement> sel_vec = obj.get_external();
	std::copy(std::begin(sel_vec), std::end(sel_vec), std::ostream_iterator<SymbolElement>(os, " "));
	out << obj.get_internal() << Prefices::ARW << os.str();
	return out;
}