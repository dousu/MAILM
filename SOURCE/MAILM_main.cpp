/*
 * File:   MAILM_main.cpp
 * Author: hiroki
 *
 * Created on October 28, 2016, 6:12 PM
 */

#include "MAILM_main.h"

void
make_symbols(
	std::vector<std::pair<std::string, std::string > >& data,
	Rule& src,
	XMLreader& reader,
	Dictionary& dic
) {
	src.external.clear();
	std::vector<
		std::pair<
		std::string,
		std::string
		>
	>::iterator it = data.begin();
	for (; it != data.end(); it++) {
		Element tmp;
		tmp.set_sym(
			dic.conv_symbol[
				reader.alias[
					(*it).second
				]
			]
		);
		src.external.push_back(tmp);
	}
}

void
load_input_data(
	std::vector<std::string>& buf,
	std::string& file_path
) {
	//存在しない場合，中身がない場合はすぐreturn 
	const boost::filesystem::path path(file_path.c_str());
	boost::system::error_code error;
	const bool result =
		boost::filesystem::exists(path, error);
	if (!result || error) {
		return;
	}
	else {
		std::string line;
		std::ifstream ifs(file_path.c_str());
		while (std::getline(ifs, line)) {
			boost::algorithm::trim_if(
				line,
				boost::algorithm::is_any_of("\r\n ")
			);
			// std::cout << line << std::endl;
			buf.push_back(line);
		}
	}
}

void output_data(
	std::string file_path,
	std::string data
) {
	//存在しない場合，中身がない場合はすぐreturn
	const boost::filesystem::path path(file_path.c_str());
	boost::system::error_code error;
	const bool result =
		boost::filesystem::exists(path, error);
	if (!result || error) {
		std::cout << file_path << " file not found" << std::endl;
		std::cout << data << std::endl;
		return;
	}
	else {
		std::ofstream ofs(file_path.c_str());
		ofs << data;
	}
}

void registration(
	std::vector<Rule>& buf,
	KnowledgeBase& kb
) {
	std::vector<Rule>::iterator buf_it = buf.begin();
	for (; buf_it != buf.end(); buf_it++) {

		// std::cout << (*buf_it).to_s() << std::endl;

		if ((*buf_it).type == RULE_TYPE::MEASURE) {
			Conception ss;
			ss.add("MEASURE");
			kb.define((*buf_it).internal.front(), ss);
			(*buf_it).type = RULE_TYPE::NOUN;
		}
		else if ((*buf_it).type == RULE_TYPE::SENTENCE) {
			Conception ss;
			ss.add("SENTENCE");
			kb.define((*buf_it).internal.front(), ss);
		}
		else if ((*buf_it).type == RULE_TYPE::NOUN) {
			Conception ss;
			//ss.add("");
			kb.define((*buf_it).internal.front(), ss);
		}
		else {
			std::cerr << "CANNOT MATCH RULE_TYPE" << std::endl;
			throw "CANNOT MATCH RULE_TYPE";
		}
	}
}

void registration(
	std::map<int, std::vector<std::string> >& core_meaning,
	KnowledgeBase& kb
) {
	for (auto& p_i_str_v : core_meaning) {
		Element ind;
		ind.set_ind(p_i_str_v.first);
		Conception ss;
		for (auto& str : p_i_str_v.second) {
			ss.add(str);
		}
		kb.define(ind, ss);
	}
}

void
initial_rules(
	std::map<int, std::vector<std::string> > i_rules_str,
	TransRules& i_rules
) {
	for (auto& p_int_vecstr : i_rules_str) {
		std::vector<Element> buf;
		for (auto& str : p_int_vecstr.second) {
			buf.push_back(Element(str));
		}

		// Rule r;
		// r.internal = buf;
		// r.type = RULE_TYPE::SENTENCE;
		// std::cout << "INIT_RULES " << p_int_vecstr.first <<": " << r.to_s() << std::endl;

		i_rules[p_int_vecstr.first] = buf;
	}

}

// std::string
// make_tree_str_for_dot(std::vector<Rule>& r_list, std::vector<int> beat_nums, int code_num){
//     std::string begin_str("digraph sample{\n"), end_str("}");
//     std::vector<Rule>::iterator ls_it;
//     ls_it=r_list.begin();
//     std::map<int, int> memo;
//     std::map<std::string, int> memo2;
//     std::vector<std::string> stack;
//     std::vector<std::vector<std::string> > vis;
//     int val=0;
//     std::multimap<int, std::string> r_int_to_str;
//     std::map<std::string, int> r_str_to_int;
//     std::vector<std::string> smp, code_v;
//     int vis_index = 0;
//     std::string top;
//     {
//         memo.clear();
//         memo2.clear();
//         stack.clear();
//         vis.clear();
//         vis.push_back(std::vector<std::string>());
//         r_int_to_str.clear();
//         r_str_to_int.clear();
//         smp.clear();
//         code_v.clear();
//         top = std::string("\"")+Prefices::SEN/*+std::to_string((*r_it).front().cat)*/+std::string("\\nI")+std::to_string((*ls_it).internal.front().obj)+std::string("\"");
//         stack.push_back(top);
//         r_int_to_str.insert(std::multimap<int,std::string>::value_type(0,top));
//         r_str_to_int.insert(std::map<std::string,int>::value_type(top,0));
//         ls_it=r_list.begin();
//         for(;ls_it!=r_list.end();ls_it++){
//             std::string frm = stack.front();
//             std::vector<std::string> mini_stack;
//             KnowledgeBase::ExType::iterator g_ex_it = (*ls_it).external.begin();
//             for(;g_ex_it!=(*ls_it).external.end();g_ex_it++){
//                 if((*g_ex_it).is_cat()){
//                     if(memo.find((*g_ex_it).cat) == memo.end()){
//                         memo.insert(std::map<int,int>::value_type((*g_ex_it).cat,1));
//                     }else{
//                         memo[(*g_ex_it).cat] += 1;
//                     }

//                     // mini_stack.push_back(std::string("\"") + std::to_string(memo[(*g_ex_it).cat]) + std::string(" ")　+ Prefices::CAT + std::to_string((*g_ex_it).cat) + std::string("\""));
//                     mini_stack.push_back(Prefices::CAT + std::to_string((*g_ex_it).cat) + Prefices::UNO + std::to_string(memo[(*g_ex_it).cat]));

//                     r_int_to_str.insert(std::multimap<int,std::string>::value_type(r_str_to_int[frm] + 1,mini_stack.back()));
//                     r_str_to_int.insert(std::map<std::string,int>::value_type(mini_stack.back(),r_str_to_int[frm] + 1));

//                     smp.push_back(frm + std::string(" -> ") + mini_stack.back());

//                 }else{
//                     if(memo2.find((*g_ex_it).to_s()) == memo2.end()){
//                         memo2.insert(std::map<std::string,int>::value_type((*g_ex_it).to_s(),1));
//                     }else{
//                         memo2[(*g_ex_it).to_s()] += 1;
//                     }

//                     std::string terminal_str;
//                     // terminal_str = std::string("\"") + std::to_string(memo2[(*g_ex_it).to_s()]) + std::string(" ") + (*g_ex_it).to_s() + std::string("\"")
//                     terminal_str = std::string("\"") + (*g_ex_it).to_s() + Prefices::UNO + std::to_string(memo2[(*g_ex_it).to_s()]) + std::string("\"");

//                     {
//                         if(vis[vis_index].size() == beat_nums[vis_index]){
//                             vis_index += 1;
//                             std::vector<std::string> emp_vis_el;
//                             vis.push_back(emp_vis_el);
//                         }
//                         vis[vis_index].push_back(terminal_str);
//                     }

//                     smp.push_back(frm + std::string(" -> ") + terminal_str);
//                 }
//             }
//             stack.erase(stack.begin());
//             stack.insert(stack.begin(),mini_stack.begin(),mini_stack.end());
//         }

//         for(int i = 0; i < vis.size(); i++){
//             std::string graph_node;
//             graph_node = std::string("c") + std::to_string(i+1);
//             code_v.push_back(graph_node);
//             for(auto& trg : vis[i]){
//                 smp.push_back(trg + std::string(" -> ") + graph_node);
//             }
//         }
//         for(int i = 0; i < vis.size(); i++){
//             smp.push_back(std::string("c") + std::to_string(i+1) + std::string("[image=\"") + std::string("./graph") + std::to_string(code_num) + Prefices::DEL + std::to_string(i+1) + std::string(".png\"]"));
//         }
//     }

//     //rankルール生成

//     //レベルの最大値検索
//     int upper_bound = 0;
//     std::multimap<int, std::string>::iterator itr=r_int_to_str.begin();
//     upper_bound = (*itr).first;
//     while((itr = r_int_to_str.upper_bound((*itr).first) )!=r_int_to_str.end()){
//         upper_bound = (*itr).first;
//     }

//     //terminal symbolのrank生成(仮としてmemo2)
//     std::string t_rank;
//     std::map<std::string,int>::iterator str_to_int_it;
//     t_rank = std::string("{rank = same;");
//     str_to_int_it = memo2.begin();
//     for(;str_to_int_it!=memo2.end();str_to_int_it++){
//         for(int i = 1; i <= (*str_to_int_it).second; i++){
//             // t_rank += (std::string(" \"") + std::to_string(i) + std::string(" ") + (*str_to_int_it).first + std::string("\";"));
//             t_rank += (std::string(" \"") + (*str_to_int_it).first + Prefices::UNO + std::to_string(i) + std::string("\";"));
//         }
//     }
//     t_rank += std::string("}");

//     //start symbol
//     std::vector<std::string> nt_ranks;
//     nt_ranks.push_back(std::string("{rank = min; ")+top+std::string(";}"));

//     //non-terminal symbol
//     for(int i=1;i<=upper_bound;i++){
//         auto ra = r_int_to_str.equal_range(i);
//         std::string nt_rank("{rank = same;");
//         while(ra.first != ra.second){
//             nt_rank += (std::string(" ") + (*ra.first).second + std::string(";"));
//             ra.first++;
//         }
//         nt_rank += std::string("}");
//         nt_ranks.push_back(nt_rank);
//     }

//     std::string c_rank;
//     std::vector<std::string>::iterator str_v_it;
//     c_rank = std::string("{rank = max;");
//     str_v_it = code_v.begin();
//     for(;str_v_it!=code_v.end();str_v_it++){
//         c_rank += (std::string(" \"") + *str_v_it + std::string("\";"));
//     }
//     c_rank += std::string("}");


//     //smp処理
//     std::string tmp = begin_str;
//     tmp += boost::algorithm::join(smp, ";\n");
//     tmp += std::string(";\n");
//     tmp += c_rank;
//     tmp += std::string("\n");
//     tmp += boost::algorithm::join(nt_ranks, "\n");
//     tmp += std::string("\n");
//     tmp += t_rank;
//     tmp += std::string("\n");
//     tmp += end_str;

//     return tmp;
// }

std::string
make_tree_str_for_dot(std::vector<Rule>& r_list, std::vector<int> beat_nums, int code_num, KnowledgeBase& kb) {
	std::string begin_str("digraph sample{\n"), end_str("}");
	std::vector<Rule>::iterator ls_it;
	ls_it = r_list.begin();
	std::map<int, int> memo;
	std::map<std::string, int> memo2;
	std::vector<std::pair<std::string, int> > stack;
	std::vector<std::vector<std::string> > vis;
	int val = 0;
	std::multimap<int, std::string> r_int_to_str;
	std::map<std::string, int> r_str_to_int;
	std::vector<std::string> smp, code_v;
	int vis_index = 0;
	std::string top;
	{
		memo.clear();
		memo2.clear();
		stack.clear();
		vis.clear();
		vis.push_back(std::vector<std::string>());
		r_int_to_str.clear();
		r_str_to_int.clear();
		smp.clear();
		code_v.clear();
		top = std::string("\"") + Prefices::SEN/*+std::to_string((*r_it).front().cat)*/ + std::string("\\n") + Prefices::IND + std::to_string((*ls_it).internal.front().obj) + std::string("\\n") + kb.meaning_no_to_s((*ls_it).internal.front().obj) + std::string("\"");
		stack.push_back(std::pair<std::string, int>(top, 0));
		r_int_to_str.insert(std::multimap<int, std::string>::value_type(0, top));
		r_str_to_int.insert(std::map<std::string, int>::value_type(top, 0));
		for (; ls_it != r_list.end(); ls_it++) {
			std::string frm = stack.front().first;
			std::string head;
			if (ls_it != r_list.begin()) {
				if (memo.find((*ls_it).cat) == memo.end()) {
					memo.insert(std::map<int, int>::value_type((*ls_it).cat, 1));
				}
				else {
					memo[(*ls_it).cat] += 1;
				}
				head = std::string("\"") + Prefices::CAT + std::to_string((*ls_it).cat) + Prefices::UNO + std::to_string(memo[(*ls_it).cat]) + std::string("\\n") + Prefices::IND + std::to_string((*ls_it).internal.front().obj) + std::string("\\n") + kb.meaning_no_to_s((*ls_it).internal.front().obj) + std::string("\"");
				smp.push_back(frm + std::string(" -> ") + head);
				stack.front().second--;
				if (stack.front().second == 0) {
					stack.erase(stack.begin());
				}
				r_int_to_str.insert(std::multimap<int, std::string>::value_type(r_str_to_int[frm] + 1, head));
				r_str_to_int.insert(std::map<std::string, int>::value_type(head, r_str_to_int[frm] + 1));
				stack.insert(stack.begin(), std::pair<std::string, int>(head, 0));
			}
			else {
				head = frm;
			}

			std::vector<std::string> mini_stack;
			KnowledgeBase::ExType::iterator g_ex_it = (*ls_it).external.begin();
			for (; g_ex_it != (*ls_it).external.end(); g_ex_it++) {
				if ((*g_ex_it).is_cat()) {

					stack.front().second++;

					// if(memo.find((*g_ex_it).cat) == memo.end()){
					//     memo.insert(std::map<int,int>::value_type((*g_ex_it).cat,1));
					// }else{
					//     memo[(*g_ex_it).cat] += 1;
					// }

					// mini_stack.push_back(std::string("\"") + std::to_string(memo[(*g_ex_it).cat]) + std::string(" ")　+ Prefices::CAT + std::to_string((*g_ex_it).cat) + std::string("\""));
					// mini_stack.push_back(Prefices::CAT + std::to_string((*g_ex_it).cat) + Prefices::UNO + std::to_string(memo[(*g_ex_it).cat]));

					// r_int_to_str.insert(std::multimap<int,std::string>::value_type(r_str_to_int[frm] + 1,mini_stack.back()));
					// r_str_to_int.insert(std::map<std::string,int>::value_type(mini_stack.back(),r_str_to_int[frm] + 1));

					// smp.push_back(frm + std::string(" -> ") + mini_stack.back());

				}
				else {
					if (memo2.find((*g_ex_it).to_s()) == memo2.end()) {
						memo2.insert(std::map<std::string, int>::value_type((*g_ex_it).to_s(), 1));
					}
					else {
						memo2[(*g_ex_it).to_s()] += 1;
					}

					std::string terminal_str;
					// terminal_str = std::string("\"") + std::to_string(memo2[(*g_ex_it).to_s()]) + std::string(" ") + (*g_ex_it).to_s() + std::string("\"")
					terminal_str = std::string("\"") + (*g_ex_it).to_s() + Prefices::UNO + std::to_string(memo2[(*g_ex_it).to_s()]) + std::string("\"");

					{
						if (vis[vis_index].size() == beat_nums[vis_index]) {
							vis_index += 1;
							std::vector<std::string> emp_vis_el;
							vis.push_back(emp_vis_el);
						}
						vis[vis_index].push_back(terminal_str);
					}

					smp.push_back(head + std::string(" -> ") + terminal_str);
				}
			}
			if (stack.front().second == 0) {
				stack.erase(stack.begin());
			}
			// stack.insert(stack.begin(),mini_stack.begin(),mini_stack.end());
		}

		for (int i = 0; i < vis.size(); i++) {
			std::string graph_node;
			graph_node = std::string("c") + std::to_string(i + 1);
			code_v.push_back(graph_node);
			for (auto& trg : vis[i]) {
				smp.push_back(trg + std::string(" -> ") + graph_node);
			}
		}
		for (int i = 0; i < vis.size(); i++) {
			smp.push_back(std::string("c") + std::to_string(i + 1) + std::string("[image=\"") + std::string("./graph") + std::to_string(code_num) + Prefices::DEL + std::to_string(i + 1) + std::string(".png\"]"));
		}
	}

	//rankルール生成

	//レベルの最大値検索
	int upper_bound = 0;
	std::multimap<int, std::string>::iterator itr = r_int_to_str.begin();
	upper_bound = (*itr).first;
	while ((itr = r_int_to_str.upper_bound((*itr).first)) != r_int_to_str.end()) {
		upper_bound = (*itr).first;
	}

	//terminal symbolのrank生成(仮としてmemo2)
	std::string t_rank;
	std::map<std::string, int>::iterator str_to_int_it;
	t_rank = std::string("{rank = same;");
	str_to_int_it = memo2.begin();
	for (; str_to_int_it != memo2.end(); str_to_int_it++) {
		for (int i = 1; i <= (*str_to_int_it).second; i++) {
			// t_rank += (std::string(" \"") + std::to_string(i) + std::string(" ") + (*str_to_int_it).first + std::string("\";"));
			t_rank += (std::string(" \"") + (*str_to_int_it).first + Prefices::UNO + std::to_string(i) + std::string("\";"));
		}
	}
	t_rank += std::string("}");

	//start symbol
	std::vector<std::string> nt_ranks;
	nt_ranks.push_back(std::string("{rank = min; ") + top + std::string(";}"));

	//non-terminal symbol
	for (int i = 1; i <= upper_bound; i++) {
		auto ra = r_int_to_str.equal_range(i);
		std::string nt_rank("{rank = same;");
		while (ra.first != ra.second) {
			nt_rank += (std::string(" ") + (*ra.first).second + std::string(";"));
			ra.first++;
		}
		nt_rank += std::string("}");
		nt_ranks.push_back(nt_rank);
	}

	std::string c_rank;
	std::vector<std::string>::iterator str_v_it;
	c_rank = std::string("{rank = max;");
	str_v_it = code_v.begin();
	for (; str_v_it != code_v.end(); str_v_it++) {
		c_rank += (std::string(" \"") + *str_v_it + std::string("\";"));
	}
	c_rank += std::string("}");


	//smp処理
	std::string tmp = begin_str;
	tmp += boost::algorithm::join(smp, ";\n");
	tmp += std::string(";\n");
	tmp += c_rank;
	tmp += std::string("\n");
	tmp += boost::algorithm::join(nt_ranks, "\n");
	tmp += std::string("\n");
	tmp += t_rank;
	tmp += std::string("\n");
	tmp += end_str;

	return tmp;
}

int main(int argc, char* argv[]) {

	/*
	 * ほぼ静的クラスだが、シリアライズのため（Dictionary）と、
	 * デストラクタ実行のため（LogBox）にインスタンスを作成
	*/
	Dictionary dic;
	XMLreader reader;
	LogBox log; //for destructor
	KnowledgeBase kb;

	KnowledgeBase::logging_on();
	LogBox::set_filepath("./test_log.txt");

	std::string xml_file = "../XML/01.xml";
	std::string dic_file = "./data.dic";
	std::string input_file = "./input.txt";
	std::string alias_file = "./alias.data";
	std::string dic_xml_file = "./dic_xml.data";
	std::string xml_dir = "../XML";
	std::string xml_ext = ".xml";
	// std::string meainig_rules_file = "./meaning_rules.data";

	std::vector<std::string> inputs;
	bool test_mode = true;

	std::map<int, std::string> labeling;

	if (!test_mode)
		load_input_data(inputs, input_file);//inputファイルを消しておけば全部やり直してくれる

	std::vector<std::string> file_list;
	if (inputs.size() == 0 || test_mode) {//input.txtがあるかチェック
		const boost::filesystem::path path(xml_dir.c_str());
		BOOST_FOREACH(
			const boost::filesystem::path& p,
			std::make_pair(
				boost::filesystem::directory_iterator(path),
				boost::filesystem::directory_iterator()
			)
		) {
			if (!boost::filesystem::is_directory(p) && (p.extension()).generic_string() == xml_ext) {
				file_list.push_back(
					xml_dir +
					"/" +
					p.filename().generic_string()
				);
				std::cout <<
					(
						xml_dir +
						"/" +
						p.filename().generic_string()
						) <<
					std::endl;
			}
		}

		reader.make_init_data(
			file_list,
			input_file,
			dic_file,
			alias_file,
			dic_xml_file//,
			// meaning_rules_file
		);
		load_input_data(
			inputs,
			input_file
		);
	}
	else {
		//input.txtがあればaliasとmapping btw xml and internal repのデータがあるはずなので，ロードする
		reader.alias_load(alias_file);
		reader.dic_xml_load(dic_xml_file);
		// reader.meaing_rules_load(meaning_rules_file);
	}
	// reader.load(xml_file,inputs);
	dic.load(dic_file);

	std::vector<Rule> buf;
	std::cout << "\n****************sample test" << std::endl;
	std::vector<std::string>::reverse_iterator
		input_it = inputs.rbegin();
	for (; input_it != inputs.rend(); input_it++) {
		Rule s_tmp(*input_it);

		// std::cout << s_tmp.to_s() << std::endl;

		buf.push_back(s_tmp);
	}
	labeling = reader.labeling;
	std::cout << "completion to parse for rules" << std::endl;

	//ルールの意味を登録
	// registration(buf,kb);
	registration(reader.core_meaning, kb);
	TransRules meaning_rules;
	initial_rules(reader.i_rules_str, meaning_rules); //XMLreaderには極力何も依存させない
	kb.init_semantics_rules(meaning_rules);
	kb.dic_add(kb.DB_dic, buf);
	kb.send_box(buf);


	TransRules i_rules;
	// i_rules.insert(*meaning_rules.begin());
	i_rules = meaning_rules;
	kb.init_semantics_rules(i_rules);

	std::cout << "MAP LIST** " << std::endl;// << Rule(reader.dic_xml["../XML/01.xml"]).to_s() << std::endl;
	TransRules::iterator map_it = i_rules.begin();
	for (; map_it != i_rules.end(); map_it++) {
		Rule tmp_rule;
		tmp_rule.type = RULE_TYPE::SENTENCE;
		tmp_rule.internal = (*map_it).second;
		std::cout << (*map_it).first << "<:>" << tmp_rule.to_s() << std::endl;
	}
	std::cout << std::endl;

	std::cout << "\n%%% previous" << std::endl;
	std::cout << kb.to_s() << std::endl;
	// std::cout << std::endl << "TransRules before learning" << std::endl << kb.intention.rules_to_s() << std::endl << std::endl;

	kb.consolidate();
	std::cout << "LEARNED" << std::endl;
	// kb.consolidate();
	std::cout << "\n%%% after" << std::endl;
	std::cout << kb.to_s() << std::endl;

	std::cout << kb.sentenceDB.size() << std::endl;
	std::cout << kb.wordDB.size() << std::endl;

	log.refresh_log();

	//parse test

	std::vector<std::string> t_list;
	std::vector<std::pair<std::string, std::string > > sample;
	std::vector<std::pair<std::string, std::string > >::iterator
		sample_it;
	std::vector<std::string>::iterator t_it;
	bool fl;

	std::cout << std::endl << "LEARNED SEMANTICS" << std::endl << kb.intention.to_s() << std::endl;

	KnowledgeBase::InType inter = kb.meaning_no(12);

	std::cout << "\nInType Test View" << std::endl;
	KnowledgeBase::InType::iterator in_it = inter.begin();
	for (; in_it != inter.end(); in_it++) {
		std::cout << (*in_it).to_s() << "(" << (*in_it).ch.front() << ") ";
	}
	std::cout << std::endl << std::endl;

	Rule r_sample;
	// r_sample.internal = kb.meaning_no(1);
	// r_sample.type = RULE_TYPE::SENTENCE;
	// r_sample.cat = 0;
	// std::cout << "TRANS TEST 01.xml" << std::endl << (kb.fabricate(r_sample)).to_s() << std::endl;

	std::vector<Rule> r_list;
	std::string tree_str;
	int no;
	std::vector<int> beat_nums;

	// no=1;
	// beat_nums = reader.beat_map[no];
	// std::string name = labeling[no];
	// r_list.clear();
	// std::cout << "Construct " << name << ".xml" << std::endl;
	// if(kb.explain(kb.meaning_no(no),r_list)){

	//     tree_str=make_tree_str_for_dot(r_list,beat_nums,no);
	//     std::cout << "tree fin." << std::endl;

	//     output_data(boost::lexical_cast<std::string>("../dot/") + name + std::string(".dot"),tree_str);
	//     std::cout << "output fin." << std::endl;

	// }else{
	//     std::cout << "Can't construct" << std::endl;
	// }

	for (int i = 1; i <= labeling.size(); i++) {
		no = i;
		beat_nums = reader.beat_map[no];
		std::string name = labeling[no];
		r_list.clear();
		std::cout << "Construct " << name << ".xml" << std::endl;
		if (kb.explain(kb.meaning_no(no), r_list)) {

			tree_str = make_tree_str_for_dot(r_list, beat_nums, no, kb);
			std::cout << "tree fin." << std::endl;

			output_data(boost::lexical_cast<std::string>("../dot/") + name + std::string(".dot"), tree_str);
			std::cout << "output fin." << std::endl;

		}
		else {
			std::cout << "Can't construct" << std::endl;
		}
	}

	log.refresh_log();

	return 0;
}
