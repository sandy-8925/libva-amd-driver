#ifndef _HAWAII_H
#define  _HAWAII_H

#include "base.h"
#include "ChipData.h"

#include <cstring>

class HawaiiChipData : public ChipData
{
private:
    vector<VAProfile> SUPPORTED_VAPROFILES = {
        VAProfileNone,
        VAProfileMPEG2Simple,
        VAProfileMPEG2Main,
        VAProfileVC1Simple,
        VAProfileVC1Main,
        VAProfileVC1Advanced,
        VAProfileH264ConstrainedBaseline,
        VAProfileH264Main,
        VAProfileH264High
    };
    
    vector<VAImageFormat> SUPPORTED_IMAGEFORMATS = {
        {.fourcc = VA_FOURCC_NV12},
        {.fourcc = VA_FOURCC_P016}
    };
    
public:
    vector<VAProfile> getSupportedVaProfiles()
    { return SUPPORTED_VAPROFILES; }
    
    vector<VAEntrypoint> getSupportedEntryPoints(VAProfile profile)
    {
        switch(profile) {
            case VAProfileNone:
                return {VAEntrypointVideoProc};
            case VAProfileMPEG2Simple:
            case VAProfileMPEG2Main:
            case VAProfileVC1Simple:
            case VAProfileVC1Main:
            case VAProfileVC1Advanced:
                return {VAEntrypointVLD};
            case VAProfileH264ConstrainedBaseline:
            case VAProfileH264Main:
            case VAProfileH264High:
                return {VAEntrypointVLD, VAEntrypointEncSlice};
        }
        return {};
    }
    
    vector<VAImageFormat> getSupportedImageFormats()
    { return SUPPORTED_IMAGEFORMATS; }
};

#endif //_HAWAII_H
