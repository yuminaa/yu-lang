// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#pragma once
#include <vector>

#include "builder.h"


namespace yu::compiler
{
    class IRAnalyzer
    {
    public:
        explicit IRAnalyzer(IRBuilder& builder): builder(builder) {}

        struct DominatorInfo
        {
            std::vector<uint32_t> idom_indices;        // Immediate dominators
            std::vector<uint32_t> dominator_counts;    // Number of dominators per block
            std::vector<uint32_t> dominators;          // Flat array of dominators
        };

        struct LivenessInfo
        {
            std::vector<uint32_t> def_points;
            std::vector<uint32_t> use_points;
            std::vector<uint32_t> live_in_counts;
            std::vector<uint32_t> live_in;
            std::vector<uint32_t> live_out_counts;
            std::vector<uint32_t> live_out;
        };

        struct LoopInfo
        {
            std::vector<uint32_t> header_indices;
            std::vector<uint32_t> loop_depths;
            std::vector<uint32_t> block_counts;
            std::vector<uint32_t> blocks;
        };

        DominatorInfo analyze_dominator(uint32_t function_index) const;
        LivenessInfo analyze_liveness_info(uint32_t function_index) const;
        LoopInfo analyze_loop(uint32_t function_index) const;

        bool validate_ssa() const;
        bool validate_type() const;
        bool validate_control_flow() const;

    private:

        struct DefUseInfo
        {
            std::unordered_map<uint32_t, uint32_t> def;  // variable -> defining instruction
            std::unordered_map<uint32_t, std::vector<uint32_t>> uses;  // variable -> using instructions
            std::unordered_map<uint32_t, uint32_t> def_block;  // variable -> defining block
            std::unordered_map<uint32_t, std::vector<uint32_t>> use_blocks;  // variable -> using blocks
        };
        IRBuilder& builder;

        void compute_dominance_frontier(uint32_t bb_index) const;
        DefUseInfo build_def_use_chains() const;

        static bool is_integer_type(lang::ir_op_t type);

        static bool is_float_type(lang::ir_op_t type);

        static bool is_pointer_type(lang::ir_op_t type);

        static lang::ir_op_t get_pointee_type();

        static uint32_t get_type_size(lang::ir_op_t type);
    };
}
