#pragma once
#include <memory>
#include "../../interfaces/include/error.h"
#include "symbol.h"
#include "../../ast/include/ast.h"
#include "../../lexer/include/lexer.h"

namespace yu::compiler
{
    class Parser
    {
    public:
        Parser(lang::TokenList &tokens, const char *source, const char *file_name, const Lexer &lexer) : lexer(lexer),
            tokens(tokens),
            source(source),
            file_name(file_name),
            error_reporter(source, file_name)
        {
            update_current_token();
        }

        ast::AST parse_program();

    private:
        Lexer lexer;
        const lang::TokenList &tokens;
        const char *source;
        const char *file_name;
        DefaultErrorReporter error_reporter;

        uint32_t current = 0;
        uint32_t current_scope = 0;
        lang::token_t current_token;

        VarDeclList var_declrs;
        TypeList types;
        SymbolList symbols;

        ast::AST ast;

        uint32_t parse_expression();

        uint32_t parse_binary(uint32_t precedence = 0);

        uint32_t parse_unary();

        uint32_t parse_primary();

        uint32_t parse_grouping();

        uint32_t parse_literal();

        uint32_t parse_identifier();

        uint32_t parse_statement();

        uint32_t parse_if_statement();

        uint32_t parse_block_statement();

        uint32_t parse_variable_decl();

        uint32_t parse_return_statement();

        uint32_t parse_function_decl();

        uint32_t parse_expression_statement();

        uint32_t parse_type();

        uint32_t parse_generic_params();

        void update_current_token();

        bool is_at_end() const
        {
            return current >= tokens.size() || tokens.types[current] == lang::TokenType::END_OF_FILE;
        }

        const lang::token_t &advance()
        {
            current++;
            update_current_token();
            return current_token;
        }

        bool match(lang::TokenType type)
        {
            if (!is_at_end() && tokens.types[current] == type)
            {
                advance();
                return true;
            }
            return false;
        }

        void expect(lang::TokenType type, const std::string &message)
        {
            if (!match(type))
            {
                auto error = ParseError {
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    message,
                    "Expected " + std::to_string(static_cast<int>(type)),
                    source,
                    file_name,
                    current_token.start,
                    current_token.length,
                    get_source_line(current),
                    create_error_pointer(current)
                };
                error_reporter.report_error(error);
                throw std::runtime_error("Parse error");
            }
        }

        uint32_t add_symbol(std::string_view name, uint32_t type_index, uint8_t flags);

        uint32_t lookup_symbol(std::string_view name) const;

        uint32_t infer_type(uint32_t expr_index);

        std::string get_source_line(uint32_t line_number) const;

        std::string create_error_pointer(uint32_t column) const;

        void synchronize();

    public:
        const VarDeclList &get_var_decls() const
        {
            return var_declrs;
        }

        const SymbolList &get_symbols() const
        {
            return symbols;
        }

        const std::vector<ParseError> &get_warnings() const
        {
            return error_reporter.get_warnings();
        }

        const std::vector<ParseError> &get_errors() const
        {
            return error_reporter.get_errors();
        }
    };
}
