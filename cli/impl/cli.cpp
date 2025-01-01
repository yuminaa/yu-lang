#include <iomanip>
#include <iostream>
#include <span>
#include <vector>
#include "compiler/analyzer.h"
#include "compiler/builder.h"

using namespace yu;

void print_test_result(const char *test_name, bool result)
{
    std::cout << std::left << std::setw(40) << test_name
            << ": " << (result ? "PASS" : "FAIL") << std::endl;
}

void test_arithmetic_operations(compiler::IRBuilder &builder)
{
    std::vector<uint32_t> empty_params;
    builder.create_function("arithmetic_test",
                            std::span<const uint32_t>(empty_params),
                            static_cast<uint32_t>(lang::ir_op_t::TYPE_I32));
    builder.create_bb();

    std::vector<uint32_t> const10_val = { 10 };
    std::vector<uint32_t> const5_val = { 5 };
    std::vector<uint32_t> const2_val = { 2 };

    const uint32_t const10 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(const10_val));
    const uint32_t const5 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(const5_val));
    const uint32_t const2 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(const2_val));

    std::cout << "Const10 instruction: " << const10 << std::endl;
    std::cout << "Const5 instruction: " << const5 << std::endl;
    std::cout << "Const2 instruction: " << const2 << std::endl;

    std::vector add_ops = { const10, const5 };
    builder.add_instruction(lang::ir_op_t::OP_ADD, std::span<const uint32_t>(add_ops));

    std::vector sub_ops = { const10, const5 };
    builder.add_instruction(lang::ir_op_t::OP_SUB, std::span<const uint32_t>(sub_ops));

    std::vector mul_ops = { const10, const2 };
    builder.add_instruction(lang::ir_op_t::OP_MUL, std::span<const uint32_t>(mul_ops));

    std::vector div_ops = { const10, const2 };
    const uint32_t div_result = builder.add_instruction(lang::ir_op_t::OP_DIV, std::span<const uint32_t>(div_ops));

    std::vector return_val = { div_result };
    builder.add_instruction(lang::ir_op_t::FLOW_RETURN, std::span<const uint32_t>(return_val));
}

void test_control_flow(compiler::IRBuilder &builder)
{
    std::vector<uint32_t> empty_params;
    builder.create_function("control_flow_test",
                            std::span<const uint32_t>(empty_params),
                            static_cast<uint32_t>(lang::ir_op_t::TYPE_I32));

    // Entry block - contains comparison and branch
    builder.create_bb();
    std::vector<uint32_t> const5_val = { 5 };
    std::vector<uint32_t> const10_val = { 10 };
    const uint32_t const5 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(const5_val));
    const uint32_t const10 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(const10_val));

    std::vector cmp_ops = { const5, const10 };
    const uint32_t cond = builder.add_instruction(lang::ir_op_t::OP_LT, std::span<const uint32_t>(cmp_ops));

    // Create other blocks
    const uint32_t then_bb = builder.create_bb();  // bb1
    const uint32_t else_bb = builder.create_bb();  // bb2
    const uint32_t merge_bb = builder.create_bb(); // bb3

    // Finish entry block with branch
    std::vector branch_ops = { cond, then_bb, else_bb };
    builder.add_instruction(lang::ir_op_t::FLOW_BRANCH, std::span<const uint32_t>(branch_ops));

    // Then block (bb1)
    builder.set_current_bb(then_bb);
    std::vector<uint32_t> const42_val = { 42 };
    const uint32_t const42 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(const42_val));
    std::vector jump_to_merge = { merge_bb };
    builder.add_instruction(lang::ir_op_t::FLOW_JUMP, std::span<const uint32_t>(jump_to_merge));

    // Else block (bb2)
    builder.set_current_bb(else_bb);
    std::vector<uint32_t> const24_val = { 24 };
    const uint32_t const24 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(const24_val));
    builder.add_instruction(lang::ir_op_t::FLOW_JUMP, std::span<const uint32_t>(jump_to_merge));

    // Merge block (bb3)
    builder.set_current_bb(merge_bb);
    std::vector phi_ops = { const42, then_bb, const24, else_bb };
    const uint32_t result = builder.add_instruction(lang::ir_op_t::SSA_PHI, std::span<const uint32_t>(phi_ops));

    std::vector return_val = { result };
    builder.add_instruction(lang::ir_op_t::FLOW_RETURN, std::span<const uint32_t>(return_val));
}

void test_type_conversion(compiler::IRBuilder &builder)
{
    std::vector<uint32_t> empty_params;
    builder.create_function("type_conversion_test",
                          std::span<const uint32_t>(empty_params),
                          static_cast<uint32_t>(lang::ir_op_t::TYPE_I64));
    builder.create_bb();

    std::vector<uint32_t> const42_val = { 42 };
    const uint32_t const42 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(const42_val));

    std::vector zext_ops = { const42 };
    const uint32_t zext_result = builder.add_conversion(
        lang::ir_op_t::CONV_ZEXT,
        std::span<const uint32_t>(zext_ops),
        lang::ir_op_t::TYPE_I32,
        lang::ir_op_t::TYPE_I64
    );

    std::vector return_val = { zext_result };
    builder.add_instruction(lang::ir_op_t::FLOW_RETURN, std::span<const uint32_t>(return_val));
}

void test_memory_operations(compiler::IRBuilder &builder)
{
    std::vector<uint32_t> empty_params;
    builder.create_function("memory_test",
                          std::span<const uint32_t>(empty_params),
                          static_cast<uint32_t>(lang::ir_op_t::TYPE_I32));
    builder.create_bb();

    std::vector<uint32_t> ptr_val = { 0x1000 };
    const uint32_t ptr_const = builder.add_instruction(lang::ir_op_t::TYPE_PTR, std::span<const uint32_t>(ptr_val));

    std::vector<uint32_t> val42_val = { 42 };
    const uint32_t val42 = builder.add_instruction(lang::ir_op_t::TYPE_I32, std::span<const uint32_t>(val42_val));

    std::vector store_ops = { ptr_const, val42 };
    builder.add_memory_op(lang::ir_op_t::MEM_STORE, std::span<const uint32_t>(store_ops), lang::ir_op_t::TYPE_I32);

    std::vector load_ops = { ptr_const };
    uint32_t loaded_val = builder.add_memory_op(lang::ir_op_t::MEM_LOAD, std::span<const uint32_t>(load_ops), lang::ir_op_t::TYPE_I32);

    std::vector return_val = { loaded_val };
    builder.add_instruction(lang::ir_op_t::FLOW_RETURN, std::span<const uint32_t>(return_val));
}

int main()
{
    {
        compiler::IRBuilder builder(1000);
        test_arithmetic_operations(builder);
        builder.seal();

        const compiler::IRAnalyzer analyzer(builder);
        const bool ssa_valid = analyzer.validate_ssa();
        const bool type_valid = analyzer.validate_type();
        const bool flow_valid = analyzer.validate_control_flow();
        print_test_result("Control Flow SSA", ssa_valid);
        print_test_result("Control Flow Type", type_valid);
        print_test_result("Control Flow Flow", flow_valid);

        std::cout << builder.dump() << "\n";
    }

    {
        compiler::IRBuilder builder(1000);
        test_control_flow(builder);
        builder.seal();

        const compiler::IRAnalyzer analyzer(builder);
        const bool ssa_valid = analyzer.validate_ssa();
        const bool type_valid = analyzer.validate_type();
        const bool flow_valid = analyzer.validate_control_flow();
        print_test_result("Control Flow SSA", ssa_valid);
        print_test_result("Control Flow Type", type_valid);
        print_test_result("Control Flow Flow", flow_valid);

        std::cout << builder.dump() << "\n";
    }

    {
        compiler::IRBuilder builder(1000);
        test_type_conversion(builder);
        builder.seal();

        const compiler::IRAnalyzer analyzer(builder);
        const bool ssa_valid = analyzer.validate_ssa();
        const bool type_valid = analyzer.validate_type();
        const bool flow_valid = analyzer.validate_control_flow();
        print_test_result("Control Flow SSA", ssa_valid);
        print_test_result("Control Flow Type", type_valid);
        print_test_result("Control Flow Flow", flow_valid);

        std::cout << builder.dump() << "\n";
    }

    {
        compiler::IRBuilder builder(1000);
        test_memory_operations(builder);
        builder.seal();

        const compiler::IRAnalyzer analyzer(builder);
        const bool ssa_valid = analyzer.validate_ssa();
        const bool type_valid = analyzer.validate_type();
        const bool flow_valid = analyzer.validate_control_flow();
        print_test_result("Control Flow SSA", ssa_valid);
        print_test_result("Control Flow Type", type_valid);
        print_test_result("Control Flow Flow", flow_valid);

        std::cout << builder.dump() << "\n";
    }

    return 0;
}
