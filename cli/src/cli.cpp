#include <filesystem>
#include <iostream>
#include <string>

#include "../../compiler/lexer.h"

namespace style
{
    const std::string reset = "\033[0m";
    const std::string bold = "\033[1m";
    const std::string dim = "\033[2m";
    const std::string italic = "\033[3m";
    const std::string underline = "\033[4m";

    const std::string red = "\033[31m";
    const std::string green = "\033[32m";
    const std::string yellow = "\033[33m";
    const std::string blue = "\033[34m";
    const std::string magenta = "\033[35m";
    const std::string cyan = "\033[36m";
    const std::string gray = "\033[90m";
}

struct config_t
{
    bool verbose = false;
    bool optimize = false;
    std::string target_dir = ".";
    std::string linker_script = "";
};

void print_version()
{
    std::cout << style::bold << style::magenta << "Yu" << style::reset
            << " v0.1.0\n";
    std::cout << style::gray << "A minimal compiler for the Yu language"
            << style::reset << "\n\n";
}

void print_help()
{
    print_version();

    std::cout << style::bold << style::yellow << "Usage:" << style::reset << "\n"
            << "  yu [options] <directory>\n\n";

    std::cout << style::bold << style::yellow << "Examples:" << style::reset << "\n"
            << "  " << style::cyan << "yu -v src/" << style::reset << "\n"
            << "  " << style::cyan << "yu -o -T=linker.ld src/" << style::reset << "\n\n";

    std::cout << style::bold << style::yellow << "Options:" << style::reset << "\n"
            << "  " << style::green << "-v" << style::reset << "\t\t"
            << "        Enable verbose output\n"
            << "  " << style::green << "-o" << style::reset << "\t\t"
            << "        Enable optimizations\n"
            << "  " << style::green << "-T=<file>" << style::reset << "\t"
            << "        Specify linker script\n";
}

void print_error(const std::string &msg)
{
    std::cout << style::bold << style::red << "✘ Error: "
            << style::reset << style::red << msg << style::reset << "\n";
}

void print_success(const std::string &msg)
{
    std::cout << style::bold << style::green << "✓ "
            << msg << style::reset << "\n";
}

config_t parse_args(int argc, char *argv[])
{
    config_t cfg;

    for (std::string::size_type i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "help")
        {
            print_help();
            exit(0);
        }

        switch (arg[0])
        {
            case '-':
                if (arg == "-v")
                {
                    cfg.verbose = true;
                }
                else if (arg == "-o")
                {
                    cfg.optimize = true;
                }
                else if (arg.rfind("-T=", 0) == 0)
                {
                    cfg.linker_script = arg.substr(3);
                }
                else
                {
                    print_error("Unknown option: " + std::string(arg));
                    exit(1);
                }
                break;

            default:
                cfg.target_dir = arg;
                break;
        }
    }
    return cfg;
}

int main(const int argc, char *argv[])
{
    if (argc == 1)
    {
        print_help();
        return 1;
    }

    auto [verbose, optimize, target_dir, linker_script] = parse_args(argc, argv);
    if (verbose)
    {
        std::cout << style::bold << style::blue
                << "⚡ Compiling project..." << style::reset << "\n\n";

        std::cout << style::dim << "Configuration:" << style::reset << "\n"
                << "  Directory:     " << target_dir << "\n"
                << "  Optimize:      " << (optimize ? "yes" : "no") << "\n"
                << "  Linker Script: "
                << (linker_script.empty() ? "default" : linker_script)
                << "\n\n";
    }

    // Dependency resolutions
    // Compilation stuff
    yu::compiler::Lexer* lexer = new yu::compiler::Lexer();
    // Build the binary at default 'build/${YU_BUILD_TYPE}/bin/'

    print_success("Compilation completed successfully");
    std::cout << style::gray << "  Output: " << target_dir
            << style::reset << "\n";

    return 0;
}
