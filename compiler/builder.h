// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#pragma once

#include <cstdint>
#include <span>

#include "ir.h"

namespace yu::compiler
{
    class IRBuilder
    {
    public:
        explicit IRBuilder(size_t initial_capacity = 1024);

        uint32_t create_function(std::string_view name,
                                 std::span<const uint32_t> param_types,
                                 uint32_t return_type);

        uint32_t create_bb();

        uint32_t add_instruction(lang::ir_op_t op,
                                 std::span<const uint32_t> operands);

        uint32_t add_conversion(lang::ir_op_t op,
                                std::span<const uint32_t> operands,
                                lang::ir_op_t from_type,
                                lang::ir_op_t to_type);

        uint32_t add_memory_op(lang::ir_op_t op,
                               std::span<const uint32_t> operands,
                               lang::ir_op_t value_type);

        void seal();

        bool is_sealed() const;

        const lang::ir_instruction_blocks_t &get_instructions() const;

        const lang::bb_list &get_bbs() const;

        const lang::function_meta_t &get_functions() const;

        void set_current_bb(uint32_t bb_index);

        std::string dump() const;

        const std::vector<std::vector<uint32_t> > &get_immediate_ops() const;

    private:
        bool sealed = false;
        uint32_t current_function = UINT32_MAX;
        uint32_t current_bb = UINT32_MAX;

        lang::ir_instruction_blocks_t instructions;
        std::vector<std::vector<uint32_t> > immediate_ops;
        std::vector<uint32_t> type_info;
        lang::bb_list basic_blocks;
        lang::function_meta_t functions;
    };
}
