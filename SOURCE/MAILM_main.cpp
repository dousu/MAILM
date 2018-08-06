#include "MAILM_main.h"

void output_data(std::string file_path, std::string data)
{
	//存在しない場合はすぐreturn
	const std::filesystem::path path(file_path.c_str());
	std::error_code error;
	const bool result = std::filesystem::exists(path, error);
	std::cout << "OUTPUT FILE: " << path.c_str() << std::endl;
	if (result)
	{
		std::ofstream ofs(path);
		ofs << data;
	}
	else
	{
		std::cout << file_path << " and empty file not found" << std::endl;
		std::cout << data << std::endl;
		return;
	}
}

std::string make_tree_str_for_dot(std::vector<Rule> &r_list, std::vector<int> beat_nums, Knowledge &kb)
{
	std::string begin_str("digraph sample{\n"), end_str("}");
	std::vector<Rule>::iterator ls_it;
	ls_it = r_list.begin();
	std::map<Category, int> memo;
	std::map<std::string, int> memo2;
	std::vector<std::pair<std::string, int>> stack;
	std::vector<std::vector<std::string>> vis;
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
		top = std::string("\"") + Prefices::SEN + std::string("\"");
		stack.push_back(std::pair<std::string, int>(top, 0));
		r_int_to_str.insert(std::multimap<int, std::string>::value_type(0, top));
		r_str_to_int.insert(std::map<std::string, int>::value_type(top, 0));
		for (; ls_it != r_list.end(); ls_it++)
		{
			std::string frm = stack.front().first;
			std::string head;
			if (ls_it != r_list.begin())
			{
				if (memo.find((*ls_it).get_internal().get_cat()) == memo.end())
				{
					memo.insert(std::map<Category, int>::value_type((*ls_it).get_internal().get_cat(), 1));
				}
				else
				{
					memo[(*ls_it).get_internal().get_cat()] += 1;
				}
				head = std::string("\"") + Prefices::CAT + (*ls_it).get_internal().get_cat().to_s() + Prefices::UNO + std::to_string(memo[(*ls_it).get_internal().get_cat()]) /*+ std::string("\\n") + Prefices::IND + std::to_string((*ls_it).internal.front().obj) + std::string("\\n") + kb.meaning_no_to_s((*ls_it).internal.front().obj)*/ + std::string("\"");
				smp.push_back(frm + std::string(" -> ") + head);
				stack.front().second--;
				if (stack.front().second == 0)
				{
					stack.erase(stack.begin());
				}
				r_int_to_str.insert(std::multimap<int, std::string>::value_type(r_str_to_int[frm] + 1, head));
				r_str_to_int.insert(std::map<std::string, int>::value_type(head, r_str_to_int[frm] + 1));
				stack.insert(stack.begin(), std::pair<std::string, int>(head, 0));
			}
			else
			{
				head = frm;
			}

			std::vector<std::string> mini_stack;
			std::list<SymbolElement>::iterator g_ex_it = (*ls_it).get_external().begin();
			for (; g_ex_it != (*ls_it).get_external().end(); g_ex_it++)
			{
				if ((*g_ex_it).type() == ELEM_TYPE::NT_TYPE)
				{
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
				else
				{
					if (memo2.find((*g_ex_it).to_s()) == memo2.end())
					{
						memo2.insert(std::map<std::string, int>::value_type((*g_ex_it).to_s(), 1));
					}
					else
					{
						memo2[(*g_ex_it).to_s()] += 1;
					}

					std::string terminal_str;
					terminal_str = std::string("\"") + (*g_ex_it).to_s() + Prefices::UNO + std::to_string(memo2[(*g_ex_it).to_s()]) + std::string("\"");

					{
						if (vis[vis_index].size() == beat_nums[vis_index])
						{
							vis_index += 1;
							std::vector<std::string> emp_vis_el;
							vis.push_back(emp_vis_el);
						}
						vis[vis_index].push_back(terminal_str);
					}

					smp.push_back(head + std::string(" -> ") + terminal_str);
				}
			}
			if (stack.front().second == 0)
			{
				stack.erase(stack.begin());
			}
			// stack.insert(stack.begin(),mini_stack.begin(),mini_stack.end());
		}
	}

	//rankルール生成

	//レベルの最大値検索
	int upper_bound = 0;
	std::multimap<int, std::string>::iterator itr = r_int_to_str.begin();
	upper_bound = (*itr).first;
	while ((itr = r_int_to_str.upper_bound((*itr).first)) != r_int_to_str.end())
	{
		upper_bound = (*itr).first;
	}

	//terminal symbolのrank生成(仮としてmemo2)
	std::string t_rank;
	std::map<std::string, int>::iterator str_to_int_it;
	t_rank = std::string("{rank = same;");
	str_to_int_it = memo2.begin();
	for (; str_to_int_it != memo2.end(); str_to_int_it++)
	{
		for (int i = 1; i <= (*str_to_int_it).second; i++)
		{
			t_rank += (std::string(" \"") + (*str_to_int_it).first + Prefices::UNO + std::to_string(i) + std::string("\";"));
		}
	}
	t_rank += std::string("}");

	//start symbol
	std::vector<std::string> nt_ranks;
	nt_ranks.push_back(std::string("{rank = min; ") + top + std::string(";}"));

	//non-terminal symbol
	for (int i = 1; i <= upper_bound; i++)
	{
		auto ra = r_int_to_str.equal_range(i);
		std::string nt_rank("{rank = same;");
		while (ra.first != ra.second)
		{
			nt_rank += (std::string(" ") + (*ra.first).second + std::string(";"));
			ra.first++;
		}
		nt_rank += std::string("}");
		nt_ranks.push_back(nt_rank);
	}

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

int main(int argc, char *argv[])
{
	/*
	 * ほぼ静的クラスだが、シリアライズのため（Dictionary）と、
	 * デストラクタ実行のため（LogBox）にインスタンスを作成
	*/
	Dictionary dic;
	XMLreader reader;
	LogBox log; //for destructor
	Knowledge kb;
	MAILMParameters param;

	// std::string meainig_rules_file = "./meaning_rules.data";

	/**************************************************
	*
	* OPTION PROCESSING
	*
	**************************************************/
	ProgramOption opt;
	opt.add_option()("help,h", "Description")
		/*ランダムシード*/
		("random-seed", ProgramOption::value<int>(), "Random seed (101010)")

		/*実験世代数*/
		("generations", ProgramOption::value<int>(), "Max generation number (100)")
		/*発話回数*/
		("utterances", ProgramOption::value<double>(), "Uttering ratio for meaning space (0.5/[0-1])")
		/*ロギング*/
		("logging", "Logging")
		/*分析*/
		("analyze", "Analyze each agent for expression and a number of rules")
		/*辞書ファイル*/
		("dictionary", ProgramOption::value<std::string>(), "Dictionary file name for meaning space(\"./data.dic\")")
		/*生成規則再利用*/
		("keep-random-rule", "Keep created rules with a random word into parent knowledge-base")
		/*FILE PREFIX*/
		("prefix", ProgramOption::value<std::string>(), "Set file prefix (\"MSILM\")")
		/*BASE PATH*/
		("path", ProgramOption::value<std::string>(), "Set folder for output files (\"../RESULT/\")")
		///*プログレスバー*/
		//("progress,p", "Show progress bar")
		;
	opt.parse(argc, argv);
	param.set_option(opt);

	if (param.LOGGING)
	{
		Knowledge::logging_on();
		LogBox::set_filepath(param.LOG_FILE);
	}
	std::vector<std::string> file_list;

	const std::filesystem::path path(param.XML_DIR.c_str());
	BOOST_FOREACH (const std::filesystem::path &p,
				   std::make_pair(std::filesystem::directory_iterator(path), std::filesystem::directory_iterator()))
	{
		if (!std::filesystem::is_directory(p) && (p.extension()).generic_string() == param.XML_EXT)
		{
			file_list.push_back(param.XML_DIR + "/" + p.filename().generic_string());
		}
	}
	reader.make_init_data(file_list);
	// Knowledge::VARIABLE_NO = reader.variable_count;
	// Knowledge::INDEX_NO = reader.index_count;
	// Knowledge::CATEGORY_NO = reader.category_count;

	std::vector<Rule> inputs = reader.input_rules;
	std::map<int, std::string> labeling = reader.labeling;
	TransRules meaning_rules = reader.i_meaning_map;

	//test
	Agent ma;
	TransRules i_rules;
	i_rules = meaning_rules;
	ma.init_semantics(i_rules);
	ma.hear(inputs, reader.core_meaning);
	std::cout << "\n%%% previous state" << std::endl;
	std::cout << ma.to_s() << std::endl;
	std::cout << "start learning" << std::endl;
	ma.learn();
	std::cout << "finish learning" << std::endl;
	std::cout << "LEARNED" << std::endl;
	ma.grow();
	std::cout << "\n%%% after" << std::endl;
	std::cout << ma.to_s() << std::endl;

	if (param.LOGGING)
	{
		log.push_log(ma.kb.to_s());
		log.push_log(ma.kb.dic_to_s());
	}

	log.refresh_log();
	//parse test

	std::vector<std::string> t_list;
	std::vector<std::pair<std::string, std::string>> sample;
	std::vector<std::pair<std::string, std::string>>::iterator sample_it;
	std::vector<std::string>::iterator t_it;
	bool fl;

	std::vector<Rule> r_list;
	std::string tree_str;
	int no;
	std::vector<int> beat_nums;
	Knowledge view_kb = ma.kb;
	for (int i = 1; i <= labeling.size(); i++)
	{
		no = i;
		beat_nums = reader.i_beat_map[no];
		std::string name = labeling[no];
		r_list.clear();
		std::cout << "Construct " << name << ".xml" << std::endl;
		if (view_kb.explain(view_kb.meaning_no(no), r_list))
		{
			std::cout << "explain: true" << std::endl;
			tree_str = make_tree_str_for_dot(r_list, beat_nums, view_kb);
			std::cout << "made tree graph (dot file)" << std::endl;

			output_data(param.RESULT_PATH + boost::lexical_cast<std::string>("dot/") + name + std::string(".dot"), tree_str);

			std::cout << "output fin." << std::endl;
		}
		else
		{
			std::cout << "Can't construct" << std::endl;
		}
	}

	exit(1);
	{
		std::cout << "SEMANTICS:" << std::endl
				  << ma.kb.intention.to_s() << std::endl;
		std::cout << ma.kb.ruleDB.size() << std::endl;

		std::vector<int> nums_v{2, 3, 4};
		Agent parent, child;
		std::map<AMean, Conception> mm, ch_mm;
		std::vector<Rule> parent_origin, ch_hear, tree;
		parent = ma;
		for (int i = 1; i < 10; i++)
		{
			child = parent.make_child();
			child.init_semantics(i_rules);
			ch_hear = inputs;
			// mm.clear();
			// ch_mm.clear();
			// for (int j = 0; j < 5; j++)
			// {
			// 	int b_num = nums_v[MT19937::irand() % nums_v.size()];
			// 	tree.clear();
			// 	parent_origin = parent.say(b_num, mm);

			// 	std::string name = std::string("generation_") + boost::lexical_cast<std::string>(i);
			// 	std::cout << "Utterance " << j + 1 << ":" << std::endl;
			// 	beat_nums = std::vector<int>(parent_origin.size(), b_num);
			// 	tree = parent_origin;
			// 	tree.insert(tree.begin(), *parent_origin.rbegin());
			// 	tree.erase(tree.end() - 1);
			// 	tree_str = make_tree_str_for_dot(tree, beat_nums, view_kb);
			// 	std::cout << "tree fin." << std::endl;
			// 	output_data(param.BASE_PATH + boost::lexical_cast<std::string>("dot/") + name + std::string("_utterance_") + boost::lexical_cast<std::string>(j + 1) + std::string(".dot"), tree_str, empty_dot);

			// 	ch_hear.insert(ch_hear.begin(), parent_origin.begin(), parent_origin.end());
			// 	ch_mm.insert(mm.begin(), mm.end());
			// }
			std::cerr << "LEARNING DATA SIZE: " << ch_hear.size() << ", BUF SIZE: " << inputs.size() << std::endl;
			child.hear(ch_hear, ch_mm);
			child.learn();
			std::cerr << "CHILD'S KNOWLEDGE:" << std::endl
					  << child.kb.to_s() << std::endl;
			std::cout << "CHILD'S SEMANTICS:" << std::endl
					  << ma.kb.intention.to_s() << std::endl;
			view_kb = child.kb;
			for (int i = 1; i <= labeling.size(); i++)
			{
				// for (int i = 14; i <= 14; i++) {
				no = i;
				beat_nums = reader.i_beat_map[no];
				std::string name = labeling[no];
				r_list.clear();
				std::cout << "Construct " << name << ".xml" << std::endl;
				if (view_kb.explain(view_kb.meaning_no(no), r_list))
				{
					std::cout << "explain true" << std::endl;
					tree_str = make_tree_str_for_dot(r_list, beat_nums, view_kb);
					std::cout << "tree fin." << std::endl;

					output_data(param.BASE_PATH + boost::lexical_cast<std::string>("dot/") + name + std::string(".dot"), tree_str);

					std::cout << "output fin." << std::endl;
				}
				else
				{
					std::cout << "Can't construct" << std::endl;
				}
			}
			child.grow();
			parent = child;
		}
		std::cout << parent.kb.to_s() << std::endl;
	}

	log.refresh_log();

	return 0;
}
