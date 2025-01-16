#include "../include/parser.h"

static uint32_t get_operator_precedence(yu::lang::TokenType type)
{
    switch (type)
    {
        case yu::lang::TokenType::STAR:
        case yu::lang::TokenType::SLASH:
        case yu::lang::TokenType::PERCENT:
            return 3;

        case yu::lang::TokenType::PLUS:
        case yu::lang::TokenType::MINUS:
            return 2;

        case yu::lang::TokenType::AND:
        case yu::lang::TokenType::OR:
        case yu::lang::TokenType::XOR:
            return 1;

        default:
            return 0;
    }
}

namespace yu::compiler
{
    ast::AST Parser::parse_program()
    {
        ast = ast::AST();
        var_declrs = VarDeclList();
        symbols = SymbolList();
        types = TypeList();

        current = 0;
        current_scope = 0;
        update_current_token();

        std::vector<uint32_t> top_level_statements;

        while (!is_at_end())
        {
            switch (current_token.type)
            {
                case lang::TokenType::VAR:
                case lang::TokenType::CONST:
                {
                    auto var_decl = parse_variable_decl();
                    top_level_statements.push_back(var_decl);
                    break;
                }
                case lang::TokenType::FUNCTION:
                {
                    auto func_decl = parse_function_decl();
                    top_level_statements.push_back(func_decl);
                    break;
                }
                case lang::TokenType::END_OF_FILE:
                {
                    ast.root_stmt_index = ast.add_root_block(0, 0);
                    return ast;
                }

                default:
                {
                    auto expr_stmt = parse_expression_statement();
                    top_level_statements.push_back(expr_stmt);
                    break;
                }
            }
        }

        ast.root_stmt_index = ast.add_root_block(0, 0);
        return ast;
    }

    uint32_t Parser::parse_expression()
    {
        return parse_binary(0);
    }

    uint32_t Parser::parse_binary(const uint32_t precedence)
    {
        uint32_t left = parse_unary();

        while (!is_at_end())
        {
            const auto op = tokens.types[current];
            const uint32_t op_precedence = get_operator_precedence(op);

            if (op_precedence <= precedence)
                break;

            advance();
            const uint32_t right = parse_binary(op_precedence);
            auto [line, col] = get_line_col(current_token);
            left = ast.expressions.add_binary(left, op, right, line, col);
        }

        return left;
    }

    uint32_t Parser::parse_unary()
    {
        if (match(lang::TokenType::MINUS) || match(lang::TokenType::BANG))
        {
            const lang::TokenType op = tokens.types[current - 1];
            const uint32_t operand = parse_unary();
            auto [line, col] = get_line_col(current_token);
            return ast.expressions.add_unary(op, operand, line, col);
        }

        return parse_primary();
    }

    uint32_t Parser::parse_primary()
    {
        if (match(lang::TokenType::NUM_LITERAL))
        {
            std::string_view value = get_token_value(current - 1);
            auto [line, col] = get_line_col(current_token);
            return ast.expressions.add_literal(value, line, col);
        }

        if (match(lang::TokenType::IDENTIFIER))
        {
            const std::string_view name = get_token_value(current - 1);
            const uint32_t symbol_index = lookup_symbol(name);
            auto [line, col] = get_line_col(current_token);
            return ast.expressions.add_identifier(name, symbol_index, line, col);
        }

        if (match(lang::TokenType::LEFT_PAREN))
        {
            return parse_grouping();
        }

        error_reporter.report_error({
            ParseErrorFlags::UNEXPECTED_TOKEN,
            ErrorSeverity::ERROR,
            "Expected expression",
            "Unexpected token",
            source,
            file_name,
            current_token.start,
            current_token.length,
            get_source_line(current),
            create_error_pointer(current)
        });

        throw std::runtime_error("Parse error");
    }

    uint32_t Parser::parse_grouping()
    {
        const uint32_t expr = parse_expression();

        expect(lang::TokenType::RIGHT_PAREN, "Expected ')' after grouped expression");
        return expr;
    }

    uint32_t Parser::parse_literal()
    {
        switch (current_token.type)
        {
            case lang::TokenType::NUM_LITERAL:
            case lang::TokenType::STR_LITERAL:
            case lang::TokenType::BOOLEAN:
            case lang::TokenType::NIL:
            {
                std::string_view value = get_token_value(current);
                auto [line, col] = get_line_col(current_token);
                advance();
                return ast.expressions.add_literal(value, line, col);
            }
            default:
                error_reporter.report_error({
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Invalid literal",
                    "Unexpected token as literal",
                    source,
                    file_name,
                    current_token.start,
                    current_token.length,
                    get_source_line(current),
                    create_error_pointer(current)
                });
                throw std::runtime_error("Invalid literal");
        }
    }

    uint32_t Parser::parse_identifier()
    {
        std::string_view name = get_token_value(current);
        const uint32_t symbol_index = lookup_symbol(name);
        auto [line, col] = get_line_col(current_token);
        advance();
        return ast.expressions.add_identifier(name, symbol_index, line, col);
    }

    uint32_t Parser::parse_statement()
    {
        if (match(lang::TokenType::IF))
        {
            return parse_if_statement();
        }

        if (match(lang::TokenType::LEFT_BRACE))
        {
            return parse_block_statement();
        }

        if (match(lang::TokenType::VAR))
        {
            return parse_variable_decl();
        }

        return parse_expression_statement();
    }

    uint32_t Parser::parse_if_statement()
    {
        expect(lang::TokenType::LEFT_PAREN, "Expected '(' after 'if'");
        const uint32_t condition = parse_expression();
        expect(lang::TokenType::RIGHT_PAREN, "Expected ')' after if condition");

        const uint32_t then_branch = parse_statement();
        const uint32_t else_branch = match(lang::TokenType::ELSE) ? parse_statement() : 0;

        auto [line, col] = get_line_col(current_token);
        return ast.statements.add_if(condition, then_branch, else_branch, line, col);
    }

    uint32_t Parser::parse_block_statement()
    {
        std::vector<uint32_t> statements;
        current_scope++;

        while (!is_at_end() && !match(lang::TokenType::RIGHT_BRACE))
        {
            statements.push_back(parse_statement());
        }

        current_scope--;
        auto [line, col] = get_line_col(current_token);
        return ast.statements.add_block(statements, current_scope, line, col);
    }

    uint32_t Parser::parse_variable_decl()
    {
        // Determine if this is a constant declaration
        bool is_const = match(lang::TokenType::CONST);
        if (!is_const)
        {
            expect(lang::TokenType::VAR,
                   "Expected 'var' or 'const' at the start of variable declaration");
        }

        // Parse variable name
        expect(lang::TokenType::IDENTIFIER,
               "Expected identifier after 'var' or 'const'");
        const std::string_view name = get_token_value(current - 1);

        // Optional type annotation
        uint32_t type_idx = std::numeric_limits<uint32_t>::max();
        if (match(lang::TokenType::COLON))
        {
            type_idx = parse_type();
        }

        // Initialization is mandatory
        expect(lang::TokenType::EQUAL,
               "Expected '=' for variable initialization");
        const uint32_t init_idx = parse_expression();

        if (type_idx == std::numeric_limits<uint32_t>::max())
        {
            if (init_idx < ast.expressions.types.size())
            {
                switch (ast.expressions.types[init_idx])
                {
                    case ast::NodeType::LITERAL:
                    {
                        if (const auto value = ast.expressions.literal_values[init_idx];
                            value.front() == '"' && value.back() == '"')
                        {
                            type_idx = static_cast<uint32_t>(lang::TokenType::STRING);
                        }
                        else if (value == "true" || value == "false" || value == "null")
                        {
                            type_idx = static_cast<uint32_t>(lang::TokenType::BOOLEAN);
                        }
                        else if (value.find('.') != std::string_view::npos)
                        {
                            type_idx = static_cast<uint32_t>(lang::TokenType::F64);
                        }
                        else
                        {
                            const bool is_negative = (value[0] == '-');
                            const size_t start_index = is_negative ? 1 : 0;

                            const uint64_t num_value = std::stoull(std::string(value.substr(start_index)));
                            type_idx = (num_value <= std::numeric_limits<int32_t>::max())
                                           ? static_cast<uint32_t>(lang::TokenType::I32)
                                           : static_cast<uint32_t>(lang::TokenType::I64);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        var_declrs.names.push_back(name);
        var_declrs.type_indices.push_back(type_idx);
        var_declrs.init_indices.push_back(init_idx);
        var_declrs.flags.push_back(is_const);

        const uint32_t symbol_idx = add_symbol(name, type_idx, is_const ? 1 : 0);

        auto [line, col] = get_line_col(current_token);
        return ast.statements.add_var_decl(
            name,
            type_idx,
            init_idx,
            symbol_idx,
            line,
            col
        );
    }

    uint32_t Parser::parse_return_statement()
    {
        expect(lang::TokenType::RETURN, "Expected 'return' keyword");

        uint32_t return_value_idx = 0;
        if (!match(lang::TokenType::SEMICOLON))
        {
            return_value_idx = parse_expression();
            expect(lang::TokenType::SEMICOLON, "Expected ';' after return value");
        }

        auto [line, col] = get_line_col(current_token);
        return ast.statements.add_return(return_value_idx, line, col);
    }

    // TODO: Make generic_param_count
    uint32_t Parser::parse_function_decl()
    {
        expect(lang::TokenType::FUNCTION, "Expected 'fn' keyword");
        uint32_t generic_param_start = 0;
        uint32_t generic_param_count = 0;
        if (match(lang::TokenType::LESS))
        {
            generic_param_start = parse_generic_params();
            expect(lang::TokenType::GREATER, "Expected '>' to close generic parameters");
            generic_param_count = 1;
        }

        expect(lang::TokenType::IDENTIFIER, "Expected function name");
        const std::string_view func_name = get_token_value(current - 1);

        expect(lang::TokenType::LEFT_PAREN, "Expected '(' to start parameter list");
        std::vector<uint32_t> param_indices;

        uint32_t param_start = types.function_params.size();
        uint32_t param_count = 0;

        while (!match(lang::TokenType::RIGHT_PAREN))
        {
            expect(lang::TokenType::IDENTIFIER, "Expected parameter name");
            const std::string_view param_name = get_token_value(current - 1);

            expect(lang::TokenType::COLON, "Expected ':' after parameter name");
            uint32_t param_type_idx = parse_type();

            types.function_params.push_back(param_type_idx);
            param_count++;

            uint32_t symbol_idx = add_symbol(param_name, param_type_idx, 0);
            param_indices.push_back(symbol_idx);

            if (!match(lang::TokenType::COMMA) && current_token.type != lang::TokenType::RIGHT_PAREN)
                break;
        }

        expect(lang::TokenType::MINUS, "Expected '-' before return type");
        expect(lang::TokenType::GREATER, "Expected '>' to complete return type arrow");
        const uint32_t return_type_idx = parse_type();

        const uint32_t function_type_index = types.names.size();
        types.names.emplace_back("function");
        types.function_param_starts.push_back(param_start);
        types.function_param_counts.push_back(param_count);
        types.function_return_types.push_back(return_type_idx);
        types.generic_starts.push_back(generic_param_start);
        types.generic_counts.push_back(generic_param_count);

        expect(lang::TokenType::LEFT_BRACE, "Expected '{' to start function body");
        const uint32_t body_idx = parse_block_statement();

        auto [line, col] = get_line_col(current_token);
        return ast.statements.add_function(
            func_name, function_type_index, param_indices, body_idx, line, col
        );
    }

    uint32_t Parser::parse_expression_statement()
    {
        uint32_t expr_idx = parse_expression();
        expect(lang::TokenType::SEMICOLON, "Expected ';' after expression");

        auto [line, col] = get_line_col(current_token);
        return ast.statements.add_return(expr_idx, line, col);
    }

    uint32_t Parser::parse_type()
    {
        switch (current_token.type)
        {
            case lang::TokenType::U8:
            case lang::TokenType::I8:
            case lang::TokenType::U16:
            case lang::TokenType::I16:
            case lang::TokenType::U32:
            case lang::TokenType::I32:
            case lang::TokenType::U64:
            case lang::TokenType::I64:
            case lang::TokenType::F32:
            case lang::TokenType::F64:
            case lang::TokenType::BOOLEAN:
            case lang::TokenType::STRING:
            case lang::TokenType::VOID:
            {
                const std::string_view type_name = get_token_value(current);

                const uint32_t type_index = types.names.size();
                types.names.push_back(type_name);

                types.generic_starts.push_back(0);
                types.generic_counts.push_back(0);

                advance();
                return type_index;
            }

            default:
            {
                error_reporter.report_error({
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Invalid type",
                    "Expected a valid type",
                    source,
                    file_name,
                    current_token.start,
                    current_token.length,
                    get_source_line(current),
                    create_error_pointer(current)
                });
                throw std::runtime_error("Parse error: Invalid type");
            }
        }
    }

    uint32_t Parser::parse_generic_params()
    {
        uint32_t generic_start = types.generic_params.size();
        uint32_t generic_count = 0;

        auto has_variadic = false;
        if (current_token.type != lang::TokenType::LESS)
        {
            error_reporter.report_error({
                ParseErrorFlags::UNEXPECTED_TOKEN,
                ErrorSeverity::ERROR,
                "Expected '<' to start generic parameters",
                "Open generic parameters with '<'",
                source,
                file_name,
                current_token.start,
                current_token.length,
                get_source_line(current),
                create_error_pointer(current)
            });
            throw std::runtime_error("Parse error: Invalid generic parameter start");
        }

        advance();
        while (current_token.type != lang::TokenType::GREATER)
        {
            if (current_token.type == lang::TokenType::DOT &&
                tokens.types[current + 1] == lang::TokenType::DOT &&
                tokens.types[current + 2] == lang::TokenType::DOT)
            {
                if (has_variadic)
                {
                    error_reporter.report_error({
                        ParseErrorFlags::INVALID_SYNTAX,
                        ErrorSeverity::ERROR,
                        "Multiple variadic generic parameters",
                        "Only one variadic generic parameter is allowed",
                        source,
                        file_name,
                        (get_line_col(current_token).first), // line
                        (get_line_col(current_token).second), // col
                        get_source_line(current),
                        create_error_pointer(current)
                    });
                    throw std::runtime_error("Parse error: Multiple variadic generics");
                }
                has_variadic = true;
                advance(); // first dot
                advance(); // second dot
                advance(); // third dot
            }

            if (current_token.type != lang::TokenType::IDENTIFIER)
            {
                error_reporter.report_error({
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected identifier in generic parameters",
                    "Provide a valid identifier for generic parameter",
                    source,
                    file_name,
                    current_token.start,
                    current_token.length,
                    get_source_line(current),
                    create_error_pointer(current)
                });
                throw std::runtime_error("Parse error: Invalid generic parameter");
            }

            const std::string_view param_name = get_token_value(current);
            uint32_t symbol_idx = add_symbol(
                param_name,
                std::numeric_limits<uint32_t>::max(),
                static_cast<uint8_t>(SymbolFlags::IS_GENERIC_PARAM)
            );

            ast.expressions.add_generic_param(
                param_name,
                symbol_idx,
                has_variadic,
                get_line_col(current_token).first, // line
                get_line_col(current_token).second // column
            );

            types.generic_params.push_back(symbol_idx);
            generic_count++;

            advance();

            if (current_token.type == lang::TokenType::LESS)
            {
                const uint32_t nested_generic_start = parse_generic_params();

                ast.expressions.add_generic_param(
                    param_name,
                    symbol_idx,
                    has_variadic,
                    get_line_col(current_token).first,
                    get_line_col(current_token).second,
                    nested_generic_start
                );
            }

            if (current_token.type == lang::TokenType::COMMA)
            {
                advance();
                has_variadic = false;
            }
            else if (current_token.type != lang::TokenType::GREATER)
            {
                error_reporter.report_error({
                    ParseErrorFlags::UNEXPECTED_TOKEN,
                    ErrorSeverity::ERROR,
                    "Expected ',' or '>' in generic parameters",
                    "Separate generic parameters with ',' or close with '>'",
                    source,
                    file_name,
                    current_token.start,
                    current_token.length,
                    get_source_line(current),
                    create_error_pointer(current)
                });
                throw std::runtime_error("Parse error: Invalid generic parameter syntax");
            }
        }

        advance(); // Consume '>'
        types.generic_starts.push_back(generic_start);
        types.generic_counts.push_back(generic_count);
        return generic_start;
    }

    void Parser::update_current_token()
    {
        if (!is_at_end())
        {
            current_token = {
                tokens.starts[current],
                tokens.lengths[current],
                tokens.types[current],
                tokens.flags[current]
            };
        }
    }

    uint32_t Parser::add_symbol(const std::string_view name, const uint32_t type_index, const uint8_t flags)
    {
        uint32_t symbol_index = symbols.names.size();

        symbols.names.push_back(name);
        symbols.type_indices.push_back(type_index);
        symbols.scopes.push_back(current_scope);
        symbols.symbol_flags.push_back(flags);

        return symbol_index;
    }

    uint32_t Parser::lookup_symbol(const std::string_view name) const
    {
        for (int32_t i = symbols.names.size() - 1; i >= 0; --i)
        {
            if (symbols.names[i] == name &&
                (current_scope == 0 || symbols.scopes[i] <= current_scope))
            {
                return i;
            }
        }
        return std::numeric_limits<uint32_t>::max();
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
            pos--;

        uint32_t line_end = pos;
        while (source[line_end] != '\n' && source[line_end] != '\0')
            line_end++;

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
            switch (current_token.type)
            {
                case lang::TokenType::FUNCTION:
                case lang::TokenType::VAR:
                case lang::TokenType::CONST:
                case lang::TokenType::IF:
                case lang::TokenType::WHILE:
                case lang::TokenType::FOR:
                case lang::TokenType::RETURN:
                case lang::TokenType::RIGHT_BRACE:
                    return;

                case lang::TokenType::SEMICOLON:
                    advance();
                    break;

                default:
                    advance();
                    break;
            }
        }
    }

    std::pair<uint32_t, uint32_t> Parser::get_line_col(const lang::Token &token) const
    {
        const auto it = std::ranges::upper_bound(line_cols, token.start);
        return { std::distance(line_cols.begin(), it), token.start - *(it - 1) + 1 };
    }

    std::string_view Parser::get_token_value(const lang::Token &token) const
    {
        return { source + token.start, token.length };
    }

    std::string_view Parser::get_token_value(const size_t pos)
    {
        return {
            source + tokens.starts[pos],
            tokens.lengths[pos]
        };
    }
}
