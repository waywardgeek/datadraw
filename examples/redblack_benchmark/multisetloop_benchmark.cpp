#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
using namespace std;

//#define NUM_NODES 30000000
#define NUM_NODES 5000000
#define NUM_LOOPS 50

int main()
{
    /* type of the collection:
     * - duplicates allowed
     * - elements are integral values
     * - descending order
     */
    typedef multiset<int,greater<int> > IntSet;
    unsigned int keySum = 0, totalKeys = 0;

    IntSet coll1;        // empty multiset container

    // insert elements in random order
    int x;
    for(x = 0; x < NUM_NODES; x++) {
	coll1.insert(x);
    }
    for(x = 0; x < NUM_LOOPS; x++) {
	IntSet::iterator lp;
	for(lp = coll1.begin(); lp != coll1.end(); lp++) {
	    keySum += *lp;
	    totalKeys++;
	}
    }
    cout << "Key sum " << keySum << ", total keys " << totalKeys << endl;
    coll1.erase(coll1.begin(), coll1.end());
}
