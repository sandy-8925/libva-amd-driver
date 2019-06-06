#include "base.h"
#include "va_private.h"

static VAStatus CreateSurfaces2(
    VADriverContextP    context,
    uint32_t            format,
    uint32_t            width,
    uint32_t            height,
    VASurfaceID        *surfaces,
    uint32_t            num_surfaces,
    VASurfaceAttrib    *attrib_list,
    uint32_t            num_attribs
    )
{
    if(context == nullptr) return VA_STATUS_ERROR_INVALID_CONTEXT;
    
    if(width<=0 || height<=0) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(num_surfaces <= 0) return VA_STATUS_ERROR_INVALID_PARAMETER;
    if(surfaces == nullptr) return VA_STATUS_ERROR_INVALID_PARAMETER;
    
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
    
    
}

static VAStatus CreateSurfaces(VADriverContextP    context,
    int32_t             width,
    int32_t             height,
    int32_t             format,
    int32_t             num_surfaces,
    VASurfaceID        *surfaces)
{
    return CreateSurfaces2(context, format, width, height, surfaces, num_surfaces,
                              NULL, 0);
}

VAStatus vaDriverInit(VADriverContextP context) {
    if(context==nullptr || context->vtable==nullptr || context->vtable_vpp==nullptr)
    { return VA_STATUS_ERROR_INVALID_CONTEXT; }
    
    context->pDriverData = nullptr;
    context->version_major = VA_MAJOR_VERSION;
    context->version_minor = VA_MINOR_VERSION;
    context->max_profiles = VA_MAX_PROFILES;
    context->max_entrypoints = VA_MAX_ENTRYPOINTS;
    context->max_attributes = (int32_t)VAConfigAttribTypeMax;
    context->max_subpic_formats = VA_MAX_SUBPIC_FORMATS;
    context->max_display_attributes = VA_MAX_DISPLAY_ATTRIBS;
    context->str_vendor = VA_VENDOR_STRING;
    
    context->vtable->vaCreateSurfaces = CreateSurfaces;
    context->vtable->vaCreateSurfaces2 = CreateSurfaces2;
    
    return VA_STATUS_SUCCESS;
}

#define VA_DRIVER_FUNCTION(_major, _minor) __vaDriverInit_##_major##_##_minor
#define VA_DRIVER_INIT_FUNCTION VA_DRIVER_FUNCTION(VA_MAJOR_VERSION, VA_MINOR_VERSION)

VAStatus VA_DRIVER_INIT_FUNCTION(VADriverContextP context) {
    return vaDriverInit(context);
}
