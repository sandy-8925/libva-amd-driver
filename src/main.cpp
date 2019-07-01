#include "base.h"
#include "va_private.h"
#include "DataTables.h"
#include "hawaii.h"

#include <cstring>
#include <stdlib.h>

using namespace std;

class Surface {
public:
    uint32_t width, height;
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
        
        ChipData* getChipData() { return chipData; }
        
        /*
         * Clears all data within this object
         */
        void cleanup();
};

void DriverData::cleanup() {
    surfaceTable.clear();
    imageTable.clear();
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
    bool found = false;
    for(auto supProf : supportedProfiles)
    {
        if(supProf == profile) {
            found = true;
            break;
        }
    }
    if(!found) return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
    
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
    
    return VA_STATUS_SUCCESS;
}

#define VA_DRIVER_FUNCTION(_major, _minor) __vaDriverInit_##_major##_##_minor
#define VA_DRIVER_INIT_FUNCTION VA_DRIVER_FUNCTION(1, 4)

extern "C" {
VAStatus VA_DRIVER_INIT_FUNCTION(VADriverContextP context) {
    return vaDriverInit(context);
}
}
