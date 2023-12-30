
#ifndef _ZQ_MPFFACTORY_H_
#define _ZQ_MPFFACTORY_H_

#include "MPFCommon.h"
#include <map>

//#define 

namespace ZQ {
namespace MPF {
namespace utils {

class ProductBase
{
public:
};

class PlantBase
{
protected:
	std::string					m_type;
	std::vector<ProductBase*>	m_instances;

public:
	PlantBase(const char* type)
	{
		m_type = type;
		//m_module = new ProductBase(module);
	}

	virtual ~PlantBase()
	{
		for (std::vector<ProductBase*>::iterator itor = m_instances.begin();
		itor != m_instances.end(); ++itor)
		{
			if (NULL != *itor)
				delete (*itor);
		}
	}

	virtual ProductBase* createProduct() = 0;

	int size()
	{
		return m_instances.size();
	}

	ProductBase* getProduct(int index)
	{
		return m_instances[index];
	}

	void removeProduct(ProductBase* productor)
	{
		std::vector<ProductBase*>::iterator itor = 
			std::find(m_instances.begin(), m_instances.end(), productor);

		delete *itor;
		m_instances.erase(itor);
	}

	const char* getType() const
	{
		return m_type.c_str();
	}
};

template <typename _Product>
class Plant : public PlantBase
{
public:
	Plant(const char* type)
		:PlantBase(type)
	{
	}

	virtual ProductBase* createProduct()
	{
		ProductBase* newproduct = new _Product;
		m_instances.push_back(newproduct);

		return newproduct;
	}
};

class Factory
{
private:
	std::map<std::string, PlantBase*>	m_plants;

public:
	Factory()
	{
	}

	virtual ~Factory()
	{
	}

	void reg(PlantBase& plant)
	{
		m_plants[std::string(plant.getType())] = &plant;
	}

	void unreg(PlantBase& plant)
	{
		m_plants.erase(std::string(plant.getType()));
	}

	ProductBase* createProduct(const char* type)
	{
		PlantBase* producer = m_plants[std::string(type)];
		if (NULL == producer)
			return NULL;

		return producer->createProduct();
	}
};

#define REG_PRODUCER(FACTARY, PRODUCT) ZQ::MPF::utils::Plant<PRODUCT> __FACTARY__##PRODUCT##__(#PRODUCT);\
	FACTARY.reg(__FACTARY__##PRODUCT##__)


}}}


#endif//_ZQ_MPFFACTORY_H_
