# Yu Programming Language Specification

## Table of Contents

1. [Introduction](#introduction)
2. [Lexical Structure](#lexical-structure)
3. [Types](#type-system)
4. [Variables](#variables)
5. [Operators](#operators)
6. [Control Structures](#control-structures)
7. [Functions](#functions)
8. [Classes](#classes)
9. [Memory Management](#memory-management)
10. [Error Handling](#error-handling)
11. [Modules](#modules)
12. [Standard Library](#standard-library)

## Introduction

Yu is an explicit, statically-typed, compiled programming language designed for systems programming. It emphasizes:

* Explicit over implicit behavior
* Strong type safety through smart pointers
* Zero-cost abstractions
* Performance-oriented design
* Simple yet powerful type system

## Lexical Structure

### Naming Conventions

All identifiers in Yu follow explicit naming conventions:

* Types: `PascalCase` (e.g., `Vector3`, `HashMap`)
* Functions: `PascalCase` (e.g., `Calculate`, `Transform`)
* Variables: `snake_case` (e.g., `counter`, `total_sum`)
* Constants: `SCREAMING_SNAKE_CASE` (e.g., `MAX_SIZE`, `PI`)

### Comments

```yu
// Single-line comment

/* Multi-line
   comment */

/* Nested /* comments */ 
   are supported */
```

### Keywords

```yu
true     false    null     import   var      const    function
inline   return   enum     if       else     for      while
break    continue switch   case     default  class    final
public   private  protect  static   await    async    try
catch    from     as       operator new      delete   override
```

### Basic Types

```yu
u8      // Unsigned 8-bit integer
i8      // Signed 8-bit integer
u16     // Unsigned 16-bit integer
i16     // Signed 16-bit integer
u32     // Unsigned 32-bit integer
i32     // Signed 32-bit integer
u64     // Unsigned 64-bit integer
i64     // Signed 64-bit integer
f32     // 32-bit floating point
f64     // 64-bit floating point
string  // UTF-8 string
boolean // true/false
void    // No value
```

## Type System

### Type Safety

Yu enforces strong type safety through:

- Static typing
- Smart pointers
- Explicit error handling
- Null safety with optional types

### Type Declarations

Explicit type declarations are required for function parameters and class members:

```yu
class Vector3
{
    public var x: f32;
    public var y: f32;
    public var z: f32;
    
    public operator new() -> Vector3
    {
        return Vector3
        {
            x = 0.0,
            y = 0.0,
            z = 0.0
        };
    }
}
```

### Generic Types

Generic type parameters must be explicitly specified:

```yu
class Optional<T>
{
    private var value: T;
    private var has_value: boolean = false;
    
    public function GetValue() -> Result<T, string>
    {
        if (!this.has_value)
        {
            return Error("No value present");
        }
        return Ok(this.value);
    }
}
```

### Type Inference

Type inference follows Luau rules while maintaining explicit typing when needed:

```yu
// Explicit (preferred for complex types)
var counter: i32 = 0;
var map: HashMap<string, i32> = new HashMap();

// Inferred (acceptable for obvious types)
var name = "John";
var numbers = [1, 2, 3];
```

## Variables

### Declaration

```yu
var x = 42;         // Type inference
var y: i32 = 42;    // Explicit type
const PI = 3.14;    // Constant
```

### Attributes

```yu
@align(16) var aligned: i32;
@packed class Compact;
@volatile var flags: u32;
@lazy var expensive = Compute();
@nodiscard function Critical() -> Result<void, Error>;
@pure function Pure(x: i32) -> i32;
@tailrec function Factorial(n: i32, acc: i32) -> i32;
```

## Operators

### Arithmetic Operators

```yu
+    // Addition
-    // Subtraction
*    // Multiplication
/    // Division
%    // Modulo
++   // Increment
--   // Decrement
```

### Comparison Operators

```yu
==   // Equal
!=   // Not equal
<    // Less than
>    // Greater than
<=   // Less or equal
>=   // Greater or equal
```

### Logical Operators

```yu
&&   // AND
||   // OR
!    // NOT
```

### Bitwise Operators

```yu
&    // AND
|    // OR
^    // XOR
~    // NOT
<<   // Left shift
>>   // Right shift
```

### Assignment Operators

```yu
=    // Assignment
+=   // Add and assign
-=   // Subtract and assign
*=   // Multiply and assign
/=   // Divide and assign
%=   // Modulo and assign
&=   // Bitwise AND and assign
|=   // Bitwise OR and assign
^=   // Bitwise XOR and assign
<<=  // Left shift and assign
>>=  // Right shift and assign
```

## Control Structures

### Conditional Statements

```yu
if (condition)
{
    // then branch
} 
else if (other) 
{
    // else if branch
} 
else 
{
    // else branch
}
```

### Loops

```yu
while (condition)
{
    if (done) 
        break;
    if (skip) 
        continue;
}

for (var i = 0; i < 10; i++) 
{
    // loop body
}
```

### Switch Statement

```yu
switch (value) 
{
    case 1:
        // code
        break;
    case 2:
        // code
        break;
    default:
        // default code
        break;
}
```

## Functions

### Function Declarations

Functions require explicit return types and parameter types:

```yu
public function Calculate(x: f64, y: f64) -> f64
{
    return x * y + 1.0;
}

async function FetchData() -> Result<string, Error>
{
    // Async operation
    return Ok("data");
}

inline function FastCalc(x: i32) -> i32
{
    return x * 2;
}
```

### Anonymous Functions

Anonymous functions follow the same explicit typing rules:

```yu
var transform: (x: i32) -> i32 = function(x: i32) -> i32
{
    return x * 2;
};
```

### Generic Functions

Generic functions require explicit type parameters:

```yu
public function Convert<TFrom, TTo>(input: TFrom) -> TTo
{
    return Transform<TFrom, TTo>(input);
}
```

## Classes

### Class Declaration

Classes must explicitly declare their members and access modifiers:

```yu
public class BinaryTree<T>
{
    private var value: T;
    private var left: Ptr<BinaryTree<T>>;
    private var right: Ptr<BinaryTree<T>>;
    
    public operator new(value: T) -> BinaryTree<T>
    {
        this.value = value;
        this.left = null;
        this.right = null;
    }

    public operator delete() -> void
    {
        // Cleanup
    }
}
```

### Inheritance

Base classes must be explicitly specified:

```yu
public class Shape
{
    public function GetArea() -> f64;
}

public class Rectangle : Shape
{
    private var width: f64;
    private var height: f64;
    
    public override function GetArea() -> f64
    {
        return this.width * this.height;
    }
}
```

### Operator Overloading

```yu
public class Vector2
{
    public var x: f32;
    public var y: f32;

    public operator +(other: Vector2) -> Vector2
    {
        return Vector2 { 
            x = this.x + other.x,
            y = this.y + other.y
        };
    }

    public operator [](index: u32) -> f32
    {
        switch(index)
        {
            case 0: return this.x;
            case 1: return this.y;
            default: throw "Index out of bounds";
        }
    }
}
```

## Memory Management

Yu provides explicit memory management through smart pointers:

```yu
public function CreateObject() -> Ptr<Object>
{
    // Ptr takes ownership
    var obj: Ptr<Object> = new Object();
    return obj;
}

public function UseObject(obj: Ptr<Object>) -> void
{
    // obj is automatically freed when Ptr goes out of scope
    obj.DoWork();
}

class File
{
    private var handle: Ptr<FileHandle>;
    
    public operator new(path: string) -> File
    {
        // RAII pattern
        handle = Ptr<FileHandle>(OpenFile(path));
    }
}
```

## Error Handling

Errors must be explicitly handled using Result type:

```yu
public function DivideNumbers(a: i32, b: i32) -> Result<i32, string>
{
    if (b == 0)
    {
        return Error("Division by zero");
    }
    return Ok(a / b);
}

// Usage
try 
{
    var result = DivideNumbers(10, 0);
    switch(result)
    {
        case Ok(value):
            Print(value);
            break;
        case Error(msg):
            LogError(msg);
            break;
    }
} 
catch (e: string) 
{
    HandleError(e);
}
```

## Modules

Module system follows TypeScript-style imports:

```yu
// Import specific items
import { Vector3, Matrix4x4 } from 'math/linear';

// Import with alias
import 'collections/hashmap' as HashMap;

// Import everything
import * from 'utils';

// Re-export
export { Sort, Find } from 'algorithms';
```

## Standard Library

Core modules (Planned):

- IO: File operations, console input/output
- Collections: Vector, HashMap, Set
- Network: TCP/UDP sockets, HTTP client
- Math: Linear algebra, numeric operations
- Async: Future, Promise, async runtime
- System: Process, Environment, OS interface