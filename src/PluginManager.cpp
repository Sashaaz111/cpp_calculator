#include "PluginManager.h"
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

PluginManager::~PluginManager() {
    unloadAll();
}

void* PluginManager::openLibrary(const std::string& filepath) {
#if defined(_WIN32) || defined(_WIN64)
    return static_cast<void*>(LoadLibraryA(filepath.c_str()));
#else
    return dlopen(filepath.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
}

GetPluginInfoFunc PluginManager::resolveEntryFunction(void* handle) {
#if defined(_WIN32) || defined(_WIN64)
    return reinterpret_cast<GetPluginInfoFunc>(
        GetProcAddress(static_cast<HMODULE>(handle), "get_plugin_info"));
#else
    return reinterpret_cast<GetPluginInfoFunc>(dlsym(handle, "get_plugin_info"));
#endif
}

void PluginManager::closeLibrary(void* handle) {
    if (!handle) {
        return;
    }
#if defined(_WIN32) || defined(_WIN64)
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

void PluginManager::unloadAll() {
    unary_functions_.clear();
    binary_operators_.clear();

    for (auto& lib : libraries_) {
        closeLibrary(lib.handle);
        lib.handle = nullptr;
    }
    libraries_.clear();
}

bool PluginManager::isSupportedPluginFile(const std::filesystem::directory_entry& entry) {
    std::error_code ec;
    if (!entry.is_regular_file(ec)) {
        return false;
    }
    const std::string ext = entry.path().extension().string();
#if defined(_WIN32) || defined(_WIN64)
    return ext == ".dll";
#elif defined(__APPLE__)
    return ext == ".dylib" || ext == ".so";
#else
    return ext == ".so";
#endif
}

bool PluginManager::isValidPluginInfo(const PluginInfo* info) {
    if (!info || !info->name || !info->eval) {
        return false;
    }
    return !std::string(info->name).empty();
}

void PluginManager::registerPlugin(const PluginInfo* info, const std::string& filename) {
    std::string symbol_name(info->name);
    if (info->type == PluginType::UNARY_FUNCTION) {
        unary_functions_[symbol_name] = info;
        std::cout << "Функция: " << symbol_name << "(x) из файла " << filename << "\n";
    }
    else if (info->type == PluginType::BINARY_OPERATOR) {
        binary_operators_[symbol_name] = info;
        std::cout << "Оператор: '" << symbol_name << "' из файла " << filename << "\n";
    }
}

bool PluginManager::tryLoadSinglePlugin(const std::filesystem::path& file_path) {
    const std::string filepath = file_path.string();
    const std::string filename = file_path.filename().string();

    void* handle = openLibrary(filepath);
    if (!handle) {
        std::cerr << "Не удалось загрузить библиотеку '" << filename << "'.\n";
        return false;
    }

    GetPluginInfoFunc get_info = resolveEntryFunction(handle);
    if (!get_info) {
        std::cerr << "В '" << filename << "' не найдена точка входа 'get_plugin_info'.\n";
        closeLibrary(handle);
        return false;
    }

    const PluginInfo* info = nullptr;
    try {
        info = get_info();
    }
    catch (...) {
        std::cerr << "Сбой при инициализации '" << filename << "'.\n";
        closeLibrary(handle);
        return false;
    }

    if (!isValidPluginInfo(info)) {
        std::cerr << "Некорректные метаданные в '" << filename << "'.\n";
        closeLibrary(handle);
        return false;
    }

    registerPlugin(info, filename);
    libraries_.push_back({ handle, filename });
    return true;
}

void PluginManager::loadPluginsFromDirectory(const std::filesystem::path& dir_path) {
    std::error_code ec;
    if (!std::filesystem::exists(dir_path, ec) || !std::filesystem::is_directory(dir_path, ec)) {
        std::cout << "Директория плагинов '" << dir_path.string() << "' не найдена. Доступны только базовые операции (+, -, *, /).\n";
        return;
    }

    int loaded_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path, ec)) {
        if (isSupportedPluginFile(entry) && tryLoadSinglePlugin(entry.path())) {
            ++loaded_count;
        }
    }

    if (loaded_count == 0) {
        std::cout << "В папке '" << dir_path.string() << "' не найдено рабочих плагинов\n";
    }
}

const PluginInfo* PluginManager::getUnaryFunction(const std::string& name) const {
    auto it = unary_functions_.find(name);
    return (it != unary_functions_.end()) ? it->second : nullptr;
}

const PluginInfo* PluginManager::getBinaryOperator(const std::string& op_symbol) const {
    auto it = binary_operators_.find(op_symbol);
    return (it != binary_operators_.end()) ? it->second : nullptr;
}

bool PluginManager::isOperatorChar(char c) const {
    std::string s(1, c);
    return binary_operators_.find(s) != binary_operators_.end();
}
