#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <pybind11/numpy.h>

#include "betting_by_time/strategies/vanilla_betting.hpp"
#include "betting_by_time/strategies/adaptive_betting.hpp"

namespace py = pybind11;
using namespace betting;

PYBIND11_MODULE(betting_by_time_cxx, m) {
    m.doc() = "Python bindings for betting-by-time C++ implementation";

    py::enum_<Mode>(m, "Mode")
        .value("Estimate", Mode::Estimate)
        .value("Bound", Mode::Bound)
        .export_values();

    m.def("vanilla_betting_geo", [](const Vector32f& samples,
                                     Float32 prior_mean,
                                     Float32 delta,
                                     Int32 grid_num,
                                     const std::vector<Int32>& breakpoints,
                                     Float32 gambler_alpha,
                                     Float32 gambler_trunc_scale,
                                     Float32 gambler_prior_var,
                                     Int32 gambler_num,
                                     Int32 gambler_sample_num,
                                     Mode mode) {
        auto [est, lb, ub, used] = vanilla_betting<GeoCheckingCapital>(
            samples, prior_mean, delta, grid_num, breakpoints,
            gambler_alpha, gambler_trunc_scale, gambler_prior_var,
            gambler_num, gambler_sample_num, mode);
        return py::make_tuple(est, lb, ub, used);
    }, py::arg("samples"),
       py::arg("prior_mean"),
       py::arg("delta"),
       py::arg("grid_num"),
       py::arg("breakpoints") = std::vector<Int32>{},
       py::arg("gambler_alpha") = 0.05f,
       py::arg("gambler_trunc_scale") = 0.5f,
       py::arg("gambler_prior_var") = 0.25f,
       py::arg("gambler_num") = 1,
       py::arg("gambler_sample_num") = 100010,
       py::arg("mode") = Mode::Estimate);

    m.def("vanilla_betting_seq", [](const Vector32f& samples,
                                     Float32 prior_mean,
                                     Float32 delta,
                                     Int32 grid_num,
                                     const std::vector<Int32>& breakpoints,
                                     Float32 gambler_alpha,
                                     Float32 gambler_trunc_scale,
                                     Float32 gambler_prior_var,
                                     Int32 gambler_num,
                                     Int32 gambler_sample_num,
                                     Mode mode) {
        auto [est, lb, ub, used] = vanilla_betting<SequenceCheckingCapital>(
            samples, prior_mean, delta, grid_num, breakpoints,
            gambler_alpha, gambler_trunc_scale, gambler_prior_var,
            gambler_num, gambler_sample_num, mode);
        return py::make_tuple(est, lb, ub, used);
    }, py::arg("samples"),
       py::arg("prior_mean"),
       py::arg("delta"),
       py::arg("grid_num"),
       py::arg("breakpoints") = std::vector<Int32>{},
       py::arg("gambler_alpha") = 0.05f,
       py::arg("gambler_trunc_scale") = 0.5f,
       py::arg("gambler_prior_var") = 0.25f,
       py::arg("gambler_num") = 1,
       py::arg("gambler_sample_num") = 100100,
       py::arg("mode") = Mode::Estimate);

    m.def("adaptive_betting_geo", [](const Vector32f& samples,
                                      Float32 prior_mean,
                                      Float32 delta,
                                      Int32 grid_num,
                                      const std::vector<Int32>& breakpoints,
                                      Float32 gambler_alpha,
                                      Float32 gambler_trunc_scale,
                                      Float32 gambler_prior_var,
                                      Int32 gambler_num,
                                      Int32 gambler_sample_num,
                                      Mode mode) {
        auto [est, lb, ub, used] = adaptive_betting<GeoCheckingCapital>(
            samples, prior_mean, delta, grid_num, breakpoints,
            gambler_alpha, gambler_trunc_scale, gambler_prior_var,
            gambler_num, gambler_sample_num, mode);
        return py::make_tuple(est, lb, ub, used);
    }, py::arg("samples"),
       py::arg("prior_mean"),
       py::arg("delta"),
       py::arg("grid_num"),
       py::arg("breakpoints") = std::vector<Int32>{},
       py::arg("gambler_alpha") = 0.05f,
       py::arg("gambler_trunc_scale") = 0.5f,
       py::arg("gambler_prior_var") = 0.25f,
       py::arg("gambler_num") = 1,
       py::arg("gambler_sample_num") = 100100,
       py::arg("mode") = Mode::Estimate);

    m.def("adaptive_betting_seq", [](const Vector32f& samples,
                                      Float32 prior_mean,
                                      Float32 delta,
                                      Int32 grid_num,
                                      const std::vector<Int32>& breakpoints,
                                      Float32 gambler_alpha,
                                      Float32 gambler_trunc_scale,
                                      Float32 gambler_prior_var,
                                      Int32 gambler_num,
                                      Int32 gambler_sample_num,
                                      Mode mode) {
        auto [est, lb, ub, used] = adaptive_betting<SequenceCheckingCapital>(
            samples, prior_mean, delta, grid_num, breakpoints,
            gambler_alpha, gambler_trunc_scale, gambler_prior_var,
            gambler_num, gambler_sample_num, mode);
        return py::make_tuple(est, lb, ub, used);
    }, py::arg("samples"),
       py::arg("prior_mean"),
       py::arg("delta"),
       py::arg("grid_num"),
       py::arg("breakpoints") = std::vector<Int32>{},
       py::arg("gambler_alpha") = 0.05f,
       py::arg("gambler_trunc_scale") = 0.5f,
       py::arg("gambler_prior_var") = 0.25f,
       py::arg("gambler_num") = 1,
       py::arg("gambler_sample_num") = 100100,
       py::arg("mode") = Mode::Estimate);
}
