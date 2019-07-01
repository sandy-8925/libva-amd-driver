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
    
    VAStatus getSupportedEntryPoints(VAProfile profile, VAEntrypoint *entrypoint_list, int *num_entrypoints)
    {
        switch(profile) {
            case VAProfileNone:
                *entrypoint_list = {VAEntrypointVideoProc};
                *num_entrypoints = 1;
                return VA_STATUS_SUCCESS;
            case VAProfileMPEG2Simple:
            case VAProfileMPEG2Main:
            case VAProfileVC1Simple:
            case VAProfileVC1Main:
            case VAProfileVC1Advanced:
                *entrypoint_list = {VAEntrypointVLD};
                *num_entrypoints = 1;
                return VA_STATUS_SUCCESS;
            case VAProfileH264ConstrainedBaseline:
            case VAProfileH264Main:
            case VAProfileH264High:
                VAEntrypoint entrypoints[] = {VAEntrypointVLD, VAEntrypointEncSlice};
                memcpy(entrypoint_list, entrypoints, sizeof(entrypoints));
                *num_entrypoints = sizeof(entrypoints)/sizeof(entrypoints[0]);
                return VA_STATUS_SUCCESS;
        }
        return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
    }
    
    vector<VAImageFormat> getSupportedImageFormats()
    { return SUPPORTED_IMAGEFORMATS; }
};

#endif //_HAWAII_H
