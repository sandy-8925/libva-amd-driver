#ifndef _CHIP_DATA
#define _CHIP_DATA

#include <vector>

using namespace std;

class ChipData
{
    public:
        virtual vector<VAProfile> getSupportedVaProfiles() = 0;
        virtual VAStatus getSupportedEntryPoints(VAProfile profile, VAEntrypoint *entrypoint_list, int *num_entrypoints) = 0;
        virtual vector<VAImageFormat> getSupportedImageFormats() = 0;
};

#endif //_CHIP_DATA
