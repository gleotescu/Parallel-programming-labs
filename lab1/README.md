
## Theory

  - ### Mutex
    - lock: locks the mutex, blocks if the mutex is not available 
    - unlock: unlocks the mutex 
  - ### Semaphore
    - P : take ownership of the semaphore - decrement count, if count == 0 then it will block
    - V : release ownership - increment count 

## Problem 1 - supermarket inventory

// types definitions
Product := struct (price : Number, quantity : Number)
Bill    := struct (total : Number, items : vector<Product>)

// ### implementation   

// initialize 
products = vector <Product> () 
bills    = vector <Bill> ()
money    = 0
 
readInputData(products)
  
// define thread function
def runFunction()
  

// create thread pool
threads = ThredPool(nr, runFunction) 
