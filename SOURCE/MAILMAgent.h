#ifndef MAILMAGENT_H
#define MAILMAGENT_H

#include "KirbyAgent.h"
#include "XMLreader.h"
#include "Semantics.h"

class MAILMAgent : public KirbyAgent
{
  public:
	MAILMAgent make_child(void);

	MAILMAgent &grow();

	MAILMAgent &operator=(const MAILMAgent &dst);

	std::vector<Rule> say(int beat_num, std::map<int, std::vector<std::string>> &mapping);
	void hear(std::vector<Rule> rules, std::map<int, std::vector<std::string>> &mapping);

	void registration(std::map<int, std::vector<std::string>> &core_meaning);

	void init_semantics(TransRules ini_sem);

	//utility
	std::string tr_vector_Rule_to_string(std::vector<Rule> vector);
	std::string tr_vector_Rule_to_string(std::vector<std::vector<Rule>> vector);

	MAILMAgent();
	virtual ~MAILMAgent();

  private:
};

#endif /* MAILMAGENT_H */
