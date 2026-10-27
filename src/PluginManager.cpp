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

void PluginManager::unloadAll() {
    unary_functions_.clear();
    binary_operators_.clear();

    for (auto& lib : libraries_) {
        if (lib.handle) {
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary(static_cast<HMODULE>(lib.handle));
#else
            dlclose(lib.handle);
#endif
            lib.handle = nullptr;
        }
    }
    libraries_.clear();
}

void PluginManager::loadPluginsFromDirectory(const std::filesystem::path& dir_path) {
    std::error_code ec;
    if (!std::filesystem::exists(dir_path, ec) || !std::filesystem::is_directory(dir_path, ec)) {
        std::cout << "Директория плагинов '" << dir_path.string()
            << "' не найдена. Доступны только базовые операции (+, -, *, /).\n";
        return;
    }

    int loaded_count = 0;

    for (const auto& entry : std::filesystem::directory_iterator(dir_path, ec)) {
        if (!entry.is_regular_file(ec)) continue;

        auto ext = entry.path().extension().string();
#if defined(_WIN32) || defined(_WIN64)
        if (ext != ".dll") continue;
#elif defined(__APPLE__)
        if (ext != ".dylib" && ext != ".so") continue;
#else
        if (ext != ".so") continue;
#endif

        std::string filepath = entry.path().string();
        std::string filename = entry.path().filename().string();

#if defined(_WIN32) || defined(_WIN64)
        void* handle = static_cast<void*>(LoadLibraryA(filepath.c_str()));
#else
        void* handle = dlopen(filepath.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif

        if (!handle) {
            std::cerr << "Не удалось загрузить библиотеку '" << filename << "' (поврежденный файл или неверная архитектура).\n";
            continue;
        }

#if defined(_WIN32) || defined(_WIN64)
        auto get_info = reinterpret_cast<GetPluginInfoFunc>(GetProcAddress(static_cast<HMODULE>(handle), "get_plugin_info"));
#else
        auto get_info = reinterpret_cast<GetPluginInfoFunc>(dlsym(handle, "get_plugin_info"));
#endif

        if (!get_info) {
            std::cerr << "Библиотека '" << filename << "' не содержит точку входа 'get_plugin_info'.\n";
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary(static_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
            continue;
        }

        const PluginInfo* info = nullptr;
        try {
            info = get_info();
        }
        catch (...) {
            std::cerr << "Сбой при инициализации '" << filename << "'.\n";
        }

        if (!info || !info->name || std::string(info->name).empty() || !info->eval) {
            std::cerr << "Некорректные метаданные в '" << filename << "'.\n";
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary(static_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
            continue;
        }

        std::string symbol_name(info->name);
        if (info->type == PluginType::UNARY_FUNCTION) {
            unary_functions_[symbol_name] = info;
            std::cout << "Плагин загружен: функция " << symbol_name << "(x) из файла " << filename << "\n";
        }
        else if (info->type == PluginType::BINARY_OPERATOR) {
            binary_operators_[symbol_name] = info;
            std::cout << "Плагин загружен: оператор '" << symbol_name << "' из файла " << filename << "\n";
        }

        libraries_.push_back({ handle, filename });
        ++loaded_count;
    }

    if (loaded_count == 0) {
        std::cout << "В папке '" << dir_path.string() << "' не найдено рабочих плагинов.\n";
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