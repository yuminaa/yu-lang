// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#include <iomanip>
#include <gtest/gtest.h>
#include "../../compiler/include/lexer.h"

using namespace yu::compiler;
using namespace yu::lang;

class TokenPrinter
{
    static constexpr auto YELLOW = "\033[33m";
    static constexpr auto RESET = "\033[0m";

    static std::string get_token_name(const token_i type)
    {
        switch (type)
        {
            case token_i::TRUE:
                return "TRUE";
            case token_i::FALSE:
                return "FALSE";
            case token_i::NIL:
                return "NIL";
            case token_i::IMPORT:
                return "IMPORT";
            case token_i::VAR:
                return "VAR";
            case token_i::CONST:
                return "CONST";
            case token_i::FUNCTION:
                return "FUNCTION";
            case token_i::INLINE:
                return "INLINE";
            case token_i::RETURN:
                return "RETURN";
            case token_i::NEW:
                return "NEW";
            case token_i::ENUM:
                return "ENUM";
            case token_i::IF:
                return "IF";
            case token_i::ELSE:
                return "ELSE";
            case token_i::FOR:
                return "FOR";
            case token_i::WHILE:
                return "WHILE";
            case token_i::BREAK:
                return "BREAK";
            case token_i::CONTINUE:
                return "CONTINUE";
            case token_i::SWITCH:
                return "SWITCH";
            case token_i::CASE:
                return "CASE";
            case token_i::DEFAULT:
                return "DEFAULT";
            case token_i::CLASS:
                return "CLASS";
            case token_i::FINAL:
                return "FINAL";
            case token_i::PUBLIC:
                return "PUBLIC";
            case token_i::PRIVATE:
                return "PRIVATE";
            case token_i::PROTECTED:
                return "PROTECTED";
            case token_i::STATIC:
                return "STATIC";
            case token_i::AWAIT:
                return "AWAIT";
            case token_i::ASYNC:
                return "ASYNC";
            case token_i::TRY:
                return "TRY";
            case token_i::CATCH:
                return "CATCH";
            case token_i::FROM:
                return "FROM";
            // Basic types
            case token_i::U8:
                return "U8";
            case token_i::I8:
                return "I8";
            case token_i::U16:
                return "U16";
            case token_i::I16:
                return "I16";
            case token_i::U32:
                return "U32";
            case token_i::I32:
                return "I32";
            case token_i::U64:
                return "U64";
            case token_i::I64:
                return "I64";
            case token_i::F32:
                return "F32";
            case token_i::F64:
                return "F64";
            case token_i::STRING:
                return "STRING";
            case token_i::BOOLEAN:
                return "BOOLEAN";
            case token_i::VOID:
                return "VOID";
            case token_i::PTR:
                return "PTR";
            // Operators
            case token_i::PLUS:
                return "PLUS";
            case token_i::MINUS:
                return "MINUS";
            case token_i::STAR:
                return "STAR";
            case token_i::SLASH:
                return "SLASH";
            case token_i::PERCENT:
                return "PERCENT";
            case token_i::EQUAL:
                return "EQUAL";
            case token_i::BANG:
                return "BANG";
            case token_i::LESS:
                return "LESS";
            case token_i::GREATER:
                return "GREATER";
            case token_i::AND:
                return "AND";
            case token_i::OR:
                return "OR";
            case token_i::XOR:
                return "XOR";
            case token_i::TILDE:
                return "TILDE";
            case token_i::DOT:
                return "DOT";
            // Delimiters
            case token_i::LEFT_PAREN:
                return "LEFT_PAREN";
            case token_i::RIGHT_PAREN:
                return "RIGHT_PAREN";
            case token_i::LEFT_BRACE:
                return "LEFT_BRACE";
            case token_i::RIGHT_BRACE:
                return "RIGHT_BRACE";
            case token_i::LEFT_BRACKET:
                return "LEFT_BRACKET";
            case token_i::RIGHT_BRACKET:
                return "RIGHT_BRACKET";
            case token_i::COMMA:
                return "COMMA";
            case token_i::COLON:
                return "COLON";
            case token_i::SEMICOLON:
                return "SEMICOLON";
            case token_i::QUESTION:
                return "QUESTION";
            case token_i::IDENTIFIER:
                return "IDENTIFIER";
            case token_i::NUM_LITERAL:
                return "NUM_LITERAL";
            case token_i::STR_LITERAL:
                return "STR_LITERAL";
            case token_i::ANNOTATION:
                return "ANNOTATION";
            case token_i::UNKNOWN:
                return "UNKNOWN";
            case token_i::END_OF_FILE:
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

    static std::string print(const token_t &token, const Lexer &lexer)
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
    /**
     * @brief Verifies the properties of a token in a token list.
     *
     * This function performs comprehensive validation of a token at a specified index,
     * checking both its type and value against expected parameters.
     *
     * @param tokens Pointer to the list of tokens to be verified
     * @param index Index of the token to verify within the token list
     * @param expected_type Expected token type from the token_i enumeration
     * @param source Original source code string view used for token extraction
     * @param lexer Lexer instance used for token processing and value retrieval
     *
     * @throws std::runtime_error If the token index is out of range
     *
     * @note Uses Google Test assertion macros (ASSERT_LT, EXPECT_EQ) for validation
     * @note Provides detailed error messages for type and value mismatches
     */
    void verify_token(const TokenList *tokens, auto index, const token_i expected_type,
                      const std::string_view &source, Lexer lexer)
    {
        ASSERT_LT(index, tokens->types.size())
            << "Token index out of range";

        const token_t token = {
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

    /**
     * @brief Prints all tokens from a token list to the console.
     *
     * Utilizes the TokenPrinter to format and output tokens generated by the lexer.
     * This static utility method helps in debugging lexer output by displaying
     * detailed token information.
     *
     * @param tokens Pointer to the list of tokens to be printed
     * @param lexer Reference to the Lexer instance used to generate the tokens
     */
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
    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // matrix
    verify_token(tokens, i++, token_i::COLON, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // Array
    verify_token(tokens, i++, token_i::LESS, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // Array
    verify_token(tokens, i++, token_i::LESS, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // Vector3
    verify_token(tokens, i++, token_i::LESS, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // T
    verify_token(tokens, i++, token_i::GREATER, source, lexer);
    verify_token(tokens, i++, token_i::GREATER, source, lexer);
    verify_token(tokens, i++, token_i::GREATER, source, lexer);
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);
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
    verify_token(tokens, i++, token_i::CLASS, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // DataStructure
    verify_token(tokens, i++, token_i::LESS, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // T
    verify_token(tokens, i++, token_i::COLON, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // Comparable
    verify_token(tokens, i++, token_i::AND, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // Serializable
    verify_token(tokens, i++, token_i::COMMA, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // U
    verify_token(tokens, i++, token_i::COLON, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // Container
    verify_token(tokens, i++, token_i::LESS, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // T
    verify_token(tokens, i++, token_i::GREATER, source, lexer);
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
    verify_token(tokens, i++, token_i::NO_DISCARD_ANNOT, source, lexer);
    verify_token(tokens, i++, token_i::PURE_ANNOT, source, lexer);
    verify_token(tokens, i++, token_i::ALIGN_ANNOT, source, lexer);
    verify_token(tokens, i++, token_i::LEFT_PAREN, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer);
    verify_token(tokens, i++, token_i::RIGHT_PAREN, source, lexer);
}

TEST_F(LexerTest, ComplexNestedExpressions)
{
    constexpr std::string_view source = R"(
        result = matrix[i * 2 + 1][j - (k * 3)].transform<U>().value;
    )";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // result
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // matrix
    verify_token(tokens, i++, token_i::LEFT_BRACKET, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // i
    verify_token(tokens, i++, token_i::STAR, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 2
    verify_token(tokens, i++, token_i::PLUS, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 1
    verify_token(tokens, i++, token_i::RIGHT_BRACKET, source, lexer);
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
    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // str1
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::STR_LITERAL, source, lexer);
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);
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
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // result
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // value
    verify_token(tokens, i++, token_i::DOT, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // map
    verify_token(tokens, i++, token_i::LESS, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // U
    verify_token(tokens, i++, token_i::GREATER, source, lexer);
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
    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // data
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // HashMap
    verify_token(tokens, i++, token_i::LESS, source, lexer);
    verify_token(tokens, i++, token_i::STRING, source, lexer); // string
    verify_token(tokens, i++, token_i::COMMA, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // vector
}

TEST_F(LexerTest, OperatorPrecedence)
{
    constexpr std::string_view source = "x = (-a * (b + c)) / (d - e);";

    Lexer lexer(source);
    const auto tokens = lexer.tokenize();

    size_t i = 0;
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // x
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::LEFT_PAREN, source, lexer);
    verify_token(tokens, i++, token_i::MINUS, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // a
    verify_token(tokens, i++, token_i::STAR, source, lexer);
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
    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // decimal
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 123
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // hex
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 0xFF
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // binary
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 0b1010
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // float_num
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 1.234
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // scientific
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 1.23e-4
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);
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
    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);

    while (i < tokens->types.size() && tokens->types[static_cast<std::vector<token_i>::size_type>(i)] !=
           token_i::SEMICOLON)
        i++;
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer); // ;

    verify_token(tokens, i++, token_i::VAR, source, lexer);         // var
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer);  // y
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);       // =
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 42
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);   // ;

    verify_token(tokens, i++, token_i::VAR, source, lexer);        // var
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // z
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);      // =
    i++;                                                           // Skip the @ token
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);  // ;

    verify_token(tokens, i++, token_i::VAR, source, lexer);        // var
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // valid
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);      // =
    verify_token(tokens, i++, token_i::TRUE, source, lexer);       // true
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);  // ;

    ASSERT_EQ(tokens->types[static_cast<std::vector<token_i>::size_type>(i)], token_i::END_OF_FILE);
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
    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::STR_LITERAL, source, lexer);
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);
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
    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // x
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 1
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // y
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 2
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // z
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 3
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer); // a
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer); // 4
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);
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
    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::STR_LITERAL, source, lexer);
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    while (tokens->types[i] != token_i::VAR)
        i++;

    verify_token(tokens, i++, token_i::VAR, source, lexer);
    verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer);
    verify_token(tokens, i++, token_i::EQUAL, source, lexer);
    verify_token(tokens, i++, token_i::STR_LITERAL, source, lexer);
    verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);
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
        verify_token(tokens, i++, token_i::VAR, source, lexer);
        verify_token(tokens, i++, token_i::IDENTIFIER, source, lexer);
        EXPECT_EQ(lexer.get_token_value({tokens->starts[static_cast<std::vector<unsigned>::size_type>(i - 1)],
                      tokens->lengths[static_cast<std::vector<unsigned>::size_type>(i - 1)],
                      tokens->types[static_cast<std::vector<unsigned>::size_type>(i - 1)],
                      tokens->flags[static_cast<std::vector<unsigned>::size_type>(i - 1)]}),
                  expected);

        verify_token(tokens, i++, token_i::EQUAL, source, lexer);
        verify_token(tokens, i++, token_i::NUM_LITERAL, source, lexer);
        verify_token(tokens, i++, token_i::SEMICOLON, source, lexer);
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

    EXPECT_EQ(tokens->types[i++], token_i::VAR);
    EXPECT_EQ(tokens->types[i++], token_i::IDENTIFIER);
    EXPECT_EQ(tokens->types[i++], token_i::COLON);

    // Array type syntax [ u8 ]
    EXPECT_EQ(tokens->types[i++], token_i::LEFT_BRACKET);
    EXPECT_EQ(tokens->types[i++], token_i::U8);
    EXPECT_EQ(tokens->types[i++], token_i::RIGHT_BRACKET);

    EXPECT_EQ(tokens->types[i++], token_i::EQUAL);

    // Array literal { 1, 2, 3, 4, 5 }
    EXPECT_EQ(tokens->types[i++], token_i::LEFT_BRACE);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::COMMA);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::COMMA);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::COMMA);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::COMMA);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::RIGHT_BRACE);
    EXPECT_EQ(tokens->types[i++], token_i::SEMICOLON);

    // Second array declaration (nested): var nested: [ [ u8 ] ] = { { 1, 2 }, { 3, 4 }, { 5 } };
    EXPECT_EQ(tokens->types[i++], token_i::VAR);

    EXPECT_EQ(tokens->types[i++], token_i::IDENTIFIER);

    EXPECT_EQ(tokens->types[i++], token_i::COLON);

    // Nested array type [ [ u8 ] ]
    EXPECT_EQ(tokens->types[i++], token_i::LEFT_BRACKET);
    EXPECT_EQ(tokens->types[i++], token_i::LEFT_BRACKET);
    EXPECT_EQ(tokens->types[i++], token_i::U8);
    EXPECT_EQ(tokens->types[i++], token_i::RIGHT_BRACKET);
    EXPECT_EQ(tokens->types[i++], token_i::RIGHT_BRACKET);

    EXPECT_EQ(tokens->types[i++], token_i::EQUAL);

    // Nested array literal { { 1, 2 }, { 3, 4 }, { 5 } }
    EXPECT_EQ(tokens->types[i++], token_i::LEFT_BRACE);

    // First inner array { 1, 2 }
    EXPECT_EQ(tokens->types[i++], token_i::LEFT_BRACE);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::COMMA);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::RIGHT_BRACE);
    EXPECT_EQ(tokens->types[i++], token_i::COMMA);

    // Second inner array { 3, 4 }
    EXPECT_EQ(tokens->types[i++], token_i::LEFT_BRACE);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::COMMA);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::RIGHT_BRACE);
    EXPECT_EQ(tokens->types[i++], token_i::COMMA);

    EXPECT_EQ(tokens->types[i++], token_i::LEFT_BRACE);
    EXPECT_EQ(tokens->types[i++], token_i::NUM_LITERAL);
    EXPECT_EQ(tokens->types[i++], token_i::RIGHT_BRACE);

    EXPECT_EQ(tokens->types[i++], token_i::RIGHT_BRACE);
    EXPECT_EQ(tokens->types[i++], token_i::SEMICOLON);

    // Verify we got all tokens and the count is correct
    EXPECT_EQ(tokens->types[i++], token_i::END_OF_FILE);
    EXPECT_EQ(i, tokens->size());
}
