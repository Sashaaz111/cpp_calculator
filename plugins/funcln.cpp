#include "PluginAPI.h"
#include <cmath>
#include <stdexcept>
#include <cstdio>
#include <string>

namespace {
    double compute_ln(double x) {
        if (x <= 0.0) {
            throw std::domain_error("логарифм ln(" + std::to_string(x) + ") не определен для неположительных чисел");
        }
        return std::log(x);
    }

    bool ln_eval(const double* args, std::size_t num_args, double* out_result, char* err_buf, std::size_t err_buf_size) {
        try {
            if (num_args != 1) {
                throw std::invalid_argument("функция ln принимает ровно 1 аргумент");
            }
            *out_result = compute_ln(args[0]);
            return true;
        }
        catch (const std::exception& e) {
            std::snprintf(err_buf, err_buf_size, "%s", e.what());
            return false;
        }
        catch (...) {
            std::snprintf(err_buf, err_buf_size, "неизвестное исключение в плагине ln");
            return false;
        }
    }

    const PluginInfo info = {
        "ln",
        PluginType::UNARY_FUNCTION,
        0,
        false,
        &ln_eval
    };
}

extern "C" PLUGIN_API const PluginInfo* get_plugin_info() {
    return &info;
}