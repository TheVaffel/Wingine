# Synchronization

## Semaphores

Semaphores, in Vulkan, are synchronization primitives that can be used to make some shader stage wait for other shader stage executions in other commands to finish. A semaphore is either signalled or unsignalled, and are one-to-one. That is, a semaphore is signalled by exactly one operation, and can be awaited by exactly one other operation. When the awaiting operation reads that that the semaphore is signalled, it is automatically unsignalled again.

In Wingine, a _Semaphore Chain_ ([./ManagedSemaphoreChain.hpp](ManagedSemaphoreChain.hpp)) is a chain of semaphores that are meant to be used by one operation, each semaphore designated to one such operation per (e.g.) frame. Furthermore, `WaitSemaphoreSet` and `SignalSemaphoreSet` are sets that each hold a number of `ManagedSemaphoreChain`s, to hold all semaphores that will be waited upon or signalled by an operation. Each `ManagedSemaphoreChain` should thus be part of exactly one `WaitSemaphoreSet` and one `SignalSemaphoreSet`. It has built-in sanity checks ensuring that it is signalled and awaited alternatedly.

For a number of operations (e.g. drawing, compute execution), it makes sense to both await semaphores and signal them. One can use an instance of the `SignalAndWaitSemaphores` class to hold all such sync state for an operation.
