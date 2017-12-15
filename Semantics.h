/*
 * Semantics.h
 *
 *  Created on: 2017/05/23
 *      Author: Hiroki Sudo
 */

#ifndef SEMANTICS_H_
#define SEMANTICS_H_
#include <map>
#include <vector>
#include <boost/range/algorithm_ext.hpp>

//templateでとるクラスTはElement.hで宣言されている
//diffは参照渡しにする
//意味はなくなるより余分にあったほうがいい
//意味は意味の集合として扱う(+,-,difference)
#include "Element.h"

// struct CompVec {
//   std::vector<int> vec_int;

//   void push_back(int a){
//   	vec_int.push_back(a);
//   }

//   // 演算子オーバーロードで比較関数を定義
//   bool operator<(const CompVec& another) const {
//     int size = vec_int.size() < another.vec_int.size() ? vec_int.size() : another.vec_int.size();
//     for(int i=0; i < size; i++){
//     	if(vec_int[i] != another.vec_int[i]){
//     		return vec_int[i] < another.vec_int[i];
//     	}
//     }
//     return vec_int.size() < another.vec_int.size();
//   }
//   bool operator==(const CompVec& another) const {
//     if(vec_int.size() != another.vec_int.size()){
//     	return false;
//     }
//     return vec_int == another.vec_int;
//   }
//   bool operator!=(const CompVec& another) const {
//     return !(*this == another.vec_int);
//   }
//   CompVec& operator=(const CompVec& dst){
//   	vec_int = dst.vec_int;
//   	return *this;
//   }
// };

// typedef std::map<CompVec,std::vector<int> > TransRules;
typedef std::map<int,std::vector<Element> >TransRules;

template <typename T>
class Semantics {
public:
	std::map<int, T > mapping;
	TransRules rules;
	std::map<int,std::set<int> > merge_list;

	T& operator[] (int x){
		return mapping[x];
	}

	void store(Element a,T v);
	bool equal(Element a,Element b);
	bool chunk_equal(Element a,Element b);
	bool merge_equal(Element a,Element b);
	bool replace_equal(Element a,Element b);
	void chunk(Element a,Element b,Element c, Element d,Element e, int pos, int d_size, int e_size, int type);
	void merge(Element a,Element b, Element c);
	void replace(Element a,Element b,Element c, int b_pos, int b_size);
	void unique_unify(Element a, std::vector<Element> v_e);//下が残る
	std::vector<Element> trans(Element a);

	Semantics<T> copy(void);
	void clear(void);
	std::string to_s(void);
	std::string rules_to_s(void);
	std::string merge_list_to_s(void);
	std::string mapping_to_s(void);
	void init_rules(TransRules& obj);

private:
	// std::vector<Element> sub_trans(CompVec a);
	void rewrite(Element a, Element b);
	int merge_trans(int a);
};

#endif /* SEMANTICS_H_ */
