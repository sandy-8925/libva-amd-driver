#include "base.h"

#ifndef _HAWAII_H
#define  _HAWAII_H

static VAProfile HAWAII_SUPPORTED_VAPROFILES[] = {
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

VAStatus hawaii_getSupportedEntryPoints(VAProfile profile, VAEntrypoint *entrypoint_list, int *num_entrypoints)
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
            *entrypoint_list = *entrypoints;
            *num_entrypoints = sizeof(entrypoints)/sizeof(VAEntrypoint);
            return VA_STATUS_SUCCESS;
    }
    return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
}

#endif //_HAWAII_H
