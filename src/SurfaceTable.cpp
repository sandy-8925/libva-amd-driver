#include "SurfaceTable.h"

VASurfaceID SurfaceTable::insert(Surface* surface)
{
    VASurfaceID allocSurfId = latestSurfId++;
    surfTab[allocSurfId] = surface;
    return allocSurfId;
}

Surface* SurfaceTable::getSurface(VASurfaceID surfId)
{
    return surfTab[surfId];
}

