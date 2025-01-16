// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#include <iomanip>
#include <gtest/gtest.h>
#include "../../compiler/lexer/include/lexer.h"

using namespace yu::compiler;
using namespace yu::lang;

class TokenPrinter
{
    static constexpr auto YELLOW = "\033[33m";
    static constexpr auto RESET = "\033[0m";

    static std::string get_token_name(const TokenType type)
    {
        switch (type)
        {
            case TokenType::TRUE:
                return "TRUE";
            case TokenType::FALSE:
                return "FALSE";
            case TokenType::NIL:
                return "NIL";
            case TokenType::IMPORT:
                return "IMPORT";
            case TokenType::VAR:
                return "VAR";
            case TokenType::CONST:
                return "CONST";
            case TokenType::FUNCTION:
                return "FUNCTION";
            case TokenType::INLINE:
                return "INLINE";
            case TokenType::RETURN:
                return "RETURN";
            case TokenType::NEW:
                return "NEW";
            case TokenType::ENUM:
                return "ENUM";
            case TokenType::IF:
                return "IF";
            case TokenType::ELSE:
                return "ELSE";
            case TokenType::FOR:
                return "FOR";
            case TokenType::WHILE:
                return "WHILE";
            case TokenType::BREAK:
                return "BREAK";
            case TokenType::CONTINUE:
                return "CONTINUE";
            case TokenType::SWITCH:
                return "SWITCH";
            case TokenType::CASE:
                return "CASE";
            case TokenType::DEFAULT:
                return "DEFAULT";
            case TokenType::CLASS:
                return "CLASS";
            case TokenType::FINAL:
                return "FINAL";
            case TokenType::PUBLIC:
                return "PUBLIC";
            case TokenType::PRIVATE:
                return "PRIVATE";
            case TokenType::PROTECTED:
                return "PROTECTED";
            case TokenType::STATIC:
                return "STATIC";
            case TokenType::AWAIT:
                return "AWAIT";
            case TokenType::ASYNC:
                return "ASYNC";
            case TokenType::TRY:
                return "TRY";
            case TokenType::CATCH:
                return "CATCH";
            case TokenType::FROM:
                return "FROM";
            // Basic types
            case TokenType::U8:
                return "U8";
            case TokenType::I8:
                return "I8";
            case TokenType::U16:
                return "U16";
            case TokenType::I16:
                return "I16";
            case TokenType::U32:
                return "U32";
            case TokenType::I32:
                return "I32";
            case TokenType::U64:
                return "U64";
            case TokenType::I64:
                return "I64";
            case TokenType::F32:
                return "F32";
            case TokenType::F64:
                return "F64";
            case TokenType::STRING:
                return "STRING";
            case TokenType::BOOLEAN:
                return "BOOLEAN";
            case TokenType::VOID:
                return "VOID";
            case TokenType::PTR:
                return "PTR";
            // Operators
            case TokenType::PLUS:
                return "PLUS";
            case TokenType::MINUS:
                return "MINUS";
            case TokenType::STAR:
                return "STAR";
            case TokenType::SLASH:
                return "SLASH";
            case TokenType::PERCENT:
                return "PERCENT";
            case TokenType::EQUAL:
                return "EQUAL";
            case TokenType::BANG:
                return "BANG";
            case TokenType::LESS:
                return "LESS";
            case TokenType::GREATER:
                return "GREATER";
            case TokenType::AND:
                return "AND";
            case TokenType::OR:
                return "OR";
            case TokenType::XOR:
                return "XOR";
            case TokenType::TILDE:
                return "TILDE";
            case TokenType::DOT:
                return "DOT";
            // Delimiters
            case TokenType::LEFT_PAREN:
                return "LEFT_PAREN";
            case TokenType::RIGHT_PAREN:
                return "RIGHT_PAREN";
            case TokenType::LEFT_BRACE:
                return "LEFT_BRACE";
            case TokenType::RIGHT_BRACE:
                return "RIGHT_BRACE";
            case TokenType::LEFT_BRACKET:
                return "LEFT_BRACKET";
            case TokenType::RIGHT_BRACKET:
                return "RIGHT_BRACKET";
            case TokenType::COMMA:
                return "COMMA";
            case TokenType::COLON:
                return "COLON";
            case TokenType::SEMICOLON:
                return "SEMICOLON";
            case TokenType::QUESTION:
                return "QUESTION";
            case TokenType::IDENTIFIER:
                return "IDENTIFIER";
            case TokenType::NUM_LITERAL:
                return "NUM_LITERAL";
            case TokenType::STR_LITERAL:
                return "STR_LITERAL";
            case TokenType::ANNOTATION:
                return "ANNOTATION";
            case TokenType::UNKNOWN:
                return "UNKNOWN";
            case TokenType::END_OF_FILE:
                return "EOF";
            default:
                return "UNHANDLED_TOKEN";
        }
    }

public:
    static std::string print(const TokenList &tokens, const Lexer &lexer)
    {
        std::stringstream ss;
        ss << YELLOW << "Tokens:" << RESET << "\n";

        for (std::vector<unsigned>::size_type i = 0; i < tokens.types.size(); ++i)
        {
            auto [line, col] = lexer.get_line_col({
                tokens.starts[i], tokens.lengths[i],
                tokens.types[i], tokens.flags[i]
            });
            const std::string_view token_text = lexer.get_token_value({
                tokens.starts[i], tokens.lengths[i],
                tokens.types[i], tokens.flags[i]
            });

            ss << YELLOW
                    << "[" << std::setw(3) << i << "] "
                    << std::setw(15) << get_token_name(tokens.types[i]) << " | "
                    << "'" << token_text << "'"
                    << " (line: " << line << ", col: " << col
                    << ", pos: " << tokens.starts[i] << ", len: " << tokens.lengths[i] << ")"
                    << RESET << "\n";
        }

        return ss.str();
    }

    static std::string print(const Token &token, const Lexer &lexer)
    {
        std::stringstream ss;
        auto [line, col] = lexer.get_line_col(token);
        const std::string_view token_text = lexer.get_token_value(token);

        ss << YELLOW
                << get_token_name(token.type) << " | "
                << "'" << token_text << "'"
                << " (line: " << line << ", col: " << col
                << ", pos: " << token.start << ", len: " << token.length << ")"
                << RESET;

        return ss.str();
    }
};


class LexerTest : public testing::Test
{
protected:
    void verify_token(const TokenList *tokens, auto index, const TokenType expected_type,
                      const std::string_view &source, Lexer lexer)
    {
        ASSERT_LT(index, tokens->types.size())
            << "Token index out of range";

        const Token token = {
            tokens->starts[index],
            tokens->lengths[index],
            tokens->types[index],
            tokens->flags[index]
        };

        EXPECT_EQ(tokens->types[index], expected_type)
            << "Token type mismatch at index " << index << ":\n"
            << "Expected: " << TokenPrinter::print({ token.start, token.length, expected_type, token.flags }, lexer) <<
 "\n"
            << "Actual:   " << TokenPrinter::print(token, lexer);

        auto actual_value = lexer.get_token_value(token);
        auto expected_value = std::string_view(source.data() + tokens->starts[index], tokens->lengths[index]);
        EXPECT_EQ(expected_value, actual_value)
            << "Token value mismatch at index " << index << ":\n"
            << "Expected: '" << expected_value << "'\n"
            << "Actual:   '" << actual_value << "'";
    }

    static void dump_tokens(const TokenList *tokens, const Lexer &lexer)
    {
        std::cout << TokenPrinter::print(*tokens, lexer);
    }

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(LexerTest, NestedGenerics)
{
    constexpr std::string_view source = "var matrix: Array<Array<Vector3<T>>>;";
    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // matrix
    verify_token(tokens, i++, TokenType::COLON, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // Array
    verify_token(tokens, i++, TokenType::LESS, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // Array
    verify_token(tokens, i++, TokenType::LESS, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // Vector3
    verify_token(tokens, i++, TokenType::LESS, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // T
    verify_token(tokens, i++, TokenType::GREATER, source, lexer);
    verify_token(tokens, i++, TokenType::GREATER, source, lexer);
    verify_token(tokens, i++, TokenType::GREATER, source, lexer);
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);
}

TEST_F(LexerTest, GenericConstraints)
{
    constexpr std::string_view source = R"(
        class DataStructure<T: Comparable & Serializable, U: Container<T>>
        {
            var data: U;
        }
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::CLASS, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // DataStructure
    verify_token(tokens, i++, TokenType::LESS, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // T
    verify_token(tokens, i++, TokenType::COLON, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // Comparable
    verify_token(tokens, i++, TokenType::AND, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // Serializable
    verify_token(tokens, i++, TokenType::COMMA, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // U
    verify_token(tokens, i++, TokenType::COLON, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // Container
    verify_token(tokens, i++, TokenType::LESS, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // T
    verify_token(tokens, i++, TokenType::GREATER, source, lexer);
}

TEST_F(LexerTest, ComplexTypeAnnotations)
{
    constexpr std::string_view source = R"(
        @nodiscard
        @pure
        @align(32)
        public function Transform<T, U>(input: Array<T>) -> Result<U, Error>
        {
            return Ok(transform_impl(input));
        }
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::NO_DISCARD_ANNOT, source, lexer);
    verify_token(tokens, i++, TokenType::PURE_ANNOT, source, lexer);
    verify_token(tokens, i++, TokenType::ALIGN_ANNOT, source, lexer);
    verify_token(tokens, i++, TokenType::LEFT_PAREN, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer);
    verify_token(tokens, i++, TokenType::RIGHT_PAREN, source, lexer);
}

TEST_F(LexerTest, ComplexNestedExpressions)
{
    constexpr std::string_view source = R"(
        result = matrix[i * 2 + 1][j - (k * 3)].transform<U>().value;
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // result
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // matrix
    verify_token(tokens, i++, TokenType::LEFT_BRACKET, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // i
    verify_token(tokens, i++, TokenType::STAR, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 2
    verify_token(tokens, i++, TokenType::PLUS, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 1
    verify_token(tokens, i++, TokenType::RIGHT_BRACKET, source, lexer);
}

TEST_F(LexerTest, ComplexStringLiterals)
{
    constexpr std::string_view source = R"(
        var str1 = "String with \"escaped\" quotes";
        var str2 = "Multi-line
                    string with
                    line breaks";
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // str1
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::STR_LITERAL, source, lexer);
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);
}

TEST_F(LexerTest, MethodChainingWithGenerics)
{
    constexpr std::string_view source = R"(
        result = value.map<U>()
                     .filter<V>()
                     .transform<W>()
                     .unwrap_or(default_value);
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // result
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // value
    verify_token(tokens, i++, TokenType::DOT, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // map
    verify_token(tokens, i++, TokenType::LESS, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // U
    verify_token(tokens, i++, TokenType::GREATER, source, lexer);
}

TEST_F(LexerTest, ComplexTemplateInstantiation)
{
    constexpr std::string_view source = R"(
        var data = HashMap<string, vector<result<T, error>>>(
            { capacity = 100, load_factor = 0.75 }
        );
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // data
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // HashMap
    verify_token(tokens, i++, TokenType::LESS, source, lexer);
    verify_token(tokens, i++, TokenType::STRING, source, lexer); // string
    verify_token(tokens, i++, TokenType::COMMA, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // vector
}

TEST_F(LexerTest, OperatorPrecedence)
{
    constexpr std::string_view source = "x = (-a * (b + c)) / (d - e);";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // x
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::LEFT_PAREN, source, lexer);
    verify_token(tokens, i++, TokenType::MINUS, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // a
    verify_token(tokens, i++, TokenType::STAR, source, lexer);
}

TEST_F(LexerTest, NumberLiterals)
{
    constexpr std::string_view source = R"(
        var decimal = 123;
        var hex = 0xFF;
        var binary = 0b1010;
        var float_num = 1.234;
        var scientific = 1.23e-4;
        var big_hex = 0xFFFFFFFFFFFFFFFF;
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // decimal
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 123
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // hex
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 0xFF
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // binary
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 0b1010
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // float_num
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 1.234
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // scientific
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 1.23e-4
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);
}

TEST_F(LexerTest, ErrorRecovery)
{
    constexpr std::string_view source = R"(
        var x = @#$%^;
        var y = 42;
        var z = @;
        var valid = true;
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);

    while (i < tokens->types.size() && tokens->types[static_cast<std::vector<TokenType>::size_type>(i)] !=
           TokenType::SEMICOLON)
        i++;
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer); // ;

    verify_token(tokens, i++, TokenType::VAR, source, lexer);         // var
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer);  // y
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);       // =
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 42
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);   // ;

    verify_token(tokens, i++, TokenType::VAR, source, lexer);        // var
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // z
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);      // =
    i++;                                                           // Skip the @ token
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);  // ;

    verify_token(tokens, i++, TokenType::VAR, source, lexer);        // var
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // valid
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);      // =
    verify_token(tokens, i++, TokenType::TRUE, source, lexer);       // true
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);  // ;

    ASSERT_EQ(tokens->types[(i)], TokenType::END_OF_FILE);
}

TEST_F(LexerTest, ComplexEscapeSequences)
{
    constexpr std::string_view source = R"(
        var str1 = "Escaped \"quotes\" and \n newlines";
        var str2 = "Tabs\t and \r returns";
        var str3 = "\x48\x65\x6C\x6C\x6F"; // Hello in hex
        var str4 = "Mixed \n\t\"escapes\"";
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::STR_LITERAL, source, lexer);
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);
}

TEST_F(LexerTest, CommentRecovery)
{
    constexpr std::string_view source = R"(
        var x = 1; // normal comment
        var y = 2; /* multi
        line comment */ var z = 3;
        // unterminated /*
        var a = 4;
        /* unterminated
        var b = 5;
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // x
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 1
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // y
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 2
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // z
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 3
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer); // a
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer); // 4
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);
}

TEST_F(LexerTest, StringRecovery)
{
    constexpr std::string_view source = R"(
        var str1 = "valid string";
        var str2 = "unterminated string
        var str3 = "string with \"escaped\" quotes";
        var str4 = "string with \n\t escapes";
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::STR_LITERAL, source, lexer);
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    while (tokens->types[i] != TokenType::VAR)
        i++;

    verify_token(tokens, i++, TokenType::VAR, source, lexer);
    verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
    verify_token(tokens, i++, TokenType::STR_LITERAL, source, lexer);
    verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);
}

TEST_F(LexerTest, ComplexIdentifiers)
{
    constexpr std::string_view source = R"(
        var _underscore = 1;
        var camelCase = 2;
        var PascalCase = 3;
        var with_numbers123 = 4;
        var @annotated = 5;
        var $invalid = 6;
        var 123invalid = 7;
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    auto verify_identifier = [&](const std::string_view &expected)
    {
        verify_token(tokens, i++, TokenType::VAR, source, lexer);
        verify_token(tokens, i++, TokenType::IDENTIFIER, source, lexer);
        EXPECT_EQ(lexer.get_token_value({tokens->starts[(i - 1)],
                      tokens->lengths[(i - 1)],
                      tokens->types[(i - 1)],
                      tokens->flags[(i - 1)]}),
                  expected);

        verify_token(tokens, i++, TokenType::EQUAL, source, lexer);
        verify_token(tokens, i++, TokenType::NUM_LITERAL, source, lexer);
        verify_token(tokens, i++, TokenType::SEMICOLON, source, lexer);
    };

    verify_identifier("_underscore");
    verify_identifier("camelCase");
    verify_identifier("PascalCase");
    verify_identifier("with_numbers123");
    verify_identifier("@annotated");
}

TEST_F(LexerTest, Array)
{
    constexpr std::string_view source = R"(
        var arr: [ u8 ] = { 1, 2, 3, 4, 5 };
        var nested: [ [ u8 ] ] = { { 1, 2 }, { 3, 4 }, { 5 } };
    )";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    std::vector<unsigned>::size_type i = 0;

    EXPECT_EQ(tokens->types[i++], TokenType::VAR);
    EXPECT_EQ(tokens->types[i++], TokenType::IDENTIFIER);
    EXPECT_EQ(tokens->types[i++], TokenType::COLON);

    // array type syntax [ u8 ]
    EXPECT_EQ(tokens->types[i++], TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens->types[i++], TokenType::U8);
    EXPECT_EQ(tokens->types[i++], TokenType::RIGHT_BRACKET);

    EXPECT_EQ(tokens->types[i++], TokenType::EQUAL);

    // array literal { 1, 2, 3, 4, 5 }
    EXPECT_EQ(tokens->types[i++], TokenType::LEFT_BRACE);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::COMMA);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::COMMA);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::COMMA);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::COMMA);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokens->types[i++], TokenType::SEMICOLON);

    // second array declaration (nested): var nested: [ [ u8 ] ] = { { 1, 2 }, { 3, 4 }, { 5 } };
    EXPECT_EQ(tokens->types[i++], TokenType::VAR);

    EXPECT_EQ(tokens->types[i++], TokenType::IDENTIFIER);

    EXPECT_EQ(tokens->types[i++], TokenType::COLON);

    // nested array type [ [ u8 ] ]
    EXPECT_EQ(tokens->types[i++], TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens->types[i++], TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens->types[i++], TokenType::U8);
    EXPECT_EQ(tokens->types[i++], TokenType::RIGHT_BRACKET);
    EXPECT_EQ(tokens->types[i++], TokenType::RIGHT_BRACKET);

    EXPECT_EQ(tokens->types[i++], TokenType::EQUAL);

    // nested array literal { { 1, 2 }, { 3, 4 }, { 5 } }
    EXPECT_EQ(tokens->types[i++], TokenType::LEFT_BRACE);

    // first inner array { 1, 2 }
    EXPECT_EQ(tokens->types[i++], TokenType::LEFT_BRACE);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::COMMA);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokens->types[i++], TokenType::COMMA);

    // second inner array { 3, 4 }
    EXPECT_EQ(tokens->types[i++], TokenType::LEFT_BRACE);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::COMMA);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokens->types[i++], TokenType::COMMA);

    EXPECT_EQ(tokens->types[i++], TokenType::LEFT_BRACE);
    EXPECT_EQ(tokens->types[i++], TokenType::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], TokenType::RIGHT_BRACE);

    EXPECT_EQ(tokens->types[i++], TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokens->types[i++], TokenType::SEMICOLON);

    EXPECT_EQ(tokens->types[i++], TokenType::END_OF_FILE);
    EXPECT_EQ(i, tokens->size());
}
