// This file is part of the Yu programming language and is licensed under MIT License;
// See LICENSE.txt for details

#pragma once

#include <cstdint>
#include <vector>

namespace yu::lang
{
    enum class ir_op_t : uint8_t
    {
        TYPE_VOID,
        TYPE_BOOL,
        TYPE_I8,
        TYPE_U8,
        TYPE_I16,
        TYPE_U16,
        TYPE_I32,
        TYPE_U32,
        TYPE_I64,
        TYPE_U64,
        TYPE_F32,
        TYPE_F64,
        TYPE_PTR,
        TYPE_ARRAY,
        TYPE_VECTOR,
        TYPE_STRUCT,

        MEM_ALLOC,
        MEM_LOAD,
        MEM_STORE,
        MEM_ATOMIC_LOAD,
        MEM_ATOMIC_STORE,
        MEM_ATOMIC_ADD,
        MEM_ATOMIC_SUB,
        MEM_ATOMIC_AND,
        MEM_ATOMIC_OR,
        MEM_ATOMIC_XOR,
        MEM_CMPXCHG,
        MEM_MEMCPY,
        MEM_MEMMOVE,
        MEM_MEMSET,
        MEM_FREE,

        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_NEG,

        OP_FADD,
        OP_FSUB,
        OP_FMUL,
        OP_FDIV,

        // Bitwise
        OP_AND,
        OP_OR,
        OP_XOR,
        OP_NOT,
        OP_SHL,
        OP_SHR,
        OP_SAR,

        OP_EQ,
        OP_NE,
        OP_LT,
        OP_LE,
        OP_GT,
        OP_GE,
        OP_FEQ,
        OP_FNE,
        OP_FLT,
        OP_FLE,
        OP_FGT,
        OP_FGE,

        CONV_ZEXT,
        CONV_SEXT,
        CONV_TRUNC,
        CONV_BITCAST,
        CONV_INTTOPTR,
        CONV_PTRTOINT,
        CONV_FPTOINT,
        CONV_INTTOFP,
        CONV_FPEXT,
        CONV_FPTRUNC,

        FLOW_JUMP,
        FLOW_BRANCH,
        FLOW_SWITCH,
        FLOW_RETURN,
        FLOW_INVOKE,
        FLOW_LANDINGPAD,
        FLOW_UNREACHABLE,

        MO_UNORDERED,
        MO_MONOTONIC,
        MO_ACQUIRE,
        MO_RELEASE,
        MO_ACQ_REL,
        MO_SEQ_CST,

        BARRIER_FULL,
        BARRIER_ACQUIRE,
        BARRIER_RELEASE,

        INTRINSIC_SIMD_ADD,
        INTRINSIC_SIMD_MUL,
        INTRINSIC_SIMD_DIV,
        INTRINSIC_SIMD_SUB,
        INTRINSIC_X86_RDTSC,
        INTRINSIC_X86_PAUSE,
        INTRINSIC_X86_CLFLUSH,
        INTRINSIC_X86_MFENCE,
        INTRINSIC_X86_LFENCE,
        INTRINSIC_X86_SFENCE,

        SYNC_MUTEX_CREATE,
        SYNC_MUTEX_LOCK,
        SYNC_MUTEX_UNLOCK,
        SYNC_CONDVAR_CREATE,
        SYNC_CONDVAR_WAIT,
        SYNC_CONDVAR_SIGNAL,
        SYNC_CONDVAR_BROADCAST,

        SSA_PHI,
        SSA_UNDEF,
        SSA_UNREACHABLE,

        CALL_DIRECT,
        CALL_INDIRECT,
        CALL_TAIL
    };

    struct ir_instruction_blocks_t
    {
        std::vector<ir_op_t> ops;            // Operation types
        std::vector<uint32_t> destinations;  // Destination registers
        std::vector<uint32_t> operand_count; // Number of operands per instruction
        std::vector<uint32_t> operands;      // Flat array of operands
        std::vector<uint32_t> operand_types; // Types for each operand
        std::vector<uint32_t> bb_indices;    // Which basic block this instruction belongs to

        struct mem_ops_t
        {
            std::vector<uint32_t> instruction_indices; // Index back to main instruction
            std::vector<uint32_t> base_regs;
            std::vector<int32_t> offsets;
            std::vector<uint32_t> index_regs;
            std::vector<uint8_t> scales;
        } mem_ops;

        struct immediate_ops_t
        {
            std::vector<uint32_t> instruction_indices;
            std::vector<int64_t> values;
        } immediate_ops;
    };

    struct bb_list // basic block list
    {
        std::vector<uint32_t> start_indices;      // Start index in instruction list
        std::vector<uint32_t> instruction_counts; // Number of instructions in block
        std::vector<uint32_t> successor_counts;   // Number of successor blocks
        std::vector<uint32_t> successors;         // Flat array of successor block indices
        std::vector<uint32_t> predecessor_counts; // Number of predecessor blocks
        std::vector<uint32_t> predecessors;       // Flat array of predecessor block indices
    };

    struct function_meta_t
    {
        std::vector<uint32_t> bb_start_indices; // Index into bb_list
        std::vector<uint32_t> bb_counts;        // Number of basic blocks per function
        std::vector<uint32_t> param_counts;     // Number of parameters per function
        std::vector<uint32_t> param_types;      // Flat array of parameter types
        std::vector<uint32_t> return_types;     // Return type for each function
        std::vector<std::string_view> names;    // Function names (separate storage for strings)
    };

    struct register_usage_t
    {
        std::vector<uint32_t> def_points;          // Where each register is defined
        std::vector<uint32_t> last_uses;           // Last use points for registers
        std::vector<uint32_t> interference_counts; // Number of interfering registers
        std::vector<uint32_t> interferences;       // Flat array of interfering register pairs
    };
}
