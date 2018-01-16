#include <iostream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

int main()
{
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml("test.xml", pt);

    // if (boost::optional<std::string> str = pt.get_optional<std::string>("score-partwise.part.measure.<xmlattr>.number")) {
    //     std::cout << str.get() << std::endl;
    // }
    // else {
    //     std::cout << "score-partwise is nothing" << std::endl;
    // }
    bool loop=true;
    std::string path1 = "score-partwise.part";
    int measure_num = 1, met_num, p_num = 0, met_max;

    while(loop){
        p_num++;
        loop = false;
        measure_num = 1;
        BOOST_FOREACH (const boost::property_tree::ptree::value_type& measure_t, pt.get_child(path1.c_str())) {
            if(measure_t.first == "measure" && (measure_t.second.get_optional<int>((std::string("<xmlattr>.number")).c_str())).get() == measure_num){
                // boost::optional<int> num = (child.second.get_optional<int>((std::string("<xmlattr>.number")).c_str()));
                // std::cout << mesure_num << ":" << num.get() << "->" << (count == num.get()) << std::endl;
                //std::cout << typeid(count/*(pt.get_optional<int>(path1 + std::string("measure.<xmlattr>.id")).get())*/).name() << std::endl;

                met_num = 1;
                met_max = (measure_t.second.get_optional<int>((std::string("<xmlattr>.metricals")).c_str())).get();
                // std::cout <<  << std::endl;
                BOOST_FOREACH (const boost::property_tree::ptree::value_type& metric_t, measure_t.second.get_child("")) {
                    if(metric_t.first == "metric" && (metric_t.second.get_optional<int>((std::string("<xmlattr>.number")).c_str())).get() == met_num){
                        bool p_fl = false;
                        BOOST_FOREACH(const boost::property_tree::ptree::value_type& period_t, metric_t.second.get_child("")) {
                            if(period_t.first == "period" && (period_t.second.get_optional<int>((std::string("<xmlattr>.number")).c_str())).get() == p_num){
                                loop |= true;
                                p_fl = true;
                                // std::cout << "*****measure " << measure_num << std::endl << "**********metric " << met_num << " period=" << p_num << std::endl;
                                break;
                            }
                        }
                        if(p_fl){
                            //ここで順番に処理していけば順序通りにとれる
                            std::string str;
                            BOOST_FOREACH(const boost::property_tree::ptree::value_type& class_t, metric_t.second.get_child("pitchclass")) {
                                if((class_t.second.get_optional<std::string>((std::string("<xmlattr>.pron")).c_str())).get() != std::string("0")){
                                    str += class_t.first + ":" + (class_t.second.get_optional<std::string>((std::string("<xmlattr>.pron")).c_str())).get() + "  ";
                                }
                            }
                            str = boost::algorithm::trim_copy(str);
                            std::cout << str << std::endl;
                        }

                        met_num++;
                    }
                }
                measure_num++;
            }
        }
    }
}
