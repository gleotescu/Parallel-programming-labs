
## Theory

  - ### Mutex
    - lock: locks the mutex, blocks if the mutex is not available 
    - unlock: unlocks the mutex 
  - ### Semaphore
    - P : take ownership of the semaphore - decrement count, if count == 0 then it will block
    - V : release ownership - increment count 

## Problem 1 - supermarket inventory
