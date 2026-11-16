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
    bool tryLoadSinglePlugin(const std::filesystem::path& file_path);
    void registerPlugin(const PluginInfo* info, const std::string& filename);

    static bool isSupportedPluginFile(const std::filesystem::directory_entry& entry);
    static bool isValidPluginInfo(const PluginInfo* info);
    static void* openLibrary(const std::string& filepath);
    static GetPluginInfoFunc resolveEntryFunction(void* handle);
    static void closeLibrary(void* handle);
};
