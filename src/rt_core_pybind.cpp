#include <pybind11/pybind11.h>
#include "rt_core.h"

namespace py = pybind11;

PYBIND11_MODULE(rt_core_py, m)
{
    m.def("target_width", &rt_core_target_width);
    m.def("target_height", &rt_core_target_height);
}