#include "MAILM_main.h"

void output_data_trunc(std::string file_path, std::string data)
{
	//存在しない場合はすぐreturn
	const std::filesystem::path path(file_path.c_str());
	std::cout << "OUTPUT FILE: " << path << std::endl;
	std::ofstream ofs(path, std::ios_base::trunc);
	ofs << data;
}

std::string make_tree_str_for_dot(std::vector<Rule> &r_list, std::vector<int> beat_nums, Knowledge &kb)
{
	std::ostringstream os;
	std::queue<std::string> q;
	std::map<Category, std::size_t> cat_num;
	std::map<Symbol, std::size_t> sym_num;
	std::map<std::string, std::size_t> rank_map;
	std::map<int, std::list<std::string>> rank_list;
	std::list<std::string> sym_list;
	std::string start;
	std::for_each(std::begin(r_list), std::end(r_list), [&](Rule &r) {
		if (q.size() == 0)
		{
			start = r.get_internal().get_cat().to_s() + "(1)";
			rank_map[start] = 0;
			q.push(start);
			cat_num.insert(std::make_pair(r.get_internal().get_cat(), 2));
		}
		std::for_each(std::begin(r.get_external()), std::end(r.get_external()), [&](SymbolElement &sel) {
			if (sel.type() == ELEM_TYPE::NT_TYPE)
			{
				if (cat_num.find(sel.get<RightNonterminal>().get_cat()) == std::end(cat_num))
				{
					cat_num.insert(std::make_pair(sel.get<RightNonterminal>().get_cat(), 1));
				}
				std::string s = sel.get<RightNonterminal>().get_cat().to_s() + "(" + std::to_string(cat_num[sel.get<RightNonterminal>().get_cat()]) + ")";
				q.push(s);
				rank_map[s] = rank_map[q.front()] + 1;
				rank_list[rank_map[s]].push_back(s);
				os << std::quoted(q.front()) << Prefices::DOTARW << std::quoted(sel.get<RightNonterminal>().get_cat().to_s() + "(" + std::to_string(cat_num[sel.get<RightNonterminal>().get_cat()]++) + ")") << std::endl;
			}
			else
			{
				if (sym_num.find(sel.get<Symbol>()) == std::end(sym_num))
				{
					sym_num.insert(std::make_pair(sel.get<Symbol>(), 1));
				}
				std::string s = sel.get<Symbol>().to_s() + "(" + std::to_string(sym_num[sel.get<Symbol>()]) + ")";
				// rank_list[rank_map[q.front()] + 1].push_back(s);
				sym_list.push_back(s);
				os << std::quoted(q.front()) << Prefices::DOTARW << std::quoted(sel.get<Symbol>().to_s() + "(" + std::to_string(sym_num[sel.get<Symbol>()]++) + ")") << std::endl;
			}
		});
		q.pop();
	});

	os << "{rank = min; " << std::quoted(start) << ";}" << std::endl;
	std::for_each(std::begin(rank_list), std::end(rank_list), [&](auto &p) {
		os << "{rank = same;";
		std::for_each(std::begin(p.second), std::end(p.second), [&](std::string &s) {
			os << " " << std::quoted(s) << ";";
		});
		os << "}" << std::endl;
	});
	os << "{rank = same;";
	std::for_each(std::begin(sym_list), std::end(sym_list), [&](std::string &s) {
		os << " " << std::quoted(s) << ";";
	});
	os << "}" << std::endl;
	return "digraph sample{\n" + os.str() + "}";
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

	const std::filesystem::path path(param.XML_DIR);
	std::for_each(std::filesystem::directory_iterator(path), std::filesystem::directory_iterator(), [&](const std::filesystem::directory_entry &p) {
		if (!p.is_directory() && (p.path().extension()).generic_string() == param.XML_EXT)
		{
			file_list.push_back(p.path().generic_string());
		}
	});
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
			std::cout << "made tree graph as a dot file" << std::endl;

			output_data_trunc(param.RESULT_PATH + boost::lexical_cast<std::string>("dot/") + name + std::string(".dot"), tree_str);

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

					output_data_trunc(param.BASE_PATH + boost::lexical_cast<std::string>("dot/") + name + std::string(".dot"), tree_str);

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
