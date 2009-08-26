#include <iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <vector>
#include <list>
using namespace std;

const unsigned int graphWidth = 2000;
const unsigned int numTraversals = 100;

typedef struct GraphClass *Graph;
typedef struct NodeClass *Node;
typedef struct EdgeClass *Edge;
typedef vector<Node> NodeVector;
typedef list<Edge> EdgeList;

struct GraphClass {
    NodeVector nodes;
    Node getX2Node(unsigned int row, unsigned int col) { return nodes[row*graphWidth + col]; }
    void setX2Node(unsigned int row, unsigned int col, Node node) { nodes[row*graphWidth + col] = node; }
    GraphClass();
    void clearNodeFlags();
};

struct NodeClass {
    unsigned int color;
    unsigned int level;
    bool visited;
    EdgeList inEdges, outEdges;
    Graph graph;

    NodeClass(Graph owningGraph, unsigned int row, unsigned int col) :
            graph(owningGraph), color(0), level(0), visited(0) {
        graph->setX2Node(row, col, this); }
};

struct EdgeClass {
    unsigned int weight;
    int riseDelay;
    int fallDelay;
    bool enabled;
    Node fromNode, toNode;

    EdgeClass(Node node1, Node node2) : fromNode(node1), toNode(node2) {
        fromNode->outEdges.push_front(this);
        toNode->inEdges.push_front(this); }
};

GraphClass::GraphClass()
{
    nodes.resize(graphWidth*graphWidth);
    for(unsigned int xRow = 0; xRow < graphWidth; xRow++) {
        for(unsigned int xCol = 0; xCol < graphWidth; xCol++) {
            Node node = new NodeClass(this, xRow, xCol);
            if(xCol > 0) {
                new EdgeClass(this->getX2Node(xRow, xCol - 1), node);
            }
            if(xRow > 0) {
                new EdgeClass(this->getX2Node(xRow - 1, xCol), node);
            }
        }
    }
}

static unsigned int traverseGraph(
    Node node)
{
    unsigned int numNodes = 1;
    node->visited = true;
    EdgeList::iterator lp;
    for(lp = node->outEdges.begin(); lp != node->outEdges.end(); lp++) {
        Edge edge = *lp;
        Node nextNode = edge->toNode;
        if(!nextNode->visited) {
            numNodes += traverseGraph(nextNode);
        }
    }
    return numNodes;
}

void GraphClass::clearNodeFlags()
{
    NodeVector::iterator lp;
    for(lp = nodes.begin(); lp != nodes.end(); lp++) {
        (*lp)->visited = false;
    }
}

int main(void)
{
    Graph graph = new GraphClass();
    cout << "Done building graph\n";
    for(unsigned int xTraversal = 0; xTraversal < numTraversals; xTraversal++) {
        graph->clearNodeFlags();
        if(traverseGraph(graph->getX2Node(0, 0)) != graphWidth*graphWidth) {
            printf("Incorrect node count\n");
            return 1;
        }
    }
    return 0;
}

