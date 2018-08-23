#include "XMLreader.h"
#include "Knowledge.h"

int main(int arg, char **argv)
{
    std::vector<int> v(10);
    std::iota(std::begin(v), std::end(v), 0);
    std::for_each(std::begin(v), std::end(v), [&](int n) {
        std::string str = Prefices::SYN + std::to_string(n);
        std::string str2 = "[" + std::to_string(n) + "]";
        XMLreader::alias.insert(std::map<std::string, std::string>::value_type(str, str2));
        XMLreader::conv_alias.insert(std::map<std::string, std::string>::value_type(str2, str));
        Dictionary::symbol.insert(std::map<int, std::string>::value_type(n, str2));
        Dictionary::conv_symbol.insert(std::map<std::string, int>::value_type(str2, n));
        XMLreader::conv_str[str] = n;
    });
    Symbol::conv_symbol = XMLreader::conv_alias;
    Rule buf;
    Knowledge kb;
    std::vector<Rule> vec;

    //dictionary check
    std::cout << "Dictionary" << std::endl;
    // std::cout << "meaning" << std::endl;
    // auto it1 = std::begin(dic.conv_individual);
    // for (; it1 != std::end(dic.conv_individual); it1++)
    // {
    //     std::cout << "\tstring: " << (*it1).first << "\tnumber: " << (*it1).second << std::endl;
    // }
    std::cout << "symbols" << std::endl;
    auto it2 = std::begin(Dictionary::conv_symbol);
    for (; it2 != std::end(Dictionary::conv_symbol); it2++)
    {
        std::cout << "\tstring: " << (*it2).first << "\tnumber: " << (*it2).second << std::endl;
    }

    //chunk1 test
    std::list<SymbolElement> s_el1{Symbol{1}, Symbol{2}, Symbol{3}},
        s_el2{Symbol{1}, Symbol{4}, Symbol{3}};
    // std::vector<MeaningElement> m_el1{Meaning{1}, Meaning{2}, Meaning{3}},
    //     m_el2{Meaning{1}, Meaning{2}, Meaning{2}};
    std::cout << "\n****************chunk1 test" << std::endl;
    buf = Rule(LeftNonterminal(Category{-1}, Meaning(AMean{0})), s_el1);
    vec.push_back(buf);
    kb.send_box(buf);
    buf = Rule(LeftNonterminal(Category{-1}, Meaning(AMean{0})), s_el2);
    vec.push_back(buf);
    kb.send_box(buf);
    kb.define(AMean{0}, Conception(Prefices::SEN));
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.chunk();
    kb.send_db(kb.input_box);
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;
    std::cout << kb.intention.mapping_to_s() << std::endl;

    //chunk2 test
    std::list<SymbolElement> s_el3{Symbol{1}, Symbol{7}, Symbol{3}};
    std::cout << "\n****************chunk2 test" << std::endl;
    buf = Rule(LeftNonterminal(Category{-2}, Meaning(AMean{1})), s_el3);
    kb.send_box(buf);
    kb.define(AMean{1}, Conception(Prefices::SEN));
    kb.define(AMean{-2}, Conception());
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.chunk();
    kb.send_db(kb.input_box);
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;
    std::cout << kb.intention.mapping_to_s() << std::endl;
    vec.push_back(buf);

    //merge test
    std::list<SymbolElement> s_el4{Symbol{4}};
    std::cout << "\n****************merge test" << std::endl;
    std::cout << "\n%%% previous" << std::endl;
    buf = Rule(LeftNonterminal(Category{-3}, Meaning(AMean{-7})), s_el4);
    kb.send_box(buf);
    kb.define(AMean{-7}, Conception());
    std::cout << kb.to_s() << std::endl;
    kb.merge();
    kb.send_db(kb.input_box);
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;
    std::cout << kb.intention.mapping_to_s() << std::endl;
    vec.push_back(buf);

    //replace test
    std::list<SymbolElement> s_el5{Symbol{3}};
    std::cout << "\n****************replace test" << std::endl;
    std::cout << "\n%%% previous" << std::endl;
    buf = Rule(LeftNonterminal(Category{-4}, Meaning(AMean{-8})), s_el5);
    kb.send_box(buf);
    kb.define(AMean{-8}, Conception(Prefices::MES));
    std::cout << kb.to_s() << std::endl;
    kb.replace();
    kb.send_db(kb.input_box);
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;
    std::cout << kb.intention.mapping_to_s() << std::endl;
    vec.push_back(buf);

    //unique test
    std::cout << "\n****************unique test" << std::endl;
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.consolidate();
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;
    std::cout << kb.intention.mapping_to_s() << std::endl;

    Rule r2 = kb.at(1);
    Rule r3 = kb.at(2);
    std::cout << r2 << std::endl
              << r3 << std::endl
              << std::boolalpha << (r2.get_external() == r3.get_external()) << std::noboolalpha << std::endl;

    std::cout << "\n****************consolidate test" << std::endl;
    TransRules tr;
    kb.clear();
    kb.send_box(vec);
    Conception c1(Prefices::SEN);
    c1.add(Prefices::MES);
    kb.define(AMean{0}, Conception(Prefices::SEN));
    kb.define(AMean{1}, c1);
    kb.define(AMean{-7}, Conception(Prefices::MES));
    kb.define(AMean{-8}, Conception(Prefices::MES));
    tr[0] = Meaning(AMean{0});
    tr[1] = Meaning(AMean{1});
    kb.init_semantics_rules(tr);
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.consolidate();
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;
    std::cout << kb.intention.mapping_to_s() << std::endl;
    std::cout << kb.intention.rules_to_s() << std::endl;

    return 0;
}
