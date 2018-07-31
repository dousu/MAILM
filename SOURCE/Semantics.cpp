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
bool Semantics<T>::equal(const AMean &a, const AMean &b)
{
	if (mapping[a].empty() || mapping[b].empty())
	{
		// std::cout << mapping.size() << std::endl;
		// std::cout << "a:" << mapping[a.get_obj_id()].to_s() << std::endl;
		// std::cout << "b:" << mapping[b.get_obj_id()].to_s() << std::endl;
		// if(mapping[a.get_obj_id()].empty() && mapping[b.get_obj_id()].empty()){
		//   return false;
		// }
		//両方emptyでもfalse
		return false;
	}
	return mapping[a] == mapping[b];
}
template <typename T>
T &Semantics<T>::get(const AMean &n)
{
	return mapping[n];
}

//共通部分があればchunkできる
template <typename T>
bool Semantics<T>::chunk_equal(const AMean &a, const AMean &b)
{
	T tmp_v;
	// if(mapping[a.get_obj_id()].empty() || mapping[b.get_obj_id()].empty()){
	//   // std::cout << mapping.size() << std::endl;
	//   // std::cout << "a:" << mapping[a.get_obj_id()].to_s() << std::endl;
	//   // std::cout << "b:" << mapping[b.get_obj_id()].to_s() << std::endl;
	//   return false;
	// }
	mapping[a].inter(mapping[b], tmp_v); //intersection
	return !tmp_v.empty();
}
//aがbを含んでいればmergeできる
template <typename T>
bool Semantics<T>::merge_equal(const AMean &a, const AMean &b)
{
	T tmp_v;
	// if(mapping[a.get_obj_id()].empty() || mapping[b.get_obj_id()].empty()){
	//   // std::cout << mapping.size() << std::endl;
	//   // std::cout << "a:" << mapping[a.get_obj_id()].to_s() << std::endl;
	//   // std::cout << "b:" << mapping[b.get_obj_id()].to_s() << std::endl;
	//   return false;
	// }
	return mapping[a.get_obj_id()].include(mapping[b.get_obj_id()]) || mapping[b.get_obj_id()].include(mapping[a.get_obj_id()]); //包含関係になっていたら真
																																 //return equal(a, b);
}
//aがbを含んでいればreplaceできる
template <typename T>
bool Semantics<T>::replace_equal(const AMean &a, const AMean &b)
{
	T tmp_v;
	// if(mapping[a.get_obj_id()].empty() || mapping[b.get_obj_id()].empty()){
	//   // std::cout << mapping.size() << std::endl;
	//   // std::cout << "a:" << mapping[a.get_obj_id()].to_s() << std::endl;
	//   // std::cout << "b:" << mapping[b.get_obj_id()].to_s() << std::endl;
	//   return false;
	// }
	return mapping[a].include(mapping[b]);
}
template <typename T>
void Semantics<T>::chunk(const AMean &a, const AMean &b, const AMean &c, const AMean &d, const AMean &e, int pos, int d_size, int e_size, int type)
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
				throw "[chunk2]MATCHING ERROR [chunk2 semantics]";
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
		throw "UNKNOWN chunk TYPE";
	}
	mapping.erase(a);
	mapping.erase(b);
}
template <typename T>
void Semantics<T>::merge(const AMean &a, const AMean &b, const AMean &c)
{
	T c_v;
	c_v = mapping[a] + mapping[b];
	mapping[c] = c_v;
	//mapping.count(a)!=0で分岐
	//a=>c, b=>c
	// std::vector<int> new_tr_a, new_tr_b;
	if (a == c)
	{
		if (b == c)
		{   //a==c, b==c
			//nop
		}
		else
		{ //b->c
			std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
				rewrite(m.second, b, c);
			});
			if (merge_list.count(c) == 0)
			{
				merge_list.insert(std::make_pair(c, std::set<AMean>{{b}}));
			}
			else
			{
				merge_list[c].insert(b);
			}
			mapping.erase(b);
		}
	}
	else
	{
		if (b == c)
		{ //a->c
			std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
				rewrite(m.second, a, c);
			});
			if (merge_list.count(c) == 0)
			{
				merge_list.insert(std::make_pair(c, std::set<AMean>{{a}}));
			}
			else
			{
				merge_list[c].insert(a);
			}
			mapping.erase(a);
		}
		else
		{ //a->c,b->c
			std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
				rewrite(m.second, a, c);
			});
			std::for_each(std::begin(rules), std::end(rules), [&](auto &m) {
				rewrite(m.second, b, c);
			});
			if (merge_list.count(c) == 0)
			{
				merge_list.insert(std::make_pair(c, std::set<AMean>{{a, b}}));
			}
			else
			{
				merge_list[c].insert(a);
				merge_list[c].insert(b);
			}
			mapping.erase(a);
			mapping.erase(b);
		}
	}
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
		std::vector<MeaningElement> buf, means;
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
	res += merge_list_to_s();
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
std::string Semantics<T>::merge_list_to_s(void)
{
	std::ostringstream os;
	os << "MERGE LIST:" << std::endl;
	std::for_each(std::begin(merge_list), std::end(merge_list), [&os](auto &&l) { os << l.first << " -- " << SemanticsUtil::set_str(l.second) << std::endl; });
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
			res += "{" + Prefices::IND + p.first.to_s() + Prefices::CLN + "[" + p.second.to_s() + "]},";
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

//明示的な実在化
template class Semantics<Conception>;