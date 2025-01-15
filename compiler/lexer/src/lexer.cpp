/**
 * @file lexer.cpp
 * @brief High-performance lexer implementation for the Yu programming language.
 *  This file is part of the Yu programming language and is licensed under MIT License;
 *  See LICENSE.txt for details
 */

#include "../include/lexer.h"
#include <array>
#include <cstdint>
#include "../../../common/arch.hpp"

namespace yu::compiler
{
    static const std::array<uint8_t, 256> char_type = []
    {
        std::array<uint8_t, 256> types {};
        for (auto i = 0; i < 256; ++i)
        {
            types[i] = (i == ' ' || i == '\t' || i == '\n' || i == '\r') * 1 +
                       (i == '/') * 2 +
                       (i == '*') * 3 +
                       (std::isalpha(i) || i == '_' || i == '@') * 4 +
                       std::isdigit(i) * 5 +
                       (i == '"') * 6;
        }
        return types;
    }();

    static constexpr std::array<lang::TokenType, 256> single_char_tokens = []
    {
        alignas(64) std::array<lang::TokenType, 256> tokens {};
        std::ranges::fill(tokens, lang::TokenType::UNKNOWN);

        tokens['+'] = lang::TokenType::PLUS;
        tokens['-'] = lang::TokenType::MINUS;
        tokens['*'] = lang::TokenType::STAR;
        tokens['/'] = lang::TokenType::SLASH;
        tokens['%'] = lang::TokenType::PERCENT;
        tokens['='] = lang::TokenType::EQUAL;
        tokens['!'] = lang::TokenType::BANG;
        tokens['<'] = lang::TokenType::LESS;
        tokens['>'] = lang::TokenType::GREATER;
        tokens['&'] = lang::TokenType::AND;
        tokens['|'] = lang::TokenType::OR;
        tokens['^'] = lang::TokenType::XOR;
        tokens['~'] = lang::TokenType::TILDE;
        tokens['.'] = lang::TokenType::DOT;
        tokens['('] = lang::TokenType::LEFT_PAREN;
        tokens[')'] = lang::TokenType::RIGHT_PAREN;
        tokens['{'] = lang::TokenType::LEFT_BRACE;
        tokens['}'] = lang::TokenType::RIGHT_BRACE;
        tokens['['] = lang::TokenType::LEFT_BRACKET;
        tokens[']'] = lang::TokenType::RIGHT_BRACKET;
        tokens[','] = lang::TokenType::COMMA;
        tokens[':'] = lang::TokenType::COLON;
        tokens[';'] = lang::TokenType::SEMICOLON;
        tokens['?'] = lang::TokenType::QUESTION;

        return tokens;
    }();

    static constexpr std::array type_to_token = {
        lang::TokenType::UNKNOWN,     // 0
        lang::TokenType::UNKNOWN,     // type 1 (whitespace)
        lang::TokenType::UNKNOWN,     // type 2 (comment start)
        lang::TokenType::UNKNOWN,     // type 3 (comment end)
        lang::TokenType::IDENTIFIER,  // type 4
        lang::TokenType::NUM_LITERAL, // type 5
        lang::TokenType::STR_LITERAL  // type 6
    };

    static constexpr std::array<uint8_t, 256> hex_lookup = []
    {
        std::array<uint8_t, 256> table {};
        for (int i = '0'; i <= '9'; ++i)
            table[i] = 1;
        for (int i = 'a'; i <= 'f'; ++i)
            table[i] = 1;
        for (int i = 'A'; i <= 'F'; ++i)
            table[i] = 1;
        return table;
    }();

    static constexpr std::array<uint8_t, 256> bin_lookup = []
    {
        std::array<uint8_t, 256> table {};
        table['0'] = table['1'] = 1;
        return table;
    }();

    static constexpr std::array<uint8_t, 256> valid_escapes = []
    {
        std::array<uint8_t, 256> table {};
        table['n'] = table['t'] = table['r'] = table['\\'] =
                                               table['"'] = table['0'] = table['x'] = 1;
        return table;
    }();

    ALWAYS_INLINE constexpr uint8_t make_flag(const bool condition, lang::token_flags flag)
    {
        return condition ? static_cast<uint8_t>(flag) : 0;
    }

    Lexer::Lexer(std::string_view src) : src(src.data())
                                         , current_pos(0)
                                         , src_length(static_cast<uint32_t>(src.length()))
    {
        tokens.reserve(src.length() / 4);
        line_starts.reserve(src.length() / 40);
        line_starts.emplace_back(0);
    }

    ALWAYS_INLINE void Lexer::prefetch_next() const
    {
        PREFETCH_L1(src + current_pos + CACHE_LINE_SIZE);
        PREFETCH_L2(src + current_pos + CACHE_LINE_SIZE * 4);
        PREFETCH_L3(src + current_pos + CACHE_LINE_SIZE * 8);
    }

    ALWAYS_INLINE HOT_FUNCTION void Lexer::skip_whitespace_comment(const char *src, uint32_t &current_pos,
                                                                   uint32_t src_length)
    {
        // Fast SIMD-style chunk checking remains unchanged
        while (current_pos + 8 <= src_length)
        {
            const uint64_t chunk = *reinterpret_cast<const uint64_t *>(src + current_pos);
            const uint64_t whitespace = chunk ^ 0x2020202020202020ULL;
            const uint64_t slashes = chunk ^ 0x2F2F2F2F2F2F2F2FULL;
            if (!whitespace)
            {
                current_pos += 8;
                continue;
            }

            if (slashes == 0)
                break;

            current_pos += __builtin_ctzll(whitespace) / 8;
            break;
        }

        while (current_pos < src_length)
        {
            const char current_char = src[current_pos];
            const uint8_t type = char_type[static_cast<uint8_t>(current_char)];
            const uint32_t is_newline = current_char == '\n';

            // Branchless line start tracking
            const size_t curr_size = line_starts.size();
            line_starts.resize(curr_size + is_newline);
            line_starts[curr_size] *= !is_newline;
            line_starts[curr_size] += (current_pos + 1) * is_newline;

            const bool has_next = current_pos + 1 < src_length;
            const char next_char = has_next ? src[current_pos + 1] : '\0';

            const uint32_t is_slash = type == 2;
            const uint32_t is_single_comment = is_slash & (next_char == '/');
            const uint32_t is_multi_comment = is_slash & (next_char == '*');

            current_pos += 2 * is_single_comment;
            while (is_single_comment && current_pos < src_length && src[current_pos] != '\n')
                ++current_pos;

            current_pos += 2 * is_multi_comment;
            uint32_t in_comment = is_multi_comment;
            while (in_comment && current_pos + 1 < src_length)
            {
                const uint32_t end_of_comment = src[current_pos] == '*' & src[current_pos + 1] == '/';

                // Handle newlines in multi-line comments
                const uint32_t comment_newline = src[current_pos] == '\n';
                const size_t comment_size = line_starts.size();
                line_starts.resize(comment_size + comment_newline);
                line_starts[comment_size] *= !comment_newline;
                line_starts[comment_size] += (current_pos + 1) * comment_newline;

                current_pos += 1 + end_of_comment;
                in_comment &= !end_of_comment;
            }

            const uint32_t should_continue = type == 1;
            current_pos += should_continue;

            if (!(should_continue | is_single_comment | is_multi_comment))
                return;
        }
    }

    ALWAYS_INLINE HOT_FUNCTION lang::token_t Lexer::next_token()
    {
        skip_whitespace_comment(src, current_pos, src_length);

        if (UNLIKELY(current_pos >= src_length))
            return { current_pos, 0, lang::TokenType::END_OF_FILE, 0 };

        switch (const char c = src[current_pos]; char_type[static_cast<uint8_t>(c)])
        {
            case 4:
                return lex_identifier();
            case 5:
                return lex_number();
            case 6:
                return lex_string();
            default:
                return { current_pos, 1, single_char_tokens[static_cast<uint8_t>(c)], 0 };
        }
    }

    HOT_FUNCTION lang::token_t Lexer::lex_identifier() const
    {
        const char *start = src + current_pos;
        const char *current = start;
        uint8_t flags = 0;

        const uint32_t is_valid_start = (*current == '_' || *current == '@' ||
                                         std::isalpha(*current));
        flags |= make_flag(!is_valid_start, lang::token_flags::INVALID_IDENTIFIER_START);

        current += (*current == '@');

        while (current < src + src_length)
        {
            const char c = *current;
            const uint32_t is_alnum = std::isalnum(c);
            const uint32_t is_underscore = c == '_';
            const uint32_t is_space = std::isspace(c);
            const uint32_t is_punct = std::ispunct(c);

            const uint32_t is_valid = is_alnum | is_underscore;
            const uint32_t is_terminator = (!is_valid) & (is_space | is_punct);

            flags |= (!is_valid & !is_terminator) *
                    static_cast<uint8_t>(lang::token_flags::INVALID_IDENTIFIER_CHAR);

            current += is_valid;
            if (!is_valid)
                break;
        }

        const uint16_t length = current - start;
        const std::string_view text(start, length);

        for (const auto &[token_text, token_type]: lang::token_map)
        {
            if (token_text.length() == length &&
                memcmp(token_text.data(), text.data(), length) == 0)
            {
                return { current_pos, length, token_type, flags };
            }
        }

        return { current_pos, length, lang::TokenType::IDENTIFIER, flags };
    }

    HOT_FUNCTION lang::token_t Lexer::lex_number() const
    {
        const char *start = src + current_pos;
        const char *current = start;
        const char *end = src + src_length;
        uint8_t flags = 0;

        if (LIKELY(*current >= '0' && *current <= '9'))
        {
            while (current + 8 <= end)
            {
                const uint64_t chunk = *reinterpret_cast<const uint64_t *>(current);
                const uint64_t digit_test = chunk - 0x3030303030303030ULL;
                const uint64_t digit_test2 = chunk + 0x4646464646464646ULL;
                current += 8 * !((digit_test | digit_test2) & 0x8080808080808080ULL);
                if ((digit_test | digit_test2) & 0x8080808080808080ULL)
                    break;
            }
        }

        const uint32_t has_next = current + 1 < end;
        const char next = current[has_next];
        const uint32_t is_zero = *current == '0';
        const uint32_t is_x = (next | 32) == 'x';
        const uint32_t is_b = (next | 32) == 'b';
        const uint32_t is_hex = is_zero & is_x;
        const uint32_t is_bin = is_zero & is_b;

        current += 2 * (is_hex | is_bin);

        uint32_t has_decimal = 0;
        while (current < end)
        {
            const char c = *current;
            const uint32_t is_digit = (c >= '0' && c <= '9');
            const uint32_t is_dot = c == '.';
            const uint32_t is_valid_hex = hex_lookup[static_cast<uint8_t>(c)];
            const uint32_t is_valid_bin = bin_lookup[static_cast<uint8_t>(c)];
            const uint32_t is_valid = (is_hex & is_valid_hex) |
                                      (is_bin & is_valid_bin) |
                                      ((!is_hex & !is_bin) & (is_digit | is_dot));

            if (!is_valid)
                break;

            has_decimal += is_dot;
            flags |= (has_decimal > 1) * static_cast<uint8_t>(lang::token_flags::MULTIPLE_DECIMAL_POINTS);

            ++current;
        }

        const uint32_t at_exp = current < end;
        const uint32_t is_exp = at_exp & ((*current | 32) == 'e');
        current += is_exp;

        const uint32_t has_sign = (current < end) & ((*current == '+') | (*current == '-'));
        current += is_exp * has_sign;

        const uint32_t exp_valid = (current < end) & ((*current >= '0') & (*current <= '9'));
        flags |= (is_exp & !exp_valid) * static_cast<uint8_t>(lang::token_flags::INVALID_EXPONENT);

        const uint32_t should_continue = is_exp & exp_valid;
        while (should_continue & (current < end) & ((*current >= '0') & (*current <= '9')))
            ++current;

        return {
            current_pos,
            static_cast<uint16_t>(current - start),
            lang::TokenType::NUM_LITERAL,
            flags
        };
    }

    HOT_FUNCTION lang::token_t Lexer::lex_string() const
    {
        const char *start = src + current_pos;
        const char *current = start + 1;
        const char *end = src + src_length;
        uint8_t flags = 0;

        while (current < end)
        {
            const char c = *current;
            const uint32_t is_escape = c == '\\';
            const uint32_t has_next = current + 1 < end;
            const char next = current[has_next];

            const uint32_t is_quote = c == '"';
            const uint32_t is_valid_escape = valid_escapes[static_cast<uint8_t>(next)];
            const uint32_t escape_advance = is_escape * (1 + (next == 'x') * 2);

            flags |= make_flag(is_escape & !is_valid_escape,
                               lang::token_flags::INVALID_ESCAPE_SEQUENCE);

            current += 1 + escape_advance;
            if (is_quote | (is_escape & !is_valid_escape))
                break;
        }

        flags |= make_flag(current >= end || *(current - 1) != '"',
                           lang::token_flags::UNTERMINATED_STRING);
        return {
            current_pos,
            static_cast<uint16_t>(current - start),
            lang::TokenType::STR_LITERAL,
            flags
        };
    }

    lang::TokenList *Lexer::tokenize()
    {
        uint32_t state = 1;
        while (state)
        {
            lang::token_t token = next_token();
            tokens.push_back(token);

            state = token.type != lang::TokenType::END_OF_FILE;
            current_pos += token.length * state;
            prefetch_next();
        }

        return &tokens;
    }

    HOT_FUNCTION std::pair<uint32_t, uint32_t> Lexer::get_line_col(const lang::token_t &token) const
    {
        const auto it = std::ranges::upper_bound(line_starts, token.start);
        return { std::distance(line_starts.begin(), it), token.start - *(it - 1) + 1 };
    }

    HOT_FUNCTION lang::TokenType Lexer::get_token_type(const char c)
    {
        const uint8_t char_class = char_type[static_cast<uint8_t>(c)];
        const lang::TokenType single_token = single_char_tokens[static_cast<uint8_t>(c)];
        const lang::TokenType type_token = type_to_token[char_class];

        return (single_token != lang::TokenType::UNKNOWN) ? single_token : type_token;
    }

    HOT_FUNCTION std::string_view Lexer::get_token_value(const lang::token_t &token) const
    {
        return { src + token.start, token.length };
    }

    HOT_FUNCTION std::string_view Lexer::get_token_value(const size_t pos)
    {
        return {
            src + tokens.starts[pos],
            tokens.lengths[pos]
        };
    }
}
