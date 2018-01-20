/*
 * XMLreader.cpp
 *
 *  Created on: 2017/01/24
 *      Author: Hiroki Sudo
 */

#include "XMLreader.h"

std::map<std::string, std::string> XMLreader::alias;
std::map<std::string, std::string> XMLreader::conv_alias;
std::map<std::string, std::string> XMLreader::dic_xml;
// std::map<int,std::vector<int> > XMLreader::meaning_rules;
std::vector<std::string> XMLreader::symbol_list;
std::map<int, std::vector<std::string> > XMLreader::i_rules_str;
std::map<int, std::vector<int> > XMLreader::beat_map;
std::map<int, std::string> XMLreader::labeling;
std::map<int, std::vector<std::string> > XMLreader::core_meaning;

int XMLreader::index_count = -1;
int XMLreader::variable_count = -1;
int XMLreader::category_count = -1;
int XMLreader::symbol_count = 1;

void
XMLreader::alias_load(std::string& file_path) {
	XMLdata data;
	std::ifstream file(file_path.c_str());
	boost::archive::text_iarchive ia(file);
	ia >> data;
	alias = data.alias;
	conv_alias = data.conv_alias;
}

void
XMLreader::dic_xml_load(std::string& file_path) {
	DicXMLdata data;
	std::ifstream file(file_path.c_str());
	boost::archive::text_iarchive ia(file);
	ia >> data;
	dic_xml = data.dic_xml;
}

// void
// XMLreader::meaning_rules_load(std::string& file_path) {
//   RulesXMLdata data;
//   std::ifstream file(file_path.c_str()); 
//   boost::archive::text_iarchive ia(file);
//   ia >> data;
//   meaning_rules = data.meaning_rules;
// }

void
XMLreader::make_init_data(std::vector<std::string>& file_paths,
	std::string& input_data_path,
	std::string& read_dic_path,
	std::string& alias_file_path,
	std::string& dic_xml_path) {//,
	// std::string& meaning_rules_path) {
	std::cout << "XMLreader3" << std::endl;
	{
		alias.clear();
		conv_alias.clear();
		dic_xml.clear();
		symbol_list.clear();
		i_rules_str.clear();
		beat_map.clear();
		labeling.clear();
		core_meaning.clear();
	}

	std::vector<std::string> buf;
	std::sort(file_paths.begin(), file_paths.end());
	int no = 1;
	std::vector<std::string>::iterator f_it;
	f_it = file_paths.begin();
	for (; f_it != file_paths.end(); f_it++) {
		std::vector<std::string> rules;
		load(*f_it, rules, no);
		buf.insert(buf.end(), rules.begin(), rules.end());
		no++;
	}
	//bufをinput_data_pathへ，
	// symbol_listをread_dic_pathへ書き込み
	if (buf.size() > 0) {
		std::vector<std::string>::iterator it;
		it = buf.begin();
		std::ofstream ofs(input_data_path.c_str(),
			std::ios::trunc);
		for (; it != buf.end(); it++) {
			ofs << *it << std::endl;
		}
		ofs.close();
	}
	if (symbol_list.size() > 0) {
		std::ofstream ofs(read_dic_path.c_str(),
			std::ios::trunc);
		ofs << "SYM="
			+ boost::algorithm::join(symbol_list, ",");
		ofs.close();
	}
	{
		XMLdata data;
		data.alias = alias;
		data.conv_alias = conv_alias;
		std::ofstream of(alias_file_path.c_str(),
			std::ios::trunc);
		boost::archive::text_oarchive oa(of);
		oa << (const XMLdata&)data;
	}
	{
		DicXMLdata dic_xml_data;
		dic_xml_data.dic_xml = dic_xml;
		std::ofstream of(dic_xml_path.c_str(),
			std::ios::trunc);
		boost::archive::text_oarchive oa(of);
		oa << (const DicXMLdata&)dic_xml_data;
	}
	// {
	//   RulesXMLdata meaning_rules_data;
	//   meaning_rules_data.meaning_rules = meaning_rules;
	//   std::ofstream of(meaning_rules_path.c_str(),
	//                     std::ios::trunc);
	//   boost::archive::text_oarchive oa(of);
	//   oa << (const RulesXMLdata&) meaning_rules_data;
	// }
}

void
XMLreader::load(std::string file_path,
	std::vector<std::string>& buf, int file_no) {
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(file_path.c_str(), pt);

	std::vector<std::string> lbuf1, lbuf2;

	boost::algorithm::split(lbuf1, file_path,
		boost::algorithm::is_any_of("."),
		boost::algorithm::token_compress_on);
	boost::algorithm::split(lbuf2, *(lbuf1.rbegin() + 1),
		boost::algorithm::is_any_of("/\\"),
		boost::algorithm::token_compress_on);

	// std::cout << *(lbuf2.rbegin()) << std::endl;

	labeling[file_no] = *(lbuf2.rbegin());
	// labeling.push_back(*(lbuf2.rbegin()));

	//初期設定
	bool loop = true;
	std::string path1 = "score";
	// std::string path1 = "score-partwise.part";
	int measure_num = 1, met_num, p_num = 0, met_max;
	std::vector<std::string> s_in, s_ex, dic_in, flat_sent;

	int sent_loc = 0, loop_loc = -1, length = 1;

	s_in.push_back(
		std::string("I") +
		Prefices::CLN +
		boost::lexical_cast<std::string>(index_count)
	);
	core_meaning[index_count] = std::vector<std::string>();
	core_meaning[index_count].push_back(std::string("SENTENCE"));
	core_meaning[index_count].push_back(std::string("s") + labeling[file_no]);

	//sentenceのインデックス記録
	dic_in.push_back(
		std::string("I") +
		Prefices::CLN +
		boost::lexical_cast<std::string>(index_count)
	);

	flat_sent.push_back(
		std::string("I") +
		Prefices::CLN +
		boost::lexical_cast<std::string>(index_count) +
		Prefices::CLN +
		std::to_string(0) +
		Prefices::CLN
	);

	index_count--;

	// beat_map[file_no] = 0;
	beat_map[file_no] = std::vector<int>();


	//楽曲の繰り返しもあるので全部検査するのを変化がなくなるまで行う
	while (loop) {

		//何巡目かカウント
		p_num++;
		//変化があればloopがtrueになる
		loop = false;
		//measure番号の初期化
		measure_num = 1;

		loop_loc = loop_loc + length + 1;
		length = 0;

		std::vector<std::string> sub_in, sub_ex;
		sub_in.push_back(
			std::string("I") +
			Prefices::CLN +
			boost::lexical_cast<std::string>(index_count)
		);
		// core_meaning[index_count] = std::vector<std::string>();
		// core_meaning[index_count].push_back(std::string(""));
		dic_in.push_back(
			std::string("I") +
			Prefices::CLN +
			boost::lexical_cast<std::string>(index_count)
		);
		flat_sent.push_back(
			std::string("I") +
			Prefices::CLN +
			boost::lexical_cast<std::string>(index_count) +
			Prefices::CLN +
			std::to_string(0) +
			Prefices::CLN
		);
		index_count--;

		//score以下の木を順番に取っていく
		BOOST_FOREACH(
			const boost::property_tree::ptree::value_type& measure_t,
			pt.get_child(path1.c_str())
		) {

			//measureであり次に検査すべきmeasureか判定
			if (measure_t.first == "measure" &&
				(measure_t.second.get_optional<int>((std::string("<xmlattr>.number")).c_str())).get() == measure_num
				) {

				//p_flで処理すべきmetricか判定
				bool p_fl = false;
				//検査対象metric以下の木をすべて取ってくる
				BOOST_FOREACH(
					const boost::property_tree::ptree::value_type& period_t,
					measure_t.second.get_child("")
				) {
					//periodであり次に検査すべきmetricかを何巡目かで判定
					if (
						period_t.first == "period" &&
						(
							period_t.second.get_optional<int>(
							(std::string("<xmlattr>.number")).c_str()
								)
							).get() == p_num
						) {
						//合致するperiodがあればこのmetricを検査
						loop |= true;
						p_fl = true;
						break;
					}
				}

				if (p_fl) {
					//metric番号の初期化
					met_num = 1;
					//入っているべきmetricの数を取得
					met_max = (measure_t.second.get_optional<int>(
						(std::string("<xmlattr>.metricals")).c_str())
						).get();
					beat_map[file_no].push_back(met_max);
					// if(beat_map[file_no] == 0){
					//   beat_map[file_no] = met_max;
					// }else if(beat_map[file_no] != met_max){
					//   std::cerr << "[XMLreader]incorrect metricals error" << std::endl;
					//   throw "[XMLreader]incorrect metricals error";
					// }
					std::vector<std::string> word_ex;
					//検査対象measure以下の木をすべて取ってくる
					BOOST_FOREACH(
						const boost::property_tree::ptree::value_type& metric_t,
						measure_t.second.get_child("")
					) {
						// if(metric_t.first == "metric"){
						//   std::cout << metric_t.first << " number=" << (metric_t.second.get_optional<int>((std::string("<xmlattr>.number")).c_str())).get() << " met_num=" << met_num << " met_max=" << met_max <<  std::endl;
						// }

						//metricであり次に検査すべきmetricか判定
						if (metric_t.first == "metric" &&
							(metric_t.second.get_optional<int>(
							(
								std::string("<xmlattr>.number")
								).c_str()
								)).get() == met_num
							) {
							//ここで順番に処理していけば順序通りにとれる
							std::string str;

							//すべてのpitch classを取得
							BOOST_FOREACH(
								const boost::property_tree::ptree::value_type& class_t,
								metric_t.second.get_child("pitchclass")
							) {
								//pronが0以外の場合は記録する
								if ((class_t.second.get_optional<int>(
									(
										std::string("<xmlattr>.pron")
										).c_str()
									)
									).get() > 0
									) {
									//pronの数は非表示
									str += class_t.first +
										// Prefices::CLN +
										// (
										//   class_t.second.get_optional<std::string>(
										//     (std::string("<xmlattr>.pron")).c_str()
										//   )
										// ).get() +
										Prefices::DSP;
								}
							}
							str = boost::algorithm::trim_copy(str);
							if (str.size() == 0) {
								str = std::string("rest");

								core_meaning[index_count] = std::vector<std::string>();
								core_meaning[index_count].push_back("rest");

							}

							//strのaliasの作成とword_exへの保存
							std::string str2;
							std::map<std::string, std::string>::iterator it;
							it = alias.find(str);
							if (it == alias.end()) {
								str2 = Prefices::SYN + Prefices::CLN +
									boost::lexical_cast<std::string>(symbol_count);
								symbol_count++;
								alias.insert(
									std::map<std::string, std::string>::value_type(str, str2)
								);
								conv_alias.insert(
									std::map<std::string, std::string>::value_type(str2, str)
								);
								symbol_list.push_back(str2);
							}
							else {
								str2 = alias[str];
							}
							word_ex.push_back(str2);
							met_num++;
						}
						//met_numがmet_maxになると小節の終わりなのでルールとして掃き出し
						if (met_num == met_max + 1) {
							//wordに少しでもたまっていたらルールにする
							if (word_ex.size() != 0) {
								//metricを全部回り終わったタイミングで単語ルール追加

								//ワードルールの左側作成
								std::string tmp =
									// Prefices::MES +
									Prefices::CAT +
									Prefices::CLN +
									boost::lexical_cast<std::string>(category_count);

								//bufにワードルールをためる
								buf.push_back(tmp +
									Prefices::DEL +
									"I" + Prefices::CLN +
									boost::lexical_cast<std::string>(index_count) +
									Prefices::ARW +
									boost::algorithm::join(word_ex, " "));
								if (core_meaning.find(index_count) == core_meaning.end()) {
									core_meaning[index_count] = std::vector<std::string>();
								}
								core_meaning[index_count].push_back(std::string("MEASURE"));
								// core_meaning[index_count].push_back(std::string("m")+std::to_string(file_no) + std::string("_") +(measure_t.second.get_optional<std::string>( (std::string("<xmlattr>.number")).c_str()) ).get());

								//variableをつけて汎用表現にする
								tmp = tmp +
									Prefices::DEL +
									Prefices::VAR +
									Prefices::CLN +
									boost::lexical_cast<std::string>(variable_count);

								//sentence internal and external
								sub_ex.push_back(tmp);
								sub_in.push_back(tmp);

								dic_in.push_back(
									std::string("I") +
									Prefices::CLN +
									boost::lexical_cast<std::string>(index_count)
								);

								flat_sent.push_back(
									std::string("I") +
									Prefices::CLN +
									boost::lexical_cast<std::string>(index_count) +
									Prefices::CLN +
									std::to_string(0) +
									Prefices::CLN +
									std::to_string(1)
								);
								//sub用
								length++;

								index_count--;
								variable_count--;
								category_count--;
							}

							break;
						}

					}
				}
				measure_num++;
			}
		}

		if (length > 0)
			flat_sent[loop_loc] += boost::lexical_cast<std::string>(length + 1);
		else
			flat_sent.pop_back();

		if (sub_ex.size() != 0) {
			std::string tmp2 =
				Prefices::CAT +
				Prefices::CLN +
				boost::lexical_cast<std::string>(category_count);

			buf.push_back(
				tmp2 +
				Prefices::DEL +
				boost::algorithm::join(sub_in, " ") +
				Prefices::ARW + boost::algorithm::join(sub_ex, " ")
			);

			tmp2 = tmp2 +
				Prefices::DEL +
				Prefices::VAR +
				Prefices::CLN +
				boost::lexical_cast<std::string>(variable_count);

			s_ex.push_back(tmp2);
			s_in.push_back(tmp2);
			category_count++;
			variable_count++;
		}
		else {
			index_count--;
		}

	}

	flat_sent[sent_loc] += boost::lexical_cast<std::string>(flat_sent.size());

	//文ルール追加
	buf.push_back(
		Prefices::SEN +
		Prefices::DEL +
		boost::algorithm::join(s_in, " ") +
		Prefices::ARW + boost::algorithm::join(s_ex, " ")
	);
	//dic_xml登録
	{
		dic_xml[file_path] =
			Prefices::SEN +
			Prefices::DEL +
			boost::algorithm::join(dic_in, " ") +
			Prefices::ARW;
	}
	{
		i_rules_str[file_no] = flat_sent;
	}
}

void
XMLreader::get_sample(
	std::string& file_path,
	std::vector<std::pair<std::string, std::string > >& buf
) {
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(
		file_path.c_str(), pt
	);
	bool loop = true;
	std::string path1 = "score-partwise.part";
	int measure_num = 1, met_num, p_num = 0, met_max;

	index_count++;

	while (loop) {
		p_num++;
		loop = false;
		measure_num = 1;
		BOOST_FOREACH(
			const boost::property_tree::ptree::value_type& measure_t,
			pt.get_child(path1.c_str())
		) {
			if (measure_t.first == "measure" &&
				(measure_t.second.get_optional<int>(
				(std::string("<xmlattr>.number")).c_str())
					).get() == measure_num
				) {

				met_num = 1;
				met_max = (
					measure_t.second.get_optional<int>(
					(std::string("<xmlattr>.metricals")).c_str())
					).get();
				// std::vector<std::string> word_ex;

				bool p_fl = false;
				BOOST_FOREACH(
					const boost::property_tree::ptree::value_type& period_t,
					measure_t.second.get_child("")
				) {
					if (
						period_t.first == "period" &&
						(
							period_t.second.get_optional<int>(
							(std::string("<xmlattr>.number")).c_str()
								)
							).get() == p_num) {
						loop |= true;
						p_fl = true;
						break;
					}
				}

				BOOST_FOREACH(
					const boost::property_tree::ptree::value_type& metric_t,
					measure_t.second.get_child("")
				) {
					if (metric_t.first == "metric" &&
						(metric_t.second.get_optional<int>(
						(std::string("<xmlattr>.number")).c_str()
							)).get() == met_num
						) {
						if (p_fl) {
							//ここで順番に処理していけば順序通りにとれる
							std::string str;
							BOOST_FOREACH(
								const boost::property_tree::ptree::value_type& class_t,
								metric_t.second.get_child("pitchclass")
							) {
								if (
									(class_t.second.get_optional<int>(
									(std::string("<xmlattr>.pron")).c_str())
										).get() > 0
									) {
									str += class_t.first +
										// Prefices::CLN +
										// (class_t.second.get_optional<std::string>(
										//   (std::string("<xmlattr>.pron")).c_str())
										// ).get() +
										Prefices::DSP;
								}
							}
							str = boost::algorithm::trim_copy(str);
							buf.push_back(
								std::pair<std::string, std::string>(
									"*****measure " +
									boost::lexical_cast<std::string>(
										measure_num
										) +
									"\n" + "**********metric " +
									boost::lexical_cast<std::string>(
										met_num
										),
									str)
							);
						}
						if (met_num == met_max) {
							break;
						}
						met_num++;
					}
				}
				measure_num++;
			}
		}
	}

}


XMLreader
XMLreader::copy(void) {
	return XMLreader();
}
