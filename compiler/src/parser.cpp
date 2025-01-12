// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#include "../include/parser.h"
#include <iomanip>
#include <iostream>
#include "../../common/styles.h"

namespace yu::compiler
{
    ParseError Parser::create_parse_error(const ParseErrorFlags flags, const ErrorSeverity severity,
                                          const std::string &message, const std::string &suggestion,
                                          const uint32_t token_index) const
    {
        const auto [line, col] = lexer.get_line_col(current_token);
        return {
            flags,
            severity,
            message,
            suggestion,
            file_name,
            line,
            col,
            get_source_line(token_index),
            create_error_pointer(token_index)
        };
    }

    Parser::Parser(lang::TokenList &tokens, const char *source, const char *file_name,
                   const Lexer &lexer): lexer(lexer),
                                        tokens(tokens), source(source),
                                        file_name(file_name)
    {
        update_current_token();
    }

    ParseResult<int> Parser::parse_program()
    {
        var_declrs = VarDeclList {};
        symbols = SymbolList {};
        types = TypeList {};
        expressions = ExprList {};
        current_scope = 0;
        current = 0;
        update_current_token();

        while (!is_at_end())
        {
            switch (current_token.type)
            {
                case lang::token_i::VAR:
                case lang::token_i::CONST:
                {
                    auto var_decl = parse_variable_decl();
                    if (!var_decl)
                    {
                        return ParseResult<int>::failure();
                    }
                    break;
                }
                case lang::token_i::FUNCTION:
                {
                    if (const auto func_decl = parse_function_decl();
                        !func_decl)
                    {
                        return ParseResult<int>::failure();
                    }
                    break;
                }
                case lang::token_i::END_OF_FILE:
                    return ParseResult(0);

                default:
                {
                    report_error(create_parse_error(
                        ParseErrorFlags::UNEXPECTED_TOKEN,
                        ErrorSeverity::ERROR,
                        "Unexpected token in program",
                        "Remove or replace this token",
                        current
                    ));
                    return ParseResult<int>::failure();
                }
            }
        }

        return ParseResult(1);
    }

    ParseResult<uint32_t> Parser::parse_function_decl()
    {
        //const uint32_t func_index = types.names.size();
        advance();

        // TODO: Make actually generics
        if (current_token.type == lang::token_i::LESS)
        {
            auto generic_result = parse_generic_params();
            if (!generic_result)
                return ParseResult<uint32_t>::failure();
        }

        // fn name
        if (current_token.type != lang::token_i::IDENTIFIER)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected function name",
                "Provide a valid function name",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        const std::string_view func_name {
            source + tokens.starts[current],
            tokens.lengths[current]
        };
        const uint32_t func_symbol_index = add_symbol(
            func_name,
            std::numeric_limits<uint32_t>::max(),
            static_cast<uint8_t>(SymbolFlags::IS_FUNCTION)
        );
        advance();
        if (current_token.type != lang::token_i::LEFT_PAREN)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected '(' to start parameter list",
                "Open parameter list with '('",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }
        advance();

        uint32_t param_start = types.function_params.size();
        uint32_t param_count = 0;
        std::vector<uint32_t> param_types;
        std::vector<std::string_view> param_names;

        while (current_token.type != lang::token_i::RIGHT_PAREN)
        {
            if (current_token.type != lang::token_i::IDENTIFIER)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected parameter name",
                    "Provide a valid parameter name",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }

            std::string_view param_name {
                source + tokens.starts[current],
                tokens.lengths[current]
            };
            param_names.push_back(param_name);
            advance();

            if (current_token.type != lang::token_i::COLON)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected ':' after parameter name",
                    "Provide type annotation for parameter",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            advance();

            auto type_result = parse_type();
            if (!type_result)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::INVALID_SYNTAX,
                    ErrorSeverity::ERROR,
                    "Invalid parameter type",
                    "Provide a valid type for parameter",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }

            add_symbol(
                param_name,
                type_result.value,
                0
            );

            param_types.push_back(type_result.value);
            types.function_params.push_back(type_result.value);
            param_count++;

            if (current_token.type == lang::token_i::COMMA)
                advance();
            else if (current_token.type != lang::token_i::RIGHT_PAREN)
                break;
        }

        if (current_token.type != lang::token_i::RIGHT_PAREN)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected ')' to close parameter list",
                "Close parameter list with ')'",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }
        advance();

        if (current_token.type != lang::token_i::MINUS)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected '-' before return type",
                "Specify return type with '->'",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }
        advance();

        if (current_token.type != lang::token_i::GREATER)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected '>' to complete return type arrow",
                "Complete return type specification with '->'",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }
        advance();

        const auto return_type_result = parse_type();
        if (!return_type_result)
        {
            report_error(create_parse_error(
                ParseErrorFlags::INVALID_SYNTAX,
                ErrorSeverity::ERROR,
                "Invalid return type",
                "Provide a valid return type",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        symbols.type_indices[func_symbol_index] = return_type_result.value;

        const uint32_t function_type_index = types.names.size();
        types.names.push_back("function");
        types.function_param_starts.push_back(param_start);
        types.function_param_counts.push_back(param_count);
        types.function_return_types.push_back(return_type_result.value);

        types.generic_starts.push_back(0);
        types.generic_counts.push_back(0);

        if (current_token.type != lang::token_i::LEFT_BRACE)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected '{' to start function body",
                "Open function body with '{'",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        if (const auto function_body = parse_statement();
            !function_body)
        {
            report_error(create_parse_error(
                ParseErrorFlags::INVALID_SYNTAX,
                ErrorSeverity::ERROR,
                "Invalid function body",
                "Provide a valid function body",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        return ParseResult(function_type_index);
    }

    ParseResult<uint32_t> Parser::parse_variable_decl()
    {
        const uint32_t var_index = var_declrs.names.size();

        const bool is_const = match(lang::token_i::CONST);
        if (!is_const && !match(lang::token_i::VAR))
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected 'var' or 'const' at the start of variable declaration",
                "Use 'var' or 'const' when declaring a variable",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        if (current_token.type != lang::token_i::IDENTIFIER)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected identifier after 'var' or 'const'",
                "Provide a valid variable name",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        var_declrs.names.emplace_back(std::string_view {
            source + tokens.starts[current],
            tokens.lengths[current]
        });
        advance();

        uint32_t type_idx = std::numeric_limits<uint32_t>::max();
        if (match(lang::token_i::COLON))
        {
            auto type_result = parse_type();
            if (!type_result)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::INVALID_SYNTAX,
                    ErrorSeverity::ERROR,
                    "Invalid type specification",
                    "Provide a valid type after ':'",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            type_idx = type_result.value;
        }

        if (!match(lang::token_i::EQUAL))
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected '=' for variable initialization",
                "Use '=' to assign a value to the variable",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        auto init_result = parse_expression();
        if (!init_result)
        {
            report_error(create_parse_error(
                ParseErrorFlags::INVALID_SYNTAX,
                ErrorSeverity::ERROR,
                "Invalid expression in variable initialization",
                "Provide a valid expression after '='",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        if (type_idx == std::numeric_limits<uint32_t>::max())
        {
            type_idx = infer_type(init_result.value);
            if (type_idx == std::numeric_limits<uint32_t>::max())
            {
                report_error(create_parse_error(
                    ParseErrorFlags::TYPE_MISMATCH,
                    ErrorSeverity::ERROR,
                    "Unable to infer type for variable",
                    "Provide an explicit type annotation",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
        }

        var_declrs.type_indices.emplace_back(type_idx);
        var_declrs.init_indices.emplace_back(init_result.value);
        var_declrs.flags.emplace_back(is_const);

        var_declrs.lines.emplace_back(current_token.start);
        var_declrs.columns.emplace_back(current_token.length);

        if (!match(lang::token_i::SEMICOLON))
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected ';' at the end of variable declaration",
                "Add ';' to complete the variable declaration",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        return ParseResult(var_index);
    }

    bool Parser::is_at_end() const
    {
        return current >= tokens.size() || tokens.types[current] == lang::token_i::END_OF_FILE;
    }

    const lang::token_t &Parser::advance()
    {
        current++;
        update_current_token();
        return current_token;
    }

    bool Parser::match(const lang::token_i type)
    {
        const bool matches = !is_at_end() && tokens.types[current] == type;
        current += matches;
        if (matches)
            update_current_token();
        return matches;
    }

    void Parser::update_current_token()
    {
        current_token = {
            tokens.starts[current],
            tokens.lengths[current],
            tokens.types[current],
            tokens.flags[current]
        };
    }

    uint32_t Parser::infer_type(uint32_t expr_index)
    {
        if (expr_index >= expressions.expr_types.size())
            return std::numeric_limits<uint32_t>::max();

        if (expressions.expr_types[expr_index] < types.names.size())
        {
            return expressions.expr_types[expr_index];
        }

        switch (static_cast<lang::token_i>(expressions.expr_types[expr_index]))
        {
            case lang::token_i::NUM_LITERAL:
            {
                const std::string_view value = expressions.values[expr_index];
                return value.find('.') != std::string_view::npos
                           ? static_cast<uint32_t>(lang::token_i::F64)
                           : static_cast<uint32_t>(lang::token_i::I32);
            }

            case lang::token_i::TRUE:
            case lang::token_i::FALSE:
                return static_cast<uint32_t>(lang::token_i::BOOLEAN);

            case lang::token_i::STR_LITERAL:
                return static_cast<uint32_t>(lang::token_i::STRING);

            case lang::token_i::NIL:
                return std::numeric_limits<uint32_t>::max();

            case lang::token_i::IDENTIFIER:
            {
                const std::string_view identifier = expressions.values[expr_index];

                if (const uint32_t symbol_index = lookup_symbol(identifier);
                    symbol_index != -1)
                    return symbols.type_indices[symbol_index];
                return std::numeric_limits<uint32_t>::max();
            }

            default:
                return std::numeric_limits<uint32_t>::max();
        }
    }

    uint32_t Parser::add_symbol(std::string_view name, uint32_t type_index, uint8_t flags)
    {
        const uint32_t symbol_index = symbols.names.size();

        symbols.names.emplace_back(name);
        symbols.type_indices.emplace_back(type_index);
        symbols.scopes.emplace_back(current_scope);
        symbols.symbol_flags.emplace_back(flags);

        return symbol_index;
    }

    uint32_t Parser::lookup_symbol(const std::string_view name) const
    {
        for (int32_t i = symbols.names.size() - 1; i >= 0; --i)
        {
            if (symbols.names[i] == name)
                return i;
        }
        return -1;
    }

    ParseResult<uint32_t> Parser::parse_type()
    {
        const uint32_t type_index = types.names.size();

        switch (current_token.type)
        {
            case lang::token_i::U8:
            case lang::token_i::I8:
            case lang::token_i::U16:
            case lang::token_i::I16:
            case lang::token_i::U32:
            case lang::token_i::I32:
            case lang::token_i::U64:
            case lang::token_i::I64:
            case lang::token_i::F32:
            case lang::token_i::F64:
            case lang::token_i::STRING:
            case lang::token_i::BOOLEAN:
            case lang::token_i::VOID:
            {
                std::string_view name {
                    source + tokens.starts[current],
                    tokens.lengths[current]
                };
                types.names.emplace_back(name);
                types.generic_starts.emplace_back(0);
                types.generic_counts.emplace_back(0);
                advance();
                return ParseResult(type_index);
            }

            case lang::token_i::PTR:
            {
                std::string_view name {
                    source + tokens.starts[current],
                    tokens.lengths[current]
                };
                types.names.emplace_back(name);
                advance();

                if (match(lang::token_i::LESS))
                {
                    const uint32_t generic_start = types.generic_params.size();
                    uint32_t generic_count = 0;

                    do
                    {
                        auto param_type_result = parse_type();
                        if (!param_type_result)
                        {
                            report_error(create_parse_error(
                                ParseErrorFlags::INVALID_SYNTAX,
                                ErrorSeverity::ERROR,
                                "Invalid generic type parameter",
                                "Provide a valid type for generic parameter",
                                current
                            ));
                            return ParseResult<uint32_t>::failure();
                        }

                        types.generic_params.emplace_back(param_type_result.value);
                        generic_count++;
                    }
                    while (match(lang::token_i::COMMA));

                    if (!match(lang::token_i::GREATER))
                    {
                        report_error(create_parse_error(
                            ParseErrorFlags::UNEXPECTED_TOKEN,
                            ErrorSeverity::ERROR,
                            "Expected '>' to close generic type parameters",
                            "Close generic type parameters with '>'",
                            current
                        ));
                        return ParseResult<uint32_t>::failure();
                    }

                    types.generic_starts.emplace_back(generic_start);
                    types.generic_counts.emplace_back(generic_count);
                }
                else
                {
                    types.generic_starts.emplace_back(0);
                    types.generic_counts.emplace_back(0);
                }

                return ParseResult(type_index);
            }

            case lang::token_i::IDENTIFIER:
            {
                std::string_view name {
                    source + tokens.starts[current],
                    tokens.lengths[current]
                };

                uint32_t symbol_index = lookup_symbol(name);
                if (symbol_index != -1 &&
                    (symbols.symbol_flags[symbol_index] & static_cast<uint8_t>(SymbolFlags::IS_GENERIC_PARAM)))
                {
                    types.names.emplace_back(name);
                    types.generic_starts.emplace_back(0);
                    types.generic_counts.emplace_back(0);
                    advance();
                    return ParseResult(type_index);
                }

                report_error(create_parse_error(
                    ParseErrorFlags::UNRESOLVED_SYMBOL,
                    ErrorSeverity::ERROR,
                    "Unrecognized type",
                    "Use a valid type or define the type before use",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            default:
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNRESOLVED_SYMBOL,
                    ErrorSeverity::ERROR,
                    "Unrecognized type",
                    "Use a valid type or define the type before use",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
        }
    }

    ParseResult<uint32_t> Parser::parse_statement()
    {
        switch (current_token.type)
        {
            case lang::token_i::IF:
                return parse_if_statement();

            case lang::token_i::LEFT_BRACE:
                return parse_block_statement();

            case lang::token_i::VAR:
            case lang::token_i::CONST:
                return parse_variable_decl();

            case lang::token_i::RETURN:
                return parse_return_statement();

            default:
                return parse_expression_statement();
        }
    }

    ParseResult<uint32_t> Parser::parse_if_statement()
    {
        constexpr uint32_t if_index = std::numeric_limits<uint32_t>::max(); // TODO: track if statement

        advance();
        if (current_token.type != lang::token_i::LEFT_PAREN)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected '(' after 'if'",
                "Open condition with '('",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }
        advance();

        if (const auto condition_result = parse_expression();
            !condition_result)
        {
            report_error(create_parse_error(
                ParseErrorFlags::INVALID_SYNTAX,
                ErrorSeverity::ERROR,
                "Invalid condition expression",
                "Provide a valid condition",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        if (current_token.type != lang::token_i::RIGHT_PAREN)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected ')' after condition",
                "Close condition with ')'",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }
        advance();

        if (const auto then_branch = parse_statement();
            !then_branch)
        {
            report_error(create_parse_error(
                ParseErrorFlags::INVALID_SYNTAX,
                ErrorSeverity::ERROR,
                "Invalid statement in 'if' block",
                "Provide a valid statement",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }
        if (match(lang::token_i::ELSE))
        {
            if (const auto else_branch = parse_statement();
                !else_branch)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::INVALID_SYNTAX,
                    ErrorSeverity::ERROR,
                    "Invalid statement in 'else' block",
                    "Provide a valid statement",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
        }

        return ParseResult(if_index);
    }

    ParseResult<uint32_t> Parser::parse_block_statement()
    {
        constexpr uint32_t block_index = std::numeric_limits<uint32_t>::max(); // TODO: track block

        advance();
        current_scope++;
        while (current_token.type != lang::token_i::RIGHT_BRACE)
        {
            // error is handled by parse_statement();
            if (const auto statement = parse_statement();
                !statement)
            {
                return ParseResult<uint32_t>::failure();
            }
        }

        advance();
        current_scope--;
        return ParseResult(block_index);
    }

    ParseResult<uint32_t> Parser::parse_return_statement()
    {
        constexpr uint32_t return_index = std::numeric_limits<uint32_t>::max(); // TODO: track return

        advance();
        if (current_token.type != lang::token_i::SEMICOLON)
        {
            if (const auto return_expr = parse_expression();
                !return_expr)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::INVALID_SYNTAX,
                    ErrorSeverity::ERROR,
                    "Invalid return expression",
                    "Provide a valid expression to return",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
        }

        if (!match(lang::token_i::SEMICOLON))
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected ';' after return statement",
                "End return statement with ';'",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        return ParseResult(return_index);
    }


    ParseResult<uint32_t> Parser::parse_generic_params()
    {
        const uint32_t generic_start = types.generic_params.size();
        uint32_t generic_count = 0;
        auto has_variadic = false;

        if (current_token.type != lang::token_i::LESS)
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected '<' to start generic parameters",
                "Open generic parameters with '<'",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }
        advance();

        while (current_token.type != lang::token_i::GREATER)
        {
            if (current_token.type == lang::token_i::DOT &&
                tokens.types[current + 1] == lang::token_i::DOT &&
                tokens.types[current + 2] == lang::token_i::DOT)
            {
                if (has_variadic)
                {
                    report_error(create_parse_error(
                        ParseErrorFlags::INVALID_SYNTAX,
                        ErrorSeverity::ERROR,
                        "Multiple variadic generic parameters",
                        "Only one variadic generic parameter is allowed",
                        current
                    ));
                    return ParseResult<uint32_t>::failure();
                }
                has_variadic = true;
                advance(); // first dot
                advance(); // second dot
                advance(); // third dot
            }

            if (current_token.type != lang::token_i::IDENTIFIER)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected identifier in generic parameters",
                    "Provide a valid identifier for generic parameter",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }

            uint32_t param_index = add_symbol(
                std::string_view(source + tokens.starts[current], tokens.lengths[current]),
                std::numeric_limits<uint32_t>::max(),
                static_cast<uint8_t>(SymbolFlags::IS_GENERIC_PARAM)
            );

            types.generic_params.push_back(param_index);
            generic_count++;
            advance();

            if (current_token.type == lang::token_i::LESS)
            {
                if (const auto nested_result = parse_generic_params();
                    !nested_result)
                {
                    return ParseResult<uint32_t>::failure();
                }
            }

            if (current_token.type == lang::token_i::COMMA)
            {
                advance();
            }
            else if (current_token.type != lang::token_i::GREATER)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected ',' or '>' in generic parameters",
                    "Separate generic parameters with ',' or close with '>'",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
        }

        advance();
        types.generic_starts.push_back(generic_start);
        types.generic_counts.push_back(generic_count);

        return ParseResult(generic_start);
    }

    ParseResult<uint32_t> Parser::parse_expression_statement()
    {
        constexpr uint32_t expr_stmt_index = std::numeric_limits<uint32_t>::max(); // TODO: track expression statement

        if (const auto expr = parse_expression();
            !expr)
        {
            report_error(create_parse_error(
                ParseErrorFlags::INVALID_SYNTAX,
                ErrorSeverity::ERROR,
                "Invalid expression statement",
                "Provide a valid expression",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        if (!match(lang::token_i::SEMICOLON))
        {
            report_error(create_parse_error(
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected ';' after expression",
                "End expression statement with ';'",
                current
            ));
            return ParseResult<uint32_t>::failure();
        }

        return ParseResult(expr_stmt_index);
    }

    // TODO: Add function, method, and more declaration
    ParseResult<uint32_t> Parser::parse_expression()
    {
        const uint32_t expr_index = expressions.expr_types.size();

        if (current_token.type == lang::token_i::MINUS ||
            current_token.type == lang::token_i::PLUS ||
            current_token.type == lang::token_i::AND ||
            current_token.type == lang::token_i::TILDE)
        {
            expressions.expr_types.emplace_back(static_cast<uint8_t>(current_token.type));
            expressions.values.emplace_back(std::string_view {
                source + tokens.starts[current],
                tokens.lengths[current]
            });
            advance();
        }

        if (current_token.type == lang::token_i::FUNCTION)
        {
            uint32_t param_start = types.function_params.size();
            uint32_t param_count = 0;
            uint32_t generic_start = std::numeric_limits<uint32_t>::max();
            uint32_t generic_count = 0;

            advance();
            if (current_token.type == lang::token_i::LESS)
            {
                auto generic_result = parse_generic_params();
                if (!generic_result)
                    return ParseResult<uint32_t>::failure();

                generic_start = generic_result.value;
                generic_count = types.generic_counts.back();
            }

            // parse param list
            if (current_token.type != lang::token_i::LEFT_PAREN)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected '(' to start parameter list",
                    "Open parameter list with '('",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            advance();

            std::vector<uint32_t> param_types;
            while (current_token.type != lang::token_i::RIGHT_PAREN)
            {
                // variadic params handling
                auto is_variadic = false;
                if (current_token.type == lang::token_i::DOT &&
                    tokens.types[current + 1] == lang::token_i::DOT &&
                    tokens.types[current + 2] == lang::token_i::DOT)
                {
                    is_variadic = true;
                    advance(); // first dot
                    advance(); // second dot
                    advance(); // third dot
                }

                if (current_token.type != lang::token_i::IDENTIFIER)
                {
                    report_error(create_parse_error(
                        ParseErrorFlags::UNEXPECTED_TOKEN,
                        ErrorSeverity::ERROR,
                        "Expected parameter name",
                        "Provide a valid parameter name",
                        current
                    ));
                    return ParseResult<uint32_t>::failure();
                }
                advance();

                if (current_token.type != lang::token_i::COLON)
                {
                    report_error(create_parse_error(
                        ParseErrorFlags::UNEXPECTED_TOKEN,
                        ErrorSeverity::ERROR,
                        "Expected ':' after parameter name",
                        "Provide type annotation for parameter",
                        current
                    ));
                    return ParseResult<uint32_t>::failure();
                }
                advance();

                auto type_result = parse_type();
                if (!type_result)
                {
                    report_error(create_parse_error(
                        ParseErrorFlags::INVALID_SYNTAX,
                        ErrorSeverity::ERROR,
                        "Invalid parameter type",
                        "Provide a valid type for parameter",
                        current
                    ));
                    return ParseResult<uint32_t>::failure();
                }

                param_types.push_back(type_result.value);
                types.function_params.push_back(type_result.value);
                param_count++;
                if (is_variadic)
                {
                    if (current_token.type == lang::token_i::DOT &&
                        tokens.types[current + 1] == lang::token_i::DOT &&
                        tokens.types[current + 2] == lang::token_i::DOT)
                    {
                        advance(); // first dot
                        advance(); // second dot
                        advance(); // third dot
                    }
                    else
                    {
                        report_error(create_parse_error(
                            ParseErrorFlags::INVALID_SYNTAX,
                            ErrorSeverity::ERROR,
                            "Variadic parameter must have '...' after type",
                            "Add '...' after the parameter type for variadic parameters",
                            current
                        ));
                        return ParseResult<uint32_t>::failure();
                    }
                }

                if (current_token.type == lang::token_i::COMMA)
                    advance();
                else if (current_token.type != lang::token_i::RIGHT_PAREN)
                    break;
            }

            if (current_token.type != lang::token_i::RIGHT_PAREN)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected ')' to close parameter list",
                    "Close parameter list with ')'",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            advance();

            if (current_token.type != lang::token_i::MINUS)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected '-' before return type",
                    "Specify return type with '->'",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            advance();

            if (current_token.type != lang::token_i::GREATER)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected '>' to complete return type arrow",
                    "Complete return type specification with '->'",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            advance();

            auto return_type_result = parse_type();
            if (!return_type_result)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::INVALID_SYNTAX,
                    ErrorSeverity::ERROR,
                    "Invalid return type",
                    "Provide a valid return type",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }

            const uint32_t function_type_index = types.names.size();
            types.names.push_back("function");
            types.function_param_starts.push_back(param_start);
            types.function_param_counts.push_back(param_count);
            types.function_return_types.push_back(return_type_result.value);

            if (generic_start != std::numeric_limits<uint32_t>::max())
            {
                types.generic_starts.push_back(generic_start);
                types.generic_counts.push_back(generic_count);
            }
            else
            {
                types.generic_starts.push_back(0);
                types.generic_counts.push_back(0);
            }

            if (current_token.type != lang::token_i::LEFT_BRACE)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected '{' to start function body",
                    "Open function body with '{'",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            advance();

            if (const auto function_body = parse_statement();
                !function_body)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::INVALID_SYNTAX,
                    ErrorSeverity::ERROR,
                    "Invalid function body",
                    "Provide a valid function body",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            advance();

            expressions.expr_types.emplace_back(static_cast<uint8_t>(function_type_index));
            expressions.values.emplace_back(std::string_view {});
            return ParseResult(expr_index);
        }
        if (current_token.type == lang::token_i::LEFT_PAREN)
        {
            advance();
            auto inner_expr = parse_expression();
            if (!inner_expr)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::INVALID_SYNTAX,
                    ErrorSeverity::ERROR,
                    "Invalid expression inside parentheses",
                    "Provide a valid expression inside ()",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }

            if (current_token.type != lang::token_i::RIGHT_PAREN)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected closing parenthesis ')'",
                    "Close the parenthesized expression with ')'",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
            advance();
        }
        // primary expr
        else
        {
            switch (current_token.type)
            {
                case lang::token_i::NUM_LITERAL:
                case lang::token_i::TRUE:
                case lang::token_i::FALSE:
                case lang::token_i::NIL:
                case lang::token_i::STR_LITERAL:
                {
                    expressions.expr_types.emplace_back(static_cast<uint8_t>(current_token.type));
                    expressions.values.emplace_back(std::string_view {
                        source + tokens.starts[current],
                        tokens.lengths[current]
                    });
                    advance();
                    break;
                }
                case lang::token_i::IDENTIFIER:
                {
                    expressions.expr_types.emplace_back(static_cast<uint8_t>(current_token.type));
                    expressions.values.emplace_back(std::string_view {
                        source + tokens.starts[current],
                        tokens.lengths[current]
                    });
                    advance();
                    break;
                }
                default:
                {
                    report_error(create_parse_error(
                        ParseErrorFlags::UNEXPECTED_TOKEN,
                        ErrorSeverity::ERROR,
                        "Invalid expression",
                        "Provide a valid literal or expression",
                        current
                    ));
                    return ParseResult<uint32_t>::failure();
                }
            }
        }

        // check by precedence order from high to low
        while (current_token.type == lang::token_i::STAR || // mult
               current_token.type == lang::token_i::SLASH ||
               current_token.type == lang::token_i::PERCENT ||
               current_token.type == lang::token_i::PLUS || // add
               current_token.type == lang::token_i::MINUS ||
               current_token.type == lang::token_i::AND || // bitwise
               current_token.type == lang::token_i::OR ||
               current_token.type == lang::token_i::XOR)
        {
            expressions.expr_types.emplace_back(static_cast<uint8_t>(current_token.type));
            expressions.values.emplace_back(std::string_view {
                source + tokens.starts[current],
                tokens.lengths[current]
            });
            advance();

            if (const auto right_operand_result = parse_expression();
                !right_operand_result)
            {
                report_error(create_parse_error(
                    ParseErrorFlags::INVALID_SYNTAX,
                    ErrorSeverity::ERROR,
                    "Invalid expression after binary operator",
                    "Provide a valid expression after the operator",
                    current
                ));
                return ParseResult<uint32_t>::failure();
            }
        }

        return ParseResult(expr_index);
    }

    std::string Parser::get_source_line(const uint32_t line_number) const
    {
        uint32_t current_line = 1;
        uint32_t pos = 0;
        while (current_line < line_number && source[pos] != '\0')
        {
            if (source[pos] == '\n')
                current_line++;
            pos++;
        }

        while (pos > 0 && source[pos - 1] != '\n')
        {
            pos--;
        }

        uint32_t line_end = pos;
        while (source[line_end] != '\n' && source[line_end] != '\0')
        {
            line_end++;
        }

        return std::string(source + pos, line_end - pos);
    }

    std::string Parser::create_error_pointer(const uint32_t column) const
    {
        const uint32_t start = tokens.starts[column];
        const uint16_t length = tokens.lengths[column];

        uint32_t line_start = start;
        while (line_start > 0 && source[line_start - 1] != '\n')
            line_start--;

        const uint32_t col = start - line_start;

        std::string pointer(col, ' ');
        pointer += "^" + std::string(length, '~');
        return pointer;
    }

    void Parser::synchronize()
    {
        while (!is_at_end())
        {
            if (current_token.type == lang::token_i::SEMICOLON)
            {
                advance();
                return;
            }
            advance();
        }
    }

    void Parser::report_error(const ParseError &error)
    {
        if (error.severity >= ErrorSeverity::WARNING)
        {
            const auto &color = error.severity == ErrorSeverity::WARNING
                                    ? styles::color::YELLOW
                                    : styles::color::RED;

            std::cerr << color
                    << (error.severity == ErrorSeverity::WARNING ? "warning" : "error")
                    << styles::color::RESET << ": "
                    << error.message << std::endl;

            std::cerr << "  " << styles::color::BLUE << "-->" << styles::color::RESET
                    << " " << error.filename << ":"
                    << error.line << ":" << error.column << std::endl;

            if (!error.source_line.empty())
            {
                std::cerr << styles::color::BLUE << "   |" << styles::color::RESET << std::endl;
                std::cerr << styles::color::BLUE << std::setw(3) << error.line
                        << "|" << styles::color::RESET << " "
                        << error.source_line << std::endl;
                std::cerr << styles::color::BLUE << "   |" << styles::color::RESET << " "
                        << error.error_pointer << std::endl;
            }

            if (!error.suggestion.empty())
            {
                std::cerr << styles::color::BLUE << "   |" << styles::color::RESET << std::endl;
                std::cerr << styles::color::BLUE << "   = " << styles::color::RESET
                        << styles::color::GREEN << "help" << styles::color::RESET
                        << ": " << error.suggestion << std::endl;
            }

            std::cerr << styles::color::BLUE << "   = " << styles::color::RESET
                    << "note: error[" << get_error_code(error.flags) << "]" << "\n";
        }

        switch (error.severity)
        {
            case ErrorSeverity::WARNING:
                warnings.emplace_back(error);
                break;

            case ErrorSeverity::ERROR:
                synchronize();
                break;

            case ErrorSeverity::FATAL:
                throw std::runtime_error(error.message);
        }
    }

    std::string Parser::get_error_code(const ParseErrorFlags flags)
    {
        switch (flags)
        {
            case ParseErrorFlags::UNEXPECTED_TOKEN:
                return "E0001";
            case ParseErrorFlags::TYPE_MISMATCH:
                return "E0308";
            case ParseErrorFlags::INVALID_SYNTAX:
                return "E0002";
            case ParseErrorFlags::UNRESOLVED_SYMBOL:
                return "E0433";
            default:
                return "E0000";
        }
    }
}
