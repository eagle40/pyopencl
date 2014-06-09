#include "kernel.h"
#include "context.h"
#include "device.h"
#include "program.h"
#include "memory_object.h"
#include "sampler.h"
#include "command_queue.h"
#include "event.h"
#include "clhelper.h"

namespace pyopencl {

kernel::~kernel()
{
    pyopencl_call_guarded_cleanup(clReleaseKernel, this);
}

generic_info
kernel::get_info(cl_uint param) const
{
    switch ((cl_kernel_info)param) {
    case CL_KERNEL_FUNCTION_NAME:
        return pyopencl_get_str_info(Kernel, this, param);
    case CL_KERNEL_NUM_ARGS:
    case CL_KERNEL_REFERENCE_COUNT:
        return pyopencl_get_int_info(cl_uint, Kernel, this, param);
    case CL_KERNEL_CONTEXT:
        return pyopencl_get_opaque_info(context, Kernel, this, param);
    case CL_KERNEL_PROGRAM:
        return pyopencl_get_opaque_info(program, Kernel, this, param);
#if PYOPENCL_CL_VERSION >= 0x1020
    case CL_KERNEL_ATTRIBUTES:
        return pyopencl_get_str_info(Kernel, this, param);
#endif
    default:
        throw clerror("Kernel.get_info", CL_INVALID_VALUE);
    }
}

generic_info
kernel::get_work_group_info(cl_kernel_work_group_info param,
                            const device *dev) const
{
    switch (param) {
#if PYOPENCL_CL_VERSION >= 0x1010
    case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
#endif
    case CL_KERNEL_WORK_GROUP_SIZE:
        return pyopencl_get_int_info(size_t, KernelWorkGroup, this, dev, param);
    case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
        return pyopencl_get_array_info(size_t, KernelWorkGroup,
                                       this, dev, param);
    case CL_KERNEL_LOCAL_MEM_SIZE:
#if PYOPENCL_CL_VERSION >= 0x1010
    case CL_KERNEL_PRIVATE_MEM_SIZE:
#endif
        return pyopencl_get_int_info(cl_ulong, KernelWorkGroup,
                                     this, dev, param);
    default:
        throw clerror("Kernel.get_work_group_info", CL_INVALID_VALUE);
    }
}

}

// c wrapper
// Import all the names in pyopencl namespace for c wrappers.
using namespace pyopencl;

// Kernel
error*
create_kernel(clobj_t *knl, clobj_t _prog, const char *name)
{
    auto prog = static_cast<const program*>(_prog);
    return c_handle_error([&] {
            *knl = new kernel(pyopencl_call_guarded(clCreateKernel, prog,
                                                    name), false);
        });
}

error*
kernel__set_arg_null(clobj_t _knl, cl_uint arg_index)
{
    auto knl = static_cast<kernel*>(_knl);
    return c_handle_error([&] {
            const cl_mem m = 0;
            pyopencl_call_guarded(clSetKernelArg, knl,
                                  arg_index, make_sizearg(m));
        });
}

error*
kernel__set_arg_mem(clobj_t _knl, cl_uint arg_index, clobj_t _mem)
{
    auto knl = static_cast<kernel*>(_knl);
    auto mem = static_cast<memory_object*>(_mem);
    return c_handle_error([&] {
            pyopencl_call_guarded(clSetKernelArg, knl, arg_index,
                                  make_sizearg(mem->data()));
        });
}

error*
kernel__set_arg_sampler(clobj_t _knl, cl_uint arg_index, clobj_t _samp)
{
    auto knl = static_cast<kernel*>(_knl);
    auto samp = static_cast<sampler*>(_samp);
    return c_handle_error([&] {
            pyopencl_call_guarded(clSetKernelArg, knl, arg_index,
                                  make_sizearg(samp->data()));
        });
}

error*
kernel__set_arg_buf(clobj_t _knl, cl_uint arg_index,
                    const void *buffer, size_t size)
{
    auto knl = static_cast<kernel*>(_knl);
    return c_handle_error([&] {
            pyopencl_call_guarded(clSetKernelArg, knl, arg_index, size, buffer);
        });
}

error*
kernel__get_work_group_info(clobj_t _knl, cl_kernel_work_group_info param,
                            clobj_t _dev, generic_info *out)
{
    auto knl = static_cast<kernel*>(_knl);
    auto dev = static_cast<device*>(_dev);
    return c_handle_error([&] {
            *out = knl->get_work_group_info(param, dev);
        });
}

error*
enqueue_nd_range_kernel(clobj_t *_evt, clobj_t _queue, clobj_t _knl,
                        cl_uint work_dim, const size_t *global_work_offset,
                        const size_t *global_work_size,
                        const size_t *local_work_size,
                        const clobj_t *_wait_for, uint32_t num_wait_for)
{
    auto queue = static_cast<command_queue*>(_queue);
    auto knl = static_cast<kernel*>(_knl);
    const auto wait_for = buf_from_class<event>(_wait_for, num_wait_for);
    return c_handle_error([&] {
            cl_event evt;
            retry_mem_error([&] {
                    pyopencl_call_guarded(
                        clEnqueueNDRangeKernel, queue, knl,
                        work_dim, global_work_offset, global_work_size,
                        local_work_size, wait_for, &evt);
                });
            *_evt = new_event(evt);
        });
}

error*
enqueue_task(clobj_t *_evt, clobj_t _queue, clobj_t _knl,
             const clobj_t *_wait_for, uint32_t num_wait_for)
{
    auto queue = static_cast<command_queue*>(_queue);
    auto knl = static_cast<kernel*>(_knl);
    const auto wait_for = buf_from_class<event>(_wait_for, num_wait_for);
    return c_handle_error([&] {
            cl_event evt;
            retry_mem_error([&] {
                    pyopencl_call_guarded(
                        clEnqueueTask, queue, knl, wait_for, &evt);
                });
            *_evt = new_event(evt);
        });
}
