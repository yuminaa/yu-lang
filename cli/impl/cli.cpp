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

/**
 * @brief Reads the contents of a file into a string.
 *
 * @param filename Path to the file to be read.
 * @return std::string Contents of the file as a single string.
 * @throws std::runtime_error If the file cannot be opened.
 *
 * This function attempts to open the specified file and read its entire contents
 * into a string. If the file cannot be opened, it throws a runtime error with 
 * a descriptive message including the filename.
 *
 * @note Uses std::ifstream and std::stringstream for efficient file reading.
 * @note Reads the entire file contents into memory at once.
 */
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

/**
 * @brief Parses a source file and populates a ParseResult with parsing information.
 *
 * This function attempts to read, tokenize, and parse a given source file. It handles
 * the entire parsing process, capturing variable declarations, symbols, and potential
 * parsing errors.
 *
 * @param filename The path to the source file to be parsed
 * @param result Reference to a ParseResult struct to store parsing outcomes
 *
 * @note Thread-safe function that can be used in concurrent parsing scenarios
 * @note Exceptions from file reading or parsing are caught and stored in the result
 *
 * @exception Captures any std::exception that occurs during file reading or parsing
 */
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

/**
 * @brief Main entry point for the source file parsing program.
 *
 * Parses multiple source files concurrently using multithreading. Accepts file paths
 * as command-line arguments and processes each file in a separate thread.
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 *
 * @return int Exit status (0 for success, 1 for parsing failures)
 *
 * @details
 * - Requires at least one file path as a command-line argument
 * - Creates a thread for each file to parse concurrently
 * - Uses thread-safe output mechanism with mutex to prevent data races
 * - Prints parsing results and any encountered errors
 * - Supports parsing multiple files simultaneously
 *
 * @note Thread-safe implementation with synchronized console output
 * @note Handles parsing errors for individual files without stopping entire process
 */
int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file1> [file2] ...\n";
        return 1;
    }

    // Vector to store parsing results
    std::vector<ParseResult> parse_results(argc - 1);
    std::vector<std::thread> parse_threads;

    // Create a thread for each file
    for (int i = 1; i < argc; ++i)
    {
        parse_threads.emplace_back(parse_file, argv[i], std::ref(parse_results[i - 1]));
    }

    // Wait for all threads to complete
    for (auto &thread: parse_threads)
    {
        thread.join();
    }

    // Process results
    bool overall_success = true;
    for (const auto &result: parse_results)
    {
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "File: " << result.filename << std::endl;
        }

        if (!result.success)
        {
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cerr << "Error parsing " << result.filename
                        << ": " << result.error_message << std::endl;
            }
            overall_success = false;
        }
        else
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            for (size_t j = 0; j < result.var_decls.names.size(); ++j)
            {
                std::cout << "Parsed variable: " << result.var_decls.names[j] << std::endl;
            }
        }
    }

    return overall_success ? 0 : 1;
}
