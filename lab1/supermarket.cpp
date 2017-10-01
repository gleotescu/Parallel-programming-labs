#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <stdlib.h>
#include <unordered_map>
#include <condition_variable>
#include <chrono>


using namespace std;

// types definitions

class Product {
public:
	string name;
	float unitPrice;
	float quantity;
	Product(string name = "", float unitPrice = 0, float quantity = 0) : name(name), unitPrice(unitPrice), quantity(quantity) {}
};

class Bill {
public:
	unsigned int thId;
	float total;
	vector<Product> products;

	Bill(unsigned int thId) : thId(thId), total(0) {}

	void addProduct(Product product)
	{
		products.push_back(product);
		total += product.quantity * product.unitPrice;
	}

	void print()
	{	
		cout << "Thread   : " << thId << endl;
		cout << "Products : ";
		for (auto& product : products)
			cout << "[ " << product.name << " " << product.quantity << "kg x " << product.unitPrice << " $" << "]";
		cout << endl;
		cout << "Total    : " << total << endl << endl;
	}
};

void readInputData(vector<Product>& products, const string& infile)
{
	ifstream istream(infile);

	unsigned int productsNum;
	istream >> productsNum;

	for (unsigned int i = 0; i < productsNum; i++) {
		Product product;
		istream >> product.name >> product.unitPrice >> product.quantity;
		products.push_back(product);
	}

}

// global resources
vector<Product> products;
vector<Bill> bills;
float money = 0;

const unsigned int threadsNum = 5;

// sync mechanisms
unordered_map<string, mutex*> productsLocks;
mutex moneyLock, billsLock, inventoryLock, threadsLock;
bool inventoryStarted = false;
int activeThreads = 0;

class Semaphore
{
private:
  
  mutex mtx;
  condition_variable cv;
  int count;

public:

  Semaphore() : count(0) {}

  void V() // increment
  {
    unique_lock<mutex> lck(mtx);
    ++count;
    cv.notify_one();
  }

  void P() // decrement
  {
    unique_lock<mutex> lck(mtx);
    while(count == 0)
    {
      cv.wait(lck);
    }
    --count;
  }
};

Semaphore* semaphore;


Bill generateRandomTransaction(unsigned int thId)
{
	Bill bill(thId);

	unsigned int productsNum = rand() % products.size();

	vector<bool> usedproducts(products.size(), false); // masked of products included in transaction

	for (int i = 0; i < productsNum; i++) {
		unsigned int idx;

		while (true) {
			idx = rand() % products.size();
			if (!usedproducts[idx])
				break;
		}

		Product& product = products[idx];

		if (product.quantity == 0) // out of stock
			continue;

		unsigned int units = rand() % 5 + 1;
		float quantity = units + 0.25 * (rand() % 4);

		if (quantity > product.quantity)
			quantity = product.quantity;

		Product billProduct(product.name, product.unitPrice, quantity);
		bill.addProduct(billProduct);
		usedproducts[idx] = true;
	}
	return bill; 
}

void registerSell(const Product& billProduct)
{
	for (auto& product : products) {
		if (product.name == billProduct.name) {
			product.quantity -= billProduct.quantity;
			break;
		}
	}
}

bool checkInventoryConsistency()
{
	float billsTotal = 0;
	for (auto& bill : bills)
		billsTotal += bill.total;
	return money == billsTotal;
}

void runInventory(bool verbose)
{	
	threadsLock.lock(); // don't allow threads to exit while doing the inventory; otherwise the activeThreads number could be changed in the middle

	static unsigned int inventoryIdx = 0;

	cout << endl << "####### Inventory " << inventoryIdx++ <<  " #######" << endl << endl;

	cout << "## Consistent: ";
	bool isConsistent = checkInventoryConsistency();
	if (isConsistent)
		cout << "yes";
	else
		cout << "no";
	cout << endl << endl;

	if (verbose) {
		cout << "## Products: " << endl;
		for (auto& product : products) {
			cout << "name : " << product.name      << endl;
			cout << "price: " << product.unitPrice << endl;
			cout << "quant: " << product.quantity  << endl;
			cout << endl;
		}
		cout << "## Bills: " << endl << endl;
		for (auto& bill : bills)
			bill.print();

		cout << "## Money: " << money << " $" << endl << endl;

		cout << "#########################" << endl << endl;
	}

	threadsLock.unlock();
}

void threadFunction(unsigned int thId)
{	
	unsigned int transactionsNum = rand() % 5;

	cout << "Thread " << thId << " running " << transactionsNum << " transactions." << endl;

	threadsLock.lock();
	activeThreads += 1;
	threadsLock.unlock();
 
 	while (transactionsNum > 0)
	{ 
		// while the main thread will do its job, current thread will hang here
		inventoryLock.lock();
		if (inventoryStarted) {
			inventoryLock.unlock();
			continue;
		}
		inventoryLock.unlock();
		
		unsigned int sleep = rand() % 1000;
		std::this_thread::sleep_for (chrono::microseconds(sleep * 1000));
		 
		Bill bill = generateRandomTransaction(thId);
		transactionsNum--;

		for (auto& product : bill.products)
		{
		  mutex* lock = productsLocks[product.name];
		  lock->lock();   // get ownership
		  
		  registerSell(product);
		  
		  lock->unlock(); // release ownership
		}

		billsLock.lock(); // this might not be necessary if push_back is a sync operation
		bills.push_back(bill);
		billsLock.unlock();

		moneyLock.lock(); // this might not be necessary if += is a sync operation
		money += bill.total;
		moneyLock.unlock();

		semaphore->V();
	}

	threadsLock.lock();
	activeThreads -= 1;
	threadsLock.unlock();
}

int main()
{
	const string infile = "supermarket_input.txt";
	
	// initialize products
	readInputData(products, infile);

	// intialize products locks
	for (auto& product : products)
		productsLocks.emplace(product.name, new mutex());
	
	// initial inventory
	runInventory(true);

	// start threads
	vector<thread> threads;
	for (unsigned int i = 0; i < threadsNum; i++)
		threads.push_back(thread(threadFunction, i));

	while (true) {
		// check if at least one thread is still processing transactions
		if (activeThreads == 0)
			break;

		inventoryLock.lock(); // threads will block until we instantiate a new semaphore
		inventoryStarted = true; 
		delete semaphore;
    	semaphore = new Semaphore(); // instantiate a new semaphore
    	inventoryLock.unlock();

    	// wanna make sure that mainThread will perform its periodical inventory only after each thread cleared its current transaction -- aka the semaphore was incremented by all threads
    	for (int i = 0; i < activeThreads; i++)
      		semaphore->P();
   
	    inventoryLock.lock();
	    inventoryStarted = false;
	    inventoryLock.unlock();

		// run a periodical inventory
		runInventory(false);

		// sleep for a while
		unsigned int sleep = 1000;
		std::this_thread::sleep_for (chrono::microseconds(sleep * 1000));
	}

	for (auto& thread : threads)
		thread.join();

	delete semaphore;

	cout << "All threads joined" << endl << endl;

	// last inventory
	runInventory(true);

	return 0;
}