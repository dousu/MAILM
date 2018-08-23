#ifndef DICTIONARY_H_
#define DICTIONARY_H_
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <regex>
#include <cstdlib>
#include <iostream>
#include <iterator>

/*!
 * Kirbyモデルで使用する単語の辞書を提供します。例えば、内部言語列の単語「like」や「hate」など。
 * また同時に外部言語列の文字も提供します。例えば、「a,b,c,d」など。
 * この辞書を作成しなければ全てが動きません。
 *
 *
 * 辞書ファイルのフォーマットについて
 * デフォルトの辞書は以下のようになっています
 *
 \code
 * IND=admire,detest,hate,like,love,john,mary,pete,heather,gavin
 * SYM=a,b,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,t,u,v,w,y,z
 \endcode
 *
 * INDで内部言語の終端記号をカンマ区切りで並べ、定義します。また、
 * SYMで外部言語の終端記号をカンマ区切りで並べ、定義します。
 *
 * なお、boost/serializationに対応しています
 */
class Dictionary
{
public:
  typedef std::map<int, std::string> DictionaryType;

  static std::map<int, std::string> symbol;
  static std::map<std::string, int> conv_symbol;

  static Dictionary copy(void);
};

#endif /* DICTIONARY_H_ */