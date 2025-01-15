#pragma once
#include <vector>
#include <string_view>
#include "nodes.h"
#include "../../interfaces/include/tokens.h"

namespace yu::ast
{
    struct ExprList
    {
        std::vector<NodeType> types;
        std::vector<uint32_t> lines;
        std::vector<uint32_t> columns;
        std::vector<uint32_t> type_indices; // Reference to TypeList
        std::vector<ExprFlags> flags;

        std::vector<uint32_t> left_expr_indices;
        std::vector<uint32_t> right_expr_indices;
        std::vector<lang::TokenType> operators;

        std::vector<uint32_t> operand_indices;
        std::vector<lang::TokenType> unary_operators;

        std::vector<std::string_view> literal_values;

        std::vector<std::string_view> var_names;
        std::vector<uint32_t> symbol_indices;

        std::vector<uint32_t> callee_indices;
        std::vector<uint32_t> arg_list_starts;
        std::vector<uint32_t> arg_list_lengths;
        std::vector<uint32_t> arg_indices;

        uint32_t add_binary(uint32_t left, lang::TokenType op, uint32_t right,
                            uint32_t line, uint32_t col);

        uint32_t add_unary(lang::TokenType op, uint32_t operand,
                           uint32_t line, uint32_t col);

        uint32_t add_literal(std::string_view value, uint32_t line, uint32_t col);

        uint32_t add_variable(std::string_view name, uint32_t symbol_idx,
                              uint32_t line, uint32_t col);

        uint32_t add_call(uint32_t callee, const std::vector<uint32_t> &args,
                          uint32_t line, uint32_t col);
    };
}
