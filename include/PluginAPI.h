#pragma once
#include <cstddef>

#if defined(_WIN32) || defined(_WIN64)
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __attribute__((visibility("default")))
#endif

enum class PluginType : int {
    UNARY_FUNCTION = 0,
    BINARY_OPERATOR = 1 
};

struct PluginInfo {
    const char* name;        
    PluginType type;        
    int precedence;          
    bool right_associative;  

    bool (*eval)(const double* args, std::size_t num_args, double* out_result, char* err_buf, std::size_t err_buf_size);
};

extern "C" {
    typedef const PluginInfo* (*GetPluginInfoFunc)();
}