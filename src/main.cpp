#include "PluginManager.h"
#include "ExpressionParser.h"
#include <iostream>
#include <string>
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    PluginManager plugin_manager;

    std::filesystem::path plugins_dir = "./plugins";
    std::error_code ec;
    if (!std::filesystem::exists(plugins_dir, ec) && argc > 0) {
        std::filesystem::path exe_dir = std::filesystem::weakly_canonical(std::filesystem::path(argv[0]), ec).parent_path();
        if (std::filesystem::exists(exe_dir / "plugins", ec)) {
            plugins_dir = exe_dir / "plugins";
        }
    }

    plugin_manager.loadPluginsFromDirectory(plugins_dir);

    ExpressionParser parser(plugin_manager);

   
    std::cout << "Введите выражение или 'exit' для выхода.\n";

    std::string line;
    while (true) {
        std::cout << "Ввод: ";
        if (!std::getline(std::cin, line)) {
            break;
        }

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

    return 0;
}