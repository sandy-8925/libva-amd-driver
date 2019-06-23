#include <va/va.h>
#include <unordered_map>
#include <mutex>

using namespace std;

template <class Key,class Value>
class DataTable {
private:
    mutex tableMutex;
    unordered_map<Key, Value*> dataTbl;
    Key latestKey = 0;
    
public:
    Key insert(Value* val);
    Value* getValue(Key key);
    /*
     * Removes all data from this table
     */
    void clear();
};

class Surface {
public:
    uint32_t width, height;
};
