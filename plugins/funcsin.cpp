#include "PluginAPI.h"
#include <cmath>
#include <stdexcept>
#include <cstdio>

namespace {
    constexpr double PI = 3.14159265358979323846;

    double compute_sin_deg(double degrees) {
        if (!std::isfinite(degrees)) {
            throw std::domain_error("аргумент sin должен быть конечным числом");
        }
        double radians = degrees * (PI / 180.0);
        return std::sin(radians);
    }

    bool sin_eval(const double* args, std::size_t num_args, double* out_result, char* err_buf, std::size_t err_buf_size) {
        try {
            if (num_args != 1) {
                throw std::invalid_argument("функция sin принимает ровно 1 аргумент");
            }
            *out_result = compute_sin_deg(args[0]);
            return true;
        }
        catch (const std::exception& e) {
            std::snprintf(err_buf, err_buf_size, "%s", e.what());
            return false;
        }
        catch (...) {
            std::snprintf(err_buf, err_buf_size, "неизвестное исключение в плагине sin");
            return false;
        }
    }

    const PluginInfo info = {
        "sin",
        PluginType::UNARY_FUNCTION,
        0,
        false,
        &sin_eval
    };
}

extern "C" PLUGIN_API const PluginInfo* get_plugin_info() {
    return &info;
}