/*
 * Rule.cpp
 *
 *  Created on: 2016/11/16
 *      Author: Hiroki Sudo
 */
#include "Rule.h"

Dictionary Rule::dictionary = Dictionary::copy();
/*
 Rule::~Rule() {
 }
 */

Rule::Rule() {
	internal.clear();
	external.clear();
	cat = 0;
	type = 3;
}
Rule::~Rule() {
}

Rule::Rule(char* cstr) {
	Rule(std::string(cstr));
}

Rule::Rule(std::string str) {//C:6/I:8->b a n a n a
	std::vector<std::string> buf, catbuf, inbuf, exbuf;
	std::vector<std::string>::iterator it;
	std::string category;

	boost::algorithm::split(buf, str,
		boost::algorithm::is_any_of(Prefices::ARW.c_str()),
		boost::algorithm::token_compress_on);

	//Left 
	buf[0] = boost::algorithm::trim_copy(buf[0]);
	boost::algorithm::split(
		inbuf,
		buf[0],
		boost::algorithm::is_any_of(" "),
		boost::algorithm::token_compress_on
	);//arrowの前にスペースがあれば除去
	boost::algorithm::split(
		catbuf,
		inbuf[0],
		boost::algorithm::is_any_of(Prefices::DEL.c_str()),
		boost::algorithm::token_compress_on
	);//RULE_TYPEと分割
	category = catbuf[0];//"/"の左側
	inbuf[0] = catbuf[1];//inbufの最初からカテゴリを取り除いたので書き換え

	//Rule type
	if (category == Prefices::SEN) {
		type = RULE_TYPE::SENTENCE;
		cat = 0;
	}
	else if (
		category.find(
			Prefices::CLN.c_str()
		) !=
		std::string::npos
		) {
		std::vector<std::string> cbuf;
		boost::algorithm::split(
			cbuf,
			category,
			boost::algorithm::is_any_of(Prefices::CLN.c_str()),
			boost::algorithm::token_compress_on
		);
		if (cbuf[0] == Prefices::CAT) {
			type = RULE_TYPE::NOUN;
		}
		else {
			type = RULE_TYPE::MEASURE;
		}
		cat = boost::lexical_cast<int>(cbuf[1]);
	}
	else {
		std::cerr << "Illegal String" << std::endl;
		throw "Illegal String";
	}

	/*
	 * internal
	 */
	int prn; //0:nothing, 1:RPRN, 2:LPRN
	int prn_p;
	// int pos=0;
	std::vector<int> nv, loc_v;
	nv.push_back(0);

	it = inbuf.begin();
	while (it != inbuf.end()) {
		std::string trg_str;
		std::vector<std::string> pbuf;
		prn = -1;
		prn_p = 1;

		// std::cout << "'(' count " << boost::count((*it),*Prefices::LPRN.c_str()) << std::endl;
		// std::cout << "')' count " << std::count((*it).begin(),(*it).end(), *Prefices::RPRN.c_str()) << std::endl;

		trg_str = boost::trim_copy_if((*it), boost::algorithm::is_any_of((Prefices::RPRN + Prefices::LPRN).c_str()));
		if ((*it).find(Prefices::RPRN) != std::string::npos) {
			prn = 1;
			if (boost::count((*it), *Prefices::LPRN.c_str()) > 0) {
				std::cerr << "LRPRN" << std::endl;
				throw "error";
			}
			prn_p = boost::count((*it), *Prefices::RPRN.c_str());

		}
		else if ((*it).find(Prefices::LPRN) != std::string::npos) {
			prn = 2;
			prn_p = boost::count((*it), *Prefices::LPRN.c_str());
		}
		else {
			prn = 0;
			prn_p = 1;
		}
		if (trg_str.find(Prefices::DEL) != std::string::npos) {
			std::vector<std::string> tbuf, cbuf, vbuf, pbuf;

			boost::algorithm::split(pbuf, *it,
				boost::algorithm::is_any_of((Prefices::RPRN + Prefices::LPRN).c_str()),
				boost::algorithm::token_compress_on);
			if (pbuf.size() > 2) {
				std::cerr << "internal pbuf error" << std::endl;
				throw "error";
			}
			if (pbuf.size() == 2) {
				if ((*it).find(Prefices::RPRN) != std::string::npos) {
					trg_str = pbuf[0];
					prn = 1;
				}
				else if ((*it).find(Prefices::LPRN) != std::string::npos) {
					trg_str = pbuf[1];
					prn = 2;
				}
				else {
					std::cerr << "internal split error" << std::endl;
					throw "error";
				}
			}
			else {
				trg_str = pbuf[0];
				prn = 0;
			}

			boost::algorithm::split(tbuf, trg_str,
				boost::algorithm::is_any_of(Prefices::DEL.c_str()),
				boost::algorithm::token_compress_on);
			if (tbuf.size() != 2) {
				std::cerr << "internal tbuf error" << std::endl;
				throw "error";
			}

			boost::algorithm::split(cbuf, tbuf[0],
				boost::algorithm::is_any_of(Prefices::CLN.c_str()),
				boost::algorithm::token_compress_on);
			if (cbuf.size() != 2) {
				std::cerr << "internal cbuf error" << std::endl;
				throw "error";
			}

			boost::algorithm::split(vbuf, tbuf[1],
				boost::algorithm::is_any_of(Prefices::CLN.c_str()),
				boost::algorithm::token_compress_on);
			if (vbuf.size() != 2) {
				std::cerr << "internal vbuf error" << std::endl;
				throw "error";
			}

			Element var;
			int icat, ivar;
			icat = boost::lexical_cast<int>(cbuf[1].c_str());
			ivar = boost::lexical_cast<int>(vbuf[1].c_str());
			var.set_var(ivar, icat);
			internal.push_back(var);
		}
		else {
			Element ind;
			std::vector<std::string> cbuf;
			boost::algorithm::split(cbuf, trg_str,
				boost::algorithm::is_any_of(Prefices::CLN.c_str()),
				boost::algorithm::token_compress_on);
			ind.set_ind(boost::lexical_cast<int>(cbuf[1]));
			internal.push_back(ind);
		}
		int tmp_nv_back;
		switch (prn) {
		case 0:
			nv.back() += 1;
			break;
		case 2: //LPRN
			for (int i = 0; i != prn_p; i++) {
				if (i == prn_p - 1) {
					nv.push_back(1);
				}
				else {
					nv.push_back(0);
				}
				//ポインタでこのループでいれられたとこを示す
				loc_v.push_back(internal.size() - 1);
			}
			break;
		case 1: //RPRN
			nv.back() += 1;
			for (int i = 0; i != prn_p; i++) {
				//ポインタでとっておいたやつにset_chする
				internal[loc_v.back()].set_ch(nv.back());
				loc_v.pop_back();
				tmp_nv_back = nv.back();
				nv.pop_back();
				nv.back() += tmp_nv_back;
			}
			break;
		default:
			std::cerr << "RULE TYPE : " << type << std::endl;
			throw "unknown rule type";
		}
		it++;
	}

	if (buf.size() == 2 && buf[1].size() >= 1) {
		//Right
		buf[1] = boost::algorithm::trim_copy(buf[1]);
		boost::algorithm::split(
			exbuf,
			buf[1],
			boost::algorithm::is_any_of(" "),
			boost::algorithm::token_compress_on
		);

		it = exbuf.begin();
		while (it != exbuf.end()) {
			if ((*it).find(Prefices::DEL) != std::string::npos) {
				//CAT
				std::vector<std::string> tbuf, cbuf, vbuf;
				boost::algorithm::split(tbuf, *it,
					boost::algorithm::is_any_of(Prefices::DEL.c_str()),
					boost::algorithm::token_compress_on);
				if (tbuf.size() != 2) {
					std::cerr << "external var error" << std::endl;
					throw "error";
				}

				boost::algorithm::split(cbuf, tbuf[0],
					boost::algorithm::is_any_of(Prefices::CLN.c_str()),
					boost::algorithm::token_compress_on);

				boost::algorithm::split(vbuf, tbuf[1],
					boost::algorithm::is_any_of(Prefices::CLN.c_str()),
					boost::algorithm::token_compress_on);

				Element excat;
				int icat, ivar;
				icat = boost::lexical_cast<int>(cbuf[1].c_str());
				ivar = boost::lexical_cast<int>(vbuf[1].c_str());
				excat.set_cat(ivar, icat);
				external.push_back(excat);
			}
			else {
				Element sym;
				std::map<std::string, int>::iterator dic_it;
				dic_it = dictionary.conv_symbol.find(*it);

				// std::cout << "CHECK**" << (*it) << std::endl;

				if (dic_it != dictionary.conv_symbol.end()) {
					sym.set_sym((*dic_it).second);
					external.push_back(sym);
				}
				else {
					std::cerr << "external sym error" << std::endl;
					throw "error";
				}
			}
			it++;
		}
	}
	else if (buf.size() == 2 && buf[1].size() == 0) {
		external.clear();
	}
	else { std::cerr << "buf size error" << std::endl; throw "buf size error"; }
}

/*
 *
 * Operators
 *
 */

bool
Rule::operator!=(Rule& dst) const {
	return !(*this == dst);
}

bool
Rule::operator==(const Rule& dst) const {
	if (type == dst.type && internal.front() == dst.internal.front()
		&& external.size() == dst.external.size()) {
		switch (type) {
		case RULE_TYPE::SENTENCE:
			if (internal == dst.internal && external == dst.external)
				return true;
			break;

		case RULE_TYPE::NOUN:
			if (cat == dst.cat && internal == dst.internal
				&& external == dst.external)
				return true;
			break;

		default:
			std::cout << "error type" << std::endl;
		}
	}

	return false;
}

/*
 bool Rule::operator<(Rule& dst) const{
 return ;
 }
 */

Rule&
Rule::operator=(const Rule& dst) {
	type = dst.type;
	cat = dst.cat;
	internal = dst.internal;
	external = dst.external;

	return *this;
}

bool
Rule::is_sentence(void) const {
	if (type == RULE_TYPE::SENTENCE)
		return true;
	return false;
}

bool
Rule::is_noun(void) const {
	if (type == RULE_TYPE::NOUN)
		return true;
	return false;
}

int
Rule::composition(void) const {
	InType::const_iterator it;
	int comp;
	comp = 0;

	for (it = internal.begin(); it != internal.end(); it++) {
		if ((*it).is_var())
			comp++;
	}
	return comp;
}

void
Rule::lemma_to_s(
	std::vector<Element>::iterator it,
	std::vector<Element>::iterator it_end,
	std::vector<int>& ch_v,
	std::vector<std::string>& buffer
) {
	while (it != it_end) {
		if ((*it).ch.size() > 1 || (*it).ch.front() != 1) {
			for (int obj : (*it).ch) {
				ch_v.push_back(obj);
			}
			buffer.push_back((*it).to_s() + Prefices::LPRN);
			for (int i = 1; i < (*it).ch.size(); i++) {
				buffer.push_back(Prefices::LPRN);
			}
		}
		else {
			buffer.push_back((*it).to_s());
		}
		for (int& obj : ch_v) {
			obj--;
			if (obj == 0) {
				buffer.push_back(Prefices::RPRN);
			}
		}
		boost::remove_erase_if(ch_v, [](int obj) { return obj == 0; });
		it++;
	}
}

std::string
Rule::to_s(void) {
	//rule category
	std::string rule_type = "";
	std::string internal_str = "";
	std::string external_str = "";

	switch (type) {
	case RULE_TYPE::SENTENCE:
		if (cat != 0) {
			rule_type = Prefices::SEN// + Prefices::CLN
				+ boost::lexical_cast<std::string>(cat);
		}
		else { rule_type = Prefices::SEN; }
		break;

	case RULE_TYPE::NOUN:
		rule_type = Prefices::CAT// + Prefices::CLN
			+ boost::lexical_cast<std::string>(cat);
		break;
	case RULE_TYPE::MEASURE:
		rule_type = Prefices::MES// + Prefices::CLN
			+ boost::lexical_cast<std::string>(cat);
		break;
	default:
		std::cerr << "RULE TYPE : " << type << std::endl;
		throw "unknown rule type";
	}

	std::vector<std::string> buffer;

	//internalの再帰的構造がchを使って記述されている
	if (internal.size() > 0) {
		//internal
		buffer.clear();

		// InType::iterator it;
		// it = internal.begin();
		// while (it != internal.end()) {
		//   buffer.push_back((*it).to_s());
		//   it++;
		// }
		std::vector<int> ch_v;
		ch_v.push_back(0);
		lemma_to_s(internal.begin(), internal.end(), ch_v, buffer);

		internal_str += boost::algorithm::join(buffer, " ");
	}

	if (external.size() > 0) {
		ExType::iterator it;
		//external
		it = external.begin();
		buffer.clear();
		while (it != external.end()) {
			buffer.push_back((*it).to_s());
			it++;
		}
		external_str = boost::algorithm::join(buffer, " ");
	}

	return rule_type +
		Prefices::DEL +
		internal_str +
		" " +
		Prefices::ARW +
		" " +
		external_str
		;
}

void
Rule::set_noun(
	Element& dcat,
	Element& dind,
	std::vector<Element>& dex
) {
	set_noun(dcat.cat, dind, dex);
}

void
Rule::set_noun(
	int dcat,
	Element& dind,
	std::vector<Element>& dex
) {
	type = RULE_TYPE::NOUN;
	cat = dcat;
	internal.clear();
	internal.push_back(dind);
	external = dex;
}

void
Rule::set_noun(
	int dcat,
	InType& din,
	ExType& dex
) {
	type = RULE_TYPE::NOUN;
	cat = dcat;
	internal = din;
	external = dex;
}

void
Rule::set_sentence(
	std::vector<Element>& din,
	std::vector<Element>& dex
) {
	type = RULE_TYPE::SENTENCE;
	cat = 0; //feature
	internal = din;
	external = dex;
}

void
Rule::set_sentence(
	Element& dcat,
	std::vector<Element>& din,
	std::vector<Element>& dex
) {
	set_sentence(dcat.cat, din, dex);
}

void
Rule::set_sentence(
	int dcat,
	std::vector<Element>& din,
	std::vector<Element>& dex
) {
	type = RULE_TYPE::SENTENCE;
	cat = dcat; //feature
	internal = din;
	external = dex;
}
