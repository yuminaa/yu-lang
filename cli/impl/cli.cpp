#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <mutex>

#include <compiler/include/lexer.h>
#include <compiler/include/parser.h>

std::mutex cout_mutex;

struct ParseResult
{
    std::string filename;
    bool success;
    std::string error_message;
    yu::compiler::VarDeclList var_decls;
    yu::compiler::SymbolList symbols;
};

std::string read_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void parse_file(const std::string &filename, ParseResult &result)
{
    result.filename = filename;
    result.success = false;

    try
    {
        const std::string code = read_file(filename);

        yu::compiler::Lexer lexer(code);
        const auto tokens = lexer.tokenize();

        yu::compiler::Parser parser(*tokens, code.c_str(), filename.c_str(), lexer);

        auto parse_result = parser.parse_program();
        if (!parse_result)
        {
            result.error_message = "Failed to parse program";
            return;
        }

        result.var_decls = parser.get_var_decls();
        result.symbols = parser.get_symbols();
        result.success = true;
    }
    catch (const std::exception &e)
    {
        result.error_message = e.what();
    }
}

int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file1> [file2] ...\n";
        return 1;
    }

    std::vector<ParseResult> parse_results(argc - 1);
    std::vector<std::thread> parse_threads;
    for (auto i = 1; i < argc; ++i)
    {
        parse_threads.emplace_back(parse_file, argv[i], std::ref(parse_results[i - 1]));
    }

    for (auto &thread: parse_threads)
        thread.join();

    auto overall_success = true;
    for (const auto &[filename, success, error_message, var_decls, symbols]: parse_results)
    {
        {
            std::lock_guard lock(cout_mutex);
            std::cout << "File: " << filename << std::endl;
        }

        if (!success)
        {
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "Error parsing " << filename
                        << ": " << error_message << std::endl;
            }
            overall_success = false;
        }
        else
        {
            std::lock_guard lock(cout_mutex);
            for (size_t j = 0; j < var_decls.names.size(); ++j)
            {
                std::cout << "Parsed variable: " << var_decls.names[j] << std::endl;
            }
        }
    }

    return overall_success ? 0 : 1;
}
