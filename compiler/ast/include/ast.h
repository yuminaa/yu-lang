#pragma once

#include "expressions.h"
#include "statements.h"

namespace yu::ast
{
    struct AST
    {
        ExprList expressions;
        StmtList statements;
        uint32_t root_stmt_index;

        void clear();

        uint32_t add_root_block(uint32_t line, uint32_t col);

        bool validate() const;
    };
}
