#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "../../compiler/lexer.h"
#include "../../yu/include/tokens.h"

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
    std::cout << style::gray << "A compiler for the Yu programming language"
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
    if (argc <= 2)
    {
        print_help();
        return 1;
    }

    auto [verbose, optimize, target_dir, linker_script] = parse_args(argc, argv);
    try
    {
        std::filesystem::path output_dir = "build/tokens";
        create_directories(output_dir);

        for (const auto& entry : std::filesystem::directory_iterator(target_dir))
        {
            if (entry.path().extension() != ".yu")
                continue;

            if (verbose)
                std::cout << style::blue << "Lexing " << entry.path() << style::reset << "\n";

            std::ifstream file(entry.path());
            std::string source((std::istreambuf_iterator(file)),
                              std::istreambuf_iterator<char>());

            auto lexer = yu::compiler::create_lexer(source);
            auto tokens = tokenize(lexer);

            std::filesystem::path output_path = output_dir / entry.path().filename();
            output_path.replace_extension(".ytok");

            std::ofstream out(output_path);
            if (!out)
            {
                print_error("Failed to open output file: " + output_path.string());
                return 1;
            }

            for (std::vector<unsigned>::size_type i = 0; i < tokens->size(); i++)
            {
                const auto token = yu::lang::token_t {
                    tokens->starts[i],
                    tokens->lengths[i],
                    tokens->types[i],
                    tokens->flags[i]
                };
                auto value = yu::compiler::get_token_value(lexer, token);
                auto [line, col] = yu::compiler::get_line_col(lexer, token);

                out << line << ":" << col << " "
                    << yu::lang::token_type_to_string(token.type);

                if (token.type == yu::lang::token_i::IDENTIFIER ||
                    token.type == yu::lang::token_i::STR_LITERAL ||
                    token.type == yu::lang::token_i::NUM_LITERAL)
                {
                    out << " `" << value << std::string("`");
                }
                out << "\n";
            }

            if (verbose)
                std::cout << style::green << "  → " << output_path << style::reset << "\n";
        }

        print_success("Token files generated in build/tokens");

    }
    catch (const std::exception& e)
    {
        print_error(e.what());
        return 1;
    }

    return 0;
}
