#include "memory_object.h"
#include "clhelper.h"

#ifndef __PYOPENCL_IMAGE_H
#define __PYOPENCL_IMAGE_H

namespace pyopencl {

// {{{ image

class image : public memory_object {
private:
    cl_image_format m_format;
public:
    PYOPENCL_DEF_CL_CLASS(IMAGE);
    PYOPENCL_INLINE
    image(cl_mem mem, bool retain, void *hostbuf=0,
          const cl_image_format *fmt=0)
        : memory_object(mem, retain, hostbuf)
    {
        if (fmt) {
            m_format = *fmt;
        } else {
            pyopencl_call_guarded(clGetImageInfo, this, CL_IMAGE_FORMAT,
                                  make_sizearg(m_format), nullptr);
        }
    }
    PYOPENCL_USE_RESULT generic_info get_image_info(cl_image_info param) const;
    PYOPENCL_INLINE type_t
    get_fill_type()
    {
        switch (m_format.image_channel_data_type) {
        case CL_SIGNED_INT8:
        case CL_SIGNED_INT16:
        case CL_SIGNED_INT32:
            return TYPE_INT;
        case CL_UNSIGNED_INT8:
        case CL_UNSIGNED_INT16:
        case CL_UNSIGNED_INT32:
            return TYPE_UINT;
        default:
            return TYPE_FLOAT;
        }
    }
};
PYOPENCL_USE_RESULT static PYOPENCL_INLINE image*
new_image(cl_mem mem, void *buff, const cl_image_format *fmt)
{
    return pyopencl_convert_obj(image, clReleaseMemObject, mem, buff, fmt);
}

// }}}

}

#endif
