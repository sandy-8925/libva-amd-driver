#ifndef _CHIP_DATA
#define _CHIP_DATA

#include <vector>

using namespace std;

class ChipData
{
    public:
        virtual vector<VAProfile> getSupportedVaProfiles() = 0;
        virtual vector<VAEntrypoint> getSupportedEntryPoints(VAProfile profile) = 0;
        virtual vector<VAImageFormat> getSupportedImageFormats() = 0;
        virtual uint32_t getConfigAttribRTFormat(VAProfile profile) = 0;
};

#endif //_CHIP_DATA
