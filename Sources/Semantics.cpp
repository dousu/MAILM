#include "Semantics.h"

template <typename T>
void Semantics<T>::store(const AMean &a, T v)
{
	if (mapping.count(a) == 0)
	{
		mapping[a] = v;
	}
	else
	{
		std::cerr << "ALREADY EXISTS" << std::endl;
	}
}
template <typename T>
bool Semantics<T>::equal(const AMean &a, const AMean &b) const
{
	if (mapping.at(a).empty() || mapping.at(b).empty())
	{
		//両方emptyでもfalse
		return false;
	}
	return mapping.at(a) == mapping.at(b);
}
//共通部分があればchunkできる
template <typename T>
bool Semantics<T>::chunk_equal(const AMean &a, const AMean &b) const
{
	T tmp_v;
	if (mapping.at(a).empty() && mapping.at(b).empty())
	{
		return true;
	}
	mapping.at(a).inter(mapping.at(b), tmp_v); //intersection
	return !tmp_v.empty();
}
//aがbを含んでいればmergeできる
template <typename T>
bool Semantics<T>::merge_equal(const AMean &a, const AMean &b) const
{
	T tmp_v;
	if (mapping.at(a).empty() && mapping.at(b).empty())
	{
		return true;
	}
	return mapping.at(a).include(mapping.at(b)) || mapping.at(b).include(mapping.at(a)); //a includes b or b includes a?
}
//aがbを含んでいればreplaceできる
template <typename T>
bool Semantics<T>::replace_equal(const AMean &a, const AMean &b) const
{
	T tmp_v;
	if (mapping.at(a).empty() && mapping.at(b).empty())
	{
		return true;
	}
	return mapping.at(a).include(mapping.at(b));
}
template <typename T>
T Semantics<T>::get(const AMean &n)
{
	return mapping[n];
}
template <typename T>
void Semantics<T>::chunk(const AMean &a, const AMean &b, const AMean &c, const AMean &c2, const AMean &d, const AMean &e, int pos, int d_size, int e_size, int type)
{
	//type 1ならeにAMeanが入ってきているはず
	T c_v, d_v, e_v, null_v;
	mapping[a].diff(mapping[b], d_v, e_v);
	if (type == 1)
	{
		c_v = mapping[a] - d_v;

		if (c_v != mapping[b] - e_v)
		{
			std::cerr << "[chunk]MATCHING ERROR [chunk semantics]" << std::endl;
			exit(1);
		}
		mapping[c] = c_v;
		if (c2 != AMean())
		{
			mapping[c2] = c_v;
		}
		mapping[d] = d_v;
		mapping[e] = e_v;
		std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
			rewrite(m.second, a, c, d, pos, d_size);
			rewrite(m.second, b, c, e, pos, e_size);
		});
	}
	else if (type == 2)
	{
		if (e_v == null_v)
		{
			c_v = mapping[a] - d_v;
			if (c_v != mapping[b] - e_v)
			{
				std::cerr << "[chunk2]MATCHING ERROR [chunk2 semantics]" << std::endl;
				exit(1);
			}
			mapping[c] = c_v;
			mapping[d] = d_v;
		}
		else
		{
			c_v = mapping[b];
			mapping[c] = c_v;
			mapping[d] = d_v;
		}
		if (c2 != AMean())
		{
			mapping[c2] = c_v;
		}
		//b=>c
		std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
			rewrite(m.second, b, c);
		});
		//a=>cd
		std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
			rewrite(m.second, a, c, d, pos, d_size);
		});
	}
	else
	{
		std::cerr << "UNKNOWN chunk TYPE" << std::endl;
		exit(1);
	}
	mapping.erase(a);
	mapping.erase(b);
}
template <typename T>
void Semantics<T>::merge(const AMean &a, const std::set<AMean> &b_vec, const AMean &c)
{
	//cが同じだから違うbによってcが上書きされる
	mapping[c] = T();
	std::for_each(std::begin(b_vec), std::end(b_vec), [&](const AMean &b) {
		if (a == c || b == c)
		{
			std::cerr << "[merge in semantics] exception" << std::endl;
			exit(1);
		}
		T c_v;
		c_v = mapping[a] + mapping[b];
		mapping[c] = mapping[c] + c_v;

		//a->c,b->c
		std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
			rewrite(m.second, a, c);
		});
		std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
			rewrite(m.second, b, c);
		});
		if (a != b)
			mapping.erase(b);
	});
	mapping.erase(a);
}
template <typename T>
void Semantics<T>::replace(const AMean &a, const AMean &b, const AMean &c, int b_pos, int b_size)
{
	T c_v;
	c_v = mapping[a] - mapping[b];
	mapping[c] = c_v;
	//a=>cb
	std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
		rewrite(m.second, a, c, b, b_pos, b_size);
	});

	mapping.erase(a);
}
template <typename T>
Meaning &Semantics<T>::trans(int n)
{
	return rules[n];
}
template <typename T>
void Semantics<T>::rewrite(Meaning &m, const AMean &a, const AMean &b)
{
	if (m.get_base() == a)
	{
		m = Meaning(b, m.get_followings());
	}
	std::for_each(std::begin(m.get_followings()), std::end(m.get_followings()), [&](MeaningElement &obj) {
		rewrite(obj.get<Meaning>(), a, b);
	});
}
template <typename T>
void Semantics<T>::rewrite(Meaning &m, const AMean &a, const AMean &b, const AMean &c, int pos, int size)
{
	if (m.get_base() == a)
	{
		if (pos - 1 + size > m.get_followings().size())
		{
			std::cerr << m << " target:" << a << " new1:" << b << " new2:" << c << " pos:" << pos << " size:" << size << std::endl
					  << "error size : Semantics::rewrite" << std::endl;
			exit(1);
		}
		std::list<MeaningElement> buf, means;
		std::copy_n(std::begin(m.get_followings()), pos - 1, std::back_inserter(buf));
		std::copy_n(std::next(std::begin(m.get_followings()), pos - 1), size, std::back_inserter(means));
		buf.push_back(Meaning(c, means));
		std::copy(std::next(std::begin(m.get_followings()), pos - 1 + size), std::end(m.get_followings()), std::back_inserter(buf));
		m = Meaning(b, buf);
	}
	std::for_each(std::begin(m.get_followings()), std::end(m.get_followings()), [&](MeaningElement &obj) {
		rewrite(obj.get<Meaning>(), a, b, c, pos, size);
	});
}
template <typename T>
Semantics<T> Semantics<T>::copy(void)
{
	Semantics<T> dst;
	dst.mapping = mapping;
	dst.rules = rules;
	return dst;
}
template <typename T>
void Semantics<T>::clear(void)
{
	mapping.clear();
	rules.clear();
}
template <typename T>
std::string Semantics<T>::to_s(void)
{
	std::string res("###Semantics static information###\n");
	res += rules_to_s();
	res += mapping_to_s();
	return res;
}
template <typename T>
std::string Semantics<T>::rules_to_s(void)
{
	std::ostringstream os;
	os << "SUPPLEMENTARY RULES:" << std::endl;
	std::for_each(std::begin(rules), std::end(rules), [&](auto &r) { os << r.first << " -- " << r.second << std::endl; });
	return os.str();
}
template <typename T>
std::string Semantics<T>::mapping_to_s(void)
{
	std::string res("VALID INDEX and MEANINGS:\n");
	for (auto &p : mapping)
	{
		if (!p.second.empty())
		{
			res += "{" + p.first.to_s() + Prefices::CLN + "[" + p.second.to_s() + "]},";
		}
		else
		{
			res += "{" + p.first.to_s() + "[]},";
		}
	}
	res.pop_back();
	res += "\n";
	return res;
}
template <typename T>
void Semantics<T>::init_rules(TransRules &obj)
{
	rules = obj;
}
