# Yu Intermediate Representation (UIR) Specification

## Table of Contents
1. [Overview](#overview)
2. [Basic Structure](#basic-structure)
3. [Type System](#type-system)
4. [Instructions](#instructions)
5. [SSA Properties](#ssa-properties)
6. [Memory Model](#memory-model)
7. [Control Flow](#control-flow)
8. [Function Calls](#function-calls)
9. [Example Programs](#example-programs)

## Overview

UIR is a Static Single Assignment (SSA) based intermediate representation designed for the Yu programming language which 
serves as a bridge between Yu's high-level constructs and machine code generation.

### Design Goals

- Maintain SSA form for optimal analysis and optimization
- Simple, line-based syntax for easy parsing
- Direct mapping to common machine instructions
- Explicit typing and memory operations
- Support for Yu's type system and memory model

### Syntax Conventions

- Basic blocks are labeled with `bb` prefix
- Virtual registers are prefixed with `%`
- Types are explicitly stated where needed
- One instruction per line
- Comments start with `#`

## Basic Structure

### Function Definition
```
func function_name(%param1: i32, %param2: ptr) -> i32:
    bb0:
        # instructions
    bb1:
        # more instructions
```

### Basic Blocks
```
bb0:                         # Entry block
    %1 = instruction
    jump bb1

bb1:                         # Subsequent block
    %2 = phi i32 [%1, bb0]   # Phi node for SSA
```

## Type System

### Primitive Types
```
void     # No type/return
i8       # 8-bit signed integer
u8       # 8-bit unsigned integer
i16      # 16-bit signed integer
u16      # 16-bit unsigned integer
i32      # 32-bit signed integer
u32      # 32-bit unsigned integer
i64      # 64-bit signed integer
u64      # 64-bit unsigned integer
f32      # 32-bit floating point
f64      # 64-bit floating point
ptr      # Generic pointer type
```

### Compound Types
```
ptr<T>           # Typed pointer
array<T, N>      # Fixed-size array
vector<T, N>     # SIMD vector type
struct<{...}>    # Structure type
```

## Instructions

### Memory Operations
```
# Allocation
%1 = alloc i32                   # Stack allocation
%2 = alloc array<i32, 10>        # Array allocation

# Load/Store
%3 = load i32 [%ptr]             # Load from pointer
store i32 %val, [%ptr]           # Store to pointer
%4 = load i32 [%ptr + 8]         # Load with offset
store i32 %val, [%ptr + 8]       # Store with offset

# Memory Operations with SSA
%mem1 = store i32 %val, [%ptr]   # Returns new memory state
%5 = load i32 [%ptr], %mem1      # Uses memory state
```

### Arithmetic Operations
```
# Integer Arithmetic
%1 = add <type> %a, %b          # Addition
%2 = sub <type> %a, %b          # Subtraction
%3 = mul <type> %a, %b          # Multiplication
%4 = div <type> %a, %b          # Division
%5 = mod <type> %a, %b          # Modulo
%6 = neg <type> %a              # Negation

# Floating Point Arithmetic
%7 = fadd <type> %a, %b         # Float addition
%8 = fsub <type> %a, %b         # Float subtraction
%9 = fmul <type> %a, %b         # Float multiplication
%10 = fdiv <type> %a, %b        # Float division
```

### Bitwise Operations
```
%1 = and <type> %a, %b          # Bitwise AND
%2 = or <type> %a, %b           # Bitwise OR
%3 = xor <type> %a, %b          # Bitwise XOR
%4 = not <type> %a              # Bitwise NOT
%5 = shl <type> %a, %b          # Shift left
%6 = shr <type> %a, %b          # Shift right
%7 = sar <type> %a, %b          # Arithmetic shift right
```

### Comparison Operations
```
# Integer Comparisons
%1 = cmp.eq <type> %a, %b       # Equal
%2 = cmp.ne <type> %a, %b       # Not equal
%3 = cmp.lt <type> %a, %b       # Less than
%4 = cmp.le <type> %a, %b       # Less or equal
%5 = cmp.gt <type> %a, %b       # Greater than
%6 = cmp.ge <type> %a, %b       # Greater or equal

# Floating Point Comparisons
%7 = fcmp.eq <type> %a, %b      # Float equal
%8 = fcmp.ne <type> %a, %b      # Float not equal
%9 = fcmp.lt <type> %a, %b      # Float less than
# ... etc for other float comparisons
```

### Conversion Operations
```
%1 = zext <from> %a to <to>     # Zero extension
%2 = sext <from> %a to <to>     # Sign extension
%3 = trunc <from> %a to <to>    # Truncation
%4 = bitcast <from> %a to <to>  # Bitwise reinterpretation
%5 = inttoptr <from> %a to ptr  # Integer to pointer
%6 = ptrtoint ptr %a to <to>    # Pointer to integer
```

## SSA Properties

### Phi Nodes
```
# Basic phi node
%1 = phi i32 [%2, bb0], [%3, bb1]   # Value depends on predecessor block

# Multiple phi nodes
bb2:
    %2 = phi i32 [%x, bb0], [%y, bb1]
    %3 = phi ptr [%p, bb0], [%q, bb1]
```

### SSA Helpers
```
%1 = undef <type>               # Undefined value
%2 = unreachable               # Unreachable code marker
```

## Memory Model

### Memory States
```
# Explicit memory versioning
%mem0 = func_entry             # Initial memory state
%mem1 = store i32 %1, [%ptr]   # New memory state after store
%2 = load i32 [%ptr], %mem1    # Load using specific memory state
```

### Memory Barriers
```
%mem1 = barrier %mem0          # Full memory barrier
%mem2 = barrier.acq %mem1      # Acquire barrier
%mem3 = barrier.rel %mem2      # Release barrier
```

## Control Flow

### Basic Control Flow
```
jump bb1                       # Unconditional jump
branch %cond, bb1, bb2        # Conditional branch
switch %val, bb_default, [     # Switch statement
    0: bb0,
    1: bb1,
    2: bb2
]
```

### Exception Handling

```
%1 = invoke func, bb_normal, bb_exception
landingpad:
    %2 = catch                # Exception handling
    %3 = cleanup             # Cleanup code
```

## Function Calls

### Direct Calls
```
# Void function call
call void @func()

# Function call with return value
%1 = call i32 @func(i32 %arg1, ptr %arg2)
```

### Indirect Calls

```
# Function pointer call
%1 = call ptr %func_ptr(i32 %arg1)
```

### Tail Calls

```
tail call void @func()        # Tail call optimization hint
```

## Example Programs

### Simple Function

```
func add_and_multiply(a: i32, b: i32) -> i32:
    bb0:
        %1 = add i32 %a, %b
        %2 = mul i32 %1, 2
        ret i32 %2

```

### Control Flow Example

```
func max(a: i32, b: i32) -> i32:
    bb0:
        %1 = cmp.gt i32 %a, %b
        branch %1, bb1, bb2

    bb1:
        ret i32 %a

    bb2:
        ret i32 %b
```

### Memory and Phi Example

```
func conditional_increment(ptr: ptr<i32>, cond: i32) -> i32:
    bb0:
        %1 = load i32 [%ptr], %mem0
        branch %cond, bb1, bb2

    bb1:
        %2 = add i32 %1, 1
        %mem1 = store i32 %2, [%ptr]
        jump bb2

    bb2:
        %3 = phi i32 [%1, bb0], [%2, bb1]
        ret i32 %3
```

### Memory Ordering Models

```
# Memory orderings
unordered     # No ordering guarantees
monotonic     # Simple atomic ordering
acquire       # Acquire ordering
release       # Release ordering
acq_rel       # Acquire-Release ordering
seq_cst       # Sequential consistency
```

### Atomic Operations

```
# Atomic load with ordering
%1 = atomic.load i32 [%ptr], ordering

# Atomic store with ordering
atomic.store i32 %val, [%ptr], ordering

# Compare and swap
%1 = cmpxchg [%ptr], %expected, %new, success_ordering, failure_ordering

# Atomic RMW operations
%1 = atomic.add i32 [%ptr], %val, ordering
%2 = atomic.sub i32 [%ptr], %val, ordering
%3 = atomic.and i32 [%ptr], %val, ordering
%4 = atomic.or  i32 [%ptr], %val, ordering
%5 = atomic.xor i32 [%ptr], %val, ordering
```

## Intrinsics

### Hardware Intrinsics

```
# SIMD operations
%1 = intrinsic.simd.add <4 x f32> %a, %b
%2 = intrinsic.simd.mul <4 x f32> %a, %b

# CPU specific instructions
%3 = intrinsic.x86.rdtsc
%4 = intrinsic.x86.pause
%5 = intrinsic.x86.clflush [%ptr]

# Memory fence instructions
%6 = intrinsic.x86.mfence
%7 = intrinsic.x86.lfence
%8 = intrinsic.x86.sfence
```

### Memory Intrinsics

```
# Allocation intrinsics
%1 = intrinsic.alloc size: i64, align: i64
%2 = intrinsic.realloc ptr: ptr, size: i64, align: i64
intrinsic.free ptr: ptr

# Memory operation intrinsics
%3 = intrinsic.memcpy dst: ptr, src: ptr, size: i64, align: i64, volatile: bool
%4 = intrinsic.memmove dst: ptr, src: ptr, size: i64, align: i64, volatile: bool
%5 = intrinsic.memset dst: ptr, val: i8, size: i64, align: i64, volatile: bool
```

### Synchronization Intrinsics

```
# Thread operations
%1 = sync.thread.create func: ptr, arg: ptr
%2 = sync.thread.join thread: ptr

# Mutex operations
%3 = sync.mutex.create
sync.mutex.lock %3, ordering: acquire
sync.mutex.unlock %3, ordering: release

# Condition variables
%4 = sync.condvar.create
sync.condvar.wait %4, %mutex
sync.condvar.signal %4
sync.condvar.broadcast %4
```