// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include "token.h"
#include "../../common/arch.hpp"

namespace yu::compiler
{
    /**
     * @brief The lexer class.
     */
    class alignas(64) Lexer
    {
    public:
        /**
         * @brief Creates a lexer object.
         * @param src The source code to tokenize.
         * @return Lexer The lexer object.
         * @throws std::runtime_error if the source file is too large (>4GiB).
         */
        explicit Lexer(std::string_view src);

        /**
         * @brief Tokenizes the source code and returns the token type.
         * @return token_i The token type.
         */
        lang::TokenList *tokenize();

        /**
         * @brief Get the line and column for a given token.
         * @param token The token.
         * @return pair of line and column.
         */
        HOT_FUNCTION std::pair<uint32_t, uint32_t> get_line_col(const lang::token_t &token) const;

        /**
         * @brief Get the string value of a token.
         * @param token The token.
         * @return std::string_view The string value of the token.
         */
        HOT_FUNCTION std::string_view get_token_value(const lang::token_t &token) const;

        HOT_FUNCTION std::string_view get_token_value(size_t pos);

        /**
         * @brief Checks if the token is a keyword or an identifier or a type.
         * @param c The character to check.
         * @return token_i The token type.
         */
        HOT_FUNCTION static lang::token_i get_token_type(char c);

     std::vector<uint32_t> line_starts;

    private:
        const char *src {};
        uint32_t current_pos {};
        uint32_t src_length {};

        lang::TokenList tokens;

        /**
         * @brief Returns the next token.
         * @return token_t The next token.
         */
        ALWAYS_INLINE HOT_FUNCTION lang::token_t next_token();

        /**
         * @brief Prefetch the next token for optimization.
         */
        ALWAYS_INLINE HOT_FUNCTION void prefetch_next() const;

        /**
         * @brief Skips whitespace and comments.
         * @param src The source code.
         * @param current_pos The current position in the source code.
         * @param src_length The length of the source code.
        */
        ALWAYS_INLINE HOT_FUNCTION void skip_whitespace_comment(const char *src, uint32_t &current_pos,
                                                                uint32_t src_length);

        /**
         * @brief Checks if the token is a keyword or an identifier or a type.
         * @return token_t The token type.
         */
        HOT_FUNCTION lang::token_t lex_identifier() const;

        /**
         * @brief Checks if the token is a number literal. Allows for floating point numbers, hexadecimals, and binary numbers.
         * @return token_t The token type.
         */
        HOT_FUNCTION lang::token_t lex_number() const;

        /**
         * @brief Checks if the token is a string literal. Allows for escape sequences.
         * @return token_t The token type.
         */
        HOT_FUNCTION lang::token_t lex_string() const;
    };
}
