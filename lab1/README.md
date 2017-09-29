
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
productsLocks = {} // dictionary with key = product, value = lock, one lock for each product
for product in products
  productsLocks[product] = Lock()

moneyLock = Lock()
billsLock = Lock()

inventoryStarted = false
inventoryLock = Lock()

semaphore = Semaphore()


// define thread function
function threadFunction()
{
  while (true)
  {
    bool inventoryStartedStatus
    inventoryLock.lock()
    inventoryStartedStatus = inventoryStarted
    inventoryLock.unlock()
    
    // while the main thread will do its job, it will finish here
    if inventoryStartedStatus
      continue
      
    bill = generateRandomSale()

    for product : bill.products
    {
      lock = productsLocks[product]
      lock.lock()   // get ownership
      registerSell(product)
      lock.unlock() // release ownership
    }

    billsLock.lock() // this might not be necessary if push_back is a sync operation
    bills.push_back(bill)
    billsLock.unlock()

    moneyLock.lock() // this might not be necessary if += is a sync operation
    money += bill.total
    moneyLock.unlock()
    
    semaphore.V()
  }
}

function mainThreadFunction()
{
  while (true)
  {
    // wait a while between inventories
    wait(random(100))
    
    inventoryLock.lock()
    inventoryStarted = true
    semaphore = Semaphore() // instantiate a new semaphore
    inventoryLock.unlock()
        
    // wanna make sure that mainThread will perform its periodical inventory only after each thread cleared its current transaction -- aka the semaphore was incremented by all threads
    for threadId in 1:threadsNum
      semaphore.P()

    runInventory() // no sync mechanisms required in here; only the main thread will run at this point
    
    inventoryLock.lock()
    inventoryStarted = false
    inventoryLock.unlock()
  }
}

// create thread pool
threads = ThredPool(threadsNum, threadFunction, mainThreadFunction) // threadsNum threads will run the "threadFunction", one thread will run the "mainThreadFunction"
threads.startAll()

```
