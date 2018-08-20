#include "Agent.h"

bool Agent::LOGGING_FLAG = false;

Agent::Agent() { generation_index = 0; }

Agent &Agent::operator=(const Agent &dst) {
  kb = dst.kb;
  generation_index = dst.generation_index;
  LOGGING_FLAG = dst.LOGGING_FLAG;
  return *this;
}

Agent Agent::make_child(void) {
  Agent child;
  child.generation_index = generation_index + 1;
  return child;
}

Agent &Agent::grow() {
  kb.ut_index = XMLreader::index_count;
  kb.ut_category = XMLreader::category_count;
  return *this;
}

std::vector<Rule> Agent::say(std::map<AMean, Conception> &mapping, std::vector<Rule> &base) {
  try {
    std::vector<Rule> res = kb.generate_score(mapping, base);
    return res;
  } catch (...) {
    LogBox::refresh_log();
    throw;
  }
}

void Agent::hear(std::vector<Rule> rules, std::map<AMean, Conception> mapping) {
  registration(mapping);
  kb.send_box(rules);
}

void Agent::registration(std::map<AMean, Conception> &core_meaning) {
  for (auto &p_i_cc : core_meaning) {
    kb.define(p_i_cc.first, p_i_cc.second);
  }
}

void Agent::init_semantics(TransRules ini_sem) { kb.init_semantics_rules(ini_sem); }

void Agent::learn(void) { kb.consolidate(); }

std::string Agent::to_s() { return "Agent's Knowledge: \n" + kb.to_s() + "\n"; }