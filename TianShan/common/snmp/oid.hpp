#ifndef __ZQ_Snmp_oid_H__
#define __ZQ_Snmp_oid_H__
#include "ZQSnmp.hpp"
#include <string>
#include <vector>

namespace ZQ{
namespace Snmp{

class Oid
{
public:
    typedef std::vector< uint32 > Data;
    Oid(){}
	Oid(const Oid &other)
	{
		data_.empty();
		if (this != &other)
			data_ = other.data_;
	}

    Oid(const std::string& v) { read(v); }
    Oid(const uint32* data, uint32 length) { data_.assign(data, data + length); }

    bool empty() const { return data_.empty(); }
    uint32 length() const { return (uint32)data_.size(); }
    Data& data() { return data_; }
    const Data& data() const { return data_; }

    bool read(const std::string&);
    Oid& append(const Oid& other);

    int compare(const Oid& other) const { return compare(0, length(), other); }
    int compare(uint32 offset, uint32 count, const Oid& other, uint32 off2 = 0, uint32 count2 = uint32(-1)) const;
    Oid operator+(const Oid& other) const;
	Oid& operator=(const Oid& other)
	{
		if (this != &other)
            data_ = other.data_;

		return *this;
	}

    bool operator<(const Oid& other) const { return (compare(other) < 0); }

    Oid sub(uint32 offset = 0, uint32 count = uint32(-1)) const;
private:
    Data data_;
};
}} // namespace ZQ::Snmp
#endif
