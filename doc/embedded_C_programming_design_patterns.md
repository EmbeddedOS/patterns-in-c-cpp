# Embedded C programming design pattern

## I. Introduction

### 1. Introduction

- Essential design patterns for embedded firmware projects in C.

- What you will learn?
  - 1. **Object Pattern**: Group data into objects with classes and member functions for instantiation and destruction.
  - 2. **Opaque Pattern**: Keep implementation private and expose only a handle to the object with three different methods.
  - 3. **Singleton Pattern**: Ensure a class has only one instance and provide global access to it.
  - 4. **Factory Pattern**: Provide an interface for creating objects in a super class, allowing sub-classes to alter the type of objects that will be created.
  - 5. **Callback Pattern**: Handles object-oriented callbacks bound to object instances, allowing callbacks to operate on specific instances.
  - 6. **Inheritance Pattern**: Used for implementing inheritance relationships between objects and components in a C program to create a hierarchy.
  - 7. **Virtual API Pattern**: Implements virtual functions in C and makes handles to opaque objects `smart`, providing polymorphism and allowing the same interface for several implementations.
  - 8. **Bridge Pattern**: Builds upon the virtual API pattern to bridge two distinct hierarchies of objects, illustrated in Rust and C examples.
  - 9. **Return value Pattern**: Standardizes function handle return values for clear communication with caller.
  - 10. **Spin-lock Pattern**: Protects shared data from corruption by ensuring never runs when modifying it.
  - 11. **Semaphore Pattern**: Outlines a pattern of signaling between interrupt handlers and application level threads for thread-aware synchronization.
  - 12. **Mutex Pattern**: Prevents starvation by means of priority inheritance and is used for mutually exclusive access to a resource.
  - 13. **Conditional Pattern**: Signals and wakes up one or more threads from another thread or interrupt handler to check for complex conditions in response to an asynchronous event.

- Who this training is for:
  - 1. Embedded software Architects: even if you have been coding for many years, you may still find ideas in this content on how to do things better.
  - 2. Beginner Embedded Programmer.
  - 3. Embedded Manager.

- When to introduce these patterns:
  - 1. Refactoring existing code: when you already have code and you want to improve its structure.
  - 2. Starting new project: make things right from the start so that you do not need to refactor later.

- Where these patterns are most useful:
  - 1. **Structural templates**: patterns are used like reinforcement bars in your code to keep the expectations clear and the code strong.
  - 2. **More valuable if used everywhere**: a pattern you only use once is not as valuable as a pattern apply across all levels of software abstractions.
