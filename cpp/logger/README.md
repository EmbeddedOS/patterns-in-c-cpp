# Logger

## What is this?

- A simple logger with levels: INFO, DEBUG, ERROR, FATAL, WARN.
- The logger use advantages of tempatory object lifecycle. The constructor and destructor are called on the same statement.
  - Using constructor to add more informations: line, function, filename, etc.
  - Using `<<` operator to cache the data to an internal variable.
  - Using destructor to flush data or abort system.

- NOTE: User can custom output file descriptors or adding more log level.
