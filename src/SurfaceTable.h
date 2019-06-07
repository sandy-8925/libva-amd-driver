#include <va/va.h>
#include <unordered_map>

using namespace std;

class Surface {
};

class SurfaceTable {
private:
    unordered_map<VASurfaceID, Surface*> surfTab;
    VASurfaceID latestSurfId = 0;
    
public:
    VASurfaceID insert(Surface* surface);
    Surface* getSurface(VASurfaceID surfId);
};

static SurfaceTable GlobalSurfTable;
