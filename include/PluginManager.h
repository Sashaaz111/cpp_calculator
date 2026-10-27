#pragma once
#include "PluginAPI.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

class PluginManager {
public:
    PluginManager() = default;
    ~PluginManager();

    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    void loadPluginsFromDirectory(const std::filesystem::path& dir_path);

    const PluginInfo* getUnaryFunction(const std::string& name) const;
    const PluginInfo* getBinaryOperator(const std::string& op_symbol) const;
    bool isOperatorChar(char c) const;

private:
    struct LoadedLibrary {
        void* handle = nullptr;
        std::string filename;
    };

    std::vector<LoadedLibrary> libraries_;
    std::unordered_map<std::string, const PluginInfo*> unary_functions_;
    std::unordered_map<std::string, const PluginInfo*> binary_operators_;

    void unloadAll();
};