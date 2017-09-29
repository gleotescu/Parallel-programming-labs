
## Theory

  - ### Mutex
    - lock: locks the mutex, blocks if the mutex is not available 
    - unlock: unlocks the mutex 
  - ### Semaphore
    - P : take ownership of the semaphore - decrement count, if count == 0 then it will block
    - V : release ownership - increment count 

## Problem 1 - supermarket inventory

```
// types definitions
Product := struct (price : Number, quantity : Number)
Bill    := struct (total : Number, products : vector<Product>)

// implementation   

threadsNum = 100

// initialize products
products = vector <Product> () 
bills    = vector <Bill> ()
money    = 0
readInitalInventory(products)

// initialize sync mechanisms
productsLocks = {} // dictionary with key = product, value = lock, one lock for each sold product
for product in products
  productsLocks[product] = Lock()
moneyLock = Lock()
billsLock = Lock()
  
// define thread function
function threadFunction()
{
  bill = generateRandomSale()
  
  for product : bill.products
  {
    lock = productsLocks[product]
    lock.lock()   // get ownership
    registerSell(product)
    lock.unlock() // release ownership
  }
  
  billsLock.lock() // this might not be necessary if push_back is an atomic operation
  bills.push_back(bill)
  billsLock.unlock()
  
  moneyLock.lock() // this might not be necessary if += is an atomic operation
  money += bill.total
  moneyLock.unlock()
}

function mainThreadFunction()
{
  // wanna make sure that mainThread will perform its periodical inventory only after each thread cleared its current transaction
  
}

// create thread pool
threads = ThredPool(threadsNum, threadFunction, mainThreadFunction) // threadsNum threads will run the "threadFunction", one thread will run the "mainThreadFunction"
threads.startAll()

```
