#pragma once
#include <vector>
#include <string_view>
#include "nodes.h"

namespace yu::ast
{
    struct StmtList
    {
        std::vector<NodeType> types;
        std::vector<uint32_t> lines;
        std::vector<uint32_t> columns;
        std::vector<StmtFlags> flags;

        std::vector<uint32_t> condition_indices;
        std::vector<uint32_t> then_stmt_indices;
        std::vector<uint32_t> else_stmt_indices;

        std::vector<uint32_t> block_stmt_starts;
        std::vector<uint32_t> block_stmt_lengths;
        std::vector<uint32_t> block_stmt_indices;
        std::vector<uint32_t> scope_levels;

        // Variable declarations
        std::vector<std::string_view> var_names;
        std::vector<uint32_t> var_type_indices;
        std::vector<uint32_t> initializer_indices;
        std::vector<uint32_t> symbol_indices;

        std::vector<uint32_t> return_value_indices;

        std::vector<std::string_view> func_names;
        std::vector<uint32_t> func_type_indices;
        std::vector<uint32_t> func_body_indices;
        std::vector<uint32_t> param_list_starts;
        std::vector<uint32_t> param_list_lengths;
        std::vector<uint32_t> param_indices;

        uint32_t add_if(uint32_t cond, uint32_t then_stmt, uint32_t else_stmt,
                        uint32_t line, uint32_t col);

        uint32_t add_block(const std::vector<uint32_t> &stmts, uint32_t scope,
                           uint32_t line, uint32_t col);

        uint32_t add_var_decl(std::string_view name, uint32_t type_idx,
                              uint32_t init_idx, uint32_t symbol_idx,
                              uint32_t line, uint32_t col);

        uint32_t add_return(uint32_t value_idx, uint32_t line, uint32_t col);

        uint32_t add_function(std::string_view name, uint32_t type_idx,
                              const std::vector<uint32_t> &params,
                              uint32_t body_idx, uint32_t line, uint32_t col);
    };
} // namespace yu::ast
