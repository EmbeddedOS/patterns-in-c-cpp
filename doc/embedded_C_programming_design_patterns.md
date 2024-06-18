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

## II. Creation Pattern

### 2. Object Pattern

- The fastest way to make your code cleanly structured and easy to understand.

- Defining characteristics:
  - 1. **Context is passed as parameters**: We use `self` parameter to access all data that belongs to the object upon which a function operates.
  - 2. **Data is never accessed globally**: Any global or singleton data is never accessed directly but instead is accessed through singleton methods.
  - 3. **Functions do not have static data**: all data is part of the object being operated on. There is no static data in object methods. Data is either part of the object.
  - 4. **Data flows along call path**: This is an important feature that ensures we have clear boundaries between methods that use particular data. This is incredibly useful for multi-threaded environments because it makes data locking very straightforward when needed.

- Use cases:
  - 1. **Grouping data**: you should group all variables into structs (objects) and use the object patterns as means of sorting your data hierarchically.
  - 2. **Singletons**: object pattern is the primary way to implement singletons as well because all data that was previously statically defined inside the singleton implementation can now be placed into a singleton object and all private singleton object methods can be made to operate on that object.
  - 3. **Abstract interfaces**: object pattern is key component of abstract interfaces.
  - 4. **Multi-threaded design**: object pattern is essential for multi-threaded design because thread synchronization is about `locking data - not code` and objects are essential for grouping data together so that we can have one clear lock for a group of variables we need to synchronize access to.
  - 5. **Opaque handles**: a pointer to a data structure can be exposed to the outside without exposing the data structure itself. This allows for efficient way of implementing opaque handles with custom internal allocation. Object pattern is needed because all data belonging to a handle must be separate from all other handlers.

#### 2.1. Implementation

##### 2.1.1. Declaration

- In header file:

```C
struct my_object {
    uint32_t variables;
    uint32_t flags;
}

int my_object_init(struct my_object *self);
int my_object_deinit(struct my_object *self);
```

##### 2.1.2. Definition

```C
int my_object_init(struct my_object *self)
{ // Init your object.
    memset(self, 0, sizeof(*self));

    // ...
}

int my_object_deinit(struct my_object *self)
{ // Clean up your object.

}
```

##### 2.1.3. Usage

```C
struct application {
    struct my_object obj;
}

int application_init(struct application *self)
{
    my_object_init(&self->obj);
}
```

#### 2.2. Rules

- **Functions `act` on objects**: Each function needs to act on an object pointed to by a `self` argument. Any additional parameters passed to the function are there to modify the behavior being done upon the `self` object. The `self` object is the primary object being changed and also the place where results are accumulated. Any output variables can of course also be passed as arguments and such data is considered `exported` from the object and becomes the responsibility of the caller once the method returns.
- **Functions are prefixed with object name**: Each function that operates on an object should be prefixed with the type name of that object and also placed in a file with the same name. This is for clean organization and clarify when reading the code.

#### 2.3. Reentrancy

- **No global static data access**: It may not use global and static data - all data that it uses must come from the arguments (i.e. you can not access static data inside the function but the data you pass to the function can of course be static- there are only restrictions in what you get to access inside the function).
- **No self-modifiable code**: It should not modify its own code (this one is easy to adhere to in modern software). So this is not even applicable to C programming (but is still a rule that must be adhered to in order for a function to be reentrant).
- **No contamination**: It should not call other non-reentrant functions. The best way to ensure this is to make sure we always pass context to all methods that we call - making sure that we apply object pattern throughout the application.

#### 2.4. Benefits

- **Clear Scope**: Fine grained control over data scope thus minimizing unintentional data manipulation.
- **Reentrancy**: Ensures functions are re-entrant (no globally manipulated state).
- **Easy locking**: Simplifies multi-threaded programming because you can easily locate relevant data.
- **Simplifies testing**: Simplicity of testing because code can be easily compiled in isolation and fed with mock data.
- **Clear data flow**: Data flow always through the code and not outside of it. This simplifies debugging and makes the code easier to visualize when reading it without even running or testing it.
