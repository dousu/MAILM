#ifndef INDEXFACTORY_H_
#define INDEXFACTORY_H_

class IndexFactory
{
public:
	int index_counter;
	IndexFactory() : index_counter(0){};

	/*!
	 * 数字を生成します。
	 * \code
	 * IndexFactory idxf;
	 * idxf.genenrate();
	 * \endcode
	 */
	int generate(void);

	IndexFactory &operator=(const IndexFactory &dst);
};

#endif /* INDEXFACTORY_H_ */
