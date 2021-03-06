#include "base.h"
#include "va_private.h"
#include "DataTables.h"
#include "hawaii.h"

#include <cstring>
#include <stdlib.h>
#include <algorithm>

using namespace std;

class Surface {
public:
    uint32_t width, height;
    VASurfaceStatus status = VASurfaceReady;
};

class Config {
    public:
        VAProfile profile;
        VAEntrypoint entrypoint;
        vector<VAConfigAttrib> configAttributes;
};

class Context
{
    public:
        Context(
            VAConfigID configId,
            int picture_width,
            int picture_height,
            int flag,
            VASurfaceID *render_targets,
            int num_render_targets
            )
        {
            this->configId = configId;
            this->picture_height = picture_height;
            this->picture_width = picture_width;
            this->flag = flag;
            VASurfaceID* temp = render_targets;
            for(int ctr=0; ctr<num_render_targets; ctr++)
            {
                this->render_targets.push_back(*temp);
                temp++;
            }
        }
    
        VAConfigID configId;
        int picture_width;
		int picture_height;
		int flag;
		vector<VASurfaceID> render_targets;
};

class DriverData
{
    private:
        ChipData* chipData;
    public:
        DriverData(ChipData* newChipData)
        { chipData = newChipData; }
        DataTable<VASurfaceID, Surface> surfaceTable;
        DataTable<VAImageID, VAImage> imageTable;
        DataTable<VAConfigID, Config> configTable;
        DataTable<VAContextID, Context> contextTable;
        
        ChipData* getChipData() { return chipData; }
        
        /*
         * Clears all data within this object
         */
        void cleanup();
};

void DriverData::cleanup() {
    surfaceTable.clear();
    imageTable.clear();
    configTable.clear();
}

#define GET_DRIVER_DATA(context) (DriverData*)context->pDriverData

static VAStatus CreateSurfaces2(
    VADriverContextP    context,
    unsigned int        format,
    unsigned int        width,
    unsigned int        height,
    VASurfaceID        *surfaces,
    unsigned int        num_surfaces,
    VASurfaceAttrib    *attrib_list,
    unsigned int        num_attribs
    )
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    if(width<=0 || height<=0)
    {
        context->info_callback(context, "Width and/or height is negative/zero");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    if(num_surfaces <= 0)
    {
        context->info_callback(context, "Invalid value for num_surfaces parameter");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    if(surfaces == nullptr)
    {
        context->info_callback(context, "surfaces pointer is null");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    switch(format) {
        case VA_RT_FORMAT_YUV420:
        case VA_RT_FORMAT_YUV422:
        case VA_RT_FORMAT_YUV444:
        case VA_RT_FORMAT_YUV420_10BPP:
        case VA_RT_FORMAT_RGB32:
            break;
        default:
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }
    
    if(num_attribs < 0)
    {
        context->info_callback(context, "Invalid value for num_attribs parameter");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    if(num_attribs>0 && attrib_list==nullptr)
    {
        context->info_callback(context, "num_attribs is positive, but attrib_list is NULL");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    //TODO: Check the attrib_list and num_attribs parameters and handle appropriately
    
    memset(surfaces, VA_INVALID_ID, num_surfaces * sizeof(VASurfaceID));
    
    for(int ctr=0; ctr<num_surfaces; ctr++) {
        //TODO: Handle actual allocation of surfaces as well
        Surface *surf = new Surface;
        surf->width = width;
        surf->height = height;
        surfaces[ctr] = driverData->surfaceTable.insert(surf);
    }
    
    return VA_STATUS_SUCCESS;
}

static VAStatus CreateSurfaces(VADriverContextP    context,
    int             width,
    int             height,
    int             format,
    int             num_surfaces,
    VASurfaceID        *surfaces)
{
    return CreateSurfaces2(context, format, width, height, surfaces, num_surfaces,
                              NULL, 0);
}

static VAStatus DeriveImage(
		VADriverContextP context,
		VASurfaceID surfaceId,
		VAImage *image     /* out */)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
 
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    Surface* surface = driverData->surfaceTable.getValue(surfaceId);
    if(surface == nullptr) return VA_STATUS_ERROR_INVALID_SURFACE;
    
    VAImage* img = new VAImage;
    if(img == nullptr) return VA_STATUS_ERROR_ALLOCATION_FAILED;
    
    img->image_id = driverData->imageTable.insert(img);
    //TODO: Setup img->format
    img->width = surface->width;
    img->height = surface->height;
    img->num_palette_entries = 0;
    img->entry_bytes = 0;
    
    *image = *img;
    return VA_STATUS_SUCCESS;
}

VAStatus QueryConfigProfiles(VADriverContextP context, VAProfile *profile_list, int *num_profiles)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    if(profile_list == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(num_profiles == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
    vector<VAProfile> supportedProfiles = driverData->getChipData()->getSupportedVaProfiles();
    VAProfile *temp = profile_list;
    for(auto profile : supportedProfiles)
    {
        *temp = profile;
        temp++;
    }
    *num_profiles = supportedProfiles.size();
    
    return VA_STATUS_SUCCESS;
}

VAStatus QueryConfigEntrypoints (VADriverContextP context, VAProfile profile, VAEntrypoint *entrypoint_list, int *num_entrypoints)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    if(entrypoint_list == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(num_entrypoints == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;      
    
    vector<VAProfile> supportedProfiles = driverData->getChipData()->getSupportedVaProfiles();
    if(find(supportedProfiles.begin(), supportedProfiles.end(), profile) == supportedProfiles.end()) return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
    
    vector<VAEntrypoint> supportedEntryPoints = driverData->getChipData()->getSupportedEntryPoints(profile);
    
    VAEntrypoint *temp = entrypoint_list;
    for(auto entrypoint : supportedEntryPoints)
    {
        *temp = entrypoint;
        temp++;
    }
    
    *num_entrypoints = supportedEntryPoints.size();
    
    return VA_STATUS_SUCCESS;
}

VAStatus DriverTerminate( VADriverContextP context)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    driverData->cleanup();
    delete driverData;
    context->pDriverData = driverData = nullptr;
    
    return VA_STATUS_SUCCESS;
}

VAStatus QueryDisplayAttributes(VADriverContextP context, VADisplayAttribute *attr_list, int *num_attribs) 
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    if(attr_list == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(num_attribs == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
    num_attribs = 0;
    
    return VA_STATUS_SUCCESS;
}

VAStatus QueryImageFormats(VADriverContextP context, VAImageFormat *format_list, int *num_formats)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    if(format_list == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(num_formats == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
    vector<VAImageFormat> imageFormats = driverData->getChipData()->getSupportedImageFormats();
    VAImageFormat *temp = format_list;
    for(auto format : imageFormats)
    {
        *temp = format;
        temp++;
    }
    *num_formats = imageFormats.size();
    
    return VA_STATUS_SUCCESS;
}

static VAImageFormat subpicFormats[] = {
   {
   .fourcc = VA_FOURCC_BGRA,
   .byte_order = VA_LSB_FIRST,
   .bits_per_pixel = 32,
   .depth = 32,
   .red_mask   = 0x00ff0000ul,
   .green_mask = 0x0000ff00ul,
   .blue_mask  = 0x000000fful,
   .alpha_mask = 0xff000000ul,
   },
};

VAStatus QuerySubpictureFormats(VADriverContextP context, VAImageFormat *format_list, unsigned int *flags, unsigned int *num_formats)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    if(format_list == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(flags == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(num_formats == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
    memcpy(format_list, subpicFormats, sizeof(subpicFormats));
    *num_formats = sizeof(subpicFormats)/sizeof(subpicFormats[0]);
    
    return VA_STATUS_SUCCESS;
}

void getVLDConfigAttributes(ChipData *chipData, VAProfile profile, VAConfigAttrib *attrib_list, int num_attribs)
{
    for(int ctr=0; ctr<num_attribs; ctr++)
    {
        switch(attrib_list[ctr].type)
        {
            case VAConfigAttribRTFormat:
                attrib_list[ctr].value = chipData->getConfigAttribRTFormat(profile);
                break;
            default:
                attrib_list[ctr].value = VA_ATTRIB_NOT_SUPPORTED;
        }
    }
}

void getEncSliceConfigAttributes(ChipData *chipData, VAProfile profile, VAConfigAttrib *attrib_list, int num_attribs)
{
    for(int ctr=0; ctr<num_attribs; ctr++)
    {
        switch(attrib_list[ctr].type)
        {
            case VAConfigAttribRTFormat:
                attrib_list[ctr].value = chipData->getConfigAttribRTFormat(profile);
                break;
            case VAConfigAttribRateControl:
                attrib_list[ctr].value = VA_RC_CQP | VA_RC_CBR | VA_RC_VBR;
                break;
            case VAConfigAttribEncPackedHeaders:
                attrib_list[ctr].value = 0;
                break;
            case VAConfigAttribEncMaxRefFrames:
                attrib_list[ctr].value = 1;
                break;
            default:
                attrib_list[ctr].value = VA_ATTRIB_NOT_SUPPORTED;
        }
    }
}

void getVideoProcConfigAttributes(VAConfigAttrib *attrib_list, int num_attribs)
{
    for(int ctr=0; ctr<num_attribs; ctr++)
    {
        switch(attrib_list[ctr].type)
        {
            case VAConfigAttribRTFormat:
                attrib_list[ctr].value = (VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_RGB32);
                break;
            default:
                attrib_list[ctr].value = VA_ATTRIB_NOT_SUPPORTED;
        }
    }
}

VAStatus GetConfigAttributes(VADriverContextP context, VAProfile profile, VAEntrypoint entrypoint, VAConfigAttrib *attrib_list, int num_attribs)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    vector<VAProfile> supportedProfiles = driverData->getChipData()->getSupportedVaProfiles();
    if(find(supportedProfiles.begin(), supportedProfiles.end(), profile) == supportedProfiles.end()) return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
    
    vector<VAEntrypoint> supportedEntryPoints = driverData->getChipData()->getSupportedEntryPoints(profile);
    if(find(supportedEntryPoints.begin(), supportedEntryPoints.end(), entrypoint) == supportedEntryPoints.end()) return VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
    
    if(attrib_list == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
    switch(entrypoint)
    {
        case VAEntrypointVLD:
            getVLDConfigAttributes(driverData->getChipData(), profile, attrib_list, num_attribs);
            break;
        case VAEntrypointEncSlice:
            getEncSliceConfigAttributes(driverData->getChipData(), profile, attrib_list, num_attribs);
            break;
        case VAEntrypointVideoProc:
            getVideoProcConfigAttributes(attrib_list, num_attribs);
            break;
        default:
            for(int ctr=0; ctr<num_attribs; ctr++)
            { attrib_list[ctr].value = VA_ATTRIB_NOT_SUPPORTED; }
    }
    
    return VA_STATUS_SUCCESS;
}

bool isRTFormatSupported(ChipData* chipData, VAProfile profile, VAEntrypoint entrypoint, uint32_t rtFormat)
{
    switch(entrypoint)
    {
        case VAEntrypointVLD:
        case VAEntrypointEncSlice:
            return rtFormat & chipData->getConfigAttribRTFormat(profile);
            break;
        case VAEntrypointVideoProc:
            return rtFormat & (VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_RGB32);
            break;
        default:
            return false;
    }
}

VAStatus CreateConfig(
		VADriverContextP context,
		VAProfile profile, 
		VAEntrypoint entrypoint, 
		VAConfigAttrib *attrib_list,
		int num_attribs,
		VAConfigID *config_id
	)
{
    if(context==nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    vector<VAProfile> supportedProfiles = driverData->getChipData()->getSupportedVaProfiles();
    if(find(supportedProfiles.begin(), supportedProfiles.end(), profile) == supportedProfiles.end()) return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
    
    vector<VAEntrypoint> supportedEntryPoints = driverData->getChipData()->getSupportedEntryPoints(profile);
    if(find(supportedEntryPoints.begin(), supportedEntryPoints.end(), entrypoint) == supportedEntryPoints.end()) return VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
    
    if(num_attribs < 0)
    {
        context->info_callback(context, "Invalid value for num_attribs parameter");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    if(num_attribs>0 && attrib_list==nullptr)
    {
        context->info_callback(context, "num_attribs is positive, but attrib_list is NULL");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    if(config_id == nullptr)
    {
        context->info_callback(context, "config_id parameter is null");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    Config* config = new Config;
    config->profile = profile;
    config->entrypoint = entrypoint;
    
    for(int ctr=0; ctr<num_attribs; ctr++)
    {
        switch(attrib_list[ctr].type)
        {
            case VAConfigAttribRTFormat:
                if(isRTFormatSupported(driverData->getChipData(), profile, entrypoint, attrib_list[ctr].value))
                { config->configAttributes.push_back(attrib_list[ctr]); }
                else 
                { 
                    delete config;
                    return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
                }
                break;
            case VAConfigAttribRateControl:
                if(entrypoint==VAEntrypointEncSlice && (attrib_list[ctr].value & (VA_RC_CQP | VA_RC_CBR | VA_RC_VBR)))
                { config->configAttributes.push_back(attrib_list[ctr]); }
                break;
            case VAConfigAttribEncPackedHeaders:
                if(entrypoint==VAEntrypointEncSlice && attrib_list[ctr].value==0)
                { config->configAttributes.push_back(attrib_list[ctr]); }
                break;
            case VAConfigAttribEncMaxRefFrames:
                if(entrypoint==VAEntrypointEncSlice && attrib_list[ctr].value==1)
                { config->configAttributes.push_back(attrib_list[ctr]); }
                break;
        }
    }
    
    *config_id = driverData->configTable.insert(config);
    
    return VA_STATUS_SUCCESS;
}

VAStatus DestroyConfig(VADriverContextP context, VAConfigID config_id)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    if(driverData->configTable.getValue(config_id) == nullptr) return VA_STATUS_ERROR_INVALID_CONFIG;
    driverData->configTable.deleteValue(config_id);
    
    return VA_STATUS_SUCCESS;
}

VAStatus QueryConfigAttributes(VADriverContextP context,
		VAConfigID config_id, 
		VAProfile *profile,		/* out */
		VAEntrypoint *entrypoint, 	/* out */
		VAConfigAttrib *attrib_list,	/* out */
		int *num_attribs		/* out */
	)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    if(profile == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(entrypoint == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(attrib_list == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(num_attribs == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
    Config* config = driverData->configTable.getValue(config_id);
    if(config == nullptr) return VA_STATUS_ERROR_INVALID_CONFIG;
    
    *profile = config->profile;
    *entrypoint = config->entrypoint;
    VAConfigAttrib* temp = attrib_list;
    for(auto attrib : config->configAttributes)
    {
        *temp = attrib;
        *temp++;
    }
    *num_attribs = config->configAttributes.size();
    
    return VA_STATUS_SUCCESS;
}

VAStatus CreateContext(VADriverContextP driverContext,
        VAConfigID config_id,
		int picture_width,
		int picture_height,
		int flag,
		VASurfaceID *render_targets,
		int num_render_targets,
		VAContextID *contextId
	)
{
    if(driverContext == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(driverContext);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    Config* config = driverData->configTable.getValue(config_id);
    if(config == nullptr) return VA_STATUS_ERROR_INVALID_CONFIG;
    
    if(picture_height<=0 || picture_width<=0) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(render_targets == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(num_render_targets <= 0) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(contextId == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
    Context* context = new Context(config_id, picture_width, picture_height, flag, render_targets, num_render_targets);
    *contextId = driverData->contextTable.insert(context);
    
    return VA_STATUS_SUCCESS;
}

VAStatus CreateMFContext(VADriverContextP context, VAMFContextID *mfe_context)
{ return VA_STATUS_ERROR_UNIMPLEMENTED; }

VAStatus DestroyContext(VADriverContextP driverContext, VAContextID contextId)
{
    if(driverContext == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(driverContext);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    Context* context = driverData->contextTable.getValue(contextId);
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    driverData->contextTable.deleteValue(contextId);
    
    return VA_STATUS_SUCCESS;
}

VAStatus QuerySurfaceStatus(VADriverContextP context, VASurfaceID render_target, VASurfaceStatus *status)
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    DriverData* driverData = GET_DRIVER_DATA(context);
    if(driverData == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
           
    Surface* surface = driverData->surfaceTable.getValue(render_target);
    if(surface == nullptr) return VA_STATUS_ERROR_INVALID_SURFACE;
    
    if(status == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
    *status = surface->status;
    
    return VA_STATUS_SUCCESS;
}

VAStatus vaDriverInit(VADriverContextP context) {
    if(context==nullptr || context->vtable==nullptr || context->vtable_vpp==nullptr)
    { return VA_STATUS_ERROR_INVALID_CONTEXT; }
    
    context->pDriverData = new DriverData(new HawaiiChipData);
    context->version_major = VA_MAJOR_VERSION;
    context->version_minor = VA_MINOR_VERSION;
    context->max_profiles = VA_MAX_PROFILES;
    context->max_entrypoints = VA_MAX_ENTRYPOINTS;
    context->max_attributes = VAConfigAttribTypeMax;
    context->max_subpic_formats = VA_MAX_SUBPIC_FORMATS;
    context->max_display_attributes = VA_MAX_DISPLAY_ATTRIBS;
    context->max_image_formats = VA_MAX_IMAGE_FORMATS;
    context->str_vendor = VA_VENDOR_STRING;
    
    context->vtable->vaTerminate = DriverTerminate;
    context->vtable->vaCreateSurfaces = CreateSurfaces;
    context->vtable->vaCreateSurfaces2 = CreateSurfaces2;
    context->vtable->vaDeriveImage = DeriveImage;
    context->vtable->vaQueryConfigProfiles = QueryConfigProfiles;
    context->vtable->vaQueryConfigEntrypoints = QueryConfigEntrypoints;
    context->vtable->vaQueryDisplayAttributes = QueryDisplayAttributes;
    context->vtable->vaQueryImageFormats = QueryImageFormats;
    context->vtable->vaQuerySubpictureFormats = QuerySubpictureFormats;
    context->vtable->vaGetConfigAttributes = GetConfigAttributes;
    context->vtable->vaCreateConfig = CreateConfig;
    context->vtable->vaDestroyConfig = DestroyConfig;
    context->vtable->vaQueryConfigAttributes = QueryConfigAttributes;
    context->vtable->vaCreateContext = CreateContext;
    context->vtable->vaCreateMFContext = CreateMFContext;
    context->vtable->vaDestroyContext = DestroyContext;
    context->vtable->vaQuerySurfaceStatus = QuerySurfaceStatus;
    
    return VA_STATUS_SUCCESS;
}

#define VA_DRIVER_FUNCTION(_major, _minor) __vaDriverInit_##_major##_##_minor
#define VA_DRIVER_INIT_FUNCTION VA_DRIVER_FUNCTION(1, 4)

extern "C" {
VAStatus VA_DRIVER_INIT_FUNCTION(VADriverContextP context) {
    return vaDriverInit(context);
}
}
