
#include "../../mpffactory.h"

class product1 : public ZQ::MPF::utils::ProductBase
{
private:
	int a;
public:
	int work(void* param)
	{
		printf("work 1\n");
		return 1;
	}
};

class product2 : public ZQ::MPF::utils::ProductBase
{
private:
	int b;
public:
	int work(void* param)
	{
		printf("work 2\n");
		return 2;
	}
};

int main(void)
{
	ZQ::MPF::utils::Factory fact;

	REG_PRODUCER(fact, product1);
	REG_PRODUCER(fact, product2);

	ZQ::MPF::utils::ProductBase* product = fact.createProduct("product2");
	if (NULL != product)
		product->work(NULL);

	ZQ::MPF::utils::ProductBase* product1 = fact.createProduct("product1");
	if (NULL != product1)
		product1->work(NULL);

	ZQ::MPF::utils::ProductBase* product2 = fact.createProduct("product2");
	if (NULL != product2)
		product2->work(NULL);

	return 0;
}
