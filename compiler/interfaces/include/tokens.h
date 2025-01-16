// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#pragma once

#include <cstdint>
#include <vector>

namespace yu::lang
{
    /**
     * @brief Represents the type of token that the lexer has found.
     */
    enum class TokenType : uint8_t
    {
        // Keywords
        TRUE,
        FALSE,
        NIL,
        IMPORT,
        VAR,
        CONST,
        FUNCTION,
        INLINE,
        RETURN,
        ENUM,
        IF,
        ELSE,
        FOR,
        WHILE,
        BREAK,
        CONTINUE,
        SWITCH,
        CASE,
        DEFAULT,
        CLASS,
        FINAL,
        PUBLIC,
        PRIVATE,
        PROTECTED,
        STATIC,
        AWAIT,
        ASYNC,
        TRY,
        CATCH,
        FROM,
        AS,
        OPERATOR,
        NEW,
        DELETE,

        // Basic types
        U8,
        I8,
        U16,
        I16,
        U32,
        I32,
        U64,
        I64,
        F32,
        F64,
        STRING,
        BOOLEAN,
        VOID,
        PTR,

        // Single character operators
        PLUS,    // +
        MINUS,   // -
        STAR,    // *
        SLASH,   // /
        PERCENT, // %
        EQUAL,   // =
        BANG,    // !
        LESS,    // <
        GREATER, // >
        AND,     // &
        OR,      // |
        XOR,     // ^
        TILDE,   // ~
        DOT,     // .

        // Delimiters
        LEFT_PAREN,    // (
        RIGHT_PAREN,   // )
        LEFT_BRACE,    // {
        RIGHT_BRACE,   // }
        LEFT_BRACKET,  // [
        RIGHT_BRACKET, // ]
        COMMA,         // ,
        COLON,         // :
        SEMICOLON,     // ;
        QUESTION,      // ?

        // Annotations
        ALIGN_ANNOT,      // @align
        DEPRECATED_ANNOT, // @deprecated
        PACKED_ANNOT,     // @packed
        NO_DISCARD_ANNOT, // @nodiscard
        VOLATILE_ANNOT,   // @volatile
        LAZY_ANNOT,       // @lazy
        PURE_ANNOT,       // @pure
        TAIL_REC_ANNOT,   // @tailrec

        // Special tokens
        IDENTIFIER,
        NUM_LITERAL,
        STR_LITERAL,
        ANNOTATION,
        UNKNOWN,
        END_OF_FILE
    };

    /**
     * @brief Flags that can be associated with tokens to indicate errors on conditions.
    */
    enum class token_flags : uint8_t
    {
        NONE = 0,

        // String errors
        UNTERMINATED_STRING = 1 << 0,
        INVALID_ESCAPE_SEQUENCE = 1 << 1,

        // Number errors
        INVALID_DIGIT = 1 << 2,
        MULTIPLE_DECIMAL_POINTS = 1 << 3,
        INVALID_EXPONENT = 1 << 4,

        // Comment errors
        UNTERMINATED_BLOCK_COMMENT = 1 << 5,

        // Identifier errors
        INVALID_IDENTIFIER_START = 1 << 6,
        INVALID_IDENTIFIER_CHAR = 1 << 7
    };

    /**
     * @brief A map to quickly lookup tokens.
     */
    static constexpr std::pair<std::string_view, TokenType> token_map[] = {
        // Keywords
        { "true", TokenType::TRUE },
        { "false", TokenType::FALSE },
        { "null", TokenType::NIL },
        { "import", TokenType::IMPORT },
        { "var", TokenType::VAR },
        { "const", TokenType::CONST },
        { "function", TokenType::FUNCTION },
        { "inline", TokenType::INLINE },
        { "return", TokenType::RETURN },
        { "enum", TokenType::ENUM },
        { "if", TokenType::IF },
        { "else", TokenType::ELSE },
        { "for", TokenType::FOR },
        { "while", TokenType::WHILE },
        { "break", TokenType::BREAK },
        { "continue", TokenType::CONTINUE },
        { "switch", TokenType::SWITCH },
        { "case", TokenType::CASE },
        { "default", TokenType::DEFAULT },
        { "class", TokenType::CLASS },
        { "final", TokenType::FINAL },
        { "public", TokenType::PUBLIC },
        { "private", TokenType::PRIVATE },
        { "await", TokenType::AWAIT },
        { "async", TokenType::ASYNC },
        { "try", TokenType::TRY },
        { "catch", TokenType::CATCH },
        { "static", TokenType::STATIC },
        { "from", TokenType::FROM },
        { "as", TokenType::AS },
        { "operator", TokenType::OPERATOR },
        { "new", TokenType::NEW },
        { "delete", TokenType::NEW },

        // Basic types
        { "u8", TokenType::U8 },
        { "i8", TokenType::I8 },
        { "u16", TokenType::U16 },
        { "i16", TokenType::I16 },
        { "u32", TokenType::U32 },
        { "i32", TokenType::I32 },
        { "u64", TokenType::U64 },
        { "i64", TokenType::I64 },
        { "f32", TokenType::F32 },
        { "f64", TokenType::F64 },
        { "string", TokenType::STRING },
        { "bool", TokenType::BOOLEAN },
        { "void", TokenType::VOID },
        { "Ptr", TokenType::PTR },

        // Single character operators
        { "+", TokenType::PLUS },
        { "-", TokenType::MINUS },
        { "*", TokenType::STAR },
        { "/", TokenType::SLASH },
        { "%", TokenType::PERCENT },
        { "=", TokenType::EQUAL },
        { "!", TokenType::BANG },
        { "<", TokenType::LESS },
        { ">", TokenType::GREATER },
        { "&", TokenType::AND },
        { "|", TokenType::OR },
        { "^", TokenType::XOR },
        { "~", TokenType::TILDE },
        { ".", TokenType::DOT },

        // Delimiters
        { "(", TokenType::LEFT_PAREN },
        { ")", TokenType::RIGHT_PAREN },
        { "{", TokenType::LEFT_BRACE },
        { "}", TokenType::RIGHT_BRACE },
        { "[", TokenType::LEFT_BRACKET },
        { "]", TokenType::RIGHT_BRACKET },
        { ",", TokenType::COMMA },
        { ":", TokenType::COLON },
        { ";", TokenType::SEMICOLON },
        { "?", TokenType::QUESTION },

        // Annotations
        { "@align", TokenType::ALIGN_ANNOT },
        { "@deprecated", TokenType::DEPRECATED_ANNOT },
        { "@packed", TokenType::PACKED_ANNOT },
        { "@nodiscard", TokenType::NO_DISCARD_ANNOT },
        { "@volatile", TokenType::VOLATILE_ANNOT },
        { "@lazy", TokenType::LAZY_ANNOT },
        { "@pure", TokenType::PURE_ANNOT },
        { "@tailrec", TokenType::TAIL_REC_ANNOT }
    };

    /**
     * @brief Represents a token that the lexer has found.
     */
    struct alignas(8) Token
    {
        uint32_t start;
        uint16_t length;
        TokenType type;
        uint8_t flags;
    };

    /**
     * @brief A structure to efficiently store tokens.
     */
    struct alignas(8) TokenList
    {
        std::vector<uint32_t> starts;
        std::vector<uint16_t> lengths;
        std::vector<TokenType> types;
        std::vector<uint8_t> flags;

        void emplace_back(const Token &token)
        {
            starts.emplace_back(token.start);
            lengths.emplace_back(token.length);
            types.emplace_back(token.type);
            flags.emplace_back(token.flags);
        }

        void push_back(const Token &token)
        {
            starts.push_back(token.start);
            lengths.push_back(token.length);
            types.push_back(token.type);
            flags.push_back(token.flags);
        }

        void reserve(const uint32_t &n)
        {
            starts.reserve(n);
            lengths.reserve(n);
            types.reserve(n);
            flags.reserve(n);
        }

        [[nodiscard]] size_t size() const
        {
            return starts.size();
        }
    };

    static constexpr auto create_reverse_map()
    {
        std::array<std::string_view, static_cast<size_t>(TokenType::END_OF_FILE) + 1> map{};

        for (const auto& [str, token] : token_map)
            map[static_cast<size_t>(token)] = str;

        map[static_cast<size_t>(TokenType::IDENTIFIER)] = "IDENTIFIER";
        map[static_cast<size_t>(TokenType::NUM_LITERAL)] = "NUM_LITERAL";
        map[static_cast<size_t>(TokenType::STR_LITERAL)] = "STR_LITERAL";
        map[static_cast<size_t>(TokenType::ANNOTATION)] = "ANNOTATION";
        map[static_cast<size_t>(TokenType::UNKNOWN)] = "UNKNOWN";
        map[static_cast<size_t>(TokenType::END_OF_FILE)] = "EOF";

        return map;
    }

    inline std::string_view Tokenype_to_string(const TokenType &type)
    {
        static constexpr auto reverse_token_map = create_reverse_map();
        const auto idx = static_cast<size_t>(type);
        if (idx >= reverse_token_map.size())
            return "INVALID_TOKEN";
        return reverse_token_map[idx];
    }
}
