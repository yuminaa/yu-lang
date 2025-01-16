#include "../include/expressions.h"

namespace yu::ast
{
    uint32_t ExprList::add_binary(const uint32_t left, const lang::TokenType op, const uint32_t right,
                                  const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::BINARY);
        lines.push_back(line);
        columns.push_back(col);

        left_expr_indices.push_back(left);
        right_expr_indices.push_back(right);
        operators.push_back(op);

        type_indices.push_back(std::numeric_limits<uint32_t>::max());
        flags.push_back(ExprFlags::NONE);

        return idx;
    }

    uint32_t ExprList::add_unary(const lang::TokenType op, const uint32_t operand,
                                 const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::UNARY);
        lines.push_back(line);
        columns.push_back(col);

        unary_operators.push_back(op);
        operand_indices.push_back(operand);

        type_indices.push_back(std::numeric_limits<uint32_t>::max());
        flags.push_back(ExprFlags::NONE);

        return idx;
    }

    uint32_t ExprList::add_literal(const std::string_view value, const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::LITERAL);
        lines.push_back(line);
        columns.push_back(col);

        literal_values.push_back(value);

        type_indices.push_back(std::numeric_limits<uint32_t>::max());
        flags.push_back(ExprFlags::NONE);

        return idx;
    }

    uint32_t ExprList::add_identifier(const std::string_view name, const uint32_t symbol_idx,
                                      const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::VARIABLE);
        lines.push_back(line);
        columns.push_back(col);

        var_names.push_back(name);
        symbol_indices.push_back(symbol_idx);

        type_indices.push_back(std::numeric_limits<uint32_t>::max());
        flags.push_back(ExprFlags::NONE);

        return idx;
    }

    uint32_t ExprList::add_call(const uint32_t callee, const std::vector<uint32_t> &args,
                                const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::FUNCTION_CALL);
        lines.push_back(line);
        columns.push_back(col);

        callee_indices.push_back(callee);

        arg_list_starts.push_back(arg_indices.size());
        arg_list_lengths.push_back(args.size());

        arg_indices.insert(arg_indices.end(), args.begin(), args.end());
        type_indices.push_back(std::numeric_limits<uint32_t>::max());
        flags.push_back(ExprFlags::NONE);

        return idx;
    }

    uint32_t ExprList::add_generic_param(std::string_view name, uint32_t symbol_idx, bool is_variadic, uint32_t line,
        uint32_t col, uint32_t nested_generic_idx)
    {
        uint32_t idx = types.size();

        types.push_back(NodeType::GENERIC_PARAM);
        lines.push_back(line);
        columns.push_back(col);

        var_names.push_back(name);
        symbol_indices.push_back(symbol_idx);

        flags.push_back(is_variadic ? ExprFlags::VARIADIC : ExprFlags::NONE);

        if (nested_generic_idx != std::numeric_limits<uint32_t>::max())
        {
            nested_generic_indices.push_back(nested_generic_idx);
        }
        else
        {
            nested_generic_indices.push_back(std::numeric_limits<uint32_t>::max());
        }

        type_indices.push_back(std::numeric_limits<uint32_t>::max());

        return idx;
    }
}
