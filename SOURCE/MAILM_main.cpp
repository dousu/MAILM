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
	std::string data,
	std::string empty
) {
	//存在しない場合はすぐreturn
	const boost::filesystem::path path(file_path.c_str());
	boost::system::error_code error;
	const bool result =
		boost::filesystem::exists(path, error);
	// if (error) {
	// 	std::cout << file_path << " file not found" << std::endl;
	// 	std::cout << data << std::endl;
	// 	return;
	// }else{
		if(result){
			std::ofstream ofs(file_path.c_str());
			ofs << data;
		}else if(!result){
			boost::filesystem::ofstream(empty.c_str());
			boost::filesystem::copy_file(empty.c_str(),file_path);
			std::ofstream ofs(file_path.c_str());
			ofs << data;
		}else {
			std::cout << file_path << " and empty file not found" << std::endl;
			std::cout << data << std::endl;
			return;	
		}
	// }
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

		i_rules[p_int_vecstr.first] = buf;
	}

}

std::string
make_tree_str_for_dot(std::vector<Rule>& r_list, std::vector<int> beat_nums, KnowledgeBase& kb) {
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
		top = std::string("\"") + Prefices::SEN/*+std::to_string((*r_it).front().cat)*//* + std::string("\\n") + Prefices::IND + std::to_string((*ls_it).internal.front().obj) + std::string("\\n") + kb.meaning_no_to_s((*ls_it).internal.front().obj) */+ std::string("\"");
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
				Element deb = (*ls_it).internal.front();
				deb;
				head = std::string("\"") + Prefices::CAT + std::to_string((*ls_it).cat) + Prefices::UNO + std::to_string(memo[(*ls_it).cat]) /*+ std::string("\\n") + Prefices::IND + std::to_string((*ls_it).internal.front().obj) + std::string("\\n") + kb.meaning_no_to_s((*ls_it).internal.front().obj)*/ + std::string("\"");
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

		//for (int i = 0; i < vis.size(); i++) {
		//	std::string graph_node;
		//	graph_node = std::string("c") + std::to_string(i + 1);
		//	code_v.push_back(graph_node);
		//	for (auto& trg : vis[i]) {
		//		smp.push_back(trg + std::string(" -> ") + graph_node);
		//	}
		//}
		//for (int i = 0; i < vis.size(); i++) {
		//	smp.push_back(std::string("c") + std::to_string(i + 1) + std::string("[image=\"") + std::string("./graph") + std::to_string(code_num) + Prefices::DEL + std::to_string(i + 1) + std::string(".png\"]"));
		//}
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

	//std::string c_rank;
	//std::vector<std::string>::iterator str_v_it;
	//c_rank = std::string("{rank = max;");
	//str_v_it = code_v.begin();
	//for (; str_v_it != code_v.end(); str_v_it++) {
	//	c_rank += (std::string(" \"") + *str_v_it + std::string("\";"));
	//}
	//c_rank += std::string("}");


	//smp処理
	std::string tmp = begin_str;
	tmp += boost::algorithm::join(smp, ";\n");
	tmp += std::string(";\n");
	//tmp += c_rank;
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
	MAILMParameters param;

	KnowledgeBase::logging_on();
	LogBox::set_filepath("./test_log.txt");

	std::string xml_file = "../XML/01.xml";
	std::string dic_file = "./data.dic";
	std::string input_file = "./input.txt";
	std::string alias_file = "./alias.data";
	std::string dic_xml_file = "./dic_xml.data";
	std::string xml_dir = "../XML";
	std::string xml_ext = ".xml";
	std::string empty_dot = "../dot/empty.dot";
	// std::string meainig_rules_file = "./meaning_rules.data";

	/**************************************************
	*
	* OPTION PROCESSING
	*
	**************************************************/
	boost::program_options::options_description opt("option");
	opt.add_options()
		("help,h", "Description")
		//experiment parameters
		/*ランダムシード*/
		("random-seed,r", boost::program_options::value<uint32_t>(), "Random seed (101010)")

		/*実験世代数*/
		("generations,g", boost::program_options::value<int>(), "Max generation number (100)")
		/*発話回数*/
		("utterances,u", boost::program_options::value<double>(), "Uttering ratio for meaning space (0.5/[0-1])")
		///*言葉の省略*/
		//("omission", "Omission")
		/*ロギング*/
		("logging,l", "Logging")
		/*分析*/
		("analyze,a", "Analyze each agent for expression and a number of rules")
		/*生成文字列最大長*/
		("word-length,w", boost::program_options::value<int>(), "Max length of created word (3)")
		///*世代における解析間隔*/
		//("interspace-analysis", boost::program_options::value<int>(), "set analysis interspace for the number of generations")
		///*世代におけるロギング間隔*/
		//("interspace-logging", boost::program_options::value<int>(), "set logging interspace for the number of generations")
		/*辞書ファイル*/
		("dictionary,d", boost::program_options::value<std::string>(), "Dictionary file name for meaning space(\"./data.dic\")")
		/*FILE PREFIX*/
		("prefix", boost::program_options::value<std::string>(), "Set file prefix (\"MSILM\")")
		/*INPUTデータ一時保存用ファイル*/
		("input-file", boost::program_options::value<std::string>(), "Temporary input file name of music score(\"./input.txt\")")
		/*エイリアスデータ一時保存用ファイル*/
		("alias-file", boost::program_options::value<std::string>(), "A file name for symbol translation data(\"./alias.data\")")
		/*一時保存用ファイル*/
		("dic-xml-file", boost::program_options::value<std::string>(), "A file name for xml translation data(\"./dic_xml.data\")")
		/*データディレクトリ*/
		("dic-dir", boost::program_options::value<std::string>(), "A directory name for musicXML(\"../XML\")")
		/*music XML拡張子*/
		("xml-ext", boost::program_options::value<std::string>(), "Extension of musicXML(\".xml\")")
		/*BASE PATH*/
		("path", boost::program_options::value<std::string>(), "Set folder for output files (\"../RESULT/\")")
		///*再開*/
		//("resume", boost::program_options::value<std::vector<std::string> >(), "Resume an experiment from saved file name. Then used path/resume_file")
		///*保存*/
		//("last-save", "Save the last state of the experiment into the file")
		///*全保存*/
		//("all-save", "Save the all state of the experiment into the file with the prefix")
		///*保存形式*/
		//("format", boost::program_options::value<std::string>(), "Set saving format (bin/[xml])")

		/*プログレスバー*/
		("progress,p", "Show progress bar");


	//process options
	boost::program_options::variables_map vm;
	try {
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt), vm);
		boost::program_options::notify(vm);
	}
	catch (boost::program_options::invalid_syntax ee) {
		std::cerr << "invalid syntax:" << ee.tokens() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::unknown_option ee) {
		std::cerr << "unknown_option:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::ambiguous_option ee) {
		std::cerr << "ambiguous_option:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::multiple_values ee) {
		std::cerr << "multiple_values:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::multiple_occurrences ee) {
		std::cerr << "multiple_occurrences:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::validation_error ee) {
		std::cerr << "validation_error:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::too_many_positional_options_error ee) {
		std::cerr << "too_many_positional_options_error:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::invalid_command_line_style ee) {
		std::cerr << "invalid_command_line_style:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::reading_file ee) {
		std::cerr << "reading_file:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::required_option ee) {
		std::cerr << "required_option:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}

	//check options
	if (vm.count("help")) {
		std::cerr << opt << std::endl;
		return 1;
	}

	//set parameters
	try {
		param.set_option(vm);
	}
	catch (std::string e) {
		std::cerr << "option:" << e << std::endl;
		exit(0);
	}

	LogBox::set_filepath(param.BASE_PATH+param.LOG_FILE);

	std::vector<std::string> inputs;
	bool test_mode = true;

	std::map<int, std::string> labeling;

	if (!test_mode)
		load_input_data(inputs, param.INPUT_FILE);//inputファイルを消しておけば全部やり直してくれる

	std::vector<std::string> file_list;
	if (inputs.size() == 0 || test_mode) {//input.txtがあるかチェック
		const boost::filesystem::path path(param.XML_DIR.c_str());
		BOOST_FOREACH(
			const boost::filesystem::path& p,
			std::make_pair(
				boost::filesystem::directory_iterator(path),
				boost::filesystem::directory_iterator()
			)
		) {
			if (!boost::filesystem::is_directory(p) && (p.extension()).generic_string() == param.XML_EXT) {
				file_list.push_back(
					param.XML_DIR +
					"/" +
					p.filename().generic_string()
				);
				std::cout <<
					(
						param.XML_DIR +
						"/" +
						p.filename().generic_string()
						) <<
					std::endl;
			}
		}

		reader.make_init_data(
			file_list,
			param.INPUT_FILE,
			param.DICTIONARY_FILE,
			param.ALIAS_FILE,
			param.DIC_XML_FILE//,
			// meaning_rules_file
		);
		load_input_data(
			inputs,
			param.INPUT_FILE
		);
	}
	else {
		//input.txtがあればaliasとmapping btw xml and internal repのデータがあるはずなので，ロードする
		reader.alias_load(param.ALIAS_FILE);
		reader.dic_xml_load(param.DIC_XML_FILE);
		// reader.meaing_rules_load(meaning_rules_file);
	}
	dic.load(param.DICTIONARY_FILE);

	KnowledgeBase::VARIABLE_NO=reader.variable_count;
	KnowledgeBase::INDEX_NO=reader.index_count;
	KnowledgeBase::CATEGORY_NO=reader.category_count;

	std::vector<Rule> buf;
	std::cout << "\n****************sample test" << std::endl;
	std::vector<std::string>::reverse_iterator input_it = inputs.rbegin();
	for (; input_it != inputs.rend(); input_it++) {
		Rule s_tmp(*input_it);

		buf.push_back(s_tmp);
	}
	labeling = reader.labeling;
	std::cout << "completion to parse for rules" << std::endl;

	TransRules meaning_rules;
	KnowledgeBase::InType::iterator in_it;
	initial_rules(reader.i_rules_str, meaning_rules);

	KnowledgeBase::InType inter2 = meaning_rules[14];
	std::cout << "\nInType Test View before learning" << std::endl;
	in_it = inter2.begin();
	for (; in_it != inter2.end(); in_it++) {
		std::cout << (*in_it).to_s() << "(" << (*in_it).ch.front() << ") ";
	}
	std::cout << std::endl << std::endl;

	MAILMAgent ma;
	TransRules i_rules;
	// i_rules.insert(*meaning_rules.begin());
	i_rules = meaning_rules;
	ma.init_semantics(i_rules);
	ma.hear(buf, reader.core_meaning);
	std::cout << "\n%%% previous" << std::endl;
	std::cout << ma.to_s() << std::endl;
	ma.learn();
	std::cout << "LEARNED" << std::endl;
	ma.grow();
	std::cout << "\n%%% after" << std::endl;
	std::cout << ma.to_s() << std::endl;

	log.refresh_log();

	//parse test

	std::vector<std::string> t_list;
	std::vector<std::pair<std::string, std::string > > sample;
	std::vector<std::pair<std::string, std::string > >::iterator
		sample_it;
	std::vector<std::string>::iterator t_it;
	bool fl;

	//std::cout << std::endl << "LEARNED SEMANTICS" << std::endl << ma.kb.intention.to_s() << std::endl;

	KnowledgeBase::InType inter = ma.kb.meaning_no(14);
	std::cout << "\nInType Test View" << std::endl;
	in_it = inter.begin();
	for (; in_it != inter.end(); in_it++) {
		std::cout << (*in_it).to_s() << "(" << (*in_it).ch.front() << ") ";
	}
	std::cout << std::endl << std::endl;
	inter = ma.kb.meaning_no(1);
	std::cout << "\nInType Test View2" << std::endl;
	in_it = inter.begin();
	for (; in_it != inter.end(); in_it++) {
		std::cout << (*in_it).to_s() << "(" << (*in_it).ch.front() << ") ";
	}
	std::cout << std::endl << std::endl;

	// Rule r_sample;
	// r_sample.internal = kb.meaning_no(14);
	// r_sample.type = RULE_TYPE::SENTENCE;
	// r_sample.cat = 0;
	// std::cout << "TRANS TEST " << labeling[14] << std::endl << (kb.fabricate(r_sample)).to_s() << std::endl;

	std::vector<Rule> r_list;
	std::string tree_str;
	int no;
	std::vector<int> beat_nums;
	KnowledgeBase view_kb = ma.kb;
	for (int i = 1; i <= labeling.size(); i++) {
	// for (int i = 14; i <= 14; i++) {
		no = i;
		beat_nums = reader.beat_map[no];
		std::string name = labeling[no];
		r_list.clear();
		std::cout << "Construct " << name << ".xml" << std::endl;
		if (view_kb.explain(view_kb.meaning_no(no), r_list)) {

			/*
			std::cout << "Test View r_list:" << std::endl;
			for (auto& r : r_list) {
				std::cout << r.to_s() << std::endl;
			}
			std::cout << std::endl;*/

			tree_str = make_tree_str_for_dot(r_list, beat_nums, view_kb);
			std::cout << "tree fin." << std::endl;

			output_data(param.BASE_PATH+boost::lexical_cast<std::string>("dot/") + name + std::string(".dot"), tree_str,empty_dot);
			
			std::cout << "output fin." << std::endl;

		}
		else {
			std::cout << "Can't construct" << std::endl;
		}
	}

	std::cout << "SEMANTICS:" << std::endl << ma.kb.intention.to_s() << std::endl;
	std::cout << ma.kb.sentenceDB.size() << std::endl;
	std::cout << ma.kb.wordDB.size() << std::endl;

	std::vector<int> nums_v{ 3,4 };
	MAILMAgent parent,child;
	std::map<int, std::vector<std::string> > mm, ch_mm;
	std::vector<Rule> parent_origin, ch_hear;
	parent = ma;
	for (int i = 1; i < 10; i++) {
		child = parent.make_child();
		child.init_semantics(i_rules);
		ch_hear=buf;
		mm.clear();
		ch_mm.clear();
		for (int j = 0; j < 5; j++) {
			int b_num = nums_v[MT19937::irand()%nums_v.size()];
			parent_origin = parent.say(b_num, mm);

			std::string name = std::string("generation_") + boost::lexical_cast<std::string>(i);
			std::cout << "Utterance " << j << ":" << std::endl;
			beat_nums = std::vector<int>(b_num,parent_origin.size());
			tree_str = make_tree_str_for_dot(parent_origin, beat_nums, view_kb);
			std::cout << "tree fin." << std::endl;
			output_data(param.BASE_PATH+boost::lexical_cast<std::string>("dot/") + name + std::string("_utterance_") + boost::lexical_cast<std::string>(j) + std::string(".dot"), tree_str,empty_dot);

			ch_hear.insert(ch_hear.end(), parent_origin.begin(),parent_origin.end());
			ch_mm.insert(mm.begin(), mm.end());
		}
		std::cerr << "LEARNING DATA SIZE: " << ch_hear.size() << ", BUF SIZE: " << buf.size() << std::endl;
		child.hear(ch_hear, ch_mm);
		child.learn();
		std::cerr << "CHILD'S KNOWLEDGE:" << std::endl << child.kb.to_s() << std::endl;
		std::cout << "CHILD'S SEMANTICS:" << std::endl << ma.kb.intention.to_s() << std::endl;
		view_kb = child.kb;
		for (int i = 1; i <= labeling.size(); i++) {
		// for (int i = 14; i <= 14; i++) {
			no = i;
			beat_nums = reader.beat_map[no];
			std::string name = labeling[no];
			r_list.clear();
			std::cout << "Construct " << name << ".xml" << std::endl;
			if (view_kb.explain(view_kb.meaning_no(no), r_list)) {

			/*
			std::cout << "Test View r_list:" << std::endl;
			for (auto& r : r_list) {
				std::cout << r.to_s() << std::endl;
			}
			std::cout << std::endl;*/

				tree_str = make_tree_str_for_dot(r_list, beat_nums, view_kb);
				std::cout << "tree fin." << std::endl;

				output_data(param.BASE_PATH+boost::lexical_cast<std::string>("dot/") + name + std::string(".dot"), tree_str,empty_dot);
			
				std::cout << "output fin." << std::endl;

			}
			else {
				std::cout << "Can't construct" << std::endl;
			}
		}
		child.grow();
		parent = child;
	}
	std::cout << parent.kb.to_s() << std::endl;

	log.refresh_log();

	return 0;
}
