#ifndef __EventGw_MQTT_SafeStore_H__
#define __EventGw_MQTT_SafeStore_H__
#include <list>
#include <Freeze/Freeze.h>
namespace EventGateway{
namespace MQTT{

// work with freeze map that use integer as key
template< class DBMapT >
class SafeQueue
{
public:
    typedef Ice::Long IndexT;
    typedef typename DBMapT::value_type::second_type ValueT;
    typedef std::pair<IndexT, ValueT> value_type;
    typedef typename std::list< value_type > Data;

    SafeQueue(const Freeze::ConnectionPtr& conn, const std::string& name)
        :_conn(conn), _record(NULL)
    {
        try
        {
            _record = new DBMapT(conn, name);
            load();
        }catch(const Ice::Exception&)
        {
            clear();
            if(_record)
            {
                delete _record;
                _record = NULL;
            }
            throw;
        }

    }
    ~SafeQueue()
    {
        try {
            save();
        }catch(...){
            clear();
        }

        if(_record)
        {
            try {
                delete _record;
            } catch (...){}
            _record = NULL;
        }
    }
    bool empty() const
    {	// test if queue is empty
        return (_data.empty());
    }

    size_t size() const
    {	// return length of queue
        return (_data.size());
    }

    ValueT& front()
    {	// return first element of mutable queue
        return (_data.front().second);
    }

    ValueT& back()
    {	// return last element of mutable queue
        return (_data.back().second);
    }

    void push(const ValueT& _Val)
    {	// insert element at beginning
        _data.push_back(value_type((++_indexGen), _Val));
    }

    void pop()
    {	// erase element at end
        _data.pop_front();
    }
private:
    // copying forbidden
    SafeQueue(const SafeQueue&);
    SafeQueue& operator=(const SafeQueue&);
private:
    void clear()
    {
        _data.clear();
        _indexGen = IndexT();
    }

    template< class PairT >
    class KeyLess: public std::binary_function< const PairT, const PairT, bool>
    {
    public:
        bool operator() (const PairT& fst, const PairT& snd) const
        {
            return (fst.first < snd.first);
        }
    };

    void load()
    {
        clear();
        if(!(_conn && _record))
        {
            // lack of necessary resource
            return;
        }
//        _data.assign(_record->begin(), _record->end());
        for(typename DBMapT::iterator ite = _record->begin(); ite != _record->end(); ++ite) {
            _data.push_back(*ite);
        }

        _data.sort(KeyLess<value_type>());
        for(typename Data::iterator ite = _data.begin(); ite != _data.end(); ++ite)
            ite->first = (++_indexGen);
    }
    void save()
    {
        if(!(_conn && _record))
        {
            // lack of necessary resource
            return;
        }
        Freeze::TransactionPtr tran = _conn->beginTransaction();
        _record->clear();
        _record->put(_data.begin(), _data.end());
        tran->commit();
    }
private:
    IndexT _indexGen;
    Data _data;

    Freeze::ConnectionPtr _conn;
    DBMapT* _record;
};

} // namespace MQTT
} // namespace EventGateway
#endif
