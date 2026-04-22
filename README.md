# Programming Language Concepts — CS280

Coursework from CS280 (Programming Language Concepts) at NJIT, Spring 2026. The course covers how programming languages are designed and implemented under the hood — from tokenizing raw text all the way to evaluating expressions.

---

## What's in here

### PA1 — Lexer
A lexical analyzer for a Pascal-like language. Given source code as input, it scans character by character and outputs a stream of labeled tokens (keywords, identifiers, literals, operators, etc.). Handles edge cases like unterminated strings, malformed numbers, and nested comments.

**Skills:** lexical analysis, finite automata, string processing in C++

---

### PA2 — Parser
A recursive-descent parser built on top of the PA1 lexer. It validates the syntactic structure of programs written in the same Pascal-like language and reports meaningful error messages when the grammar rules are violated.

**Skills:** recursive descent parsing, context-free grammars, syntax error reporting

---

### Short Assignment 2 — Command File Parser
Reads a script file containing shell-style commands (`DIR`, `CD`, `COPY`, `DEL`) and counts how many lines are valid commands, invalid, or commented out.

**Skills:** file I/O, string parsing, basic lexical classification

---

### Short Assignment 3 — Identifier Validator
Given a list of tokens, determines whether each one is a valid identifier. Handles three identifier prefixes (`_`, `@`, `#`) with their own rules for what characters can follow.

**Skills:** identifier grammar rules, character classification, edge case handling

---

### Short Assignment 4 — Numeric Literal Parser
Scans input for numeric literals and classifies them as integers, valid floats, invalid floats, or numbers in scientific notation. Handles signs, decimal points, and exponent suffixes.

**Skills:** number grammar, tokenization, floating-point edge cases

---

### Short Assignment 5 — Value Type System
Implements a `Value` class that wraps integers, reals, booleans, and strings, and defines arithmetic and comparison operators across those types. This acts as the runtime value layer for a simple interpreter.

**Skills:** operator overloading in C++, type coercion, interpreter internals

---

## Tech

All projects are written in **C++17**. Compile with:

```bash
g++ -std=c++17 -o output <source>.cpp
```

---

## Course

**CS280 — Programming Language Concepts**  
New Jersey Institute of Technology (NJIT)
