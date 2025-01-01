#include "compiler/builder.h"

#include <cassert>
#include <sstream>

namespace yu::compiler
{
    IRBuilder::IRBuilder(const size_t initial_capacity)
    {
        instructions.ops.reserve(initial_capacity);
        instructions.destinations.reserve(initial_capacity);
        instructions.operand_count.reserve(initial_capacity);
        instructions.operands.reserve(initial_capacity * 2);
        immediate_ops.reserve(initial_capacity);
        current_function = UINT32_MAX;
    }

    uint32_t IRBuilder::create_function(const std::string_view name,
                                        std::span<const uint32_t> param_types,
                                        const uint32_t return_type)
    {
        assert(!sealed && "Cannot create function after sealing");

        const uint32_t func_index = functions.bb_start_indices.size();

        functions.bb_start_indices.push_back(basic_blocks.start_indices.size());
        functions.bb_counts.push_back(0);
        functions.param_counts.push_back(param_types.size());
        functions.param_types.insert(functions.param_types.end(),
                                     param_types.begin(), param_types.end());
        functions.return_types.push_back(return_type);
        functions.names.push_back(std::string(name));

        current_function = func_index;
        return func_index;
    }

    uint32_t IRBuilder::create_bb()
    {
        assert(!sealed && "Cannot create basic block after sealing");
        assert(current_function != UINT32_MAX && "Must create function before basic block");

        const auto bb_index = static_cast<uint32_t>(basic_blocks.start_indices.size());

        basic_blocks.start_indices.push_back(static_cast<uint32_t>(instructions.ops.size()));
        basic_blocks.instruction_counts.push_back(0);
        basic_blocks.successor_counts.push_back(0);
        basic_blocks.predecessor_counts.push_back(0);

        functions.bb_counts[current_function]++;

        current_bb = bb_index;
        return bb_index;
    }

    uint32_t IRBuilder::add_instruction(const lang::ir_op_t op, std::span<const uint32_t> operands)
    {
        assert(!sealed && "Cannot add instruction after sealing");
        assert(current_function != UINT32_MAX && "Must create function before adding instructions");
        assert(current_bb != UINT32_MAX && "Must create basic block before adding instructions");

        const uint32_t dest = instructions.destinations.size();

        instructions.ops.push_back(op);
        instructions.destinations.push_back(dest);
        instructions.operand_count.push_back(operands.size());
        instructions.operands.insert(instructions.operands.end(),
                                     operands.begin(), operands.end());
        immediate_ops.push_back(std::vector(operands.begin(), operands.end()));
        instructions.bb_indices.push_back(current_bb);

        basic_blocks.instruction_counts[current_bb]++;
        return dest;
    }

    uint32_t IRBuilder::add_conversion(lang::ir_op_t op,
                                       std::span<const uint32_t> operands,
                                       lang::ir_op_t from_type,
                                       lang::ir_op_t to_type)
    {
        const uint32_t inst = add_instruction(op, operands);
        instructions.operand_types.push_back(static_cast<uint32_t>(from_type));
        instructions.operand_types.push_back(static_cast<uint32_t>(to_type));
        return inst;
    }

    uint32_t IRBuilder::add_memory_op(lang::ir_op_t op,
                                      std::span<const uint32_t> operands,
                                      lang::ir_op_t value_type)
    {
        const uint32_t inst = add_instruction(op, operands);
        instructions.operand_types.push_back(static_cast<uint32_t>(value_type));
        return inst;
    }

    const std::vector<std::vector<uint32_t> > &IRBuilder::get_immediate_ops() const
    {
        return immediate_ops;
    }

    void IRBuilder::seal()
    {
        sealed = true;
    }

    bool IRBuilder::is_sealed() const
    {
        return sealed;
    }

    const lang::ir_instruction_blocks_t &IRBuilder::get_instructions() const
    {
        return instructions;
    }

    const lang::bb_list &IRBuilder::get_bbs() const
    {
        return basic_blocks;
    }

    const lang::function_meta_t &IRBuilder::get_functions() const
    {
        return functions;
    }

    void IRBuilder::set_current_bb(const uint32_t bb_index)
    {
        assert(!sealed && "Cannot set basic block after sealing");
        current_bb = bb_index;
    }

    std::string IRBuilder::dump() const
    {
        std::stringstream ss;
        auto op_to_string = [](lang::ir_op_t op) -> std::string
        {
            switch (op)
            {
                case lang::ir_op_t::TYPE_VOID:
                    return "void";
                case lang::ir_op_t::TYPE_BOOL:
                    return "bool";
                case lang::ir_op_t::TYPE_I8:
                    return "i8";
                case lang::ir_op_t::TYPE_U8:
                    return "u8";
                case lang::ir_op_t::TYPE_I16:
                    return "i16";
                case lang::ir_op_t::TYPE_U16:
                    return "u16";
                case lang::ir_op_t::TYPE_I32:
                    return "i32";
                case lang::ir_op_t::TYPE_U32:
                    return "u32";
                case lang::ir_op_t::TYPE_I64:
                    return "i64";
                case lang::ir_op_t::TYPE_U64:
                    return "u64";
                case lang::ir_op_t::TYPE_F32:
                    return "f32";
                case lang::ir_op_t::TYPE_F64:
                    return "f64";
                case lang::ir_op_t::TYPE_PTR:
                    return "ptr";
                case lang::ir_op_t::OP_ADD:
                    return "add";
                case lang::ir_op_t::OP_SUB:
                    return "sub";
                case lang::ir_op_t::OP_MUL:
                    return "mul";
                case lang::ir_op_t::OP_DIV:
                    return "div";
                case lang::ir_op_t::OP_MOD:
                    return "mod";
                case lang::ir_op_t::OP_LT:
                    return "lt";
                case lang::ir_op_t::OP_LE:
                    return "le";
                case lang::ir_op_t::OP_GT:
                    return "gt";
                case lang::ir_op_t::OP_GE:
                    return "ge";
                case lang::ir_op_t::MEM_LOAD:
                    return "load";
                case lang::ir_op_t::MEM_STORE:
                    return "store";
                case lang::ir_op_t::CONV_ZEXT:
                    return "zext";
                case lang::ir_op_t::FLOW_RETURN:
                    return "ret";
                case lang::ir_op_t::FLOW_BRANCH:
                    return "br";
                case lang::ir_op_t::FLOW_JUMP:
                    return "jump";
                case lang::ir_op_t::SSA_PHI:
                    return "phi";
                default:
                    return std::string("OP_") + std::to_string(static_cast<int>(op));
            }
        };

        for (size_t func_idx = 0; func_idx < functions.bb_start_indices.size(); func_idx++)
        {
            ss << "func ";
            ss << "(";
            uint32_t param_start = 0;
            if (func_idx > 0)
            {
                param_start = functions.param_counts[func_idx - 1];
            }
            for (uint32_t i = 0; i < functions.param_counts[func_idx]; i++)
            {
                if (i > 0)
                    ss << ", ";
                ss << "%p" << i << ": " << op_to_string(static_cast<lang::ir_op_t>(
                    functions.param_types[param_start + i]));
            }
            ss << ") -> " << op_to_string(static_cast<lang::ir_op_t>(functions.return_types[func_idx]))
                    << ":\n";

            const uint32_t bb_start = functions.bb_start_indices[func_idx];
            const uint32_t bb_end = bb_start + functions.bb_counts[func_idx];
            for (uint32_t bb = bb_start; bb < bb_end; bb++)
            {
                ss << "bb" << (bb - bb_start) << ":\n";

                // Changed section: iterate through all instructions and check bb_indices
                for (size_t inst_idx = 0; inst_idx < instructions.ops.size(); inst_idx++)
                {
                    if (instructions.bb_indices[inst_idx] != bb)
                    {
                        continue;
                    }

                    ss << "    ";
                    const auto op = instructions.ops[inst_idx];
                    const auto dest = instructions.destinations[inst_idx];

                    uint32_t operand_start = 0;
                    for (uint32_t j = 0; j < inst_idx; j++)
                    {
                        operand_start += instructions.operand_count[j];
                    }

                    if (op >= lang::ir_op_t::TYPE_VOID && op <= lang::ir_op_t::TYPE_PTR)
                    {
                        ss << "%" << dest << " = " << op_to_string(op);
                        if (!immediate_ops[inst_idx].empty())
                        {
                            ss << " " << immediate_ops[inst_idx][0];
                        }
                    }
                    else if (op == lang::ir_op_t::FLOW_BRANCH)
                    {
                        ss << "br %";
                        ss << instructions.operands[operand_start];
                        ss << ", bb" << instructions.operands[operand_start + 1] - bb_start;
                        ss << ", bb" << instructions.operands[operand_start + 2] - bb_start;
                    }
                    else if (op == lang::ir_op_t::FLOW_JUMP)
                    {
                        ss << "jump bb" << instructions.operands[operand_start] - bb_start;
                    }
                    else if (op == lang::ir_op_t::FLOW_RETURN)
                    {
                        ss << "ret";
                        if (instructions.operand_count[inst_idx] > 0)
                        {
                            ss << " %" << instructions.operands[operand_start];
                        }
                    }
                    else if (op == lang::ir_op_t::SSA_PHI)
                    {
                        ss << "%" << dest << " = phi i32";
                        for (uint32_t j = 0; j < instructions.operand_count[inst_idx]; j += 2)
                        {
                            ss << (j == 0 ? " [" : ", ");
                            ss << "%" << instructions.operands[operand_start + j];
                            ss << ", bb" << instructions.operands[operand_start + j + 1] - bb_start;
                        }
                        ss << "]";
                    }
                    else if (op == lang::ir_op_t::MEM_STORE)
                    {
                        ss << "store %" << instructions.operands[operand_start];
                        ss << ", %" << instructions.operands[operand_start + 1];
                    }
                    else if (op == lang::ir_op_t::MEM_LOAD)
                    {
                        ss << "%" << dest << " = load %" << instructions.operands[operand_start];
                    }
                    else if (op == lang::ir_op_t::CONV_ZEXT)
                    {
                        ss << "%" << dest << " = zext i32 %" << instructions.operands[operand_start] << " to i64";
                    }
                    else
                    {
                        ss << "%" << dest << " = " << op_to_string(op) << " %";
                        for (uint32_t j = 0; j < instructions.operand_count[inst_idx]; j++)
                        {
                            if (j > 0)
                                ss << ", %";
                            ss << instructions.operands[operand_start + j];
                        }
                    }

                    ss << "\n";
                }
                ss << "\n";
            }
        }

        return ss.str();
    }
}
