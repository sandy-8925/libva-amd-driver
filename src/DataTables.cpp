#include "DataTables.h"

template <class Key, class Value>
Key DataTable<Key,Value>::insert(Value* val)
{
    Key allocKey = latestKey++;
    dataTbl[allocKey] = val;
    return allocKey;
}

template <class Key, class Value>
Value* DataTable<Key,Value>::getValue(Key key)
{
    return dataTbl[key];
}
