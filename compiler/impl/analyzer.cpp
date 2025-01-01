#include "../analyzer.h"
#include <cassert>
#include <iostream>
#include <queue>
#include <unordered_set>

namespace yu::compiler
{
    bool IRAnalyzer::is_integer_type(const lang::ir_op_t type)
    {
        switch (type)
        {
            case lang::ir_op_t::TYPE_I8:
            case lang::ir_op_t::TYPE_U8:
            case lang::ir_op_t::TYPE_I16:
            case lang::ir_op_t::TYPE_U16:
            case lang::ir_op_t::TYPE_I32:
            case lang::ir_op_t::TYPE_U32:
            case lang::ir_op_t::TYPE_I64:
            case lang::ir_op_t::TYPE_U64:
                return true;
            default:
                return false;
        }
    }

    bool IRAnalyzer::is_float_type(const lang::ir_op_t type)
    {
        switch (type)
        {
            case lang::ir_op_t::TYPE_F32:
            case lang::ir_op_t::TYPE_F64:
                return true;
            default:
                return false;
        }
    }

    bool IRAnalyzer::is_pointer_type(const lang::ir_op_t type)
    {
        return type == lang::ir_op_t::TYPE_PTR;
    }

    lang::ir_op_t IRAnalyzer::get_pointee_type()
    {
        // TBA: A more sophisticated version will be needed
        return lang::ir_op_t::TYPE_VOID;
    }

    uint32_t IRAnalyzer::get_type_size(lang::ir_op_t type)
    {
        switch (type)
        {
            case lang::ir_op_t::TYPE_BOOL:
            case lang::ir_op_t::TYPE_I8:
            case lang::ir_op_t::TYPE_U8:
                return 1;
            case lang::ir_op_t::TYPE_I16:
            case lang::ir_op_t::TYPE_U16:
                return 2;
            case lang::ir_op_t::TYPE_I32:
            case lang::ir_op_t::TYPE_U32:
            case lang::ir_op_t::TYPE_F32:
                return 4;
            case lang::ir_op_t::TYPE_I64:
            case lang::ir_op_t::TYPE_U64:
            case lang::ir_op_t::TYPE_F64:
                return 8;
            case lang::ir_op_t::TYPE_PTR:
                return 8;
            default:
                return 0;
        }
    }

    bool IRAnalyzer::validate_type() const
    {
        const auto &[ops, destinations, operand_count, operands, operand_types, bb_indices, mem_ops, immediate_ops] =
                builder.get_instructions();
        const auto &imm_ops = builder.get_immediate_ops();
        const auto &[bb_start_indices, bb_counts, param_counts, param_types, return_types, names] = builder.
                get_functions();
        const auto &bbs = builder.get_bbs();

        struct TypeInfo
        {
            lang::ir_op_t type = lang::ir_op_t::TYPE_VOID;
            bool is_defined = false;
            std::vector<uint32_t> known_values;
        };
        std::vector<TypeInfo> reg_types(destinations.size());

        for (size_t func = 0; func < bb_counts.size(); ++func)
        {
            const uint32_t start_bb = bb_start_indices[func];
            const uint32_t bb_count = bb_counts[func];
            const uint32_t param_count = param_counts[func];
            for (uint32_t i = 0; i < param_count; ++i)
            {
                reg_types[start_bb + i].type = static_cast<lang::ir_op_t>(param_types[i]);
                reg_types[start_bb + i].is_defined = true;
            }

            for (uint32_t bb = 0; bb < bb_count; ++bb)
            {
                const uint32_t curr_bb = start_bb + bb;
                const uint32_t start_inst = bbs.start_indices[curr_bb];
                const uint32_t inst_count = bbs.instruction_counts[curr_bb];
                for (uint32_t i = 0; i < inst_count; ++i)
                {
                    const uint32_t inst_idx = start_inst + i;
                    const lang::ir_op_t op = ops[inst_idx];
                    const uint32_t dest = destinations[inst_idx];
                    if (op >= lang::ir_op_t::TYPE_VOID && op <= lang::ir_op_t::TYPE_PTR)
                    {
                        reg_types[dest].type = op;
                        reg_types[dest].is_defined = true;
                        if (!imm_ops[inst_idx].empty())
                        {
                            reg_types[dest].known_values = imm_ops[inst_idx];
                        }
                        continue;
                    }

                    std::vector<lang::ir_op_t> operand_type_list;
                    std::vector<uint32_t> operand_indices;

                    // Collect operand types
                    for (uint32_t op_idx = 0; op_idx < operand_count[inst_idx]; ++op_idx)
                    {
                        const uint32_t operand = operands[op_idx];
                        if (operand >= reg_types.size())
                        {
                            std::cout << "Invalid operand index\n";
                            return false;
                        }

                        if (!reg_types[operand].is_defined)
                        {
                            std::cout << "Use of undefined register\n";
                            return false;
                        }

                        operand_type_list.push_back(reg_types[operand].type);
                        operand_indices.push_back(operand);
                    }

                    switch (op)
                    {
                        case lang::ir_op_t::OP_ADD:
                        case lang::ir_op_t::OP_SUB:
                        case lang::ir_op_t::OP_MUL:
                        case lang::ir_op_t::OP_DIV:
                        case lang::ir_op_t::OP_MOD:
                        {
                            if (operand_count[inst_idx] != 2)
                            {
                                std::cout << "Invalid operand count for arithmetic operation\n";
                                return false;
                            }

                            if (!is_integer_type(operand_type_list[0]) || !is_integer_type(operand_type_list[1]))
                            {
                                std::cout << "Integer type required for arithmetic operation\n";
                                return false;
                            }

                            // Result type is the wider of the two operand types
                            reg_types[dest].type = std::max(operand_type_list[0], operand_type_list[1]);
                            reg_types[dest].is_defined = true;
                            break;
                        }

                        case lang::ir_op_t::OP_FADD:
                        case lang::ir_op_t::OP_FSUB:
                        case lang::ir_op_t::OP_FMUL:
                        case lang::ir_op_t::OP_FDIV:
                        {
                            if (operand_count[inst_idx] != 2)
                            {
                                std::cout << "Invalid operand count for floating-point operation\n";
                                return false;
                            }

                            if (!is_float_type(operand_type_list[0]) || !is_float_type(operand_type_list[1]))
                            {
                                std::cout << "Float type required for floating-point operation\n";
                                return false;
                            }

                            reg_types[dest].type = std::max(operand_type_list[0], operand_type_list[1]);
                            reg_types[dest].is_defined = true;
                            break;
                        }

                        case lang::ir_op_t::OP_AND:
                        case lang::ir_op_t::OP_OR:
                        case lang::ir_op_t::OP_XOR:
                        {
                            if (operand_count[inst_idx] != 2)
                            {
                                std::cout << "Invalid operand count for bitwise operation\n";
                                return false;
                            }

                            if (!is_integer_type(operand_type_list[0]) || !is_integer_type(operand_type_list[1]))
                            {
                                std::cout << "Integer type required for bitwise operation\n";
                                return false;
                            }

                            reg_types[dest].type = std::max(operand_type_list[0], operand_type_list[1]);
                            reg_types[dest].is_defined = true;
                            break;
                        }

                        case lang::ir_op_t::OP_NOT:
                        {
                            if (operand_count[inst_idx] != 1)
                            {
                                std::cout << "Invalid operand count for NOT operation\n";
                                return false;
                            }

                            if (!is_integer_type(operand_type_list[0]))
                            {
                                std::cout << "Integer type required for NOT operation\n";
                                return false;
                            }

                            reg_types[dest].type = operand_type_list[0];
                            reg_types[dest].is_defined = true;
                            break;
                        }

                        case lang::ir_op_t::FLOW_BRANCH:
                        {
                            if (operand_count[inst_idx] != 3)
                            {
                                std::cout << "Branch requires condition and two target blocks\n";
                                return false;
                            }

                            if (operand_type_list[0] != lang::ir_op_t::TYPE_BOOL)
                            {
                                std::cout << "Branch condition must be boolean\n";
                                return false;
                            }
                            if (!is_integer_type(operand_type_list[1]) || !is_integer_type(operand_type_list[2]))
                            {
                                std::cout << "Branch targets must be integer block indices\n";
                                return false;
                            }
                            break;
                        }

                        case lang::ir_op_t::FLOW_RETURN:
                        {
                            if (operand_count[inst_idx] > 0)
                            {
                                if (operand_type_list[0] != static_cast<lang::ir_op_t>(return_types[func]))
                                {
                                    std::cout << "Return type mismatch\n";
                                    return false;
                                }
                            }
                            else if (return_types[func] != static_cast<uint32_t>(lang::ir_op_t::TYPE_VOID))
                            {
                                std::cout << "Missing return value\n";
                                return false;
                            }
                            break;
                        }

                        case lang::ir_op_t::SSA_PHI:
                        {
                            if (operand_count[inst_idx] < 2 || operand_count[inst_idx] % 2 != 0)
                            {
                                std::cout << "Invalid PHI node operand count\n";
                                return false;
                            }
                            const lang::ir_op_t phi_type = operand_type_list[0];
                            for (uint32_t op1 = 0; op1 < operand_count[inst_idx]; op1 += 2)
                            {
                                if (operand_type_list[op1] != phi_type)
                                {
                                    std::cout << "Inconsistent types in PHI node\n";
                                    return false;
                                }
                                // Block indices must be integers
                                if (!is_integer_type(reg_types[operands[op1 + 1]].type))
                                {
                                    std::cout << "PHI node block indices must be integers\n";
                                    return false;
                                }
                            }

                            reg_types[dest].type = phi_type;
                            reg_types[dest].is_defined = true;
                            break;
                        }

                        case lang::ir_op_t::MEM_LOAD:
                        {
                            if (operand_count[inst_idx] != 1)
                            {
                                std::cout << "Load requires exactly one pointer operand\n";
                                return false;
                            }
                            if (!is_pointer_type(operand_type_list[0]))
                            {
                                std::cout << "Load requires pointer operand\n";
                                return false;
                            }
                            reg_types[dest].type = get_pointee_type();
                            reg_types[dest].is_defined = true;
                            break;
                        }

                        case lang::ir_op_t::MEM_STORE:
                        {
                            if (operand_count[inst_idx] != 2)
                            {
                                std::cout << "Store requires pointer and value operands\n";
                                return false;
                            }
                            if (!is_pointer_type(operand_type_list[0]))
                            {
                                std::cout << "Store first operand must be pointer\n";
                                return false;
                            }
                            break;
                        }

                        case lang::ir_op_t::FLOW_JUMP:
                        {
                            if (operand_count[inst_idx] != 1)
                            {
                                std::cout << "Jump requires exactly one target block\n";
                                return false;
                            }
                            if (!is_integer_type(operand_type_list[0]))
                            {
                                std::cout << "Jump target must be block index\n";
                                return false;
                            }
                            break;
                        }

                        case lang::ir_op_t::CONV_SEXT:
                        case lang::ir_op_t::CONV_ZEXT:
                        {
                            if (operand_count[inst_idx] != 1)
                            {
                                std::cout << "Extension requires one operand\n";
                                return false;
                            }
                            if (!is_integer_type(operand_type_list[0]))
                            {
                                std::cout << "Extension requires integer operand\n";
                                return false;
                            }
                            if (get_type_size(reg_types[dest].type) <= get_type_size(operand_type_list[0]))
                            {
                                std::cout << "Extension target type must be wider\n";
                                return false;
                            }
                            reg_types[dest].is_defined = true;
                            break;
                        }

                        case lang::ir_op_t::CONV_TRUNC:
                        {
                            if (operand_count[inst_idx] != 1)
                            {
                                std::cout << "Truncation requires one operand\n";
                                return false;
                            }
                            if (!is_integer_type(operand_type_list[0]))
                            {
                                std::cout << "Truncation requires integer operand\n";
                                return false;
                            }
                            if (get_type_size(reg_types[dest].type) >= get_type_size(operand_type_list[0]))
                            {
                                std::cout << "Truncation target type must be narrower\n";
                                return false;
                            }
                            reg_types[dest].is_defined = true;
                            break;
                        }

                        default:
                        {
                            std::cout << "Unknown operation type: " << static_cast<int>(op) << "\n";
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    bool IRAnalyzer::validate_control_flow() const
    {
        const auto &[start_indices, instruction_counts, successor_counts, successors,
            predecessor_counts, predecessors] = builder.get_bbs();
        const auto &[bb_start_indices, bb_counts, param_counts, param_types,
            return_types, names] = builder.get_functions();

        for (size_t func = 0; func < bb_counts.size(); ++func)
        {
            uint32_t start_bb = bb_start_indices[func];
            uint32_t bb_count = bb_counts[func];
            if (predecessor_counts[start_bb] != 0)
            {
                std::cout << "Entry block cannot have predecessors\n";
                return false;
            }

            std::vector reachable(bb_count, false);
            std::queue<uint32_t> worklist;

            worklist.push(start_bb);
            reachable[0] = true;

            for (uint32_t bb = 0; bb < bb_count; ++bb)
            {
                uint32_t curr_bb = start_bb + bb;
                for (uint32_t succ = 0; succ < successor_counts[curr_bb]; ++succ)
                {
                    uint32_t target = successors[succ];
                    if (target < start_bb || target >= start_bb + bb_count)
                    {
                        std::cout << "Invalid branch target block index\n";
                        return false;
                    }
                }

                while (!worklist.empty())
                {
                    uint32_t bbb = worklist.front();
                    worklist.pop();

                    for (uint32_t succ = 0; succ < successor_counts[bbb]; ++succ)
                    {
                        uint32_t succ_idx = successors[succ];
                        if (!reachable[succ_idx - start_bb])
                        {
                            reachable[succ_idx - start_bb] = true;
                            worklist.push(succ_idx);
                        }
                    }
                }
            }
            for (uint32_t bb = 0; bb < bb_count; ++bb)
            {
                if (!reachable[bb])
                {
                    std::cout << "Unreachable block detected: " << (start_bb + bb) << "\n";
                    return false;
                }
            }

            for (uint32_t bb = 0; bb < bb_count; ++bb)
            {
                uint32_t curr_bb = start_bb + bb;
                if (successor_counts[curr_bb] == 0)
                {
                    const auto [ops, destinations, operand_count, operands, operand_types, bb_indices, mem_ops,
                        immediate_ops] = builder.get_instructions();
                    if (uint32_t last_inst = start_indices[curr_bb] + instruction_counts[curr_bb] - 1;
                        ops[last_inst] != lang::ir_op_t::FLOW_RETURN &&
                        ops[last_inst] != lang::ir_op_t::FLOW_UNREACHABLE)
                    {
                        std::cout << "Block must end with return or unreachable\n";
                        return false;
                    }
                }
            }
        }

        return true;
    }

    IRAnalyzer::DominatorInfo IRAnalyzer::analyze_dominator(uint32_t function_index) const
    {
        return DominatorInfo();
    }

    IRAnalyzer::LivenessInfo IRAnalyzer::analyze_liveness_info(uint32_t function_index) const
    {
        return LivenessInfo();
    }

    IRAnalyzer::LoopInfo IRAnalyzer::analyze_loop(uint32_t function_index) const {
        return LoopInfo();
    }

    bool IRAnalyzer::validate_ssa() const
    {
        const auto &[ops, destinations, operand_count, operands, operand_types, bb_indices, mem_ops, immediate_ops] =
                builder.get_instructions();
        const auto &imm_ops = builder.get_immediate_ops();
        const auto &[bb_start_indices, bb_counts, param_counts, param_types, return_types, names] = builder.
                get_functions();

        std::vector<bool> defined(destinations.size(), false);
        for (size_t i = 0; i < param_types.size(); ++i)
        {
            defined[i] = true;
        }

        for (size_t i = 0; i < ops.size(); ++i)
        {
            if (ops[i] >= lang::ir_op_t::TYPE_VOID && ops[i] <= lang::ir_op_t::TYPE_PTR)
            {
                const uint32_t dest = destinations[i];
                if (defined[dest])
                {
                    std::cout << "SSA violation: Multiple definitions of " << dest << "\n";
                    return false;
                }
                defined[dest] = true;
            }
            else if (!imm_ops[i].empty())
            {
                const uint32_t dest = destinations[i];
                if (defined[dest])
                {
                    std::cout << "SSA violation: Multiple definitions of " << dest << "\n";
                    return false;
                }
                defined[dest] = true;
            }
        }

        for (size_t func = 0; func < bb_counts.size(); ++func)
        {
            const uint32_t start_bb = bb_start_indices[func];
            const uint32_t bb_count = bb_counts[func];
            std::vector<std::unordered_set<uint32_t> > bb_defs(bb_count);
            for (size_t i = 0; i < ops.size(); ++i)
            {
                const uint32_t curr_bb = bb_indices[i];
                if (curr_bb < start_bb || curr_bb >= start_bb + bb_count)
                    continue;

                if (ops[i] >= lang::ir_op_t::TYPE_VOID && ops[i] <= lang::ir_op_t::TYPE_PTR)
                    continue;
                if (!imm_ops[i].empty())
                    continue;

                for (uint32_t op = 0; op < operand_count[i]; ++op)
                {
                    const uint32_t operand = operands[op];
                    if (ops[i] == lang::ir_op_t::SSA_PHI)
                    {
                        if (op % 2 == 1)
                            continue;

                        const uint32_t pred_block = operands[op + 1];
                        if (!defined[operand] && !bb_defs[pred_block - start_bb].contains(operand))
                        {
                            std::cout << "SSA violation: PHI node uses undefined value " << operand
                                    << " from block " << pred_block << "\n";
                            return false;
                        }
                        continue;
                    }

                    if (operand >= defined.size() || !defined[operand])
                    {
                        std::cout << "SSA violation: Use of undefined value " << operand
                                << " in instruction " << i << "\n";
                        return false;
                    }
                }

                const uint32_t dest = destinations[i];
                if (defined[dest])
                {
                    std::cout << "SSA violation: Multiple definitions of " << dest
                            << " in instruction " << i << "\n";
                    return false;
                }

                defined[dest] = true;
                bb_defs[curr_bb - start_bb].insert(dest);
            }
        }

        return true;
    }

    void IRAnalyzer::compute_dominance_frontier(const uint32_t function_index) const
    {
        const auto &[start_indices, instruction_counts, successor_counts, successors, predecessor_counts, predecessors]
                = builder.get_bbs();
        const auto &[bb_start_indices, bb_counts, param_counts, param_types, return_types, names] = builder.
                get_functions();

        const uint32_t start_bb = bb_start_indices[function_index];
        const uint32_t bb_count = bb_counts[function_index];

        auto [idom_indices, dominator_counts, dominators] = analyze_dominator(function_index);
        std::vector<std::unordered_set<uint32_t> > dominance_frontiers(bb_count);

        for (uint32_t bb = 0; bb < bb_count; ++bb)
        {
            const uint32_t curr_bb = start_bb + bb;
            for (uint32_t pred_idx = 0; pred_idx < predecessor_counts[curr_bb]; ++pred_idx)
            {
                uint32_t runner = predecessors[pred_idx];
                while (runner != idom_indices[bb] && runner != UINT32_MAX)
                {
                    dominance_frontiers[runner - start_bb].insert(curr_bb);
                    runner = idom_indices[runner - start_bb];
                }
            }
        }
    }

    IRAnalyzer::DefUseInfo IRAnalyzer::build_def_use_chains() const
    {
        const auto &[ops, destinations, operand_count, operands, operand_types, bb_indices, mem_ops, immediate_ops] =
                builder.get_instructions();
        const auto &[start_indices, instruction_counts, successor_counts, successors, predecessor_counts, predecessors]
                = builder.get_bbs();
        const auto &[bb_start_indices, bb_counts, param_counts, param_types, return_types, names] = builder.
                get_functions();

        DefUseInfo du_info;
        for (size_t i = 0; i < ops.size(); ++i)
        {
            if (const uint32_t dest = destinations[i];
                dest != UINT32_MAX)
            {
                du_info.def[dest] = i;
                du_info.def_block[dest] = bb_indices[i];
            }
        }

        uint32_t param_offset = 0;
        for (size_t func = 0; func < bb_counts.size(); ++func)
        {
            const uint32_t param_count = param_counts[func];
            for (uint32_t p = 0; p < param_count; ++p)
            {
                du_info.def[param_offset + p] = UINT32_MAX;
                du_info.def_block[param_offset + p] = bb_start_indices[func];
            }
            param_offset += param_count;
        }

        for (size_t i = 0; i < ops.size(); ++i)
        {
            if (ops[i] >= lang::ir_op_t::TYPE_VOID && ops[i] <= lang::ir_op_t::TYPE_PTR)
                continue;

            if (ops[i] == lang::ir_op_t::SSA_PHI)
            {
                for (uint32_t op = 0; op < operand_count[i]; op += 2)
                {
                    const uint32_t value = operands[op];
                    const uint32_t pred_block = operands[op + 1];

                    du_info.uses[value].push_back(i);
                    du_info.use_blocks[value].push_back(pred_block);
                }
            }
            else
            {
                for (uint32_t op = 0; op < operand_count[i]; ++op)
                {
                    const uint32_t used_var = operands[op];
                    du_info.uses[used_var].push_back(i);
                    du_info.use_blocks[used_var].push_back(bb_indices[i]);
                }
            }
        }

        for (const auto &[var, uses]: du_info.uses)
        {
            if (du_info.def.find(var) == du_info.def.end() &&
                var >= param_offset)
            {
                std::cout << "Warning: Found use of undefined variable " << var << "\n";
            }
        }

        return du_info;
    }
}
