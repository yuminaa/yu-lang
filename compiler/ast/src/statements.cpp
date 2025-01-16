#include "../include/statements.h"

namespace yu::ast
{
    uint32_t StmtList::add_if(const uint32_t cond, const uint32_t then_stmt, const uint32_t else_stmt,
                              const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::IF);
        lines.push_back(line);
        columns.push_back(col);

        condition_indices.push_back(cond);
        then_stmt_indices.push_back(then_stmt);
        else_stmt_indices.push_back(else_stmt);

        flags.push_back(StmtFlags::NONE);

        return idx;
    }

    uint32_t StmtList::add_block(const std::vector<uint32_t> &stmts, const uint32_t scope,
                                 const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::BLOCK);
        lines.push_back(line);
        columns.push_back(col);

        block_stmt_starts.push_back(block_stmt_indices.size());
        block_stmt_lengths.push_back(stmts.size());
        block_stmt_indices.insert(block_stmt_indices.end(), stmts.begin(), stmts.end());

        scope_levels.push_back(scope);

        flags.push_back(StmtFlags::NONE);

        return idx;
    }

    uint32_t StmtList::add_var_decl(const std::string_view name, const uint32_t type_idx,
                                    const uint32_t init_idx, const uint32_t symbol_idx,
                                    const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::VAR_DECL);
        lines.push_back(line);
        columns.push_back(col);

        var_names.push_back(name);
        var_type_indices.push_back(type_idx);
        initializer_indices.push_back(init_idx);
        symbol_indices.push_back(symbol_idx);

        flags.push_back(StmtFlags::NONE);

        return idx;
    }

    uint32_t StmtList::add_return(const uint32_t value_idx, const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::RETURN);
        lines.push_back(line);
        columns.push_back(col);

        return_value_indices.push_back(value_idx);

        flags.push_back(StmtFlags::NONE);

        return idx;
    }

    uint32_t StmtList::add_function(const std::string_view name, const uint32_t type_idx,
                                    const std::vector<uint32_t> &params,
                                    const uint32_t body_idx, const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::FUNCTION_DECL);
        lines.push_back(line);
        columns.push_back(col);

        func_names.push_back(name);
        func_type_indices.push_back(type_idx);
        func_body_indices.push_back(body_idx);

        param_list_starts.push_back(param_indices.size());
        param_list_lengths.push_back(params.size());
        param_indices.insert(param_indices.end(), params.begin(), params.end());

        flags.push_back(StmtFlags::NONE);

        return idx;
    }

    uint32_t StmtList::add_expression_statement(const uint32_t expr_idx,
                                                const uint32_t line, const uint32_t col)
    {
        const uint32_t idx = types.size();

        types.push_back(NodeType::EXPRESSION_STMT);
        lines.push_back(line);
        columns.push_back(col);

        return_value_indices.push_back(expr_idx);

        flags.push_back(StmtFlags::NONE);

        return idx;
    }
}
