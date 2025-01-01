# Yu Programming Language

[![Build Status](https://github.com/yuminaa/yu-lang/actions/workflows/workflow.yml/badge.svg)](https://github.com/yuminaa/yu-lang/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Yu is a statically typed, compiled programming language designed for data-oriented programming while maintaining
familiar OOP-like syntax. It emphasizes data layout, cache coherency, and efficient memory patterns while providing a
comfortable, modern programming interface.

> [!IMPORTANT]
> The compiler is currently under development and not ready for production use.

## Philosophy

Yu uses class-like syntax as a familiar interface for defining data structures and their associated operations, but
internally treats data and behavior separately.

## Code Example

```yu
import { io } from 'sys'; 

@align(16)
class Vector3
{
    var x: i32 = 0;
    var y: i32 = 0;
    var z: i32 = 0;
};

class Player : Entity
{
    private var position: Vector3 = Vector3({ x = 10, y = 20, z = -10 });
    private var velocity: Vector3 = Vector3();
    
    public operator new() -> Player 
    {
        return new Ptr<Player>();
    }
    
    public function MoveTo(target: Vector3) -> void 
    {
        this.position = target;
    }
};

class Generic<T>
{
    var value: T;
    public function GetValue() -> T 
    {
        return this.value;
    }
};
```

# Roadmap

- [X] Lexer
- [ ] Parser
- [ ] Semantic Analysis
- [ ] Code Generation
- [ ] Optimization
- [ ] Standard Library