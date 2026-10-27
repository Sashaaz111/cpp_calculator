#include "PluginAPI.h"
#include <cmath>
#include <stdexcept>
#include <cstdio>

namespace {
    double compute_pow(double base, double exp) {
        if (base == 0.0 && exp <= 0.0) {
            throw std::domain_error("возведение нуля в неположительную степень не определено");
        }
        if (base < 0.0 && std::floor(exp) != exp) {
            throw std::domain_error("возведение отрицательного числа в дробную степень");
        }
        double res = std::pow(base, exp);
        if (!std::isfinite(res)) {
            throw std::overflow_error("переполнение при возведении в степень");
        }
        return res;
    }

    bool pow_eval(const double* args, std::size_t num_args, double* out_result, char* err_buf, std::size_t err_buf_size) {
        try {
            if (num_args != 2) {
                throw std::invalid_argument("оператор '^' требует 2 аргумента");
            }
            *out_result = compute_pow(args[0], args[1]);
            return true;
        }
        catch (const std::exception& e) {
            std::snprintf(err_buf, err_buf_size, "%s", e.what());
            return false;
        }
        catch (...) {
            std::snprintf(err_buf, err_buf_size, "неизвестное исключение в плагине '^'");
            return false;
        }
    }

    const PluginInfo info = {
        "^",
        PluginType::BINARY_OPERATOR,
        3,     
        true, 
        &pow_eval
    };
}

extern "C" PLUGIN_API const PluginInfo* get_plugin_info() {
    return &info;
}