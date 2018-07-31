#include "Rule.h"

Rule::Rule()
{
	internal = LeftNonterminal();
	external.clear();
}

Rule::Rule(const LeftNonterminal &lt, const std::vector<SymbolElement> &ex) : internal(lt), external(ex)
{
}

// Rule::Rule(char *cstr)
// {
// 	Rule(std::string(cstr));
// }
// Rule::Rule(std::string str)
// {
// 	external.clear();
// 	std::regex arrow(Prefices::ARW),
// 		slash(Prefices::DEL),
// 		pas1("\\((*)\\)"),
// 		pas2(std::string("[") + Prefices::CNM + std::string("]")),
// 		num("[1-9][0-9]*"),
// 		nums(".([1-9][0-9]*)\\/.([1-9][0-9]*)");
// 	std::string l, r, cat_alp, mean, category;
// 	std::vector<std::string> meanings;

// 	std::sregex_token_iterator it = std::sregex_token_iterator();
// 	std::sregex_token_iterator it1(std::begin(str), std::end(str), arrow, -1);
// 	int i = 0;
// 	for (; it1 != it; it1++, i++)
// 	{
// 		if (i == 2)
// 		{
// 			std::cerr << "too many right arrow" << std::endl;
// 			exit(1);
// 		}
// 		else if (i == 0)
// 		{
// 			l = *it1;
// 		}
// 		else
// 		{
// 			r = *it1;
// 		}
// 	}
// 	std::sregex_token_iterator it2(std::begin(l), std::end(l), slash, -1);
// 	for (i = 0; it2 != it; it2++, i++)
// 	{
// 		if (i == 2)
// 		{
// 			std::cerr << "too many slash" << std::endl;
// 			exit(1);
// 		}
// 		else if (i == 0)
// 		{
// 			cat_alp = *it2;
// 		}
// 		else
// 		{
// 			mean = *it2;
// 		}
// 	}
// 	int cnum;
// 	std::varinant<Meaning, Variable> m;
// 	std::function<MeaningElement &(std::string &)> construct_meaning = [&construct_meaning, &pas1, &pas2, &num](std::string &str) -> MeaningElement & {
// 		if (std::equal(std::begin(str), std::next(std::begin(str), Prefices::VAR.size()), std::begin(Prefices::VAR), std::end(Prefices::VAR)))
// 		{
// 			AMean am;
// 			std::vector<MeaningElement> means;
// 			if (str.find("(") > 0)
// 			{
// 				std::map<std::string, int>::iterator dic_it;
// 				dic_it = dictionary.conv_symbol.find(std::string{str, 0, str.find("(")});
// 				if (dic_it != dictionary.conv_symbol.end())
// 				{
// 					am = AMean((*dic_it).second);
// 				}
// 				else
// 				{
// 					std::cerr << "no candidate in dictionary" << std::endl;
// 					exit(1);
// 				}
// 				std::string followings;
// 				std::sregex_token_iterator it_m = std::sregex_token_iterator();
// 				std::sregex_token_iterator it_m1(std::begin(str), std::end(str), pas1, 1);
// 				for (int i = 0; it_m1 != it_m; it_m1++, i++)
// 				{
// 					if (i == 1)
// 					{
// 						std::cerr << "something is uncorrect" << std::endl;
// 						exit(1);
// 					}
// 					else
// 					{
// 						followings = *it_m1;
// 					}
// 				}
// 				std::sregex_token_iterator it_m2(std::begin(followings), std::end(followings), pas2, -1);
// 				std::for_each(it_m2, it_m, [&construct_meaning, &pas1, &pas2, &num, &means](std::string &obj) {
// 					means.push_back(construct_meaning(obj));
// 				});
// 			}
// 			else
// 			{
// 				std::map<std::string, int>::iterator dic_it;
// 				dic_it = dictionary.conv_symbol.find(std::string{str, 0, str.find(",")});
// 				if (dic_it != dictionary.conv_symbol.end())
// 				{
// 					am = AMean((*dic_it).second);
// 				}
// 				else
// 				{
// 					std::cerr << "no candidate in dictionary" << std::endl;
// 					exit(1);
// 				}
// 			}
// 			return MeaningElement(Meaning(am, means));
// 		}
// 		else
// 		{
// 			std::sregex_token_iterator it_m3(std::begin(str), std::end(str), num, 0);
// 			return MeaningElement(Variable(std::stoi(*it_m3)));
// 		}
// 	};
// 	MeaningElement m_el = construct_meaning(mean);
// 	switch (m_el.type())
// 	{
// 	case MEANING_TYPE::MEANING_TYPE:
// 		m = Meaning(std::get<MEANING_TYPE::MEANING_TYPE>(m_el));
// 		break;
// 	case MEANING_TYPE::VAR_TYPE:
// 		exit(1);
// 		break;
// 	default:
// 		exit(1);
// 	}
// 	if (cat_alp.size() > Prefices::SEN.size() && cat_alp.size() > Prefices::CAT.size())
// 	{
// 		std::sregex_token_iterator it4(std::begin(cat_alp), std::end(cat_alp), num, {-1, 0});
// 		for (i = 0; it4 != it; it4++, i++)
// 		{
// 			if (i == 2)
// 			{
// 				std::cerr << "A category name doesn't include numbers." << std::endl;
// 				exit(1);
// 			}
// 			else if (i == 0)
// 			{
// 				category = *it4;
// 			}
// 			else
// 			{
// 				cnum = std::stoi(*it4);
// 			}
// 		}
// 		if (category != Prefices::CAT && category != Prefices::MES)
// 		{
// 			std::cerr << category << " is not category charactor." << std::endl;
// 			exit(1);
// 		}
// 		else if (category == Prefices::CAT)
// 		{
// 			internal = LeftNonterminal(Category(cnum), m);
// 		}
// 		else
// 		{
// 			internal = LeftNonterminal(Category(cnum), m);
// 		}
// 	}
// 	else
// 	{
// 		category = cat_alp;
// 		cnum = 0;
// 		if (category == Prefices::SEN)
// 		{
// 			internal = LeftNonterminal(Category(0), m);
// 		}
// 		else
// 		{
// 			std::cerr << category << " is not start symbol." << std::endl;
// 			exit(1);
// 		}
// 	}
// 	auto r_it = std::begin(r);
// 	for (; r_it != std::end(r); r_it++)
// 	{
// 		if (std::string{*r_it} == Prefices::CAT)
// 		{
// 			int var_num, cat_num;
// 			std::sregex_token_iterator it6(r_it, std::end(r), nums, {1, 2});
// 			for (i = 0; it6 != it; it6++, i++)
// 			{
// 				if (i == 2)
// 				{
// 					break;
// 				}
// 				else if (i == 1)
// 				{
// 					var_num = std::stoi(*it6);
// 					r_it += std::string(*it6).size();
// 				}
// 				else
// 				{
// 					cat_num = std::stoi(*it6);
// 					r_it += std::string(*it6).size();
// 				}
// 			}
// 			SymbolElement ex_cat;
// 			ex_cat = RightNonterminal(Category(cat_num), Variable(var_num));
// 			external.push_back(ex_cat);
// 			r_it++; // for slash
// 			r_it++; // for variable symbol
// 		}
// 		else
// 		{
// 			SymbolElement sym;
// 			std::map<std::string, int>::iterator dic_it;
// 			dic_it = dictionary.conv_symbol.find(std::string{*r_it});
// 			if (dic_it != dictionary.conv_symbol.end())
// 			{
// 				sym = Symbol((*dic_it).second);
// 			}
// 			else
// 			{
// 				std::cerr << "no candidate in dictionary" << std::endl;
// 				exit(1);
// 			}
// 			external.push_back(sym);
// 		}
// 	}
// }

/*
 *
 * Operators
 *
 */

bool Rule::operator!=(Rule &dst) const
{
	return !(*this == dst);
}

bool Rule::operator==(const Rule &dst) const
{
	return internal.get_base() == dst.internal.get_base() && external.size() == dst.external.size() && external == dst.external;
}

bool Rule::operator<(const Rule &dst) const
{
	return internal.get_base() < dst.internal.get_base() || (internal.get_base() == dst.internal.get_base() && external == dst.external);
}

Rule &Rule::operator=(const Rule &dst)
{
	internal = dst.internal;
	external = dst.external;

	return *this;
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

void Rule::set_rule(LeftNonterminal &nt, std::vector<SymbolElement> &ex)
{
	internal = nt;
	external = ex;
}
std::ostream &operator<<(std::ostream &out, const Rule &obj)
{
	std::ostringstream os;
	std::vector<SymbolElement> sel_vec = obj.get_external();
	std::copy(std::begin(sel_vec), std::end(sel_vec), std::ostream_iterator<SymbolElement>(os, " "));
	out << obj.get_internal() << Prefices::ARW << os.str();
	return out;
}