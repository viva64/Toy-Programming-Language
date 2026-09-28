# Toy Programming Language

A small programming language implemented from scratch in C++ as part of the **"Let's Make a Programming Language"** course by PVS-Studio.

The project is designed to help developers understand what happens under the hood of a programming language. Instead of treating a compiler or interpreter as a black box, we build the core components step by step and see how they work together.

## What is this project?

This is a deliberately simple **toy programming language**. It is not intended to be a production-ready language. Its purpose is educational: to demonstrate the basic concepts and mechanisms behind language processing.

The entire project is implemented in **C++**.

## How does it work?

At a high level, the language processes source code through several stages:

```text
Source code
    ↓
Lexer
    ↓
Parser
    ↓
AST
    ↓
Semantic analysis
    ↓
Evaluator
    ↓
Result
```

### Lexer

The lexer reads the source code and converts a stream of characters into a sequence of tokens.

### Parser

The parser takes the tokens produced by the lexer and checks whether they form valid language constructs according to the grammar. It builds an AST representing the structure of the program.

### AST

The Abstract Syntax Tree provides a structured representation of the source code that is easier for the rest of the language implementation to work with.

### Semantic analysis

Syntax alone is not enough to understand a program. Semantic analysis deals with names, scopes, types, and the rules that determine which operations are valid.

### Evaluator

The evaluator walks through the AST and executes the program by calculating expressions, resolving variables, and calling functions.


## About the course

The project was created during the **"Let's Make a Programming Language"** course by PVS-Studio.

The course is presented in a live-coding format by **Yuri Minaev**, an architect of the C++ analyzer at PVS-Studio. Instead of focusing on dry theory, the course builds a language step by step in C++.

Learn more about the course:

https://youtu.be/ylPuGE2itQY

## Disclaimer

This is an educational project. The language is intentionally limited and should be considered a playground for exploring the fundamentals of programming language implementation.
