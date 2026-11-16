#include "PluginManager.h"
#include "ExpressionParser.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <exception>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace {
    std::filesystem::path resolvePluginsDir(int argc, char* argv[]) {
        std::error_code ec;
#if defined(_WIN32) || defined(_WIN64)
        (void)argc;
        (void)argv;
        wchar_t exe_path_buf[MAX_PATH];
        if (GetModuleFileNameW(nullptr, exe_path_buf, MAX_PATH) > 0) {
            std::filesystem::path exe_dir = std::filesystem::path(exe_path_buf).parent_path();
            if (std::filesystem::exists(exe_dir / "plugins", ec)) {
                return exe_dir / "plugins";
            }
        }
#else
        if (argc > 0 && argv != nullptr && argv[0] != nullptr) {
            std::filesystem::path exe_dir = std::filesystem::weakly_canonical(std::filesystem::path(argv[0]), ec).parent_path();
            if (std::filesystem::exists(exe_dir / "plugins", ec)) {
                return exe_dir / "plugins";
            }
        }
#endif
        return "./plugins";
    }
}

int main(int argc, char* argv[]) {
    try {
#if defined(_WIN32) || defined(_WIN64)
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif

        std::filesystem::path plugins_dir = resolvePluginsDir(argc, argv);

        PluginManager plugin_manager;
        plugin_manager.loadPluginsFromDirectory(plugins_dir);

        ExpressionParser parser(plugin_manager);

        std::cout << "Введите выражение или 'exit' для выхода.\n";

        std::string line;
        while (std::cout << "Ввод: " && std::getline(std::cin, line)) {
            if (line == "exit" || line == "quit") {
                break;
            }
            if (line.empty()) {
                continue;
            }

            try {
                double result = parser.evaluate(line);
                std::cout << "Вывод: " << result << "\n\n";
            }
            catch (const std::exception& ex) {
                std::cerr << "Ошибка: " << ex.what() << "\n\n";
            }
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Критическая ошибка: " << ex.what() << "\n";
        return 1;
    }
    catch (...) {
        std::cerr << "Неизвестная критическая ошибка.\n";
        return 1;
    }

    return 0;
}
