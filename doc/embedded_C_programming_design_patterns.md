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

#### 2.5. Drawbacks

- **No ability to hide implementation**: In it's basic form (with struct declared in the header file) private fields are exposed. This add dependencies to the code using our struct. We can only hide implementation by using an extension of this pattern such as the singleton pattern or heap object pattern.

- **Can result in high memory consumption**: Sometimes it is necessary to declare static structures inside a C file - for example when you need to share some data between all instances of a particular object. This warrants occasional mix of of the object pattern with singleton pattern internally where a part of the object implementation is in fact a singleton.

#### 2.6. Best Practice

- **Avoid static**: you should avoid static variables inside functions entirely because they break the object oriented design that the object pattern is designed to solve. They make your functions depend on more data than what is directly available through the `self` pointer.
- **Use `self`**: unless your object pointer is an interface handle (from which you would then retrieve a `self` pointer) do not use any other names for the variable that designates the pointer to the main context you are operating on. Use `self` because it is compatible with C++ compiler and you remove ambiguity when you always use the same name to refer to `self` (do not use `me`, `dev`, `obj` or some other name) - `self` is a standard that has become widespread even in Python and Rust.
- **Use consistent naming**: The main struct should have the same name as the header file it is declared. All methods that operate on instances of the struct have the same prefix that is also the same as the struct.
- **Standardize init/deinit**: There should be two standard functions: `my_object_init()` and `my_object_deinit()` that initialize a new instance and de-initialize it. When user instantiates an object, he should always call `<object>_init()` and this init function must at the very least always clear the memory of the object to zero (this is not done automatically when we are allocating a stack variable!).

#### 2.7. Pitfalls

##### 2.7.1. No `self`

```C
// Instead of doing this:
int my_object_do_something(struct my_object *self, int arg)
{
    self->some_var = arg;
}

// You are doing this:
static struct my_object _self;
int my_object_do_something(int arg)
{
    _self.some_var = arg;
}
```

- **Reducing clarity**: It is no longer clear wether this method operates on data aggregated under `struct my_object` or not. You lose this clarify.
- **More difficult to test**: It is bad for testability because the caller is no longer responsible for the memory and we can not easily create multiple instances or inspect the internals of the object when testing.

##### 2.7.2. Extern variables

- Bad header file:

```C
extern uint32_t value;
```

- Bad C file:

```C
#include <bad_header.h>

void your_method(struct your *self)
{
    value = 123;
}
```

#### 2.8. Alternatives

- **Opaque Pattern**: This pattern hides implementation of the object completely by taking on the responsibility for allocating and de-allocating instances of the object. The only structure visible to the outside becomes a pointer to an instance without exposing the internals of an object instance outside of its implementation.

- **Singleton Pattern**: This is another variation of the object pattern where a subsystem may want to keep the instance of the object entirely private and only expose a global interface. This pattern is useful for services that are shared across the whole application such as `logging`, `networking stack`, etc.

#### 2.9. Conclusion

- **Universal Pattern**: Apply this pattern as much as possible. If in doubt always make into an `object`.
- **Fastest way to cleanup code**: Dependencies become clear when you separate code into distinct objects. Use this pattern for refactoring old code.

#### 2.10. Quiz

- 1. Why is it so important to avoid static variables inside functions in C? Specially if the function is an object method?
  - Because the static variables make functions become non-reentrancy: Same arguments but can make different results -> hard for testing.
  - In multi-threaded environment, using globals variables need protection mechanisms -> locking -> slow system.
  - The code is not clear, hard to maintain.

- 2. Why do we avoid functions without parameters? What negative property do these functions possess that make them a very bad design flaw in C source code?
  - No parameters mean these functions will get input from global data -> that violates object pattern properties.

- 3. Why do we call our pointer to context `self` and why should avoid using other names to refer to `self`?
  - `self` is a pointer to the context that holds all object's data. The it's methods receive it like a first parameter.
  - `self` don't conflict with another keyword.
  - Using `self` name like a standard that hold the object's data. Another languages like python, rust use this name like a standard. `me`, `dev`, `object`, `this`, etc. make confusing or cannot compile with another compiler like C++.

- 4. Why is it sometimes necessary to instantiate object locally in the C file as singletons?
  - Some kind of object needs only one instance for example: logging, networking stack, etc.

- 5. Why is it a good practice to always name the header and the C file with the same name as the data object they implement?
  - Easy to manage and search.
  - The code will be neated and organized, like we separate the object to different namespace.

- 6. Why should you never use `extern` declared variables anywhere in you C code?
  - `extern` means global objects, that means you expose your data for everyone, that make functions become non-reentrancy, hard to debug who change this variable, etc.
  - These variables need guarantee to access in multithread environment.

### 3. Opaque pattern

- Objects with a private definition -> abstraction.

#### 3.1. Defining characteristics

- 1. **Object definition is in the C file**: The struct itself is defined in the C file implementing the object instead of being defined in the header file.

- 2. **Implementation responsible for allocation**: Opaque objects are either statically allocated in the implementation C file or allocated on heap (for example all objects can be allocated at startup). This is a necessary consequence of struct not being visible outside of the C file.

- 3. **Implementation uses object pattern internally**: All functions can still take `self` pointer instance as parameter - but the caller is no longer responsible for allocating the instance. If this feature is not present then we are likely dealing with the singleton pattern.

- 4. **Uses `new` and `delete` idiom**: This is necessary to make a distinction between opaque object and fully defined objects and to make it clear to the caller that the caller must call delete when done with the object.

- 5. **Application (user) only deals with pointers**: since implementation is fully hidden, we only with opaque handles (pointer based) outside of the implementation.

#### 3.2. Use Cases

- **Isolating dependencies**: We want to keep dependencies local to the implementation.
- **Prevent direct data access by user**: Since the object fields are private to the implementation, they can not be accessed, changed or seen by any other code outside of the implementation.

#### 3.3. Implementation

##### 3.3.1. Public API

- File: `opaque.h` - public interface.

```C
struct opaque; // Just a declaration.

// init and deinit.
int opaque_init(struct opaque *self);
int opaque_deinit(struct opaque *self);

// methods that operate on an opaque.
void opaque_set_data(struct opaque *self, uint32_t data);
uint32_t opaque_get_data(struct opaque *self);
```

##### 3.3.2. Private Implementation

- File: `opaque.c` - private implementation.

```C
// Actual definition of the struct in private space of the C file.
struct opaque {
    uint32_t data;
};

int opaque_init(struct opaque *self)
{
    memset(self, 0, sizeof(*self));

    /* Do other initialization. */

    return 0;
}

int opaque_deinit(struct opaque *self)
{
    /* Free any internal resources and return to know state. */
    self->data = 0;
    return 0;
}

void opaque_set_data(struct opaque *self, uint32_t data)
{
    self->data = data;
}

void opaque_get_data(struct opaque *self)
{
    return self->data;
}
```

#### 3.4. Allocation schemes

- **Stack allocation**: This scheme uses standard dynamic allocation on the stack using `alloca()` method. `alloca()` is like `malloc()`, except that the allocation happens on the stack and is automatically released when the function returns. This method doesn't suffer from memory fragmentation like `malloc()` below.

- **Dynamic allocation**: here we have to use `malloc()` or an RTOS alternative. If you use this method on resource constrained devices then make sure that you either only allocate during initialization or that you do not use this method at all due to the risk of fragmenting the memory and reaching the point where no new instances can be allocated because of that.

- **Static allocation**: This method uses code generation or a static array. Code generation is preferable but does require that you instantiate your objects using a data representation such as using the device tree which is then parsed during build and from which instances can be created at compile time.

##### 3.4.1. Stack allocation

- The stack allocation scheme use standard `alloca()` function in the C library to allocate the opaque structure on the stack:

```C
size_t opaque_size(void)
{
    return sizeof(struct opaque);
}
```

- In User code:

```C
// Create an opaque type on the stack.
struct opaque *obj = alloca(opaque_size());

/* Standard init. */
opaque_init(obj);

/* Operate on the opaque object. */
opaque_set_data(obj, 123);

/* Done with the object `deinit` it. */
opaque_deinit(obj);
```

##### 3.4.2. Dynamic Allocation

- File: `opaque.c` - allocation (dynamic)

```C
struct opaque *opaque_new()
{
    return malloc(sizeof(struct opaque));
}

void opaque_free(struct opaque **self)
{
    /* Free dynamically allocated instance. */
    free(*self);

    /* Set the passed pointer to NULL! */
    *self = NULL;
}
```

- User code:

```C
/* Allocate new opaque on the heap. */
struct opaque *obj = opaque_new();
__ASSERT(obj, "Memory allocation failed!");

/* Call standard init. */
opaque_init(obj);

/* Operate on the opaque. */
opaque_set_data(obj, 456);

/* Deinit the object. */
opaque_deinit(obj);

/* Discard memory. */
opaque_free(&obj);
```

- NOTE: Using `malloc()` a lot can cause fragment memory.

##### 3.4.3. Static Allocation: Zephyr Driver Model

```C
#define PWM_DEVICE_INIT(index)                                          \
    static struct pwm_stm32_data pwm_stm32_data_##index;                \
    static const struct soc_gpio_pinctrl pwm_pins_##index[] =           \
        ST_STM32_DT_INST_PINCTRL(index, 0);                             \
                                                                        \
    static const struct pwm_stm32_config pwm_stm32_config_##index[] = { \
        .timer = (TIME_TypeDef *)DT_REG_ADDR(                           \
            DT_PARENT(DT_DRV_INST(index))),                             \
        .prescaler = DT_INST_PROP(index, st_prescaler),                 \
        .pclken = DT_INST_CLK(index, timer),                            \
        .pinctrl = pwm_pins_##index,                                    \
        .pinctrl_len = ARRAY_SIZE(pwm_pins_##index),                    \
    };                                                                  \
                                                                        \
    DEVICE_DT_INST_DEFINE(index, &pwm_stm32_init, NULL                  \
        &pwm_stm32_data_##index,                                        \
        &pwm_stm32_config_##index, POST_KERNEL,                         \
        CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                             \
        &pwm_stm32_driver_api);

DT_INST_FOREACH_STATUS_OKAY(PWM_DEVICE_INIT)
```

#### 3.5. Benefits

- **Hides implementation**: removes the need of the code that uses the object to include dependencies upon which the module depends.
- **Limits dependencies**: the components and libraries upon the object depends no longer `leak` into the code that uses the object - not even through header files.

#### 3.6. Drawbacks

- **Requires allocation scheme**: This pattern must either use `alloca()`, `malloc()` or implement a custom static allocation scheme for creating new objects - which increases complexity.

- **Prevents data structure**: with object pattern you would normally instantiate each object as part of a clean data hierarchy with `application` struct being the top level enclosing data structure. Since opaque pattern actively prevents instantiation of object on the stack, it also prevents objects from being instantiated inside that data hierarchy.

#### 3.7. Best practices

- **Use stack allocation whenever possible**: This works exactly the same as working with object pattern and is the most lightweight approach.
- **Use the `new`/`delete` idiom**: If you do use heap, it's good idea to make this clear, by adding the new/delete methods. Always check for failed allocations.

#### 3.8. Common pitfalls

- **Running out of stack space**: If your objects are too big and you allocate them using the stack allocation scheme, then you can easily smash the stack. However, this is not specifically a problem with this pattern, but rather a general danger that is always present when you use stack allocated variables in general.

- **Memory fragmentation**: If you are using the dynamic memory allocation scheme and you frequently allocate/de-allocate large numbers of objects of different sizes then you can end up running out of contiguous blocks of memory and your memory allocation will fail. To avoid this, if you have limited memory avoid completely at runtime. If you really need to use it, use it only during initialization stage and do not re-allocate anything at run time.

#### 3.9. Alternatives

- **Object Pattern**: The object pattern is a clear alternative, with slightly more simplicity - if you are ok with exposing all dependencies to any other code that includes your header.

- **Singleton Pattern**: The singleton pattern is essentially the opaque pattern - but with the main difference that not even the pointer to the context is passed around. Singleton pattern should be mainly limited to implementation of software wide sub-systems and services where instantiation of multiple instances does not make sense at all. Note however that even a singleton object should internally use the object pattern to increase quality of code organization and ease of testing.

- **Abstract API Pattern**: An abstract interface by definition acts as an opaque object. It is a structure that holds only function pointers and each function is then able to retrieve pointer to the implementation specific data using `CONTAINER_OF` macro. Abstract interfaces are useful when we want to generalize over a class of objects - and we then by definition make all of our objects opaque. This pattern is more heavyweight than simple opaque object.

#### 3.10. Conclusion

- **Enhanced alternative to the object pattern**: Very small overhead when using stack allocation.
- **Size must be made available using member function**: So that we can allocate the memory.
- **Opaque handles only require type declaration**: It is enough to simply declare a struct without a body in order to be able to use pointers to that struct.

#### 3.11. Quiz

- 1. What are some of the reasons why we would want to hide the data structure of an object outside of its implementation?
  - We don't want to expose data to customer, easier for them.
  - Reduce dependencies with another modules.
  - Some time we don't change the interface but the implementation changes a lot, so we should hide them from user application.

- 2. Why does opaque pattern require a custom allocation scheme?
  - Because we just provide `struct declaration` to user application, they CANNOT create the object like normal.

- 3. How does using the opaque pattern affect the structure of the data of the application? How does it change the way we structure application data?
  - The application structure can only hold the pointer to opaque object. User application need to use custom allocation schemes for the opaque property.

- 4. Why is it that allocation must be private when data type is private?
  - Because they don't know the data size.
- 5. Why is it a good idea to automate the allocation of objects at compile time?
  - We can avoid memory fragment if use heap or smash stack if you use stack.

### 4. Singleton Pattern

#### 4.1. Definition

- **Control over instantiation and use**: single pattern prevents multiple instances from existing in the system at the same time. Sometimes this is also extended to enforce single user at a time as well.

- **Private Construction**: In C this basically means that construction is automatic and is done privately inside the module source code - and only once.

- **Stateless Interface**: Provides a method for getting the instance or implements only stateless methods with a call order agnostic API (methods do not take an instance pointer).

#### 4.2. Use cases

- **Logging**: When you use the `LOG_INF` or `LOG_DBG` macros. Instead of passing an object into these macros, they reference a logger that is created using the `LOG_MODULE_REGISTER` macro at the top of your C file. The same logger singleton instance can be shared across multiple files by using the `LOG_MODULE_DECLARE` macro. By doing this, you are effectively sharing and referencing the global logger instance that was created with the `LOG_MODULE_REGISTER` macro.

- **Configuration**: The Zephyr settings sub-system acts as a singleton, allowing other subsystems to register endpoints that will be called upon loading and saving settings. Callbacks are a common feature of singletons, as the singleton object acts as a `manager` for many other objects that are registered with it.

- **Firmware Subsystems**: Such as power management, device initialization, and networking stacks.

- **Thread scheduler**: This is another example of a singleton. There is only one thread scheduler across the application and no need for multiple scheduler. Threads are `registered` with this scheduler and the scheduler than manages the threads. We let the system instantiate the scheduler.

#### 4.3. Benefits of the singleton pattern

- **Single instance requirement**: The pattern ensures that there is only one instance of the object being created.
- **Simplifies code**: No requirement to pass the context along code execution path.
- **Improving performance**: Additional instances do not need to be created, initialized and destroyed. This comes at the expense of having to lock the resource while it is in use and serializing access to it.
- **Managing shared resources**: Your application components may require a single global point of access with which other subsystems can register their callbacks. Acts like a coordinator.

#### 4.4. Reasons to avoid using singleton

- **UART driver**: you can have multiple UARTs and using a singleton for this reduces flexibility. Instead, use multiple instances with a factory and abstract interface patterns to enable generic access to these instances.

- **Network protocol driver**: You must expect that there will be multiple devices using the same protocol. Therefore, the protocol implementation should be able to be instantiated multiple times and the states of these instances should be separate.

- **Abstract interfaces**: When you need to switch between multiple different implementations behind the same interface. A singleton actively prevents you from being able to implement such flexibility.

- **Concurrent operation**: When you have a shared singleton, it will always be a bottleneck if any operation within it takes time. This may not be a problem when part of your requirements is processing data in sequence, but it can be a bottleneck when many subsystems try to access the singleton from multiple threads.

#### 4.5. Implementation

- Instantiated objects:

```C
void object_method(struct object *self)
{
    // ...
}
```

- Singleton objects:

```C
static struct object _singleton;

void object_method(void)
{
    // ...
}
```

##### 4.5.1. Safe `on demand` creation

```C
struct my_object *my_object_get_singleton(void);
```

```C
#define DEFINE_SINGLE_TYPE(type)                                \
    static struct type *_##type##_self;                         \
    static struct k_spinlock _##type##_lock;                    \
    struct type *type##_get_singleton(void)                     \
    {                                                           \
        static struct type _singleton = {0};                    \
        k_spinlock_key_t key = k_spin_lock(& _##type##_lock);   \
        if (!_##type##_self)                                    \
        {                                                       \
            _##type##_self = &_singleton;                       \
            ##type##_init(_##type##_self);                      \
        }                                                       \
                                                                \
        k_spin_unlock(& _##type##_lock, key);                   \
        return _##type##_self;                                  \
    }
```

```C
DEFINE_SINGLETON_TYPE(my_object);
```

```C
void main(void)
{
    struct my_object *obj = my_object_get_singleton();
    my_object_do_something(obj);
}
```

#### 4.6. Interface considerations

- **`self` pointer as cue**: to indicate that state and ownership are important.
- **Stateless API**: If ownership is not important.
- **Race conditions**: be ware and consider if it is a possibility or not.

- Avoid this: this is stateful API usage pattern.

```C
my_singleton_object_lock();
my_singleton_object_do_something();
my_singleton_object_do_something_else();
my_singleton_object_unlock();
```

- You should put `lock()` and `unlock()` to inside or consider this implementation:

```C
#define DEFINE_SINGLETON_ACQUIRE_RELEASE(type)                  \
    static struct type *_##type##_in_use;                       \
    struct type *type##_acquire_singleton(void)                 \
    {                                                           \
        k_spinlock_key_t key = k_spin_lock(&_##type##lock);     \
        if (!_##type##_in_use)                                  \
        {                                                       \
            _##type##_in_use = &_##type##_singleton;            \
            k_spin_unlock(&_##type##_lock, key);                \
            return _##type##_in_use;                            \
        }                                                       \
                                                                \
        k_spin_unlock(&_##type##_lock, key);                    \
        return NULL;                                            \
    }                                                           \
    void type##_release_singleton(struct type **self)           \
    {                                                           \
        k_spinlock_key_t key = k_spin_lock(&_##type##lock);     \
        if ((self != NULL) && (*self == _##type##_in_use))      \
        {                                                       \
            _##type##_in_use = NULL;                            \
            *self = NULL;                                       \
        }                                                       \
                                                                \
        k_spin_unlock(&_##type##_lock, key);                    \
    }
```

```C
void main(void)
{
    struct object *self = object_acquire_singleton();
    if (self)
    {
        object_do_something();
        object_do_something_else();
        object_release_singleton(&self);
    }
}
```

- So if somebody else try to acquire the object at the same time, It's not block but return NULL.

#### 4.7. Best practices

- **Automatic initialization**: Initialize the instance at system startup. Use initialization priority to determine exactly when.
- **Stateless public API**: Use `my_object_operation(params)` instead of `my_object_operation(self, params)` for singletons to avoid giving a pointer to this object to user.
- **Avoid singletons as much as possible**: Avoid using singletons as much as possible and use conventional object oriented design in most of your code. Only use singleton pattern when it is an absolute requirement that there is only one instance of the object in your application at any time.
- **Use lock**: Use a lock when initializing the instance to avoid race conditions when multiple threads try to create or access the singleton object at the same time (not always necessary).
- **Hide the instance**: Use a static variable to hold the instance to ensure that the instance is only visible to the object implementation, and is not directly accessible through any other means than the singleton interface.

#### 4.8. Common pitfalls

- **Overuse**: Unthinkingly using the singleton pattern for everything will severely damage the flexibility of your architecture.
- **Stateful interface**: Since singletons are shared across many services that are part of your application, their API should not rely on call ordering.
- **Cutting corners**: Thinking that `now we only need one instance, maybe later we can shift to using more instances`. The singleton pattern explicitly states that the only time you should use it is when there is no possibility at all that there will ever be more than one instance of the thing you are creating.

- DO NOT do this:

```C
my_object_lock();
my_object_do_something();
my_object_unlock();
```

#### 4.9. Alternatives to single pattern

- **Object Pattern**: Where the instance is created by user and then passed to the singleton. This is also a variation of dependency injection pattern.
- **Factory Pattern**: Where you hide the details of creation of the instance from the user thus simplifying the user code.
- **Prototype Pattern**: Where you copy an existing object using a virtual constructor (a variation of the virtual API pattern).
- **Endpoint API Pattern**: Where you make a call to a global function and provide a path such as `path/to/endpoint` and the function then resolves the path to a previously registered callback. This is a form of requesting routing. This can also be referred to as `Service Locator Pattern`.

#### 4.10. Quiz

- 1. What is the main purpose of the singleton pattern?
  - For some entire that require only one instance in our application. To avoid create more instances.
- 2. When should you avoid using the singleton pattern?
  - Every time, unless we need only one instance and we have to prevent user create more.
- 3. How is the singleton pattern implemented in C?
  - Static variable, hidden in C file, and initialized with specific macro to make sure it have only one instance.
- 4. Give a few examples of common use cases for the singleton pattern in firmware development?
  - Logger, and thread scheduler (we need only one scheduler to schedule system).
- 5. What is best practices when implementing the singleton pattern?
  - Avoid this. Make sure thread-safe. Should Automatically initialize object (using macro or gcc extension (constructor, priority)). Hidden object.
- 6. What are common pitfalls?
  - Using singleton for objects that may have multiple instances.
  - No thread-safe.

### 5. Factory Pattern

- This pattern is designed for cases where you need to create object either on startup or pre-defined ways. For example you have your device tree and you need to create your driver structure.

#### 5.1. Defining Characteristics

- What objects are we creating?
- How is memory allocated?
- How do we store and pass creation parameters?
- Decoupled implementation details.
- What code accesses and uses the factory?

#### 5.2. Use cases for factory pattern

- Instantiation of device driver data such as for example DEVICE_DT_DEFINE macro.
- Memory pools and buffer pools.
- Network device creation macros that L2 network interfaces.
- File system objects (ie `fopen()` on Linux and Lower level functions on Zephyr).
- Network packet creation and protocol buffer packing.

#### 5.3. PROS

- Increase flexibility without having to modify `client` code. Client code maintains stable, generic API towards the implementation.
- Improved code organization and separation of concerns. Object creation of specific objects is centrally handled - thus easier to maintain.
- Data driven object creation (ex. device tree). Object layout is defined as data `recipe`.
- Improved testability. All user code always uses a limited, generic API for querying and creating objects of a specific type.
- Improved code reuse. Object creation is removed from code meaning that it can easily be implemented differently on different build targets.
- Improved scalability. Internal object structure can be modified while guaranteeing that no setup code needs to be modified.

#### 5.4. CONS

- Added complexity. Objects are created from parameters - thus there is a `data` API along side of creation API.
- Lack of flexibility. If creation functions set too tight policy on object creation.
- Difficultly testing. Slight increase in complexity when macros are used as **factory** methods.

#### 5.5. Implementation

- **Simple factory**: An `object-less` method that returns created object.
- **Embedded factory**: Useful for embedded applications.
- **Abstract factory**: A factory with a generic interface and multiple implementations.
- **Prototype factory**: A factory that does a deep copy of existing objects (prototypes).
- **Pooled factory**: An optimization technique where objects are kept in a `free` and `in-use` queue.

##### 5.5.1. Simple factory

- This is a public header:

```C
enum shape_type {
    SHAPE_CIRCLE,
    SHAPE_SQUARE
};

/* Abstract API for the shade. */
struct shape_api {
    void (*draw)(shape_t shade);
};

typedef struct shape_api **shape_t;

struct shape {
    const struct shape_api * const api;
};

/* We make only one API for avoid duplicating create many APIs for each shape
 * type. For example we have up to a thousand of shade type.
 */
shape_t shape_create(enum shape_type type);

/* Virtual API with function pointer like an abstract object.
 */
void shape_draw(shape_t shape);
```

- In the C file implementation:

```C
#include <shape.h>

/* shape circle and shape rectangle structures inherit struct shape with `draw`
 * method. */
/* User don't need to care about the detailed shape types. */
struct shape_circle {
    struct shape shape;
    float radius;
}

struct shape_rectangle {
    struct shape shape;
    float width, hight;
}
```

- For shape circle implementation:

```C
static void _shape_circle_draw(shape_t shape)
{
    /* `shape` object is pointer to pointer to `struct shape_api`.
     * `struct shape_circle` have `struct shape` like parent struct.
     * `struct shape` have `struct shape_api` that actually is `shape` object.
     *
     * So what we do here is: Get Original `struct shape_circle` object from the
     * `shade_t` object. So we can get more data of the original object, this is
     * similar like we are getting the `this` pointer in CPP, and use it to do
     * more specific task of object, access private data, methods, etc.
     */

    struct shape_circle *self = CONTAINER_OF(shape, struct shape_circle, shape.api);

    /* Now we can access private methods. */
    _do_draw_circle(self->radius);
}

const struct shape_api _circle_api = {
    .draw = _shape_circle_draw
}

static void shape_circle_init(struct shape_circle *self)
{
    memset(self, 0, sizeof(*self));
    self->api = &_circle_api;
}
```

- And similar way for rectangle object:

```C
static void _shape_rectangle_draw(shape_t shape)
{
    struct shape_rectangle *self = CONTAINER_OF(shape, struct shape_rectangle, shape.api);

    // Now we can access private methods.
    _do_draw_rectangle(self->width, self->height);
}

const struct shape_api _rectangle_api = {
    .draw = _shape_rectangle_draw
}

static void shape_rectangle_init(struct shape_rectangle *self)
{
    memset(self, 0, sizeof(*self));
    self->api = &_rectangle_api;
}
```

- And in the `shape_create()` API and `shape_draw()` API, that we actually provide to customer here are implemented like this:

```C
void shape_draw(shape_t shape)
{
    /* Call object virtual method. */

}

shape_t shape_create(enum shape_type type)
{
    switch (type)
    {
        case SHAPE_CIRCLE: {
            struct shape_circle *shape = (struct shape_circle *)your_alloc();
            shape_circle_init(shape);
            return &shape->api;
        } break;
        case SHAPE_RECTANGLE: {
            struct shape_rectangle *shape = (struct shape_rectangle *)your_alloc();
            shape_rectangle_init(shape);
            return &shape->api;
        } break;
    }

    return NULL;
}
```

- In application code:

```C
int main(void)
{
    shape_t rectangle = shape_create(SHAPE_RECTANGLE);
    shape_t circle = shape_create(SHAPE_CIRCLE);
    shape_draw(rectangle);
    shape_draw(circle);
}
```

##### 5.5.2. Embedded Factory

```C
DEVICE_DT_INST_DEFINE(index,        \
    &uart_stm32_init,               \
    NULL,                           \
    &uart_stm32_data_##index,       \
    &uart_stm32_cfg_##index,        \
    PRE_KERNEL_1,                   \
    CONFIG_SERIAL_INIT_PRIORITY,    \
    &uart_stm32_driver_api);

STM32_UART_IRQ_HANDLER(index)

DT_INST_FOREACH_STATUS_OKAY(STM32_UART_INIT)
```

- `DEVICE_DT_INST_DEFINE` this macro take same parameters:
  - `&uart_stm32_init` this function pointer for initialize UART for stm32.

- What this macro does?
  - place some pointers into tables, and the Zephyrs startup will look and run functions in tables.
  - Almost code, configuration from device tree, it can do automatically. The use code only need to define some device handlers

- In CPP we have constructors for global, static objects.

##### 5.5.3. Abstract factory

- The basic idea of abstract factory is having abstract objects. We do the same for the factory, so we can have many flexibility builders, for objects.

```C
struct shape_factory {
    const struct shape_factory_api * const api;
}

shape_t shape_factory_create_shape(shape_factory_t factory, enum shape_type type);
```

- For example, we have a factory to build yellow shapes:
- Header file:

```C
struct shape_factory_yellow
{
    struct shape_factory factory;
};

static inline shape_factory_t shape_factory_yellow_cast_to_factory(struct shape_factory_yellow *self)
{
    return &self->factory.api;
}

void shape_factory_yellow_init(struct shape_factory_yellow *self);
```

- Source file:

```C
shape_t _yellow_create(shape_factory_t factory, enum shape_type type)
{
    struct shape_factory_yellow *self = CONTAINER_OF(factory, struct shape_factory_yellow, factory.api);
    switch(type)
    {
        case SHAPE_CIRCLE: {
            return _create_yellow_circle();
        } break;
        case SHAPE_RECTANGLE: {
            return _create_yellow_rectangle();
        } break;
    }

    return NULL;
}

const struct shape_factory_api _yellow_api = {
    .create = _yellow_create
}

void shape_factory_yellow_init(struct shape_factory_yellow *self)
{
    memset(self, 0, sizeof(*self));
    self->api = &_yellow_api;
}
```

- And in the user code:

```C
void main(void)
{
    struct shape_factory_yellow yellow_fac;
    shape_factory_yellow_init(&yellow_fac);

    shape_factory_t fac = shape_factory_yellow_cast_to_factory(&yellow_fac);
    shape_t circle = shape_factory_create_shape(fac, SHAPE_CIRCLE);
    shape_draw(circle);
}
```

##### 5.5.4. Prototype Factory

- The key ideal of prototype factory is create an prototype object and every time users want to create new one, we clone from the prototype.
- This pattern is appropriate for objects that take long time to create, load, for example, have to read from the disk. So we simply duplicate it, that quicker than we create an object from scratch.

```C
struct shape_api {
    void (*draw)(shape_t shape);
    void (*clone)(shape_t shape);
};

//...
static shape_t _circle_prototype;
static shape_t _rectangle_prototype;
shape_t shape_create(enum shape_type type)
{
    switch(type) {
        case SHAPE_CIRCLE: {
            return shape_clone(_circle_prototype);
        } break;
        case SHAPE_RECTANGLE: {
            return shape_clone(_rectangle_prototype);
        } break;
    }

    return NULL;
}

void shape_factory_init(void)
{
    _circle_prototype = _load_circle_from_disk();
    _rectangle_prototype = _load_rectangle_from_disk();
}
```

##### 5.5.5. Pooled Factory

- We allocate a memory pool, so we don't worry about allocate objects and free objects at run time (just set-clear `empty` flag and reuse the memory).
- But limitation of that pattern is that we have limited object.

```C
union shape_slot {
    struct shape_circle circle;
    struct shape_rectangle rectangle;
};

struct shape_slot
{
    bool empty;
    union shape_slot data;
}

static struct shape_slot _slots[CONFIG_MAX_SLOTS];
```

#### 5.6. Conclusion

- Hiding implementation details behind a factory which returns abstract handles to create objects.
- Abstract factories that themselves are abstract objects.
- Object creation by copy.
- Object allocation factory.

#### 5.7. Best practices

- **Clear interfaces**: Clear define the interface for creating objects, as well as the specific types of objects that can be created. You should be naming your objects properly and structuring your code according to principles of good object oriented design.
- **Singleton factory**: Use a singleton factory class to ensure that only one instance of the factory exists at any given time. In C this is almost the default mode of operation for the simply factory. For abstract factories you may want to initialize the factories at boot-before entering `main`. You can accomplish this by using boot time initialization callback in Zephyr.
- **Parameterize your creation**: Think of the factory as a way of creating memory objects from an XML or JSON description. Your factory takes in parameters in a simple form and creates specific objects according to these parameters without exposing the object implementation to the user.
- **Utilize the abstract API pattern**: Use `CONTAINER_OF` and use the abstract api method. This keeps your code very type safe and robust at runtime. It is difficult to go wrong when you apply the API pattern.
- **Optimize when needed**: Use the property factory when a complex object takes longer time to create from scratch than to copy in memory.

#### 5.8. Pitfalls of factory pattern

- Insufficient parameterization.
- Too complex api for creating objects.

#### 5.9. Alternatives

- **Object Pattern**: Sometimes a simple `my_object_init` constructor is sufficient. Most of the time you would be creating the objects directly - either in static memory or inside a memory pool. You would use the standard object `init` pattern for constructor and `deinit` for cleanup. For most use cases this is sufficient and no factory is needed.
- **Singleton Pattern**: This design patterns uses an API of functions that do not take an object as parameter. This effectively helps one to hide the implementation inside the C file but unfortunately has the drawback of there only ever being one instance of the object to operate.
- **Bridge Pattern**: This design pattern involves creating an abstraction layer between two different components, allowing them to interact without being tightly coupled. This method means that we `extend` one object with additional methods defined in a separate C file exposing a separate API where the public headers of that API do not depend on the implementation of our specific objects being created. This means that the bridge API depends on accessing private data - but not our user code.

#### 5.10. Quiz

- 1. What is the main purpose of the factory design pattern?
  - Try to separate and hide implementation of creating objects.
  - Reuse construction code.
- 2. What are the key elements of the factory design pattern?
  - Create objects by parametric way.
  - factory, abstract object, CONTAINER_OF().
- 3. What are some benefits of using the factory design pattern?
  - Provide generic APIs to users so they can create objects easily.
  - Separate, de-couple mean easy to test, scale and maintain.
  - Data driven object -> init device tree model.
- 4. What are some examples of when the factory pattern is used in Zephyr? How are objects and devices initialized before main application is started?
  - Using macros to create object - register function pointers to tables with priority, the start-up code will look for this table and start calling init functions.
- 5. In what scenarios is the factory pattern useful for managing the creation and lifetime of objects?
  - Pooled factory when objects is managed by static array and generic APIs.
- 6. How can we easily have a pool of slots from which to allocate our objects and manage it in an object oriented (and type safe) manner?
  - Create an static array of slots, slots contain union data of object structures.
- 7. What are some of the PROS and CONS of the factory?
  - PROS: complexity.
- 8. WHat are some alternatives?
  - Object patterns is a best choice.
  - Bridge patterns to de-couple between components is another choice.

### 6. Callback pattern

- Object oriented callbacks implementation in C for embedded firmware.

#### 6.1. Definition

- **Object being observed**: This can be an opaque object (with hidden implementation) exposing only an interface to register callbacks for various events. Observers register callbacks with this object.
- **Callback receiver**: This is the code that executes the callback. One important consideration is that this function must have a way to retrieve callback owner state because from the perspective of the object being observed, we don't know about that is observing it - we only have the callback.
- **Full separation of concerns**: The object being observed is typically fully separated from the objects observing it. This is an important feature of this pattern because it allows clean separation of concerns.

#### 6.2. Use cases

- **A mechanism for notification**: the core concept behind this pattern is ability to register and receive notifications.
- **Used to `Observe` State Of Object**: This can be done in response to event notifications or directly through parameters of the callback.
- **An idiom for `One-To-Many` notifications**: It is not uncommon to have unlimited number of observers spread around your code. The pattern helps keep them independent of each other.
- **Used for patterns like the work queue**: When used together with a work queue, we can build observer of a work queue item which will execute a callback when a timeout expires. This is very powerful way to avoid unnecessary threads in embedded systems.

#### 6.3. Benefits

- 1. **Decoupling**: A callback can be represented by a structure making it the only data structure that needs to be shared between the observing code and the object being observed. This is important because we don't want our object being observed to have to depend on every single other that needs to receive notifications from it.

- 2. **Enables One-To-Many Notifications**: Since the only shared data structure is our callback structure, we can easily add features to this callback, such as the ability to arrange our callbacks into a list which in turn allows our object to send notifications to multiple receivers in sequence.

- 3. **Code reuse**: Callback listeners (observers) can be reused in multiple subjects, reducing the duplication of code.

- 4. **Scalability**: The callback pattern makes it easy to add new observers without affecting the subject or other observers (without the callback pattern, our subject would have to be directly dependent on API of all observers - which is clearly very bad).

- 5. **Flexibility**: The callback pattern provides a way to change the notification mechanism dynamically, making the system more flexibility.

- 6. **Ease of maintenance**: The observer pattern makes it easy to maintain the system, as it reduces the dependencies between objects and makes it easier to modify or extend the system. One of the biggest sources of maintenance drag in many C projects is `dependencies out of control`.

#### 6.4. Drawbacks

- **Performance overhead**: Delivering all of the notifications of a change to all listeners can result in a significant performance overhead, especially if there are many listeners.
- **Lockup**: If one observer/listener happens to block execution for whatever reason, all other listeners will not have the chance to run. Preferably, any action done as part of the response to a notification must make sure that it does not sleep (block) the execution in any significant way.
- **Complexity**: The callback pattern can increase the complexity of the system, as it requires implementing and maintaining the observer list and the notification mechanism. Luckily we can reuse ready made data structures for this which are available to us in Zephyr RTOS.
- **Tight coupling risk**: If not implemented carefully, the callback pattern can result in tight coupling between the subject and observers, making it difficult to change to extend the system. This is particularly a risk when the callback is implemented as a direct call instead of a function pointer (which is something that is easily identified by a trained embedded engineer).
- But can be very subtle for somebody not experienced enough.
- **Order of notifications**: There is typically no predictable sequence in which observers are notified. This can create problems if the listeners are designed in a way that does not allow them to perform actions independently.

#### 6.5. Implementation

```C
struct button_callback
{
    void (*cb)(struct button_callback *cb);
};
```

- Advantage #1: We can easily instantiate the callback itself - which works as a much cleaner alternative to typedef.
- Advantage #2: We can use pointers to the callback structure to get address of higher level context of module a instance which allows us to decouple A and B. This would be impossible if our callback was just a function pointer.

- Application Example: Button Header

```h
#include <zephyr/sys/slist.h>
#include <stddef.h>

struct button {
    /* Alternative 1: single callback. */
    struct button_callback *cb;

    /* Alternative 2: multiple callbacks. */
    sys_slist_t callbacks;
};

// This is a public callback definition.
struct button_callback
{
    sys_node_t node;
    void (*cb)(struct button_callback *cb);
};

void button_init(struct button *self);
void button_deinit(struct button *self);
void button_add_callback(struct button *self, struct button_callback *cb);
void button_remove_callback(struct button *self, struct button_callback *cb);
void button_do_something(struct button *self);
```

- Application Example: Button Implementation

```C
#include <button.h>
#include <zephyr/sys/slist.h>
#include <string.h>

void button_init(struct button *self)
{
    memset(self, 0, sizeof(*self));
}

void button_deinit(struct button *self) {
    self->cb = NULL;
}

void button_add_callback(struct button *self, struct button_callback *cb)
{
    self->cb = cb;
}

void button_delete_callback(struct button *self, struct button_callback *cb)
{
    self->cb = NULL;
}

void button_do_something(struct button *self)
{
    // Do something.

    // Notify another.
    if (self->cb)
    {
        self->cb->cb(self->cb);
    }
}
```

##### 6.5.1. Observer Implementation

```C
struct observer {
    /** Button Instance **/
    struct button *btn;

    /** Button Callback **/
    struct button_callback cb;  // Our callback.
};

static void observer_on_button_cb(struct button_callback *cb)
{
    // This is how we can now get pointer to instance of B
    // notice that the callback is generic.
    struct observer *self = CONTAINER_OF(cb, struct observer, cb);
    printk("Observer callback for button %p\n", self->btn);
}

void observer_init(struct observer *self, struct button *a)
{
    self->btn = a;
    self->cb.cb = observer_on_button_cb;
    button_add_callback(a, &self->cb);
}

void observer_deinit(struct observer *self)
{
    button_remove_callback(self->btn, &self->cb);
}
```

##### 6.5.2. Application usage

```C
struct controller ctrl;
struct button btn;


button_init(&btn);
controller_init(&ctrl, &btn);

/* This will call controller callback functions. */
button_do_something(&btn);

controller_deinit(&ctrl);
button_deinit(&btn);
```

##### 6.5.3. List of callbacks

- In case you want more lists of callbacks instead of only one.

```C
struct button {
    /* List of callbacks. */
    sys_slist_t callbacks;
}

struct button_callback {
    /* Callback list node. */
    sys_slist_t node;
    void (*cb)(struct button_callback *cb);
}

void button_add_callback(struct button *self, struct button_callback *cb)
{
    sys_slist_append(&self->callbacks, &cb->node);
}

void button_delete_callback(struct button *self, struct button_callback *cb)
{
    sys_slist_find_and_remove(&self->callbacks, &cb->node);
}

void button_do_something(struct button *self)
{
    sys_node_t *node;

    /* Call all the callbacks. */
    SYS_SLIST_FOR_EACH_NODE(&self->callbacks, node)
    {
        // once again we use container of here since we know node points to cb->node.
        struct button_callback *cb = CONTAINER_OF(node, struct button_callback, node);
        cb->cb(cb);
    }
}
```

#### 6.6. Best practices

- **Always use struct for the callback**: This gives context and most importantly ensures that we can use it easily together with the object pattern and make our architecture truly object oriented.
- **Always pass context to callback**: do not use `custom` context such as `user data` or some other thing you have invented just for passing data to the callback. Instead place the data either in the callback struct itself or in the enclosing object.
- **Avoid time consuming operations in callback**: It is better to defer operations to a dedicated work queue if they take longer time because a time consuming callback will stall all other callback handlers that are also waiting for the notification.
- **Use CONTAINER_OF**: Do not try to pass context as a separate `user data` variable or anything like that. The callback pattern makes these things unnecessary.

#### 6.7. Common Pitfalls

- **Callback going out of scope**: failing to unregister callbacks before destroying a callback observer is one of the bigger pitfalls of this pattern.
- **You make your data global**: this may look like a nice convenience, but it completely violates the data flow through the code. If you for instance make the callback operate on global data, you have not only messed up the data flow for one listener - but for all listeners by not passing a pointer to a structure that holds the callback.
- **Inventing `user-data` passing**: whenever I see some variation of `void * user_data` in a structure or callback, I know that the developer does not really understand how to property implement the observer pattern. Remember: you do not need to store pointer to user data at all. You get access to user data using `CONTAINER_OF` macro.
- **Life cycle of objects**: If you not pay close attention to when a stack allocated listener goes out of scope. you may run into very weird problems caused by garbage pointers (due to the fact that the object no longer exists but it still referenced from the list of callbacks!). So pay close attention to where you data resides. Of course, if you use the object pattern, you will have a pretty good order in your data.

#### 6.8. Alternatives

- **Event Bus Pattern**: This pattern is slightly different from observer in that we have a global event `queue` (observer/callback pattern has NO event queue) that stores events in transit. This pattern is used to implement conventional `publish/subscribe` mechanisms at small scale in C programs. Both observer and the observed become decoupled.
- **MVC (Model-View-Controller) Pattern**: This pattern is a derivation of the callback pattern in that the controller acts as an adapter between the model and the view. The controller uses the model to represent state of the program and registers callbacks with the visual representation to respond to events that are necessary for smooth visual rendering.
- **Command Request Pattern**: This pattern uses a command request as central way to pass an event. It is different from the event pattern in that the request itself can be queued and the object that queues the event does not need to know about the type of object that receives the event.

#### 6.9. Conclusion

#### 6.10. Quiz

- 1. What is main problem that is being resolved by using the callback mechanism?
  - Decoupling observer and observed object. And we can have many listeners for one speaker.
- 2. How does the observer/callback pattern help maintain object oriented code organization in C?
  - Listening for object property changes.
  - From the `cb` argument we can get the original struct -> take more private data for custom callbacks.
  - Making sure that we have a type safe way to invoke a callback that then operates on a particular instance of another object.
- 3. What would happen if you delivered the event using a direct call instead? what implications would it have for structure of your code?
  - The code is coupled -> observed object depends on observer object.
- 4. Why are we always enclosing our callback in a struct? How would it negatively impact our design if we did not do this?
  - struct to easy edit properties without changing API.
  - We can change any callback in the structure without change the observed object.
  - Easy to get enclosing structure with CONTAINER_OF, and get more data from that.
- 5. Why do we use `CONTAINER_OF` to get our context and not just make the context global or pass it directly from the subject to the callback?
  - global means need to protect. Hard to see data flow.
  - pass direct means we can not have generic APIs for all callbacks. CONTAINER_OF help us to design a generic API with many behaviors.

### 7. Inheritance Pattern

#### 7.1. Defining characteristics

- **Containment of base class**: our derived class simply contains data of its base class and uses base class methods to operate on that data. Derived object never references the base class data through a pointer.
- **Derived object tightly coupled to base data**: The derived class directly access base class data even though we always try to use base class methods to manipulate base class, the inheritance pattern doesn't forbid direct access to base class data.
- **Derived object must provide method wrappers**: If we want to manipulate base class data then we must always explicitly define derived object methods that would then either delegate this operation to base class method or modify the data directly. We never modify base class data directly.

#### 7.2. Use cases

- **Code reuse**: Whenever we need to `extend` the functionality of a base object, we can wrap it into a derived class and a derived interface around the base object - but we can still reuse the code that is part of the base object implementation.
- **Generic data structures**: We use the inheritance pattern to create generic data structures such as lists in C. This allows us to reuse list manipulation functionality and even organize arbitrary structures into lists (as we have seen with the callback pattern) by simply `inheriting` from a list node. We can do the same when we implement many other data structures.
- **Extending other objects**: through code reuse, we can create new data structures that have extended functionality which underneath use a base data structure but expose to the user a far richer interface than the original data structure.

#### 7.3. Benefits

- **Clean Design**: When we reuse the same functionality in many places it becomes easy to know what to expect when looking at the code. The opposite of this is re-implementing for example, the same list algorithms throughout many different objects in the firmware objects in the firmware - which leads to never knowing what to expect from such as implementation.
- **Clear expectation**: We can think of our code much hierarchically and humans are far better at grasping hierarchical concepts than concepts that have many small details that need to be considered at the same time. If we structure our data hierarchically, we also create a software architecture that is very easy to grasp.
- **Simple high level architecture**: through inheritance we can compartmentalize the logic at different levels of abstraction, leading to a much simpler design at the application level because we only need to deal with high level abstractions at that level.

#### 7.4. Drawbacks

- **Unnecessary code when wrapping methods**: If we want to fully inherit a data structure and expose a full interface to the user of that structure, we must also create wrappers for the base class methods in order to be able to use the structure in a clean way.
- **Violation of encapsulation**: Inheritance by definition means that all the data of the base class automatically belongs to and can be modified by the deriving class. If the derived class directly accesses data of the base class then this can have unintended effects.
- **Added complexity**: This is particularly true for virtual inheritance. Without automatic language features it becomes harder to keep things clean and consistent across the whole project because it places the responsibility on developers to follow design patterns.

#### 7.5. Implementation

- Very simple inheriting objects: `struct my_object` inherits two structures `struct base_one` and `struct base_two`.

```C
struct my_object {
    struct base_one base_one;
    struct base_two base_two;
};
```

##### 7.5.1. Implementation: List Item type example

```C
struct _snode
{
    struct _snode *next;
}

typedef struct _snode sys_snode_t;

typedef _slist {
    sys_node_t *head;
    sys_node_t *tail;
};


typedef struct _slist sys_slist_t;
```

- In the application code, we can inherit list item by simply embed the `sys_node_t` to user structure:

```C
struct my_object
{
    sys_node_t node;
};

sys_slist_t list;
struct my_object obj1;
struct my_object obj2;

sys_slist_init(&list);
sys_slist_append(&list, &obj1.node);
sys_slist_append(&list, &obj2.node);
```

- So from now we can treat `my_object` like a list item.
- Iterating the list:

```C
sys_snode_t *node;
SYS_SLIST_FOR_EACH_NODE(&list, node)
{
    struct my_object *self = CONTAINER_OF(node, struct my_object, node);
    //...
}
```

##### 7.5.2. Implementation: Multiple inheritance

- We have two base classes:

```C
struct base_one
{
    uint32_t x;
};

struct base_two
{
    uint32_t y;
};

void base_one_init(struct base_one *self)
{
    memset(self, 0, sizeof(*self));
}

void base_one_do_something(struct base_one *self)
{

}

void base_two_init(struct base_one *self)
{
    memset(self, 0, sizeof(*self));
}

```

- And in the derived classes:

```C
struct derived
{
    struct base_one base_one;
    struct base_two base_two;
}

void derived_init(struct derived *self)
{
    base_one_init(&self->base_one);
    base_two_init(&self->base_two);
}

void derived_do_something(struct derived *self)
{
    base_one_do_something(&self->base_one);
}

void main(void)
{
    struct derived derived;
    derived_init(&derived);
    derived_do_something(&derived);
}
```

##### 7.5.3. Implementation: Traits and behaviors

- Trait basically is a structure that do a specific characteristic of object.

```C
struct trait_x
{
    void (*do_x)(struct trait_x *self);
}

struct trait_y
{
    void (*do_y)(struct trait_y *self);
}

static inline void trait_x_do(struct trait_x *self)
{
    self->do_x(self);
}

static inline void trait_y_do(struct trait_y *self)
{
    self->do_y(self);
}
```

##### 7.5.4. Implementation: Deriving Traits

```C
struct derived_with_traits
{
    struct trait_x trait_x;
    struct trait_y trait_y;
};

static void derived_with_traits_do_x(struct trait_x *trait)
{
    struct derived_with_traits *self = CONTAINER_OF(trait, struct derived_with_traits, trait_x);
    // So we can get derived class here with a base API.
}

static void derived_with_traits_do_y(struct trait_y *trait)
{
    struct derived_with_traits *self = CONTAINER_OF(trait, struct derived_with_traits, trait_y);
    // So we can get derived class here with a base API.
}

void derived_with_traits_init(struct derived_with_traits *self)
{
    self-> trait_x = (struct trait_x) {
        .do_x = derived_with_traits_do_x
    };

    self-> trait_y = (struct trait_y) {
        .do_y = derived_with_traits_do_y
    };
}
```

- Using trait objects:

```C
void main(void)
{
    struct derived_with_traits dwt;
    derived_with_traits_init(&dwt);

    struct trait_x *dwx = &dwt.trait_x;
    struct trait_y *dwy = &dwt.trait_y;
    trait_x_do(dwx);
    trait_y_do(dwy);
}
```

#### 7.6. Best practices

- **Always use Object Pattern**: This is a major prerequisite for being able to further clean up your architecture using inheritance. If your application is not currently using object pattern for organizing all variables then you have a hard time implementing clean inheritance as well.

- **Understand the role of inheritance**: The primary role of inheritance is code reuse. The secondary role is to keep your functionality hierarchical so that it is easier to maintain.

- **Use delegator methods**: When you need to access some data or functionality in the base class and this property can be considered a property of the derived class as well, create a derived class method and then simply delegate to the base class.

- **Trait callbacks always static**: Always implement trait callbacks as static functions inside the implementation C file. This ensures that you not forget to update them when you make changes to your implementation.

#### 7.7. Common pitfalls

- **Fragile base class**: If changes to the base class alter the behavior of the derived class then it becomes specially important that you thoroughly verify the expectations you place on the derived class.

- **Code duplication**: If you have base class methods and then you implement lots of wrappers in the derived class, you end up with many functions that just forward the call to another method of the derived class. This is essentially dead weight.

- **Trying to implement inheritance before Object Pattern**: This is worth repeating multiple times: use the Object Pattern as much as possible and it is likely that the proper inheritance relationships will come into view naturally.

#### 7.8. Alternatives

- **Improving the base class**: sometimes it is better to simply add additional functionality to the base class rather than try to create a derived class. Inheritance in C should be used primarily in cases where you would like to combine several functionalities in a higher level object.
- **Virtual API Pattern**: This pattern deals with fully abstract interfaces. The difference is that an abstract interface is a shared data structure that is always constant and shared between all instances of an object while a trait is a structure contained in each instance where the function pointers are not necessarily always constant. Virtual API is a more robust and memory efficient way to implement pure virtual functions in C.
- **Facade Pattern**: Since C language doesn't naturally provide a syntax for separating inheritance from composition, many patterns look similar in their implementation. A pattern where an object inherits functions and then delegates the calls to a base class may look very similar to a facade. The difference is in the intent (the intent of a Facade is to simplify the interface which is not the same as the intent of direct delegation).

#### 7.9. Quiz

- 1. What are the main reasons for using inheritance in your software architecture?
  - **Re-use** source code of base object, can make various derived objects.
- 2. How is multiple inheritance pattern implemented in C and what naming convention is it good to use when naming your methods to make this easier to maintain?
  - Include more objects in the derived class, define more wrapper APIs, traits and behaviors.
- 3. What are the benefits of using traits and how do they relate to multiple inheritance?
  - Flex behavior of objects.
  - Trait object just contains function pointers that represent operations that corresponding to a particular `trait`.
- 4. What main drawback do traits have when they embedded into the derived struct compared to pure virtual interfaces which are constant and only referenced from the derived struct though a pointer to the interface?
  - More wrapper functions.
  - Don't have generic interface.

### 8. Virtual API Pattern

- We have a way to implement polymorphism.
- Make sure we have type-safe and memory efficiency way.

#### 8.1. Defining characteristics

- **Constant trait objects and methods that operate on them**: These are our generic virtual functions which can be implemented in derived classes. These correspond to `Abstract Interface` in languages like C++.
- **Implementation of abstract interface**: any type that implements a particular abstract interface, always does so by implementing the virtual functions in the trait that these types inherit.
- **Traits are stored as pointers in derived class**: The trait object is always constant and is stored as a pointer. It is also passed as pointer to pointer type to all trait callbacks. This is mainly to make sure that we can scale to many instances without duplicating the function pointer table across all instances.

#### 8.2. Use cases

- **Device Drivers**: This pattern is useful when you have many different implementations of essentially the same interface - such as for example a UART. Thus it is very often used for implementing many different device drivers that all implement the same API and to be able to treat all implementations just the same (polymorphism).

- **Plugins**: One very good example for this pattern is when you load shared library and call some predefined function inside it which then returns an abstract interface. Plugins use this pattern a lot. This interface will continue to work even if the implementation changes since the trait definition is the only shared dependency.

- **Abstract data types**: any time we want to treat a collection of same or different objects just the same, we can use the abstract API pattern to implement an interface for interacting with these objects. Each implementation then needs to implement this interface in order to comply with the expectations of any code that would then use this object generically.

#### 8.3. Benefits

- **Decoupling**: Since the interface is very lightweight and consists only of function pointers, it almost completely decouples user from the implementation. If we then also use the opaque pattern, we achieve full interface decoupling.

- **Type safety**: One of the biggest benefits of implementing abstract interfaces as described in this training module is that it is completed type safe. There are no `void *` pointers used and no casting that can potentially lead to incorrect results. Instead we use `CONTAINER_OF` and keep our callback data structure always constant from compile time.

- **Opaque handles**: The user deals only with opaque handles representing the abstract API and does not need to call contained functions directly. This means that user can not accidentally modify the content of this handle because there is no visible data structure for it. The data structure is only visible to implementation of generic API and the multitude of implementations of the abstract interface.

#### 8.4. Drawbacks

- **Complexity**: This pattern is more complex to implement than the traits pattern we looked at in the `inheritance` module. It requires the programmer to follow a precise model of implementation without which the pattern would lack key functionality.

- **Inflexibility**: One of the key factors of this pattern is that the interface must be constant and relatively generic. If your objects are not sufficiently describable by a generic interface then it is possible that you may need multiple different interfaces in parallel.

- **Performance overhead**: Although this overhead is very small, there is the overhead to always be calling the interface methods through two indirections. First one is when we call the generic wrapper method and the second indirection is when that method calls our implementation. However it is worth noting that this overhead is tiny compared to everything you are likely to do as part of the implementation method. Therefore it is usually a fair price for the improved flexibility.

#### 8.5. Implementation

- **Generic API**: This part is exposed outside of your library. It does not include any data structures at all. All the member functions of this interface operate on pointer to pointer of the API data structure meaning that the data structure it self only needs to be declared but does not need to be defined in the public interface.

- **API Interface**: This is similar to how we previously implemented traits in the chapter on inheritance. The difference is that the API data structure is only shared between the generic API and the implementations of this interface.

- **Implementation**: This is the concrete implementation. It only depends on the shared API and all other aspects of the implementation are hidden. You will have to use Opaque Pattern here or have some other way to register the abstract interface pointer so that the application can query it (for example, by maintaining a list of all implementations indexed by a string or an enum - or through dynamic allocation).

##### 8.5.1. Trait definition

- In the `serial_ops.h`:

```h
/* Serial API. */
struct serial_ops {
    int (*write)(const struct serial_ops ** handle, const char *data, size_t size);
    int (*read)(const struct serial_ops ** handle, char *data, size_t size);
}

/* You can typedef it like this if you want. */
/* typedef const struct serial_ops ** serial_t; */
```

##### 8.5.2. Generic Trait API

- We will only `serial.h` to the customer:

```h
struct serial_ops;
int serial_write(const struct serial_ops **handle, const char *data, size_t size);
int serial_read(const struct serial_ops **handle, const *data, size_t size);
```

- And here is our implementation for generic APIs `serial.c`:

```C
int serial_write(const struct serial_ops **handle, const char *data, size_t size)
{ // This method just wrap: (*handle)->write(), so we can avoid expose and call it everywhere in the application code.
    /* Here we do have access to the API so we can use it directly. */
    return (*handle)->write(handle, data, size);
}

int serial_read(const struct serial_ops ** handle, char *data, size_t size)
{
    /* Here we do have access to the API so we can use it directly. */
    return (*handle)->read(handle, data, size);
}
```

##### 8.5.3. Implementation header

- Now we implement variants from base generic APIs. This is specific for implementations. For example we can have many types like: stm32 uart 1, etc. We also provide it with generic API header to the customer.

- A header follow the format:

```h
struct serial_impl {
    /* Pointer to base operation. */
    const struct serial_ops *serial;

    /* More private data here. */
};

void serial_impl_init(struct serial_impl *self);
const struct serial_ops ** serial_impl_to_serial(struct serial_impl *self);
```

##### 8.5.4. API Implementation

```C
#include "serial_ops.h"
static serial_impl_write(const struct serial_ops ** handle, const char *data, size_t size)
{
    // We here can resolve concrete type in the usual type safe way.
    struct serial *self = CONTAINER_OF(handle, struct serial_impl, serial);

    return size;
}

static serial_impl_read(const struct serial_ops ** handle, char *data, size_t size)
{
    // We here can resolve concrete type in the usual type safe way.
    struct serial *self = CONTAINER_OF(handle, struct serial_impl, serial);

    return size;
}

static const struct serial_ops _ops = {
    .write = serial_impl_write,
    .read = serial_impl_read
}

void serial_impl_init(struct serial_impl *self)
{
    memset(self, 0, sizeof(*self));
    self->serial = &_ops;
}


const struct serial_ops ** serial_impl_to_serial(struct serial_impl *self)
{
    return &self->serial;
}
```

##### 8.5.5. Usage: Application code

```C
// Initialization of implementation (usually would be done separately from usage below).
struct serial_impl serial;
serial_impl_init(serial);


// Get the handle that can be passed around and used with generic API.
const struct serial_ops ** handle = serial_impl_to_serial(&serial);
const char *data = "Hello world\n";
serial_write(handle, data, strlen(data));
```

#### 8.6. Best practices

- **Trait ops must be thoroughly defined**: Every implementation must follow clear implementation guidelines and the behavior of each implementation should be the same.
- **Typedef the handle type for simplicity**: since the handle type actually declares a new type and is used repetitively in many places it is good to typedef it. You can make a typedef exception for this type and add it to your typedef file.
- **Keep your traits constant**: this pattern relies on the interfaces rarely changing so you should try to think ahead when designing the interface and then keep it constant. Any changes to the expected behavior of the interface itself will likely require substantial changes to all implementations.

#### 8.7. Common pitfalls

- **Overusing abstract interfaces**: If everything in your software is done through an abstract interface, you will have interfaces that need to be updated often and updating the interface requires reworking parts of the implementations. So you should reserve use of abstract interfaces only for places where polymorphism makes sense (ie. treating many items using the same set of actions).
- **Insufficient testing**: The full set of interface requirements must always be verified using unit tests for each implementation of that interface. Not doing this will lead to missed bugs that occur when you update the interface but fail to update all implementations.
- **Overly specific interfaces**: The interface should capture generic functions of a large category of objects. You should design your interface in such a way that you rarely experience situations where you need to write dummy methods for an interface because some operation of the interface does not make sense for your implementation.
- **Inconsistent implementation of virtual API pattern**: it is important that you apply this pattern consistently and always follow the same implementation approach.

#### 8.8. Alternatives

- **Callback pattern**: The callback pattern is a simpler version of the abstract interface where instead of defining a generic API we work with lists of callbacks.
- **Event Bus Pattern**: An event bus can implement a data driven abstract interface where both the function name and data can be encoded into an event and published on the event bus. This pattern is much more complex than the abstract interface pattern and often more suitable for many-to-many publish subscribe scenarios.

#### 8.9. Quiz

- 1. How does the Virtual API pattern differ from the trait objects described in the inheritance Pattern?
  - We hidden the trait behavior internally.
  - Virtual API define same API for all instance and make pointers are fixed.
- 2. How does defining our API handles as pointer to pointer help use stay memory efficient, particularly with large numbers of instances?
  - pointer is small.
  - We have a lot of constant API handle per instance.
- 3. Why is it so important that the virtual API does not contain any implementation specific details at all?
  - Because this is generic for everyone, so, it should be small and reusability.
- 4. Why do we need to have a generic API along side of our trait objects?
  - Virtual API allow us to generic an API and hides all implementations of the trait object itself.

### 9. Bridge Pattern

- Connecting two independent hierarchies of objects.

#### 9.1. Defining Characteristics

- **Decoupling of abstractions and implementations**: This is done both at the interface towards the user, and at the point where the controlling implementation interfaces with the object being controlled. Both interfaces must be abstract interfaces (i.e. must use the Virtual API Pattern).
- **Data flow through a `bridge`**: Classes are extended by controlling an interface on one end of the bridge that controls an implementation on the other end of the bridge. Hence being called the `bridge` pattern.
- **Two distinct hierarchies**: The pattern is most clear when you can see two different hierarchies that are loosely coupled over a `bridge` through which data flows. The loosely coupled hierarchies are a positive consequence of the bridge pattern and in fact what the bridge pattern is trying to achieve in the first place.

#### 9.2. Uses cases

- **Abstraction and implementation need to vary**: The Bridge Design Pattern is ideal when the requirements for the abstraction and implementation are likely to change independently. The beauty of a virtual method table is that we can pass it across DLL boundaries. This allows us to define the implementation in one compilation unit, export the abstraction and then have another implementation use this abstraction in another compilation unit (shared library).

- **Multiple implementations**: The bridge design pattern is also ideal when there are multiple implementations possible for a single abstraction and the implementation needs to be chosen dynamically at runtime. A good example of this is the old Quake3 engine where the engine itself could evolve separately from the game dll and the game was calling into the engine using an exported abstract interface.

- **Abstracted Access to Hardware or Platform Services**: The Bridge Design Pattern is ideal when the system needs to access hardware or platform services, as it provides a way to abstract these services from the rest of the system and allow for different implementations to be used on different platforms. An example of this use case is the **Linux System Call interface** and the corresponding C library that provides an abstraction that is actually seen by the application.

- **Plug-In Architecture**: The Bridge Design Pattern is ideal when the system needs to support plug-ins or add-ons, as it provides a way to create a plug-in architecture that allows for new implementations to be added to the system without affecting the rest of the system. The old Quake games were actually implemented as `plugins` to the game engine which was compiled as the main executable and the game itself was a dynamic library.

#### 9.3. Benefits

- **Bridge between two object hierarchies**: You have two representations of complex data structures with a loosely coupled interface between them.
- **Dependency separation**: The concrete implementation of one hierarchy does not need to pull in dependencies of the other hierarchy (example: in a game where every object has a physics body and a mesh body, you can separate these two hierarchies and implement bridge pattern between them to keep them separate and possibly even on different machines).
- **Separation of concerns**: enable two hierarchies to be created representing different parts of the same system while allowing each hierarchy to evolve independently in terms of implementation.

#### 9.4. Drawbacks

- **Increased complexity**: Instead of just a single hierarchy and mixed dependencies you have to maintain two separate hierarchies and links between them.
- **Additional indirection**: Since each operation involves calls through multiple interfaces, there is a potential with a lot of very `light` methods which simply make another call.
- **Increased development time**: Due to additional data structures and classes that have to be implemented.

#### 9.5. Implementations

- We gonna use an example:
- **Client/Server**: We will be creating two local hierarchies of objects where one represents the client structures and the other represents server structures. We will use the example of physics and drawable objects.
- **Two hierarchies**: Client hierarchy handles visual representation while server hierarchy handles physical behaviors. We will implement both in Rust and C.
- **Client: Drawable**: the objects on client side implement `Drawable` interface which we will define.
- **Server: PhysicsObject**: The objects on the server side implement `PhysicsObject` interface.
- **User interacts with client objects**: finally we will allow the user to interact with client side instances and the bridge will connect these to the server side instance.

##### 9.5.1. Implementation: Rust: Server side objects

```rust
pub struct Circle {

}

pub struct Square {

}

pub struct Point2D (f32, f32);

impl Circle {
    pub fn new() -> Self {
        return Self {};
    }
}

impl Square {
    pub fn new() -> Self {
        return Self {};
    }
}
```

##### 9.5.2. Implementation: Rust: Client Interface

- Physics object interface:

```Rust

/* Provide this abstract API to user. */
pub trait PhysicsObject {
    fn position(&self) -> Point2D;
}

/* Client have two different virtual method for Circle and Square. */
impl PhysicsObject for Circle {
    fn position(&self) -> Point2D {
        printk("server: get position for circle\n");
        return Point2D(1.0, 1.0);
    }
}

impl PhysicsObject for Square {
    fn position(&self) -> Point2D {
        printk("server: get position for square\n");
        return Point2D(1.0, 1.0);
    }
}
```

##### 9.5.3. Implementation: Rust: Client Side Object

```Rust
pub struct Circle<P: server::PhysicsObject> {
    physics: P,
}

pub struct Square<P: server::PhysicsObject> {
    physics: P,
}

impl<P: server::PhysicsObject> Circle<P> {
    pub fn new(p: P) -> Self {
        return Self { Physics: p };
    }
}

impl<P: server::PhysicsObject> Square<P> {
    pub fn new(p: P) -> Self {
        return Self { Physics: p };
    }
}
```

##### 9.5.4. Implementation: Rust: Client Side drawable interface

```Rust
pub trait Drawable {
    fn draw(&self);
}

impl<P: server::PhysicsObject> Drawable for Circle<P> {
    fn draw(&self) {
        let _p = self.physics.position();
        printk("Client: drawing circle at position\n");
    }
}

impl<P: server::PhysicsObject> Drawable for Square<P> {
    fn draw(&self) {
        let _p = self.physics.position();
        printk("Client: drawing square at position\n");
    }
}

```

##### 9.5.5. Implementation: Rust: Application code

```Rust
use client::{Drawable};
let circle = client::Circle::new(server::Circle::new());
let square = client::Circle::new(server::Square::new());
circle.draw();
square.draw();
```

##### 9.5.6. Implementation: C

```C
struct point2d {
    uint32_t x, y;
}
```

```C
struct physics_object_ops {
    void (*position)(const struct physics_object_ops **ops, struct point2d *out);
}

typedef const struct physics_object_ops ** physics_object_t;

static inline void physics_object_position(const struct physics_object_ops **ops, struct point2d *out)
{
    (*ops)->position(ops, out);
}
```

##### 9.5.7. Implementation: C : Server side objects

```C
struct server_circle {
    struct point2d pos;
    const struct physics_object_ops *ops;
};

static void server_circle_position(const struct physics_object_ops **ops, struct point2d *out)
{
    struct server_circle *server = CONTAINER_OF(ops, struct server_circle, ops);
    printk("Server get position for circle.");
}

static const struct physics_object_ops _ops = {
    .position = server_circle_position
}

void server_circle_init(struct server_circle *self)
{
    memset(self, 0, sizeof(*self));
    self->ops = &circle_ops;
}
```

- And similar implementation for square.

##### 9.5.7. Implementation: C : Drawable interface

```C
struct drawable_ops {
    void (*draw)(const struct drawable_ops **ops);
}

typedef const struct drawable_ops ** drawable_t;

static inline void drawable_draw(const struct drawable_ops **ops) {
    (*ops)->draw(ops);
}
```

##### 9.5.8. Implementation: C : Client side objects

```C
struct client_circle {
    physics_object_t server_object;
    const struct drawable_ops *drawable;
};

void client_circle_draw(drawable_t ops)
{
    struct client_circle *self = CONTAINER_OF(ops, struct client_circle, drawable);
    struct point2d pos = {0};
    physics_object_position(self->server_object, &ops);
    printk("CLient: draw circle at position\n");
}

static const struct drawable_ops _ops = {
    .draw = client_circle_draw
}

void client_circle_init(struct client_circle *self, physics_object_t server_object)
{
    memset(self, 0, sizeof(*self));
    self->server_object = server_object;
    self->drawable = &_ops;
}
```

- And similar implementation for square.

##### 9.5.9. Implementation: C : Usage

```C
struct server_circle s_circle;
struct server_square s_square;
struct client_circle circle;
struct client_square square;


server_circle_init(&s_circle);
server_square_init(&s_square);
client_circle_init(&circle, &s_circle.ops);
client_square_init(&square, &s_square.ops);

drawable_draw(&circle.drawable);
drawable_draw(&square.drawable);
```

#### 9.6. Best practices

- **Identify component hierarchies that need separation**: These should be largely independent hierarchies where the bridge pattern can truly simplify the interactions with objects on one side of the bridge.
- **Define clear interfaces**: Since every interaction will pass through the abstract interfaces, it is important to have a well defined set of interfaces that will be implemented on the other side.
- **Thorough testing**: encode all requirements into integration and system tests and make sure that you verify that the whole chain of command works as expected from one side of the bridge to the other.

#### 9.7. Common pitfalls

- **Over-engineering**: The bridge pattern should be used sparing and only when it is truly necessary to achieve complete separation of concerns, such as in the client server architecture scenarios.
- **Complex interfaces**: the interface should be designed with the idea of providing `means, not policy`. The interface should be simple and generic and provide means of communication with implementations on the other side of the bridge without enforcing too many rules.

#### 9.8. Alternatives

- **Adapter pattern**: more lightweight and it's primary use is to alter the interface of a class to suit user's needs. It's not a complete bridge thought and so lacks the ability to fully separate two hierarchies of objects.
- **Strategy Pattern**: This pattern focuses on breaking out behaviors of objects and making them interchangeable. It is useful when you need to only have interchangeable behaviors where an object can be configured to use one behavior strategy or another. It can be considered as an alternative to the Bridge pattern when the implementation of an algorithm is the primary concern.

#### 9.9. Quiz

- 1. What is the main defining characteristic of the bridge pattern?
  - Separate abstract interface code and implementation code by adding an hierarchy in both side to communicate (like libc and kernel, app and shared lib, two different machines via network).
- 2. How does the bridge pattern decouple abstractions and implementations?
  - Implement more hierarchy in abstraction side along with the hierarchy in implementations side.
- 3. What is the role of abstraction in the bridge pattern?
  - Can work separately and provide interface to client, and communicate with implementation via the bridge.
- 4. What is the different bw bridge and abstract interface?
  - Bridge require more step: add hierarchy an abstraction side to communicate with hierarchy in the implementation side.
- 5. How can you use this pattern to simplify the sw architecture of your current project?
  - Separate implementations and abstract interface.

## IV. Behavior pattern

### 10. Return value pattern

- Standardize status reporting from functions in C.
- The C don't have exception features so we cannot emit and catch particular case.

#### 10.1. Definition

- **Signed integer return value**: We use standard way of reporting status by returning a status value as a signed int in C.
- **Zero means no error**: A zero return value shall always mean that there was no status to return - meaning that the operation has been successful.
- **Negative value means error**: A negative status means that something negative has happened and the value of the negative number that is returned shall specify what error has occurred.
- **Positive value means status**: a positive status means that the operation was at least partially successful and the return code shall be function specific success return code. For functions that read and write data, this value could for example mean number of items successfully written or not.

#### 10.2. Use cases

- **Every non-trivial function**: standardized return values should be used at all times when a function does any operation that may fail or which can produce different status.
- **Every function that can fail**: Any function that fail must return a negative error code upon failure condition.
- **Every function that has status to report**: Any function that may succeed in at least two ways.

#### 10.3. Benefits

- **Standardized expectations**: We can have standard expectations about every function that returns a status code. We don't need to open documentation each time.
- **Clean code**: Our code can be clean and easy to understand.

#### 10.4. Drawbacks

- **Pressure on programmers**: This pattern puts extra pressure on programmer to write clean code. Return values are often not understood and much less often checked. If you are using tools like PC-Lint you will be inclined to check all return values because otherwise it would be a lint error - but for programmers who are not used to top quality programming this can be a difficult pattern to follow.
- **Longer code**: since we always check return values we need additional `if` statements and blocks which make our code longer. There is no good way around this.
- **Limited level of detail**: Sometimes we have to return status codes that do not accurately describe the actual error since status codes are generic. However, in most cases this is not an issue we simply treat a negative code as a generic error code.

#### 10.5. Implementation

- **Every non-trivial function must return integer status code**: C does not support exceptions with stack unwinding like C++ so the easiest way is to pass status code through return value.
- **Return values must be checked**: Every call of a function that has a return value must be checked when the function is called.
- **Error cods are always constant**: upon getting an error code, we try to handle the error if we can not handle it we always return a well defined error code to the caller.

```C
int my_object_method(struct my_object *self)
{
    if (there_is_error)
    {
        return -ERNOCODE;
    }

    return 0;
}
```

#### 10.6. Error code convention

- **Negative error codes correspond to ERRNO codes**: we always use the `errno.h` for negative error codes.
- **Zero always means success**: returning a zero status code always means operation completed without issues.
- **Positive error codes are function specific**: The meaning of positive values must always be documented in documentation of the function that returns them.

#### 10.7. Best practice

- **Always check return codes**: not doing so must be configured to be an error in your lint setup.
- **Always return errno**: If your function fails then return a negative value using one of the predefined `errno` codes.
- **Error status always well defined**: If a function that your function calls fails then return a negative errno code that best describes the state of your object but not directly return the return code of the function you have called. Instead return an error code that is constant for your function.
- **Zero means success**: If your function succeeds and does not require reporting partial success then always return 0.
- **Partial success always uses positive numbers**: If your function requires partial success indication (such as number of items processed) then return this number as a positive integer.
- **Use parameters for complex return values**: If you need to return a complex return value then simply return zero to indicate success and write this complex value into a structure pointed to by one of your function's parameters. Do not return complex structures as return values.

#### 10.8. Pitfalls

- **No clear definition of standard return values**: The most serious pitfall with return values is not defining their meaning. If every function returns values that can be potentially mixed up with other values then you can never really know what to expect. This is why we have adopted `errno` codes as standard way to indicate errors - because they are defined as macros and they are ubiquitous - they are available everywhere - they are standardized. This means we always can deduce what the problem may be looking at the `errno` code.

- **Return structs**: When you need to return a struct, it is better to pass it as a parameter and them simply return an integer status code signaling whether the struct parameter was filled with valid data or not.

#### 10.9. Alternatives

- **Exceptions**: This is only possible in CPP or Rust. In C, return code is the only way to pass exceptions back to the caller.
- **Logging**: another way to handle status by generating a log message. You should combine this with status code.
- **Panic**: an assertion should be used for fatal errors and the application should call system panic function which in turn should reboot the device after logging the reason.
- **Long jump**: switch context during an exception and returning to an arbitrary place in the application where a corresponding `setjump` call was made. Don't use it except you know what exactly you wanna do.

#### 10.10. Quiz

- 1. What status code should return upon success when there is no more information that needs to be conveyed?
  - 0.
- 2. What status code function should return on failure?
  - negative number that follow errno.
- 3. Why is it not a good idea to forward a return status from function you have called and return it without changes?
  - No one want to handle them?
  - Make the error code non-constant.
- 4. When should you return a positive status code?
  - number of affected items.

## V. Concurrency Pattern

### 11. Concurrency Pattern

- The pattern we do when we want to control what each CPU do next.

### 11.1. Defining characteristics

- **Scheduler**: this is the code that has a higher level view over what the CPU is doing and is able to direct the CPU to run any other portion of the software.
- **Hardware Support**: The scheduler almost always relies on hardware timers to drive it. The hardware is a very important aspect in concurrency which must be taken into account since at the core it is always the hardware that makes scheduling possible.
- **Synchronization**: This consists of software constructs that can be used in the application to control what the scheduler will do next. These consist of spinlock, semaphores, mutexes, condition variables and other higher level concepts.

### 11.2. Use Cases

- **Parking CPU while waiting for IO**: If our calculation requires an IO value that is not available yet (availability can be signalled using an electrical signal). In such a scenario it is a good idea to load the CPU with a different state and let other calculations happen while we are waiting.

- **Concurrent hardware operations**: Each peripheral on a SoC is an independent hardware unit driven by a shared clock. This hardware unit can do things independently of other hardware units. However, to make use of this functionality we need a way to jump between multiple code paths that execute operations in sequence on each piece of hardware. We need concurrency in order to do this.

- **Multiple CPU cores**: We need concurrency in software when we have multiple cores that can access the same peripherals (or memory) and we need ways to synchronize this access in software.

- **Real-time timing constraints**: We need our software to respond to events very quickly and we want to be able to start complex chains of software events when a hardware occurs. We need concurrency in order to switch contexts quickly in response to hardware events without the need of running some code to completion.

### 11.3. Benefits

- **Low power consumption with low CPU load**: CPU load is measured by looking at how much time the CPU spends in `wait for event` state. This state is entered by executing a CPU instruction (WFE/WFI) which halts the CPU and wait for an interrupt or some kind of event. Many CPUs also have power management functionality where you configure what low power mode the CPU can enter into once it executes the WFE instruction. This is impossible to achieve when your application must poll for changes. Concurrency eliminates need for polling and your application can be event based.

- **Separation of tasks by time domains**: Applications often consist of sequential tasks which involve a lot of waiting for IO. We can of course make our code into a state machine (making the sequential task much less obvious) or we can decide to keep our code sequential but separate the different sequences of operations into multiple threads. Concurrency shows us how to do it properly. A time domain is simply a sequences of actions with delays in between that must be executed as a single block. Delays typically occur naturally by having to wait for IO to complete.
- **Basis for many other patterns**: If we want to implement an event processor or async/await pattern in our code, this becomes a lot easier if we have a scheduler that we can use for separating one state machine completely from another and running these step machines as two separate threads.
- **Timeouts and repetitive tasks**: Hardware only has a limited set of timers. we need concurrency to have support for flexible and unlimited delayed tasks. We need concurrency patterns in order to implement these things properly along side of interrupts.

### 11.4. Drawbacks

- **Complexity**: yes it is complex to have to deal with synchronization primitives, threads and interrupts on top of that. But this is how our hardware works and this is also how our physical world works. This complexity is absolutely necessary at system level to be able to transcend hardware limitations in software (such as having the ability to instantiate infinite number of software timers using only one hardware timer).
- **Memory**: A realtime scheduler can be implemented using 10-20KB of flash memory. If your application has 128KB of flash or more then you should be using concurrency because your code will be simpler and more responsive. If you have less than that then concurrency will come at a price of leaving too little space for actual logic of your app. Unless you are trying to write firmware for devices with ridiculously little space, you should always use an RTOS and use concurrency where appropriate in your application.
- **Performance**: context switching takes time and if we have a lot of threads then our software will be slower due to excessive context switching. It is important to manage thread priorities to keep context switching to a minimum.

### 11.5. Implementation

- **Hardware concurrency**: This concurrency is a direct consequence of the hardware being able to toggle unlimited number of signals on a single clock cycle. Things do happen in parallel in the processor and your job as a programmer is to make the best use of it. Hardware concurrency is mostly invisible for you but if you can understand where it is happening and can use it in your software (for example, by starting multiple DMA transfers at once or doing transmissions in parallel) you can tremendously improve the performance of your application. Hardware concurrency also applies to multiple CPU cores executing code instructions in parallel.

- **Interrupt concurrency**: This level of concurrency is tied to the hardware support for interrupts. When an interrupt signal (electrical level) arrives at an interrupt input of the CPU, the CPU can be configured to save all main registers to memory (to stack of currently running code) and branch off to a separate location (an interrupt handler). You can configure which function will run by placing it into hardware interrupt vector table and then handle the event in that function (in C).
- **Software concurrency**: This level is supported by special interrupts (PendSV and SVC on ARM) and is able to switch software contexts (threads) when instructed to do so either by a timer interrupt or by an explicit CPU instruction (you `pend` the switch and then it happens when CPU consider it safe to do it). This kind of concurrency extends into the realm of threads, synchronization primitives, async/await idioms, work queues and other software only concepts.

### 11.6. Best practices

- **Separate by `Time Domains`**: When thinking about which tasks you want to split into multiple software threads, think of it in terms of which sequences of operations you would like to interleave. An interleaved sequence means that in code you write the code sequentially, but when you code is waiting for IO then other code can run (be interleaved) with your code. Threading is about optimizing the utilization of the CPU and the splitting of code into tasks should be primarily driven by the necessary to separate sequences of tasks into isolated blocks of code that can be maintained separately.

- **Interrupts only for loading hardware**: use interrupts only for operating on prepared data that needs to either be pumped out through the hardware or data needs to be received from hardware and placed into a prepared buffer. Interrupts are not for application logic. Interrupts must only operate on the premise of responding to a hardware event and quickly preparing next operation if appropriate. If your task is above the lowest level of abstraction (ie a GPIO operating over an I2C gpio expander) then you need to defer this work into the system work queue or another thread.

- **Utilize system work queue**: The system work queue provides a way of queuing cooperative blocks of operations which are then executed in sequence as part of the work queue thread loop. You can queue work from interrupts or from other threads. This is an excellent way to avoid creating unnecessary threads in your application.

### 11.6. Common pitfalls

- **Do not reinvent the scheduler**: You should be using an existing scheduler/RTOS like Zephyr. Do not try to invent your own scheduling. This problem has been thoroughly solved many many times since the concept first appeared in 1980. Any solution that you implement yourself will be a nightmare to maintain.
- **Do not use threads as level of abstraction**: One of the worst misuses of software concurrency is using thread as ways of `layering abstractions`. For example, you have a UART `low level thread` which reads characters and places lines into a queue. Then you have your main app thread reading from that `line queue`. This is abstraction layering and it is almost always a mistake. The intermediate thread is completely unnecessary and just wastes resources. Threads are for **Time domain separation** - Not for layering abstraction.

### 11.7. Alternatives

- **Interrupt Driven Concurrency**: You have seen how the interrupt controller already provides us with a way of nesting tasks of different priorities. Since concurrency can thus be achieved using only the interrupt controller. However, such concurrency is extremely limited because it does not give use a way to jump to an arbitrary location at any given time.

- **State machines**: Run to completion state machines are another approach. This approach can used together with conventional threading in order to create more reliable software because a state machine can be mathematically proven. The biggest drawback is that state machines need to run to completion. Preemption is only possible if you have a conventional scheduler as well.

### 11.8. Quiz

- 1. What is the main reason that interrupt based concurrency is not enough for having full flexibility when writing higher level applications? What limitations does it have?
  - Interrupt only can jump to another upon receiving an event.
  - Interrupt context is different, and we can have many kind of application to run concurrency.
  - Interrupt should be used for hardware event or high priority task.
  - Interrupt should quick.
- 2. How does software concurrency enable you to make the most use of your CPU without requiring major changes to your code structure and software architecture?
  - Allow you to jump to another code.
- 3. How would you typically structure your priorities for maximum flexibility?
  - Same priority for app threads.
- 4. What happens to an interrupt handler if another lower priority interrupt is triggered?
  - nothing happen.

### 12. Spinlock pattern

- Primary way of synchronization between interrupt handlers and layers above. Synchronization between multiple CPUs.
- On top of this pattern, we can build concurrency pattern like: semaphore, mutexes, condition variables.
- Understand this pattern help you understand how other concurrency primitives work, because they all use the spinlock at the bottom.

#### 12.1. Defining characteristics

- **Atomic variable**: This variable is used together with a set of special atomic operations to ensure that it can only be set by a single core at any given time.
- **Tight loop while locked**: This is only used when we want our code to support safe execution on a multiprocessor system. This mechanism loops on an atomic variable. We loop until we are able to atomically set the variable when it was previously zero (atomic compare and swap).
- **Interrupt masking**: we then disable interrupts and store the previous state of the interrupts in a variable called `key`. This allows spinlock to be locked recursively without any issues (interrupts are only enabled when the first spinlock is unlocked and the key is restored - we will look at how this works in this module).

#### 12.2. Use cases

- **Data access synchronization with interrupts**: Any code that needs to access the same data that is also accessed by an interrupt handler must disable at least that interrupt handler before accessing the data. Spinlock provides a generic mechanism to do this.
- **Multi-core synchronization**: In a multi-core system, an interrupt can executed by any one of the cores. Atomic operations together with a memory barrier are the only way to ensure that a certain memory location is only modified by a single core and no other code. Spinlock provides a standard mechanism for achieving this kind of synchronization.

#### 12.3. Benefits

- **Generalized Approach**: This minimizes potential bugs specially in multi-core systems and ensures that all locking is agnostic with regards to number of cores in the system.
- **Extremely lightweight**: The spinlock algorithm is very lightweight. In fact, on a single core system you can simple omit the looping part and then it simply translates to locking/unlocking of interrupts (but you still retain the possibility of building your code for a multi-core system without having to chase down every place where you disable interrupts).
- **Usable in interrupt handlers**: The spinlock can be used in interrupts to disable higher priority interrupts and also to synchronize between more than once core - making it useful for synchronization even between interrupt handlers as well (although interrupts should not share data).

#### 12.4. Drawbacks

- **No concept of threads**: A spinlock is very simple mechanism which has no concept of threads. Unlike mutex, a spinlock has no possibility to redirect execution to another thread once it is unlocked. It is simply a mechanism to synchronize data access between code and actions called directly by hardware (the interrupt handlers).
- **Disable Interrupts**: This is both a feature and also potential problem because when a single spinlock is held, NO other code can interrupt the critical section. This is because all interrupts are disabled and all events are held back until the spinlock is released. This means that under no circumstances should you ever hold a spinlock longer than necessary synchronize data access (lock data - not code).

#### 12.5. Implementation

- **Global Interrupt mask**: On ARM architecture this is called `PRIMASK` and it's a signal that controls whether exceptions are enabled at all. We can disable exception handling at any time using special CPU instructions. Any exceptions that occur while exceptions are disabled will be handled once exceptions are enabled again if they are still pending by then.
- **Interrupt Priorities**: Each interrupt can be configured with a priority. Value of zero is highest priority and then as the priority number increases we get the `turn` at which a particular interrupt will be handled. If an exception has priority number 3 then its turn to run is only after exceptions with priority numbers lower than 3 have been handled.
- **Base priority**: On ARM this is called `BASEPRI`. This is a setting that allows us to disable exception handling of exceptions with the same or lower priority than the base priority. This allows disabling of non-system interrupts while allowing other time critical interrupts to occur (with certain limitations of course as to what the interrupt handler is allowed to do in such scenario). Setting base priority to zero value disables it.

##### 12.5.1. Disabling interrupts

```C
static ALWAYS_INLINE unsigned int arch_irq_lock(void)
{
    unsigned int key;

    __asm__ volatile("mrs %0, PRIMASK;"
        "cpsid i"
        : "=r" (key)
        :
        : "memory");

    return key;
}

```

##### 12.5.2. Sharing data with ISR

```C
struct data {
    int foo;
}

static struct data data;

void interrupt_routine(void)
{
    data.foo += 5;
}

void data_user_code(struct data *self)
{
    int tag = arch_irq_lock();
    self->foo += 8;
    irq_unlock(tag);
}

void main(void)
{
    data_user_code(&data);
}
```

##### 12.5.3. Spinlock

- **Locking Interrupts**: This solves the concurrent data access problem between exceptions and lower priority code just like before.
- **Atomic compare-and-swap (CAS)**: This operation is used inside an infinite loop to compare a variable to an expected value and set it only if the comparison is successful. If comparison is unsuccessful then the code loops and retries the same operation until it succeeds.
  - Means one CPU success, another CPU will failure if try to CAS the atomic var also.
  - We just need this in SMP system.

```C
struct k_spinlock {
    // Only needed on multiprocessor systems.
    atomic_t locked;
}

static ALWAYS_INLINE k_spinlock_key_t k_spin_lock(struct k_spinlock *l)
{
    k_spinlock_key_t k;

    k.key = arch_irq_lock();

    // Only needed on SMP systems. Compare `locked` to 0 and if true set to 1
    // and return previous value (0).
    // CAS is implemented using atomic CPU instruction.
    while (!atomic_cas(&l->locked, 0, 1));

    return k;
}

static ALWAYS_INLINE void k_spin_unlock(struct k_spinlock *l)
{
    atomic_clear(&l->locked);

    arch_irq_unlock(key.key);
}
```

#### 12.6. Best practice

- **Use spin-locks even on bare-metal**: even if you are doing bare metal development without an RTOS, spinlock is a valuable abstraction that makes your code more robust. You can easily port spin-lock based ode to a system with multiple cores to a system that uses an RTOS - as long as your spin-lock implementation itself is portable.
- **Lock data, not code**: this is a central rule that is especially important with spin-locks because they delay handling of all incoming events. The critical section locked by a spinlock should be very small and only contain memory operations and preferably no calls to other functions.

#### 12.7. Common pitfalls

- **Locking across functions**: when you hold a spinlock across function call, you are always introducing a risk. Take longer time, all incoming events will be delayed.
- **Often little sense in multiple instances**: Every instances have different spin-lock object, that waste memory. **Zephyr have only one static global spinlock**.

#### 12.8. Alternatives

- **Raw interrupt disable**: This works for only single core system. in SMP you have to implement spinlock.
- **Scheduler locking**: Not protect you from interrupt code, but protect you from context switching, so if you only care about multi thread issue, just disable this.

#### 12.9. Quiz

- 1. What main problem is spinlock designed to solve?
  - protect critical section, only one person can access at a time in all system: Hardware interrupts, process, threads, etc. all of them.
- 2. Why in spinlock we have to disable the interrupt first after that acquire the lock?
  - we disable int to make sure no context switch first and after that acquire the lock. Also, because the lock is taken by another CPU, so if you don't, maybe the interrupt still happen on your CPU when you acquire the lock.
- 3. Why we need to save the interrupt state before disable into a variable in stack?
  - because we don't know that the state maybe is disabled before, so we cache the state to make sure we don't enable them incorrectly.
  - We want allow multiple spin-locks to be held at the same time.
- 4. Why never call function in critical sections?
  - Taken more time, call another lock, etc.
- 5. What operations does locking and unlocking a spinlock essentially translate into a single core system?
  - Disable interrupt is enough.
- 6. Is it fine to lock another spinlock in holding a different lock?
  - Yes for different spinlock but no for another type of lock, maybe deadlock.
- 7. Why should you this pattern even your system have only one core?
  - Abstract the lock mechanism, save interrupt state.
- 8. Why it is wrong when compare spinlock with `thread aware` primitives?
  - spinlock pause all system don't care about thread, process, interrupt.
  - `thread aware` primitives, maybe just locking another threads, process that acquire same lock.

### 13. Semaphore Pattern

- Unblocking waiting threads from interrupt handler or from another thread.

#### 13.1. Defining characteristic

- **Give/Take**: These are two main operations on a semaphore. **Give** increments the internal counter and marks the semaphore as free. **Take** decrements the counter or queue current in semaphore's internal queue until it becomes free again.

- **Maintains counter**: The semaphore is usually implemented as a counting semaphore - meaning that it can be taken multiple times without delay and then once the counter reaches zero, the semaphore will queue the calling thread until another thread gives the semaphore. For simple signalling, the semaphore usually has a count of 1 or 0 meaning that it can only be either free or taken.

- **Thread aware**: The semaphore pattern is thread aware from scheduler perspective - meaning that it can instruct the OS scheduler to jump to a new piece of code when one other piece of code gives the semaphore. This is different from simple spinlock because we are now able to call `k_sem_give()` in one thread (or interrupt) and come out of `k_sem_take()` in another thread at first valid opportunity. A semaphore thus almost works like a teleport-er for our CPU core by teleport-ing it to different places in code which are market by a call to `k_sem_take()`.

- **Maintains thread queue**: multiple threads can wait on same semaphore and these are maintained in a priority queue of waiting threads inside the semaphore data structure. This allows the semaphore logic to wake up the highest priority thread when a semaphore is given and the scheduler to then jump to that thread immediately. This allows multiple threads to try to lock the same semaphore without spinning in a flag like with spinlock - instead the threads are swapped out and not returned to until the semaphore is given.

#### 13.2. Use cases

- **Signaling from interrupt handler**: A semaphore (if it provides interrupt safe API) can be used to wake up (unblock) an application thread when an interrupt handler runs. For this, we try to take the semaphore from an application thread, and if the semaphore is busy, out thread will then be swapped out to the semaphore thread queue. Then, inside an interrupt handler, we can give the semaphore and the semaphore will mark the first waiting thread as ready and pend a context switch. Once the interrupt handler is finished, the scheduler interrupt will execute and jump the CPU to the place where the other thread was trying to acquire the semaphore. Thus our code can continue in the place where it was waiting for the interrupt event.

- **Signal between threads**: Just like we can signal between interrupts, we can also use semaphores to signal between threads. we can have one thread attempt to take a semaphore and be placed to sleep if this was not successful. Then another thread can give the semaphore and the semaphore will wake up the waiting thread and pend a context switch. The scheduler will then run the waiting thread at the first opportunity where it makes sense - i.e if the waiting thread is lower priority than the currently running thread then it will have to wait - otherwise it will be waken up right away.

- **Waking up a thread pool**: We can have multiple threads waiting on a counting semaphore and once we give that semaphore, we would be waking up one thread. Thus we can give the semaphore multiple times to wake up more than one thread. The worker thread would then perform some action after taking the semaphore and then attempt to take it again. If there is more work to do then it will successfully proceed or otherwise be put to sleep. Note however, that all threads are not woken up atomically - only one at a time. For atomic wake up of multiple threads you can use broadcasting feature of condition variable pattern.

#### 13.3. Benefits

- **Very lightweight**: The semaphore is very lightweight compared to other thread aware synchronization primitives. It's main purpose is to pend current thread until semaphore becomes available and then wake up the highest priority thread in response to the semaphore becoming available. There is no complex logic beyond.
- **Usable from interrupt handlers**: semaphores are the simplest and most lightweight. way of sending a signal from an interrupt handler to an application thread. Mutexes can not be used in interrupt handlers and work queues are more memory intensive when used to defer work from interrupts.

#### 13.4. Drawbacks

- **No priority boosting**: If a high priority thread tries to take a semaphore that has been taken by a low priority thread, the low priority thread will not be boosted in priority in order to complete faster. Generally a semaphore should therefore never be used for mutual exclusion between threads - only for signalling.

- **Not suitable for mutual exclusion**: design decisions of a semaphore are not compatible with design decisions of a mutex. Use mutex instead for mutual exclusion.
