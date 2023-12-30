#include "oid.hpp"
namespace ZQ{
namespace Snmp{

// Oid definition
bool Oid::read(const std::string& str)
{
    Data data;
    std::string buf; // buffer
    std::string::size_type pos = 0;
    while(pos < str.size())
    {
        if(isdigit(str[pos]))
        {
            buf += str[pos];
            ++pos;
        }
        else if(str[pos] == '.')
        {
            if(!buf.empty())
            {
                data.push_back(atoi(buf.c_str()));
                buf.clear();
            }

            ++pos;
        }
        else
        { // invalid oid string
            return false;
        }
    }
    if(!buf.empty())
        data.push_back(atoi(buf.c_str()));

    data_.swap(data);
    return true;
}
int Oid::compare(uint32 offset, uint32 count, const Oid& other, uint32 off2, uint32 count2) const
{
    Data::const_iterator beg = (offset < data_.size() ? data_.begin() + offset : data_.end());
    Data::const_iterator end = (offset < data_.size() && count < (data_.size() - offset) ? data_.begin() + offset + count : data_.end());

    Data::const_iterator beg2 = (off2 < other.data_.size() ? other.data_.begin() + off2 : other.data_.end());
    Data::const_iterator end2 = (off2 < other.data_.size() && count2 < (other.data_.size() - off2) ? other.data_.begin() + off2 + count2 : other.data_.end());

    if(lexicographical_compare(beg, end, beg2, end2))
        return -1;
    else if(lexicographical_compare(beg2, end2, beg, end))
        return 1;
    else
        return 0;
}
Oid& Oid::append(const Oid& other)
{
    data_.reserve(length() + other.length());
    std::copy(other.data().begin(), other.data().end(), std::back_inserter(data_));
    return (*this);
}
Oid Oid::operator+(const Oid& other) const
{
    Oid result;
    result.data().reserve(length() + other.length());
    result.data().assign(data_.begin(), data_.end());
    std::copy(other.data().begin(), other.data().end(), std::back_inserter(result.data()));
    return result;
}
Oid Oid::sub(uint32 offset, uint32 count) const
{
    if(offset < data_.size() && count != 0)
#ifdef ZQ_OS_MSWIN
		return Oid(&data_[offset], _cpp_min(count, (uint32)(data_.size()) - offset));
#else
        return Oid(&data_[offset], std::min(count, (uint32)(data_.size()) - offset));
#endif
    else
        return Oid();
}

}} // namespace ZQ::Snmp
