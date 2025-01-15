#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "../../../common/styles.h"

namespace yu::compiler
{
    enum class ParseErrorFlags : uint8_t
    {
        NONE = 0,
        UNEXPECTED_TOKEN = 1 << 0,
        TYPE_MISMATCH = 1 << 1,
        INVALID_SYNTAX = 1 << 2,
        UNRESOLVED_SYMBOL = 1 << 3,
        UNIMPLEMENTED_FEATURE = 1 << 4
    };

    enum class ErrorSeverity
    {
        WARNING,
        ERROR,
        FATAL
    };

    template<typename SourceT = const char *>
    struct BasicParseError
    {
        ParseErrorFlags flags;
        ErrorSeverity severity;
        std::string message;
        std::string suggestion;
        SourceT source;
        const char *filename;
        uint32_t line;
        uint32_t column;
        std::string source_line;
        std::string error_pointer;
    };

    // Deduction guide
    template<typename SourceT>
    BasicParseError(ParseErrorFlags, ErrorSeverity, std::string, std::string,
                    SourceT, const char *, uint32_t, uint32_t, std::string, std::string)
        -> BasicParseError<SourceT>;

    using ParseError = BasicParseError<const char *>;

    // Error code mapping trait
    template<typename T>
    struct ErrorCodeMapping
    {
        static std::string get_code(T flags) = delete;
    };

    template<>
    struct ErrorCodeMapping<ParseErrorFlags>
    {
        static std::string get_code(ParseErrorFlags flags)
        {
            switch (flags)
            {
                case ParseErrorFlags::UNEXPECTED_TOKEN:
                    return "E0001";
                case ParseErrorFlags::TYPE_MISMATCH:
                    return "E0308";
                case ParseErrorFlags::INVALID_SYNTAX:
                    return "E0002";
                case ParseErrorFlags::UNRESOLVED_SYMBOL:
                    return "E0433";
                default:
                    return "E0000";
            }
        }
    };

    // Customization point for error formatting
    template<typename ErrorT>
    struct ErrorFormatter
    {
        static std::string format_message(const ErrorT &error);

        static std::string format_location(const ErrorT &error);

        static std::string format_pointer(const ErrorT &error);

        static std::string format_suggestion(const ErrorT &error);
    };

    template<typename ErrorT = ParseError,
        typename FormatterT = ErrorFormatter<ErrorT>,
        typename StorageT = std::vector<ErrorT> >
    class BasicErrorReporter
    {
    public:
        using error_type = ErrorT;
        using formatter_type = FormatterT;
        using storage_type = StorageT;

        template<typename... Args>
        BasicErrorReporter(Args &&... args) : storage_args(std::forward<Args>(args)...) {}

        void report_error(const ErrorT &error)
        {
            if (error.severity >= ErrorSeverity::WARNING)
            {
                const auto &color = error.severity == ErrorSeverity::WARNING
                                        ? styles::color::YELLOW
                                        : styles::color::RED;

                // Main error message
                std::cerr << color
                        << FormatterT::format_message(error)
                        << styles::color::RESET << "\n";

                // Location
                std::cerr << FormatterT::format_location(error) << "\n";

                // Source line and pointer if available
                if (!error.source_line.empty())
                {
                    std::cerr << FormatterT::format_pointer(error) << "\n";
                }

                // Suggestion if available
                if (!error.suggestion.empty())
                {
                    std::cerr << FormatterT::format_suggestion(error) << "\n";
                }

                // Error code
                std::cerr << styles::color::BLUE << "   = " << styles::color::RESET
                        << "note: error[" << ErrorCodeMapping<ParseErrorFlags>::get_code(error.flags) << "]\n";
            }

            if (error.severity >= ErrorSeverity::ERROR)
            {
                errors.push_back(error);
            }
            else
            {
                warnings.push_back(error);
            }
        }

        void report_warning(const ErrorT &error)
        {
            warnings.push_back(error);
        }

        const storage_type &get_errors() const
        {
            return errors;
        }

        const storage_type &get_warnings() const
        {
            return warnings;
        }

        void clear_errors()
        {
            errors.clear();
            warnings.clear();
        }

    private:
        storage_type errors;
        storage_type warnings;
        StorageT storage_args;
    };

    template<typename ErrorT>
    struct ErrorFormatter
    {
        static std::string format_message(const ErrorT &error)
        {
            return (error.severity == ErrorSeverity::WARNING ? "warning" : "error")
                   + std::string(": ") + error.message;
        }

        static std::string format_location(const ErrorT &error)
        {
            return "  " + styles::color::BLUE + "-->" + styles::color::RESET + " "
                   + error.filename + ":" + std::to_string(error.line)
                   + ":" + std::to_string(error.column);
        }

        static std::string format_pointer(const ErrorT &error)
        {
            return styles::color::BLUE + "   |" + styles::color::RESET + "\n"
                   + styles::color::BLUE + std::setw(3) + std::to_string(error.line)
                   + "|" + styles::color::RESET + " " + error.source_line + "\n"
                   + styles::color::BLUE + "   |" + styles::color::RESET + " "
                   + error.error_pointer;
        }

        static std::string format_suggestion(const ErrorT &error)
        {
            return styles::color::BLUE + "   |" + styles::color::RESET + "\n"
                   + styles::color::BLUE + "   = " + styles::color::RESET
                   + styles::color::GREEN + "help" + styles::color::RESET
                   + ": " + error.suggestion;
        }
    };

    using DefaultErrorReporter = BasicErrorReporter<ParseError>;
}
