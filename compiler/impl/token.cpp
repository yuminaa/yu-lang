#include "../token.h"

namespace yu::lang
{
    void TokenList::push_back(const token_t &token)
    {
        starts.emplace_back(token.start);
        lengths.emplace_back(token.length);
        types.emplace_back(token.type);
        flags.emplace_back(token.flags);
    }

    void TokenList::reserve(const uint32_t &n)
    {
        starts.reserve(n);
        lengths.reserve(n);
        types.reserve(n);
        flags.reserve(n);
    }

    size_t TokenList::size() const
    {
        return starts.size();
    }
}

static constexpr auto create_reverse_map()
{
    std::array<std::string_view, static_cast<size_t>(yu::lang::token_i::END_OF_FILE) + 1> map{};

    for (const auto& [str, token] : yu::lang::token_map)
        map[static_cast<size_t>(token)] = str;

    map[static_cast<size_t>(yu::lang::token_i::IDENTIFIER)] = "IDENTIFIER";
    map[static_cast<size_t>(yu::lang::token_i::NUM_LITERAL)] = "NUM_LITERAL";
    map[static_cast<size_t>(yu::lang::token_i::STR_LITERAL)] = "STR_LITERAL";
    map[static_cast<size_t>(yu::lang::token_i::ANNOTATION)] = "ANNOTATION";
    map[static_cast<size_t>(yu::lang::token_i::UNKNOWN)] = "UNKNOWN";
    map[static_cast<size_t>(yu::lang::token_i::END_OF_FILE)] = "EOF";

    return map;
}

std::string_view yu::lang::token_type_to_string(const token_i &type)
{
    static constexpr auto reverse_token_map = create_reverse_map();
    const auto idx = static_cast<size_t>(type);
    if (idx >= reverse_token_map.size())
        return "INVALID_TOKEN";
    return reverse_token_map[idx];
}