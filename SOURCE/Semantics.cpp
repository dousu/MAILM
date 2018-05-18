/*
 * Semantics.cpp
 *
 *  Created on: 2017/05/23
 *      Author: Hiroki Sudo
 */

#include "Semantics.h"

template <typename T>
void Semantics<T>::store(Element a, T v) {
	if (a.is_ind()) {
		if (mapping.count(a.obj) == 0) {
			mapping[a.obj] = v;
		}
		else {
			std::cerr << "ALREADY EXISTS" << std::endl;
		}
	}
	else {
		std::cerr << "NOT MEANING" << std::endl;
		throw "NOT MEANING";
	}
}
template <typename T>
bool Semantics<T>::equal(Element a, Element b) {
	if (!(a.is_ind() && b.is_ind())) {
		std::cerr << "[equal]INCORRECT ELEMENT" << std::endl;
		throw "[equal]INCORRECT ELEMENT";
	}
	if (mapping[a.obj].empty() || mapping[b.obj].empty()) {
		// std::cout << mapping.size() << std::endl;
		// std::cout << "a:" << mapping[a.obj].to_s() << std::endl;
		// std::cout << "b:" << mapping[b.obj].to_s() << std::endl;
		// if(mapping[a.obj].empty() && mapping[b.obj].empty()){
		//   return false;
		// }
		//両方emptyでもfalse 
		return false;
	}
	return mapping[a.obj] == mapping[b.obj];
}
//共通部分があればchunkできる
template <typename T>
bool Semantics<T>::chunk_equal(Element a, Element b) {
	// std::cout << "\n****************test check1" << std::endl;
	T tmp_v;
	if (!(a.is_ind() && b.is_ind())) {
		std::cerr << "[chunk_equal]INCORRECT ELEMENT" << std::endl;
		throw "[chunk_equal]INCORRECT ELEMENT";
	}
	// if(mapping[a.obj].empty() || mapping[b.obj].empty()){
	//   // std::cout << mapping.size() << std::endl;
	//   // std::cout << "a:" << mapping[a.obj].to_s() << std::endl;
	//   // std::cout << "b:" << mapping[b.obj].to_s() << std::endl;
	//   return false;
	// }
	mapping[a.obj].inter(mapping[b.obj], tmp_v);//intersection
	// std::cout << "\n****************test check1 end" << std::endl;
	return !tmp_v.empty(); //共通部分あれば真
}
//aがbを含んでいればmergeできる
template <typename T>
bool Semantics<T>::merge_equal(Element a, Element b) {
	T tmp_v;
	if (!(a.is_ind() && b.is_ind())) {
		std::cerr << "[merge_equal]INCORRECT ELEMENT" << std::endl;
		throw "[merge_equal]INCORRECT ELEMENT";
	}
	// if(mapping[a.obj].empty() || mapping[b.obj].empty()){
	//   // std::cout << mapping.size() << std::endl;
	//   // std::cout << "a:" << mapping[a.obj].to_s() << std::endl;
	//   // std::cout << "b:" << mapping[b.obj].to_s() << std::endl;
	//   return false;
	// }
	return mapping[a.obj].include(mapping[b.obj]) || mapping[b.obj].include(mapping[a.obj]); //包含関係になっていたら真
}
//aがbを含んでいればreplaceできる
template <typename T>
bool Semantics<T>::replace_equal(Element a, Element b) {
	T tmp_v;
	if (!(a.is_ind() && b.is_ind())) {
		std::cerr << "[replace_equal]INCORRECT ELEMENT" << std::endl;
		throw "[replace_equal]INCORRECT ELEMENT";
	}
	// if(mapping[a.obj].empty() || mapping[b.obj].empty()){
	//   // std::cout << mapping.size() << std::endl;
	//   // std::cout << "a:" << mapping[a.obj].to_s() << std::endl;
	//   // std::cout << "b:" << mapping[b.obj].to_s() << std::endl;
	//   return false;
	// }
	return mapping[a.obj].include(mapping[b.obj]);
}
template <typename T>
void Semantics<T>::chunk(Element a, Element b, Element c, Element d, Element e, int pos, int d_size, int e_size, int type) {

	// std::cout << "\n****************test check2" << std::endl;

	// std::cout << "SEMANTICS chunk type=" << type << " pos=" << pos << " d_size=" << d_size << " e_size=" << e_size << std::endl;
	// std::cout << a.to_s() << " => " << c.to_s() << " " << d.to_s() << std::endl;
	// if(type == 1){
	//   std::cout << b.to_s() << " => " << c.to_s() << " " << e.to_s() << std::endl;
	// }
	// std::cout << rules_to_s() << std::endl;

	if (!(a.is_ind() && b.is_ind() && c.is_ind() && d.is_ind())) {
		std::cerr << "[chunk]INCORRECT ELEMENT" << std::endl;
		throw "[chunk]INCORRECT ELEMENT";
	}
	//type 1ならeにindex Elementが入ってきているはず
	if (type == 1) {
		if (!e.is_ind()) {
			std::cerr << "[chunk2]INCORRECT ELEMENT" << std::endl;
			throw "[chunk2]INCORRECT ELEMENT";
		}
	}
	//d_in and e_in are calculatable
	//change a and b to c or c2
	//then, change d_in and e_in to d(d_in_res) and e(e_in_res) respectively
	T c_v, d_v, e_v, null_v;
	mapping[a.obj].diff(mapping[b.obj], d_v, e_v);
	if (type == 1) {
		c_v = mapping[a.obj] - d_v;

		// std::cerr << "c_v: " << c_v.to_s() << std::endl;
		// std::cerr << "a_v: " << mapping[a.obj].to_s() << std::endl;
		// std::cerr << "b_v: " << mapping[b.obj].to_s() << std::endl;
		// std::cerr << "d_v: " << d_v.to_s() << std::endl;
		// std::cerr << "e_v: " << e_v.to_s() << std::endl;

		if (c_v != mapping[b.obj] - e_v) {
			std::cerr << "[chunk]MATCHING ERROR [chunk semantics]" << std::endl;
			throw "[chunk]MATCHING ERROR [chunk semantics]";
		}
		mapping[c.obj] = c_v;
		mapping[d.obj] = d_v;
		mapping[e.obj] = e_v;
		//Transrules
		std::vector<Element> d_in_res, e_in_res;

		std::vector<int> outer_list;
		std::vector<int> ch_v;
		int next_pos;

		int size;
		for (auto& rule : rules) {
			std::vector<Element>::iterator it;
			it = rule.second.begin();//itをイテレータから整数へ，順番は保存されているからdの位置を調べる→ループ構造変更
			size = d_size;

			next_pos = -1;
			ch_v.clear();
			outer_list.clear();
			while (next_pos < (int)rule.second.size()) {//最後に調べた場所がendかどうかチェック
				next_pos += 1;//次の位置が戻ってきたときの最後に調べた位置になる
				if ((rule.second.begin() + next_pos) == rule.second.end()) {
					it = rule.second.end();
					break;//次に調べる場所がendだったらbreak;
				}
				it = rule.second.begin() + next_pos;//見つかった場所から一つ進めてまた探索

				//aを一回探す
				//while (it != rule.second.end()) {

				//ch_vの処理,outer_list処理.
				if ((*it).ch.front() != 1) {
					ch_v.push_back((*it).ch.front());
					outer_list.push_back(it - rule.second.begin());
				}

				if ((*it) == a) {
						// target_f=true;
					if ((*it).ch.front() == 1) {//これがcにかわるから確実に複数入っているから1でもいれる
						ch_v.push_back(1);
						outer_list.push_back(it - rule.second.begin());
					}
					for (int o_el : outer_list) {
						// std::cout << "INCREMENT: [" << o_el << "]" << std::endl;
						(*(rule.second.begin() + o_el)).ch.front()++;//ひとつしかch持ってない想定
					}
					for (int& obj : ch_v) {
						obj++;
					}
					//aが見つかった時の内側の処理を行う．aのchの処理はしなくてよい．outer_listで処理するから
					int num = (*it).ch.front() - 1, it_id = it - rule.second.begin();//beginからの絶対値取得
					Element new_d;
					new_d = d;
					d_in_res.clear();

					//aをcへ
					(*it).obj = c.obj;

					//outerをカウントしてposになったら，
					//rule.second.begin()からの位置insert_posを決定，
					//そしてsize分outerとそれに連なるinnerをとって
					//（rule.secondからは削除してほかの部分に移すeraseを使ってitの更新をしてもよい．iteratorは取っておいてない）
					//dのchを決定とともにdが含む挿入部分の完成，
					//insert_posに挿入部分を組み込む
					int outer_c = 0, inner_c;
					bool inner_flag = false;
					int insert_pos = 0, er_num = 0;
					//最初はindexなのでスキップ
					it++;
					// std::cout << "\n****************test check49 it=\"" << (*(it-1)).to_s() << "(" << (*(it-1)).ch.front() << ") " << (it == rule.second.end()) << "\"" << std::endl;
					outer_c++;
					
					while (it != rule.second.begin() + it_id + num - er_num) {
						// std::cout << "\n****************test check50" << std::endl;
						int it_ch = (*it).ch.front();
						// std::cout << "\n****************test check46" << std::endl;
						if (!inner_flag && outer_c == pos) {
							inner_c = 0;
							inner_flag = true;
							insert_pos = it - rule.second.begin();
							if (inner_c == size) {
								break;
							}
						}
						if (inner_flag) {
							int itr_num = (*it).ch.front();
							for (int j = 0; j < itr_num; j++) {
								auto r = *it;
								d_in_res.push_back(r);
								it = rule.second.erase(it);
								er_num++;
							}

							inner_c++;
							if (inner_c == size) {
								break;
							}
							//eraseによって次の値になってるからitの移動はいらない
						}
						else {
							it += it_ch;
							outer_c++;
						}
					}
					//最後に追加するパターン
					if (insert_pos == 0) {
						insert_pos = next_pos + 1;
					}

					if(d_in_res.size()>0){
						new_d.set_ch(d_in_res.size() + 1);
					}

					//挿入する部分の完成
					d_in_res.insert(d_in_res.begin(), new_d);

					//適切な位置への挿入
					rule.second.insert(rule.second.begin() + insert_pos, d_in_res.begin(), d_in_res.end());
				}

				for (int& obj : ch_v) {
					obj--;
					if (obj == 0) {
						// buffer.push_back(Prefices::RPRN);
						outer_list.pop_back();
					}
				}
				boost::remove_erase_if(ch_v, [](int obj) { return obj == 0; });
			}

			size = e_size;
			it = rule.second.begin();

			// std::cout << "\n****************test check41b" << std::endl;

			//bを一回探すのを繰り返す
			next_pos = -1;
			ch_v.clear();
			outer_list.clear();
			while (next_pos < (int)rule.second.size()) {//最後に調べた場所がendかどうかチェック
				next_pos += 1;//次の位置が戻ってきたときの最後に調べた位置になる
				if ((rule.second.begin() + next_pos) == rule.second.end()) {
					it = rule.second.end();
					break;//次に調べる場所がendだったらbreak;
				}
				it = rule.second.begin() + next_pos;//見つかった場所から一つ進めてまた探索

				//aを一回探す
				//ch_vの処理,outer_list処理.
				if ((*it).ch.front() != 1) {
					ch_v.push_back((*it).ch.front());
					outer_list.push_back(it - rule.second.begin());
				}

				if ((*it) == b) {
						// target_f=true;
					if ((*it).ch.front() == 1) {//これがcにかわるから確実に複数入っているから1でもいれる
						ch_v.push_back(1);
						outer_list.push_back(it - rule.second.begin());
					}
					for (int o_el : outer_list) {
						// std::cout << "ADDING: [" << o_el << "]" << std::endl;
						(*(rule.second.begin() + o_el)).ch.front()++;//ひとつしかch持ってない想定
					}
					for (int& obj : ch_v) {
						obj++;
					}
					//aが見つかった時の内側の処理を行う．aのchの処理はしなくてよい．outer_listで処理するから
					int num = (*it).ch.front() - 1, it_id = it - rule.second.begin();//beginからの絶対値取得
					Element new_e;
					new_e = e;
					e_in_res.clear();

					//aをcへ
					(*it).obj = c.obj;

					//outerをカウントしてposになったら，
					//rule.second.begin()からの位置insert_posを決定，
					//そしてsize分outerとそれに連なるinnerをとって
					//（rule.secondからは削除してほかの部分に移すeraseを使ってitの更新をしてもよい．iteratorは取っておいてない）
					//dのchを決定とともにdが含む挿入部分の完成，
					//insert_posに挿入部分を組み込む
					int outer_c = 0, inner_c;
					bool inner_flag = false;
					int insert_pos = 0, er_num = 0;
					//最初はindexなのでスキップ
					it++;
					// std::cout << "\n****************test check49b it=\"" << (*(it-1)).to_s() << "(" << (*(it-1)).ch.front() << ") " << (it == rule.second.end()) << "\"" << std::endl;
					outer_c++;
					// std::cout << "\n****************test check49.5b it_id=" << it - rule.second.begin() - 1 << " it=" << (it - rule.second.begin()) << " num=" << num << " er_num=" << er_num << std::endl;
					while (it != rule.second.begin() + it_id + num - er_num) {
						// std::cout << "\n****************test check50b" << std::endl;
						int it_ch = (*it).ch.front();
						// std::cout << "\n****************test check46b" << std::endl;
						if (!inner_flag && outer_c == pos) {
							inner_c = 0;
							inner_flag = true;
							insert_pos = it - rule.second.begin();
							if (inner_c == size) {
								break;
							}
						}
						if (inner_flag) {
							int itr_num = (*it).ch.front();
							for (int j = 0; j < itr_num; j++) {
								auto r = *it;
								e_in_res.push_back(r);
								it = rule.second.erase(it);
								er_num++;
							}

							inner_c++;
							if (inner_c == size) {
								break;
							}
							//eraseによって次の値になってるからitの移動はいらない
						}
						else {
							it += it_ch;
							outer_c++;
						}
					}
					//最後に追加するパターン
					if (insert_pos == 0) {
						insert_pos = next_pos + 1;
					}

					if(e_in_res.size()>0){
						new_e.set_ch(e_in_res.size() + 1);
					}

					//挿入する部分の完成
					e_in_res.insert(e_in_res.begin(), new_e);

					//適切な位置への挿入
					rule.second.insert(rule.second.begin() + insert_pos, e_in_res.begin(), e_in_res.end());
				}

				for (int& obj : ch_v) {
					obj--;
					if (obj == 0) {
						// buffer.push_back(Prefices::RPRN);
						outer_list.pop_back();
					}
				}
				boost::remove_erase_if(ch_v, [](int obj) { return obj == 0; });
			}
		}
	}
	else if (type == 2) {
		if (e_v == null_v) {
			c_v = mapping[a.obj] - d_v;
			if (c_v != mapping[b.obj] - e_v) {
				std::cerr << "[chunk2]MATCHING ERROR [chunk2 semantics]" << std::endl;
				throw "[chunk2]MATCHING ERROR [chunk2 semantics]";
			}
			mapping[c.obj] = c_v;
			mapping[d.obj] = d_v;
		}
		else {
			c_v = mapping[b.obj];
			mapping[c.obj] = c_v;
			mapping[d.obj] = d_v;
		}
		//b=>c
		rewrite(b, c);
		//a=>cd
		std::vector<Element> d_in_res;

		std::vector<int> outer_list;
		std::vector<int> ch_v;
		int next_pos;

		int size;
		for (auto& rule : rules) {
			std::vector<Element>::iterator it;
			it = rule.second.begin();//itをイテレータから整数へ，順番は保存されているからdの位置を調べる→ループ構造変更
			size = d_size;

			//aを一回探すのを繰り返す
			next_pos = -1;
			ch_v.clear();
			outer_list.clear();
			while (next_pos < (int)rule.second.size()) {//最後に調べた場所がendかどうかチェック
				next_pos += 1;//次の位置が戻ってきたときの最後に調べた位置になる
				if ((rule.second.begin() + next_pos) == rule.second.end()) {
					it = rule.second.end();
					break;//次に調べる場所がendだったらbreak;
				}
				it = rule.second.begin() + next_pos;//見つかった場所から一つ進めてまた探索

				//aを一回探す
				//while (it != rule.second.end()) {

				//ch_vの処理,outer_list処理.
				if ((*it).ch.front() != 1) {
					ch_v.push_back((*it).ch.front());
					outer_list.push_back(it - rule.second.begin());
				}

				if ((*it) == a) {
						// target_f=true;
					if ((*it).ch.front() == 1) {//これがcにかわるから確実に複数入っているから1でもいれる
						ch_v.push_back(1);
						outer_list.push_back(it - rule.second.begin());
					}
					for (int o_el : outer_list) {
						// std::cout << "ADDING: [" << o_el << "]" << std::endl;
						(*(rule.second.begin() + o_el)).ch.front()++;//ひとつしかch持ってない想定
					}
					for (int& obj : ch_v) {
						obj++;
					}
					//aが見つかった時の内側の処理を行う．aのchの処理はしなくてよい．outer_listで処理するから
					int num = (*it).ch.front() - 1, it_id = it - rule.second.begin();//beginからの絶対値取得
					Element new_d;
					new_d = d;
					d_in_res.clear();

					//aをcへ
					(*it).obj = c.obj;

					//outerをカウントしてposになったら，
					//rule.second.begin()からの位置insert_posを決定，
					//そしてsize分outerとそれに連なるinnerをとって
					//（rule.secondからは削除してほかの部分に移すeraseを使ってitの更新をしてもよい．iteratorは取っておいてない）
					//dのchを決定とともにdが含む挿入部分の完成，
					//insert_posに挿入部分を組み込む
					int outer_c = 0, inner_c;
					bool inner_flag = false;
					int insert_pos = 0, er_num = 0;
					//最初はindexなのでスキップ
					it++;
					// std::cout << "\n****************test check49c it=\"" << (*(it-1)).to_s() << "(" << (*(it-1)).ch.front() << ") " << (it == rule.second.end()) << "\"" << std::endl;
					outer_c++;
					
					while (it != rule.second.begin() + it_id + num - er_num) {
						// std::cout << "\n****************test check50c" << std::endl;
						int it_ch = (*it).ch.front();
						// std::cout << "\n****************test check46c" << std::endl;
						if (!inner_flag && outer_c == pos) {
							inner_c = 0;
							inner_flag = true;
							insert_pos = it - rule.second.begin();
							if (inner_c == size) {
								break;
							}
						}
						if (inner_flag) {
							int itr_num = (*it).ch.front();
							for (int j = 0; j < itr_num; j++) {
								auto r = *it;
								d_in_res.push_back(r);
								it = rule.second.erase(it);
								er_num++;
							}

							inner_c++;
							if (inner_c == size) {
								break;
							}
							//eraseによって次の値になってるからitの移動はいらない
						}
						else {
							it += it_ch;
							outer_c++;
						}
					}
					//最後に追加するパターン
					if (insert_pos == 0) {
						insert_pos = next_pos + 1;
					}

					if(d_in_res.size()>0){
						new_d.set_ch(d_in_res.size() + 1);
					}

					//挿入する部分の完成
					d_in_res.insert(d_in_res.begin(), new_d);

					//適切な位置への挿入
					rule.second.insert(rule.second.begin() + insert_pos, d_in_res.begin(), d_in_res.end());
				}

				for (int& obj : ch_v) {
					obj--;
					if (obj == 0) {
						// buffer.push_back(Prefices::RPRN);
						outer_list.pop_back();
					}
				}
				boost::remove_erase_if(ch_v, [](int obj) { return obj == 0; });
			}
		}
	}
	else {
		std::cerr << "UNKNOWN chunk TYPE" << std::endl;
		throw "UNKNOWN chunk TYPE";
	}
	mapping.erase(a.obj);
	mapping.erase(b.obj);
	// std::cout << "\n****************test check2 end" << std::endl;

	// std::cout << rules_to_s() << std::endl;

}
template <typename T>
void Semantics<T>::merge(Element a, Element b, Element c) {
	if (!(a.is_ind() && b.is_ind() && c.is_ind())) {
		std::cerr << "[merge]INCORRECT ELEMENT" << std::endl;
		throw "[merge]INCORRECT ELEMENT";
	}
	T c_v;
	c_v = mapping[a.obj] + mapping[b.obj];
	mapping[c.obj] = c_v;
	//mapping.count(a.obj)!=0とかで分岐
	//a=>c, b=>c
	// std::vector<int> new_tr_a, new_tr_b;
	if (a.obj == c.obj) {
		if (b.obj == c.obj) {//a==c, b==c
		  //nop
		}
		else {//b->c
		 // new_tr_b.push_back(c.obj);
		 // rules[b.obj] = new_tr_b;
			rewrite(b, c);
			if (merge_list.count(c.obj) == 0) {
				merge_list.insert(std::make_pair(c.obj, std::set<int>({ b.obj })));
			}
			else {
				merge_list[c.obj].insert(b.obj);
			}
			mapping.erase(b.obj);
		}
	}
	else {
		if (b.obj == c.obj) {//a->c
		  // new_tr_a.push_back(c.obj);
		  // rules[a.obj] = new_tr_a;
			rewrite(a, c);
			if (merge_list.count(c.obj) == 0) {
				merge_list.insert(std::make_pair(c.obj, std::set<int>({ a.obj })));
			}
			else {
				merge_list[c.obj].insert(a.obj);
			}
			mapping.erase(a.obj);
		}
		else {//a->c,b->c
		 // new_tr_a.push_back(c.obj);
		 // new_tr_b.push_back(c.obj);
		 // rules[a.obj] = new_tr_a;
		 // rules[b.obj] = new_tr_b;
			rewrite(a, c);
			rewrite(b, c);
			if (merge_list.count(c.obj) == 0) {
				merge_list.insert(std::make_pair(c.obj, std::set<int>({ a.obj,b.obj })));
			}
			else {
				merge_list[c.obj].insert(a.obj);
				merge_list[c.obj].insert(b.obj);
			}
			mapping.erase(a.obj);
			mapping.erase(b.obj);
		}
	}
}
template <typename T>
void Semantics<T>::replace(Element a, Element b, Element c, int b_pos, int b_size) {
	if (!(a.is_ind() && b.is_ind() && c.is_ind())) {
		std::cerr << "[replace]INCORRECT ELEMENT" << std::endl;
		throw "[replace]INCORRECT ELEMENT";
	}

	// std::cout << "SEMANTICS replace" << std::endl;
	// std::cout << a.to_s() << " => " << c.to_s() << " " << b.to_s() << " pos=" << b_pos << " b_size=" << b_size << std::endl;
	// std::cout << rules_to_s() << std::endl;

	T c_v;
	c_v = mapping[a.obj] - mapping[b.obj];
	mapping[c.obj] = c_v;
	//a=>cb
	std::vector<Element> b_in_res;

	std::vector<int> outer_list;
	std::vector<int> ch_v;
	int next_pos;

	// std::cout << "\n****************test check30" << std::endl;

	int pos, size;
	for (auto& rule : rules) {

		// std::cout << "\n****************test check31" << std::endl;

		std::vector<Element>::iterator it;
		pos = b_pos;
		size = b_size;

		//aを一回探すのを繰り返す
		next_pos = -1;
		ch_v.clear();
		outer_list.clear();
		while (next_pos < (int)rule.second.size()) {//最後に調べた場所がendかどうかチェック
			next_pos += 1;//次の位置が戻ってきたときの最後に調べた位置になる
			if ((rule.second.begin() + next_pos) == rule.second.end()) {
				it = rule.second.end();
				break;//次に調べる場所がendだったらbreak;
			}
			it = rule.second.begin() + next_pos;//見つかった場所から一つ進めてまた探索

			//aを一回探す
			//while (it != rule.second.end()) {

			// std::cout << "\n****************test check42" << std::endl;

			//ch_vの処理,outer_list処理.
			if ((*it).ch.front() != 1) {
				ch_v.push_back((*it).ch.front());
				outer_list.push_back(it - rule.second.begin());
			}

			// std::cout << "\n****************test check43" << std::endl;

			if ((*it) == a) {
					// target_f=true;
				if ((*it).ch.front() == 1) {//これがcにかわるから確実に複数入っているから1でもいれる
					ch_v.push_back(1);
					outer_list.push_back(it - rule.second.begin());
				}
				for (int o_el : outer_list) {
					// std::cout << "ADDING: [" << o_el << "]" << std::endl;
					(*(rule.second.begin() + o_el)).ch.front()++;//ひとつしかch持ってない想定
				}
				for (int& obj : ch_v) {
					obj++;
				}
				//aが見つかった時の内側の処理を行う．aのchの処理はしなくてよい．outer_listで処理するから
				int num = (*it).ch.front() - 1, it_id = it - rule.second.begin();//beginからの絶対値取得
				Element new_b;
				new_b = b;
				b_in_res.clear();

				//aをcへ
				(*it).obj = c.obj;

				// std::cout << "\n****************test check44" << std::endl;

				//outerをカウントしてposになったら，
				//rule.second.begin()からの位置insert_posを決定，
				//そしてsize分outerとそれに連なるinnerをとって
				//（rule.secondからは削除してほかの部分に移すeraseを使ってitの更新をしてもよい．iteratorは取っておいてない）
				//dのchを決定とともにdが含む挿入部分の完成，
				//insert_posに挿入部分を組み込む
				int outer_c = 0, inner_c;
				bool inner_flag = false;
				int insert_pos = 0, er_num = 0;
				//最初はindexなのでスキップ
				it++;
				// std::cout << "\n****************test check49 it=\"" << (*(it-1)).to_s() << "(" << (*(it-1)).ch.front() << ") " << (it == rule.second.end()) << "\"" << std::endl;
				outer_c++;
					
				while (it != rule.second.begin() + it_id + num - er_num) {
					// std::cout << "\n****************test check50" << std::endl;
					int it_ch = (*it).ch.front();
					// std::cout << "\n****************test check46" << std::endl;
					if (!inner_flag && outer_c == pos) {
						inner_c = 0;
						inner_flag = true;
						insert_pos = it - rule.second.begin();
						if (inner_c == size) {
							break;
						}
					}
					// std::cout << "\n****************test check47" << std::endl;
					if (inner_flag) {
						int itr_num = (*it).ch.front();
						for (int j = 0; j < itr_num; j++) {
							auto r = *it;
							b_in_res.push_back(r);
							it = rule.second.erase(it);
							er_num++;
						}

						inner_c++;
						if (inner_c == size) {
							break;
						}
						//eraseによって次の値になってるからitの移動はいらない
					}
					else {
						// std::cout << "\n****************test check48" << std::endl;
						it += it_ch;
						outer_c++;
					}
				}
				//最後に追加するパターン
				if (insert_pos == 0) {
					insert_pos = next_pos + 1;
				}

				// std::cout << "\n****************test check45" << std::endl;

				if(b_in_res.size()>0){
					new_b.set_ch(b_in_res.size() + 1);
				}

				//挿入する部分の完成
				b_in_res.insert(b_in_res.begin(), new_b);

				//適切な位置への挿入
				rule.second.insert(rule.second.begin() + insert_pos, b_in_res.begin(), b_in_res.end());
			}

			for (int& obj : ch_v) {
				obj--;
				if (obj == 0) {
					// buffer.push_back(Prefices::RPRN);
					outer_list.pop_back();
				}
			}
			boost::remove_erase_if(ch_v, [](int obj) { return obj == 0; });
		}
	}

	// std::cout << rules_to_s() << std::endl;
	mapping.erase(a.obj);

}
//free A
template <typename T>//渡されたのを削る
void Semantics<T>::unique_unify(Element a, std::vector<Element> v_e) {
	if (!(a.is_ind())) {
		std::cerr << "[unique unify]INCORRECT ELEMENT" << std::endl;
		throw "[unique unify]INCORRECT ELEMENT";
	}
	//aに統合する
	for (auto& b : v_e) {
		if (!(b.is_ind())) {
			std::cerr << "[unique unify]INCORRECT ELEMENT in vector" << std::endl;
			throw "[unique unify]INCORRECT ELEMENT in vector";
		}
		T b_v;
		b_v = mapping[b.obj] + mapping[a.obj];
		mapping[b.obj] = b_v;
	}

	// std::cerr << "Unify to a(" << a.obj << ")" << std::endl;

	//ルールの変更aを取り除いてouterを1減らす
	std::vector<Element> b_in_res;
	std::vector<int> outer_list;
	std::vector<int> ch_v;
	int next_pos;
	int pos, size;
	for (auto& rule : rules) {
		std::vector<Element>::iterator it;
		pos = 1;
		size = 1;

		//aを一回探すのを繰り返す
		next_pos = -1;
		while (next_pos < (int)rule.second.size()) {//最後に調べた場所がendかどうかチェック
			next_pos += 1;//次の位置が戻ってきたときの最後に調べた位置になる
			if ((rule.second.begin() + next_pos) == rule.second.end()) {
				it = rule.second.end();
				break;//次に調べる場所がendだったらbreak;
			}
			it = rule.second.begin() + next_pos;//見つかった場所から一つ進めてまた探索

			ch_v.clear();
			outer_list.clear();
			//aを一回探す
			while (it != rule.second.end()) {
				//ch_vの処理,outer_list処理.これはtarget_fがfalseの間だけ処理をする
				if ((*it).ch.front() != 1) {
					ch_v.push_back((*it).ch.front());
					outer_list.push_back(it - rule.second.begin());
				}

				if ((*it) == a) {
					if ((*it).ch.front() == 1) {//これでaの分がpushされてないことはなくなる
						ch_v.push_back(1);
						outer_list.push_back(it - rule.second.begin());
					}
					outer_list.pop_back();//aの部分は削除するので絶対値参照したときに違う値になっているのでaの分はpop_backする

					//aを削除
					it = rule.second.erase(it);

					//aが見つかった時の内側の処理を行う．aのchの処理はしなくてよい．outer_listで処理するから
					int num = (*it).ch.front(), it_id = it - rule.second.begin();//beginからの絶対値取得//内部が始まる位置はずれる
					b_in_res.clear();

					//outerをカウントしてposになったら，
					//rule.second.begin()からの位置insert_posを決定，
					//そしてsize分outerとそれに連なるinnerをとって
					//（rule.secondからは削除してほかの部分に移すeraseを使ってitの更新をしてもよい．iteratorは取っておいてない）
					//dのchを決定とともにdが含む挿入部分の完成，
					//insert_posに挿入部分を組み込む
					int outer_c = 0, inner_c;
					bool inner_flag = false;
					int insert_pos = 0, er_num = 0;
					//最初はindexなのでスキップ
					it++;
					outer_c++;
					while (it != rule.second.begin() + it_id + num - er_num) {
						int it_ch = (*it).ch.front();
						if (!inner_flag && outer_c == pos) {
							inner_c = 0;
							inner_flag = true;
							insert_pos = it - rule.second.begin();
						}
						if (inner_flag) {
							int itr_num = (*it).ch.front();
							for (int j = 0; j < itr_num; j++) {
								auto r = *it;
								b_in_res.push_back(r);
								it = rule.second.erase(it);
								er_num++;
							}

							inner_c++;
							if (inner_c == size) {
								break;
							}
							//eraseによって次の値になってるからitの移動はいらない
						}
						else {
							it += it_ch;
							outer_c++;
						}
					}

					//この場合はあり得ない
					//一番最後に追加するパターン
					// if(insert_pos==0&&outer_c==pos){
					//   if(size!=0){
					//     std::cerr << "ALERT" << std::endl;
					//   }
					//   it = rule.second.insert(it,new_b);
					//   break;
					// }

					//挿入なし
					//挿入する部分の完成
					// new_b.set_ch(b_in_res.size() + 1);
					// b_in_res.insert(b_in_res.begin(),new_b);

					//適切な位置への挿入
					it = rule.second.insert(rule.second.begin() + insert_pos, b_in_res.begin(), b_in_res.end());

					break;
				}


				for (int& obj : ch_v) {
					obj--;
					if (obj == 0) {
						// buffer.push_back(Prefices::RPRN);
						outer_list.pop_back();
					}
				}
				boost::remove_erase_if(ch_v, [](int obj) { return obj == 0; });

				it++;
				next_pos = it - rule.second.begin();
			}

			//itの位置でouter_listをリセットする
			if (it == rule.second.end()) {
				outer_list.clear();
			}

			for (int o_el : outer_list) {
				(*(rule.second.begin() + o_el)).ch.front()--;//ひとつしかch持ってない想定
			}
		}
	}
}
template <typename T>
std::vector<Element> Semantics<T>::trans(Element a) {
	if (!a.is_ind()) {
		std::cerr << "[trans]INCORRECT ELEMENT" << std::endl;
		throw "[trans]INCORRECT ELEMENT";
	}
	// std::vector<Element> internal_meaning;
	// CompVec cv_a;
	// cv_a.push_back(a.obj);
	// internal_meaning=sub_trans(cv_a);
	return rules[a.obj];
}
// template <typename T>
// std::vector<Element> Semantics<T>::sub_trans(CompVec a){
//   std::vector<Element> internal_meaning;
//   std::vector<int> v;
//   if(rules.count(a)==0){
//     int loc=merge_trans(a);

//     if(rules.count(loc) == 0){
//       Element el;
//       el.set_ind(loc);
//       internal_meaning.push_back(el);
//       return internal_meaning;
//       // std::cerr << "[sub trans]INCORRECT NUMBER no rule :" << loc << std::endl;
//       // throw "[sub trans]INCORRECT NUMBER no rule";
//     }
//     v=rules[loc];
//   }else{
//     v=rules[a];
//   }
//   std::vector<int>::iterator it = v.begin();
//   bool first = true;
//   for(;it != v.end();it++){
//     if(rules.count(*it) == 0){
//       Element el;
//       el.set_ind(*it);
//       internal_meaning.push_back(el);
//     }else{
//       std::vector<Element> sub_v = sub_trans(*it);
//       if(!first) sub_v.front().set_ch(sub_v.size());
//       internal_meaning.insert(internal_meaning.end(),sub_v.begin(),sub_v.end());
//     }
//     if(first) first = false;
//   }
//   return internal_meaning;
// }
template <typename T>
void Semantics<T>::rewrite(Element a, Element b) {
	for (auto& p_int_vec : rules) {
		for (auto& el : p_int_vec.second) {
			if (el.is_ind() && el.obj == a.obj) {
				el.obj = b.obj;
			}
		}
		//   if(p_int_vec.first == a.obj){
		//     std::pair<int, std::vector<Element> > p = std::make_pair(b.obj,p_int_vec.second);
		//     for(auto& el : p.second){
		//       if(el == a){
		//         el = b;
		//       }
		//     }
		//     st.push_back(p);
		//   }else{
		//     for(auto& el : p_int_vec.second){
		//       if(el == a){
		//         el = b;
		//       }
		//     }
		//   }
		// }
		// if(st.size()>0){
		//   rules.erase(a);
		//   for(auto& obj : st){
		//     rules.insert(obj);
		//   }
	}
}
template <typename T>
int Semantics<T>::merge_trans(int a) {
	bool flag = true;
	int loc = a;
	while (flag) {
		flag = false;
		for (auto& p_int_s : merge_list) {
			if (p_int_s.second.count(loc) != 0) {
				loc = p_int_s.first;
				flag = true;
			}
		}
	}
	return loc;
}
template <typename T>
Semantics<T> Semantics<T>::copy(void) {
	Semantics<T> dst;
	dst.mapping = mapping;
	dst.rules = rules;
	return dst;
}
template <typename T>
void Semantics<T>::clear(void) {
	mapping.clear();
	rules.clear();
}
template <typename T>
std::string Semantics<T>::to_s(void) {
	std::string res("###Semantics static information###\n");
	res += rules_to_s();
	res += merge_list_to_s();
	res += mapping_to_s();
	return res;
}
template <typename T>
std::string Semantics<T>::rules_to_s(void) {
	// std::cerr << "rules_to_s" << std::endl;
	std::string res("SUPPLEMENTARY RULES:\n");
	TransRules::iterator it = rules.begin();
	for (; it != rules.end(); it++) {
		res += std::to_string((*it).first);
		res += " -->";
		std::vector<Element> sec_v = (*it).second;
		std::vector<Element>::iterator s_it = sec_v.begin();
		for (; s_it != sec_v.end(); s_it++) {
			res += " " + (*s_it).to_s() + std::string("(") + std::to_string((*s_it).ch.front()) + std::string(")");
		}
		res += "\n";
	}
	return res;
}
template <typename T>
std::string Semantics<T>::merge_list_to_s(void) {
	// std::cerr << "rules_to_s" << std::endl;
	std::string res("MERGE LIST:\n");
	std::map<int, std::set<int> >::iterator it = merge_list.begin();
	for (; it != merge_list.end(); it++) {
		res += std::to_string((*it).first) + " --> {";
		std::set<int> sec_v = (*it).second;
		std::set<int>::iterator s_it = sec_v.begin();
		for (; s_it != sec_v.end(); s_it++) {
			res += " " + std::to_string((*s_it));
		}
		res += " },";
	}
	res.pop_back();
	res += "\n";
	return res;
}
template <typename T>
std::string Semantics<T>::mapping_to_s(void) {
	std::string res("VALID INDEX and MEANINGS:\n");
	for (auto& p : mapping) {
		if (!p.second.empty()) {
			res += "{" + Prefices::IND + std::to_string(p.first) + Prefices::CLN + "[" + p.second.to_s() + "]},";
		}
	}
	res.pop_back();
	res += "\n";
	return res;
}
template <typename T>
void Semantics<T>::init_rules(TransRules& obj) {
	rules = obj;
}
// void init_rules(std::map<int,std::vector<int> >& obj){
//   TransRules res;
//   for(auto& m : obj){
//     CompVec i;
//     i.vec_int.push_back(m.first);
//     res.insert(std::make_pair(i,m.second));
//   }
// }

//明示的な実在化
template class Semantics<Conception>;