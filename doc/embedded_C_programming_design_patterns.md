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
