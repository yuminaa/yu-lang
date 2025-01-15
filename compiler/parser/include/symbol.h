#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace yu::compiler
{
    enum class SymbolFlags : uint8_t
    {
        NONE = 0,
        IS_TYPE = 1 << 0,
        IS_CONST = 1 << 1,
        IS_FUNCTION = 1 << 2,
        IS_GENERIC_PARAM = 1 << 3,
        IS_VARIADIC = 1 << 4,
        HAS_VARIADIC_GENERIC = 1 << 5,
        IS_ENUM = 1 << 6,
        IS_ENUM_MEMBER = 1 << 7
    };

    struct VarDeclList
    {
        std::vector<std::string_view> names;
        std::vector<uint32_t> type_indices;
        std::vector<uint32_t> init_indices;
        std::vector<uint8_t> flags;
        std::vector<uint32_t> lines;
        std::vector<uint32_t> columns;
    };

    struct TypeList
    {
        std::vector<std::string_view> names;
        std::vector<uint32_t> generic_starts;
        std::vector<uint32_t> generic_counts;
        std::vector<uint32_t> generic_params;

        std::vector<uint32_t> function_param_starts;
        std::vector<uint32_t> function_param_counts;
        std::vector<uint32_t> function_params;
        std::vector<uint32_t> function_return_types;
    };

    struct SymbolList
    {
        std::vector<std::string_view> names;
        std::vector<uint32_t> type_indices;
        std::vector<uint32_t> scopes;
        std::vector<uint8_t> symbol_flags;
    };
}
