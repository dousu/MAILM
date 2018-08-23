#include "Element.h"

std::map<std::string, std::string> Symbol::conv_symbol;

std::ostream &operator<<(std::ostream &out, const AMean &obj) {
  out << obj.to_s();
  return out;
}
std::ostream &operator<<(std::ostream &out, const Variable &obj) {
  out << obj.to_s();
  return out;
}
std::ostream &operator<<(std::ostream &out, const Category &obj) {
  out << obj.to_s();
  return out;
}
std::ostream &operator<<(std::ostream &out, const Symbol &obj) {
  out << obj.to_s();
  return out;
}
std::ostream &operator<<(std::ostream &out, const MeaningElement &obj) {
  out << obj.to_s();
  return out;
}
std::ostream &operator<<(std::ostream &out, const Meaning &obj) {
  out << obj.to_s();
  return out;
}
std::ostream &operator<<(std::ostream &out, const LeftNonterminal &obj) {
  out << obj.to_s();
  return out;
}
std::ostream &operator<<(std::ostream &out, const RightNonterminal &obj) {
  out << obj.to_s();
  return out;
}
std::ostream &operator<<(std::ostream &out, const SymbolElement &obj) {
  out << obj.to_s();
  return out;
}

std::ostream &operator<<(std::ostream &out, const Conception &obj) {
  out << obj.to_s();
  return out;
}

Meaning Meaning::replaced(std::size_t n, std::size_t size, const MeaningElement &el) const {
  if (size > means.size() || n < 0 ||
      (n == 0 && (size != 1 || el.type() != MEANING_TYPE::MEANING_TYPE || el.get<Meaning>().get_followings().size() != 0))) {
    std::cerr << "Cannot remove" << std::endl;
    exit(1);
  }
  if (n == 0) return Meaning(el.get<Meaning>().get_base(), means);
  std::list<MeaningElement> tmp = means;
  auto it = tmp.erase(std::next(std::begin(tmp), n - 1), std::next(std::begin(tmp), n - 1 + size));
  tmp.insert(it, el);
  return Meaning(base, tmp);
}

AMean Meaning::flat(std::size_t &ind) {
  if (ind == 0) return get_base();
  ind--;
  AMean res;
  std::for_each(std::begin(means), std::end(means), [&](MeaningElement &m) {
    if (m.type() == MEANING_TYPE::MEANING_TYPE) {
      if (res == AMean()) {
        res = m.template get<Meaning>().flat(ind);
      }
    } else {
      std::cerr << "Error in Meaning::flat(std::size_t&)" << std::endl;
      exit(1);
    }
  });
  return res;
}