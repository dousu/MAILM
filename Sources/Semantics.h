#ifndef SEMANTICS_H_
#define SEMANTICS_H_
#include <map>
#include <vector>
#include <ostream>
#include <algorithm>
#include <set>
#include <ostream>
#include <sstream>

#include "Element.h"
#include "LogBox.h"

typedef std::map<int, Meaning> TransRules;

template <typename T>
class Semantics
{
	std::map<AMean, T> mapping;
	TransRules rules;

  public:
	T &operator[](int x)
	{
		return mapping[x];
	}

	void store(const AMean &a, T v);
	T get(const AMean &n);
	bool equal(const AMean &a, const AMean &b) const;
	bool chunk_equal(const AMean &a, const AMean &b) const;
	bool merge_equal(const AMean &a, const AMean &b) const;
	bool replace_equal(const AMean &a, const AMean &b) const;
	void chunk(const AMean &a, const AMean &b, const AMean &c, const AMean &c2, const AMean &d, const AMean &e, int pos, int d_size, int e_size, int type);
	void merge(const AMean &a, const std::set<AMean> &b_vec, const AMean &c);
	void replace(const AMean &a, const AMean &b, const AMean &c, int b_pos, int b_size);
	Meaning &trans(int n);

	Semantics<T> copy(void);
	void clear(void);
	std::string to_s(void);
	std::string rules_to_s(void);
	std::string mapping_to_s(void);
	void init_rules(TransRules &obj);

  private:
	void rewrite(Meaning &m, const AMean &a, const AMean &b);
	void rewrite(Meaning &m, const AMean &a, const AMean &b, const AMean &c, int pos, int size);
};
namespace SemanticsUtil
{
template <typename T>
std::string set_str(std::set<T> s)
{
	std::ostringstream os;
	os << "{";
	std::copy(std::begin(s), std::end(s), std::ostream_iterator<T>(os, " "));
	std::string res = os.str();
	res.pop_back();
	res += "}";
	return res;
};
} // namespace SemanticsUtil

template class Semantics<Conception>;

#endif /* SEMANTICS_H_ */
