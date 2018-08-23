#ifndef Agent_H
#define Agent_H

#include "Knowledge.h"
#include "Semantics.h"
#include "XMLreader.h"

class Agent {
 public:
  //! エージェントが覚えている規則（Rule）が格納されています。
  Knowledge kb;

  int generation_index;  //!<世代数を表します。
  static bool LOGGING_FLAG;

  /*!
   * hearで溜めた規則を使って学習を行います。
   */
  void learn(void);

  static void logging_on(void);
  static void logging_off(void);

  Agent make_child(void);

  Agent &grow();

  Agent &operator=(const Agent &dst);

  std::vector<Rule> say(std::map<AMean, Conception> &mapping, std::vector<Rule> &base);
  void hear(std::vector<Rule> rules, std::map<AMean, Conception> mapping);

  void registration(std::map<AMean, Conception> &core_meaning);

  void init_semantics(TransRules ini_sem);

  // utility
  std::string tr_vector_Rule_to_string(std::vector<Rule> vector);
  std::string tr_vector_Rule_to_string(std::vector<std::vector<Rule>> vector);

  Agent();

  /*!
   * エージェントの文字列表現を返します。具体的には、自分の知識の文字列表現を返します。
   */
  std::string to_s(void);

 private:
  IndexFactory serial_generator;
};

#endif /* Agent_H */
