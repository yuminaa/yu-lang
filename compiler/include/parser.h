// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#pragma once

#include <limits>
#include <vector>
#include "lexer.h"
#include "token.h"
#include "../../common/arch.hpp"

namespace yu::compiler
{
    struct VarDeclList
    {
        std::vector<std::string_view> names;
        std::vector<uint32_t> type_indices; // index into TypeList
        std::vector<uint32_t> init_indices; // index into ExprList
        std::vector<uint8_t> flags;         // bitflags for const, etc.
        std::vector<uint32_t> lines;
        std::vector<uint32_t> columns;
    };

    struct TypeList
    {
        std::vector<std::string_view> names;
        std::vector<uint32_t> generic_starts; // start index into generic_params
        std::vector<uint32_t> generic_counts; // number of generic params
        std::vector<uint32_t> generic_params; // indices into TypeList

        std::vector<uint32_t> function_param_starts; // start index into function_params
        std::vector<uint32_t> function_param_counts; // number of function params
        std::vector<uint32_t> function_params;       // parameter type indices
        std::vector<uint32_t> function_return_types; // return type indices
    };

    struct alignas(8) ExprList
    {
        std::vector<uint8_t> expr_types;      // kind of expr
        std::vector<std::string_view> values; // for literals
    };

    struct SymbolList
    {
        std::vector<std::string_view> names; // symbol names
        std::vector<uint32_t> type_indices;  // index into TypeList
        std::vector<uint32_t> scopes;        // which scope does it belong to
        std::vector<uint8_t> symbol_flags;   // something like IS_TYPE, IS_CONST, IS_FUNCTION
    };

    struct TypeInferenceTask
    {
        uint32_t var_decl_index; // which variable needs inference
        uint32_t expr_index;     // expression to infer from
    };

    enum class SymbolFlags : uint8_t
    {
        IS_TYPE = 1 << 0,
        IS_CONST = 1 << 1,
        IS_FUNCTION = 1 << 2,
        IS_GENERIC_PARAM = 1 << 3
    };

    enum class ParseErrorFlags : uint8_t
    {
        NONE = 0,
        UNEXPECTED_TOKEN = 1 << 0,
        TYPE_MISMATCH = 1 << 1,
        INVALID_SYNTAX = 1 << 2,
        UNRESOLVED_SYMBOL = 1 << 3
    };

    enum class ErrorSeverity
    {
        WARNING,
        ERROR,
        FATAL
    };

    struct ParseError
    {
        ParseErrorFlags flags;
        ErrorSeverity severity;
        std::string message;
        std::string suggestion;

        const char *filename;
        uint32_t line;
        uint32_t column;

        std::string source_line;
        std::string error_pointer;
    };

    template<typename T>
    struct ParseResult
    {
        bool success;
        T value;

        explicit ParseResult(T val) : success(true), value(val) {}

        static ParseResult failure()
        {
            ParseResult res(std::numeric_limits<T>::max());
            res.success = false;
            return res;
        }

        explicit operator bool() const
        {
            return success;
        }
    };

    class Parser
    {
    public:
        ParseError create_parse_error(ParseErrorFlags flags, ErrorSeverity severity,
                                      const std::string &message, const std::string &suggestion,
                                      uint32_t token_index) const;

        Parser(lang::TokenList &tokens, const char *source, const char *file_name, const Lexer &lexer);

        ParseResult<int> parse_program();

        ParseResult<uint32_t> parse_function_decl();

        ParseResult<uint32_t> parse_variable_decl();

        // Debug methods
        VarDeclList get_var_decls() const
        {
            return var_declrs;
        }

        const SymbolList& get_symbols() const
        {
            return symbols;
        }

        const std::vector<ParseError> &get_warnings() const
        {
            return warnings;
        }

    private:
        Lexer lexer;
        const lang::TokenList &tokens;
        const char *source;
        const char *file_name;
        uint32_t current = 0;
        uint32_t current_scope = 0;

        VarDeclList var_declrs;
        TypeList types;
        ExprList expressions;
        SymbolList symbols;
        std::vector<TypeInferenceTask> inference_queue;
        std::vector<ParseError> warnings;
        lang::token_t current_token;

        bool is_at_end() const;

        const lang::token_t &advance();

        bool match(lang::token_i type);

        void update_current_token();

        uint32_t infer_type(uint32_t expr_index);

        uint32_t add_symbol(std::string_view name, uint32_t type_index, uint8_t flags);

        uint32_t lookup_symbol(std::string_view name) const;

        ParseResult<uint32_t> parse_type();

        ParseResult<uint32_t> parse_statement();

        ParseResult<uint32_t> parse_if_statement();

        ParseResult<uint32_t> parse_block_statement();

        ParseResult<uint32_t> parse_return_statement();

        ParseResult<uint32_t> parse_generic_params();

        ParseResult<uint32_t> parse_expression_statement();

        ParseResult<uint32_t> parse_expression();

        std::string get_source_line(uint32_t line_number) const;

        std::string create_error_pointer(uint32_t column) const;

        void synchronize();

        void report_error(const ParseError &error);

        static std::string get_error_code(ParseErrorFlags flags);
    };
}
