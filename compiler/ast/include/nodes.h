#pragma once
#include <cstdint>

namespace yu::ast
{
    enum class NodeType : uint8_t
    {
        LITERAL,
        BINARY,
        UNARY,
        GROUPING,
        VARIABLE,
        FUNCTION_CALL,

        IF,
        WHILE,
        FOR,
        BLOCK,
        VAR_DECL,
        FUNCTION_DECL,
        RETURN,
        EXPRESSION_STMT,

        GENERIC_PARAM
    };

    enum class ExprFlags : uint8_t
    {
        NONE = 0,
        CONSTANT = 1 << 0,
        PURE = 1 << 1,
        EVALUATED = 1 << 2,
        HAS_SIDE_EFFECTS = 1 << 3,
        VARIADIC = 1 << 4
    };

    enum class StmtFlags : uint8_t
    {
        NONE = 0,
        IS_CONST = 1 << 0,
        IS_PUBLIC = 1 << 1,
        IS_STATIC = 1 << 2,
        IS_ASYNC = 1 << 3,
    };
}
