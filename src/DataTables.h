#include <va/va.h>
#include <unordered_map>

using namespace std;

template <class Key,class Value>
class DataTable {
private:
    unordered_map<Key, Value*> dataTbl;
    Key latestKey = 0;
    
public:
    Key insert(Value* surface);
    Value* getValue(Key surfId);
};

class Surface {
public:
    uint32_t width, height;
};

static DataTable<VASurfaceID, Surface> GlobalSurfTable;
static DataTable<VAImageID, VAImage> GlobalImageTable;
