#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
using namespace std;

//#define NUM_NODES 30000000
#define NUM_NODES 5000000

int main()
{
    /* type of the collection:
     * - duplicates allowed
     * - elements are integral values
     * - descending order
     */
    typedef multiset<int,greater<int> > IntSet;

    IntSet coll1;        // empty multiset container

    // insert elements in random order
    int x;
    for(x = 0; x < NUM_NODES; x++) {
	int randKey = rand();
	coll1.insert(randKey);
    }
    for(x = 0; x < NUM_NODES; x++) {
	int randKey = rand();
	coll1.insert(randKey);
	randKey = rand();
	IntSet::iterator lowerKey = coll1.lower_bound(randKey);
	if(lowerKey == coll1.end()) {
	    lowerKey = coll1.begin();
	}
	coll1.erase(*lowerKey);
    }
    coll1.erase(coll1.begin(), coll1.end());
}
