#include "DataTables.h"

template <class Key, class Value>
Key DataTable<Key,Value>::insert(Value* val)
{
    tableMutex.lock();
    Key allocKey = latestKey++;
    tableMutex.unlock();
    dataTbl[allocKey] = val;
    return allocKey;
}

template <class Key, class Value>
Value* DataTable<Key,Value>::getValue(Key key)
{
    return dataTbl[key];
}
