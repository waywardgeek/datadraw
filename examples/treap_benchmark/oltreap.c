/*----------------------------------------------------------------------------------------
  Database ol
----------------------------------------------------------------------------------------*/

#include "oltreap.h"

union olTempType_ olTemp_;
struct olRootType_ olRootData;
uint8 olModuleID;
struct olRootFields olRoots;
struct olNodeFields olNodes;

/*----------------------------------------------------------------------------------------
  Constructor/Destructor hooks.
----------------------------------------------------------------------------------------*/
void(*olRootConstructorCallback)(olRoot);
void(*olRootDestructorCallback)(olRoot);
void(*olNodeConstructorCallback)(olNode);
void(*olNodeDestructorCallback)(olNode);

/*----------------------------------------------------------------------------------------
  Destroy Root including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void olRootDestroy(
    olRoot Root)
{
    olNode Node_, nextNode_;

    if(olRootDestructorCallback != NULL) {
        olRootDestructorCallback(Root);
    }
    for(Node_ = olRootGetFirstNode(Root); Node_ != olNodeNull;
            Node_ = nextNode_) {
        nextNode_ = olNodeGetNextRootNode(Node_);
        olNodeSetRoot(Node_, olRootNull);
    }
    olRootFree(Root);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocRoot(void)
{
    olRoot Root = olRootAlloc();

    return olRoot2Index(Root);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyRoot(
    uint64 objectIndex)
{
    olRootDestroy(olIndex2Root((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Root.
----------------------------------------------------------------------------------------*/
static void allocRoots(void)
{
    olSetAllocatedRoot(2);
    olSetUsedRoot(0);
    olSetFirstFreeRoot(olRootNull);
    olRoots.RootNode = utNewA(olNode, (olAllocatedRoot()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Root.
----------------------------------------------------------------------------------------*/
static void reallocRoots(
    uint32 newSize)
{
    utResizeArray(olRoots.RootNode, (newSize));
    olSetAllocatedRoot(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Roots.
----------------------------------------------------------------------------------------*/
void olRootAllocMore(void)
{
    reallocRoots((uint32)(olAllocatedRoot() + (olAllocatedRoot() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Root.
----------------------------------------------------------------------------------------*/
void olRootCopyProps(
    olRoot oldRoot,
    olRoot newRoot)
{
}

/*----------------------------------------------------------------------------------------
  Compare two heap elements.
----------------------------------------------------------------------------------------*/
int olRootCompareNode(
    olNode left,
    olNode right)
{
    return olNodeGetKey(right) - olNodeGetKey(left);
}

/*----------------------------------------------------------------------------------------
  Retreive the smallest node in the Ordered List i.e. the first one
----------------------------------------------------------------------------------------*/
static olNode nodeFindMinRootNode(
    olNode Node)
{
    while(olNodeGetLeftRootNode(Node) != olNodeNull) {
        Node = olNodeGetLeftRootNode(Node);
    }
    return Node;
}

/*----------------------------------------------------------------------------------------
  Retreive the largest node in the Ordered List i.e. the last one
----------------------------------------------------------------------------------------*/
static olNode nodeFindMaxRootNode(
    olNode Node)
{
    while(olNodeGetRightRootNode(Node) != olNodeNull) {
        Node = olNodeGetRightRootNode(Node);
    }
    return Node;
}

/*----------------------------------------------------------------------------------------
  Return the first Node in the root.
----------------------------------------------------------------------------------------*/
olNode olRootGetFirstNode(
    olRoot Root)
{
    olNode node = olRootGetRootNode(Root);

    if(node == olNodeNull) {
        return olNodeNull;
    }
    return nodeFindMinRootNode(olRootGetRootNode(Root));
}

/*----------------------------------------------------------------------------------------
  Return the last Node in the root.
----------------------------------------------------------------------------------------*/
olNode olRootGetLastNode(
    olRoot Root)
{
    olNode node = olRootGetRootNode(Root);

    if(node == olNodeNull) {
        return olNodeNull;
    }
    return nodeFindMaxRootNode(olRootGetRootNode(Root));
}

/*----------------------------------------------------------------------------------------
  Find the Node after this one in the tree.
----------------------------------------------------------------------------------------*/
olNode olNodeGetNextRootNode(
    olNode Node)
{
    if(olNodeGetRightRootNode(Node) != olNodeNull) {
        return nodeFindMinRootNode(olNodeGetRightRootNode(Node));
    }
    while(olNodeGetParentRootNode(Node) != olNodeNull && 
            olNodeGetRightRootNode(olNodeGetParentRootNode(Node)) == Node) {
        Node = olNodeGetParentRootNode(Node);
    }
    return olNodeGetParentRootNode(Node);
}

/*----------------------------------------------------------------------------------------
  Find the Node before this one in the tree.
----------------------------------------------------------------------------------------*/
olNode olNodeGetPrevRootNode(
    olNode Node)
{
    if(olNodeGetLeftRootNode(Node) != olNodeNull) {
        return nodeFindMaxRootNode(olNodeGetLeftRootNode(Node));
    }
    while(olNodeGetParentRootNode(Node) != olNodeNull && 
            olNodeGetLeftRootNode(olNodeGetParentRootNode(Node)) == Node) {
        Node = olNodeGetParentRootNode(Node);
    }
    return olNodeGetParentRootNode(Node);
}

/*----------------------------------------------------------------------------------------
  Rotate a node left.
----------------------------------------------------------------------------------------*/
static olNode rotateLeftRootNode(
    olNode current)
{
    olNode parent = olNodeGetParentRootNode(current);
    olNode right = olNodeGetRightRootNode(current);
    olNode middleChild = olNodeGetLeftRootNode(right);

    if(parent != olNodeNull) {
        if(olNodeGetLeftRootNode(parent) == current) {
            olNodeSetLeftRootNode(parent, right);
        } else {
            olNodeSetRightRootNode(parent, right);
        }
    }
    olNodeSetParentRootNode(right, parent);
    olNodeSetRightRootNode(current, middleChild);
    if(middleChild != olNodeNull) {
        olNodeSetParentRootNode(middleChild, current);
    }
    olNodeSetLeftRootNode(right, current);
    olNodeSetParentRootNode(current, right);
    return right;
}

/*----------------------------------------------------------------------------------------
  Rotate a node right.
----------------------------------------------------------------------------------------*/
static olNode rotateRightRootNode(
    olNode current)
{
    olNode parent = olNodeGetParentRootNode(current);
    olNode left = olNodeGetLeftRootNode(current);
    olNode middleChild = olNodeGetRightRootNode(left);

    if(parent != olNodeNull) {
        if(olNodeGetLeftRootNode(parent) == current) {
            olNodeSetLeftRootNode(parent, left);
        } else {
            olNodeSetRightRootNode(parent, left);
        }
    }
    olNodeSetParentRootNode(left, parent);
    olNodeSetLeftRootNode(current, middleChild);
    if(middleChild != olNodeNull) {
        olNodeSetParentRootNode(middleChild, current);
    }
    olNodeSetParentRootNode(current, left);
    olNodeSetRightRootNode(left, current);
    return left;
}

/*----------------------------------------------------------------------------------------
  Float a node up the treap until it's children are not bigger than it.
----------------------------------------------------------------------------------------*/
static olNode floatUpRootNode(
    olNode node)
{
    olNode parent;

    utDo {
        parent = olNodeGetParentRootNode(node);
    } utWhile(parent != olNodeNull && olNodeGetPriorityRootNode(parent) < olNodeGetPriorityRootNode(node)) {
        if(olNodeGetLeftRootNode(parent) == node) {
            rotateRightRootNode(parent);
        } else {
            rotateLeftRootNode(parent);
        }
    } utRepeat;
    return node;
}

/*----------------------------------------------------------------------------------------
  Insert a node as a child of this one.
----------------------------------------------------------------------------------------*/
static olNode nodeInsertRootNode(
    olNode current,
    olNode toinsert)
{
    int cmp;

    while(1) {
        cmp = olRootCompareNode(current, toinsert);
        if(cmp < 0) {
            if(olNodeGetLeftRootNode(current) == olNodeNull) {
                olNodeSetLeftRootNode(current, toinsert);
                olNodeSetParentRootNode(toinsert, current);
                break;
            } else {
                current = olNodeGetLeftRootNode(current);
            }
        } else {
            if(olNodeGetRightRootNode(current) == olNodeNull) {
                olNodeSetRightRootNode(current, toinsert);
                olNodeSetParentRootNode(toinsert, current);
                break;
            } else {
                current = olNodeGetRightRootNode(current);
            }
        }
    }
    return floatUpRootNode(toinsert);
}

/*----------------------------------------------------------------------------------------
  Insert a node into a root.
----------------------------------------------------------------------------------------*/
void olRootInsertNode(
    olRoot Root,
    olNode _Node)
{
    olNode root = olRootGetRootNode(Root);
    uint32 priority = utHashUint32(olNode2Index(_Node));

    olNodeSetPriorityRootNode(_Node, priority);
    olNodeSetRoot(_Node, Root);
    olNodeSetParentRootNode(_Node, olNodeNull);
    olNodeSetLeftRootNode(_Node, olNodeNull);
    olNodeSetRightRootNode(_Node, olNodeNull);
    if(root != olNodeNull) {
        _Node = nodeInsertRootNode(root, _Node);
    }
    if(olNodeGetParentRootNode(_Node) == olNodeNull) {
        olRootSetRootNode(Root, _Node);
    }
}

/*----------------------------------------------------------------------------------------
  Swap the two node's positions in the tree, and their priorities.
----------------------------------------------------------------------------------------*/
static void swapNodesRootNode(
    olNode node1,
    olNode node2)
{
    olNode parent1 = olNodeGetParentRootNode(node1);
    olNode left1 = olNodeGetLeftRootNode(node1);
    olNode right1 = olNodeGetRightRootNode(node1);
    uint32 priority1 = olNodeGetPriorityRootNode(node1);
    olNode parent2 = olNodeGetParentRootNode(node2);
    olNode left2 = olNodeGetLeftRootNode(node2);
    uint32 priority2 = olNodeGetPriorityRootNode(node2);

    if(left1 != node2) {
        olNodeSetParentRootNode(node1, parent2);
        olNodeSetLeftRootNode(node2, left1);
        if(left1 != olNodeNull) {
            olNodeSetParentRootNode(left1, node2);
        }
        if(olNodeGetLeftRootNode(parent2) == node2) {
            olNodeSetLeftRootNode(parent2, node1);
        } else {
            olNodeSetRightRootNode(parent2, node1);
        }
    } else {
        olNodeSetParentRootNode(node1, node2);
        olNodeSetLeftRootNode(node2, node1);
    }
    olNodeSetLeftRootNode(node1, left2);
    olNodeSetRightRootNode(node1, olNodeNull);
    olNodeSetPriorityRootNode(node1, priority2);
    olNodeSetParentRootNode(node2, parent1);
    olNodeSetRightRootNode(node2, right1);
    olNodeSetPriorityRootNode(node2, priority1);
    if(right1 != olNodeNull) {
        olNodeSetParentRootNode(right1, node2);
    }
    if(left2 != olNodeNull) {
        olNodeSetParentRootNode(left2, node1);
    }
    if(parent1 != olNodeNull) {
        if(olNodeGetLeftRootNode(parent1) == node1) {
            olNodeSetLeftRootNode(parent1, node2);
        } else {
            olNodeSetRightRootNode(parent1, node2);
        }
    }
}

/*----------------------------------------------------------------------------------------
  Remove the Node from the Root.
----------------------------------------------------------------------------------------*/
void olRootRemoveNode(
    olRoot Root,
    olNode _Node)
{
    olNode parent, child;
    olNode left = olNodeGetLeftRootNode(_Node);
    olNode right = olNodeGetRightRootNode(_Node);

    if(left != olNodeNull && right != olNodeNull) {
        child = nodeFindMaxRootNode(left);
        if(olRootGetRootNode(Root) == _Node) {
            olRootSetRootNode(Root, child);
        }
        swapNodesRootNode(_Node, child);
        child = olNodeGetLeftRootNode(_Node);
    } else {
        child = left == olNodeNull? right : left;
    }
    parent = olNodeGetParentRootNode(_Node);
    olNodeSetParentRootNode(_Node, olNodeNull);
    olNodeSetLeftRootNode(_Node, olNodeNull);
    olNodeSetRightRootNode(_Node, olNodeNull);
    if(parent == olNodeNull) {
        olRootSetRootNode(Root, child);
    } else {
        if(olNodeGetLeftRootNode(parent) == _Node) {
            olNodeSetLeftRootNode(parent, child);
        } else {
            olNodeSetRightRootNode(parent, child);
        }
    }
    if(child != olNodeNull) {
        olNodeSetParentRootNode(child, parent);
    }
}

/*----------------------------------------------------------------------------------------
  Find the node in the graph.
----------------------------------------------------------------------------------------*/
olNode olRootFindNode(
    olRoot Root,
    uint32 Key)
{
    olNode node = olRootGetRootNode(Root);
    int comparison;

    while(node != olNodeNull) {
        comparison = 0;
        comparison = Key - olNodeGetKey(node);
        if(comparison < 0) {
            node = olNodeGetLeftRootNode(node);
        } else if(comparison > 0) {
            node = olNodeGetRightRootNode(node);
        } else {
            return node;
        }
    }
    return olNodeNull;
}

/*----------------------------------------------------------------------------------------
  Find the node in the graph.
----------------------------------------------------------------------------------------*/
olNode olRootFindPrevNode(
    olRoot Root,
    uint32 Key)
{
    olNode node = olRootGetRootNode(Root);
    olNode prevNode = olNodeNull;
    int comparison;

    while(node != olNodeNull) {
        prevNode = node;
        comparison = 0;
        comparison = Key - olNodeGetKey(node);
        if(comparison < 0) {
            node = olNodeGetLeftRootNode(node);
        } else if(comparison > 0) {
            node = olNodeGetRightRootNode(node);
        } else {
            return node;
        }
    }
    if(prevNode != olNodeNull) {
        if(comparison < 0) {
            return olNodeGetPrevRootNode(prevNode);
        }
    }
    return prevNode;
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void olShowRoot(
    olRoot Root)
{
    utDatabaseShowObject("ol", "Root", olRoot2Index(Root));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Node including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void olNodeDestroy(
    olNode Node)
{
    olRoot owningRoot = olNodeGetRoot(Node);

    if(olNodeDestructorCallback != NULL) {
        olNodeDestructorCallback(Node);
    }
    if(owningRoot != olRootNull) {
        olRootRemoveNode(owningRoot, Node);
    }
    olNodeFree(Node);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocNode(void)
{
    olNode Node = olNodeAlloc();

    return olNode2Index(Node);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyNode(
    uint64 objectIndex)
{
    olNodeDestroy(olIndex2Node((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Node.
----------------------------------------------------------------------------------------*/
static void allocNodes(void)
{
    olSetAllocatedNode(2);
    olSetUsedNode(0);
    olSetFirstFreeNode(olNodeNull);
    olNodes.Key = utNewA(uint32, (olAllocatedNode()));
    olNodes.Root = utNewA(olRoot, (olAllocatedNode()));
    olNodes.ParentRootNode = utNewA(olNode, (olAllocatedNode()));
    olNodes.LeftRootNode = utNewA(olNode, (olAllocatedNode()));
    olNodes.RightRootNode = utNewA(olNode, (olAllocatedNode()));
    olNodes.PriorityRootNode = utNewA(uint32, (olAllocatedNode()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Node.
----------------------------------------------------------------------------------------*/
static void reallocNodes(
    uint32 newSize)
{
    utResizeArray(olNodes.Key, (newSize));
    utResizeArray(olNodes.Root, (newSize));
    utResizeArray(olNodes.ParentRootNode, (newSize));
    utResizeArray(olNodes.LeftRootNode, (newSize));
    utResizeArray(olNodes.RightRootNode, (newSize));
    utResizeArray(olNodes.PriorityRootNode, (newSize));
    olSetAllocatedNode(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Nodes.
----------------------------------------------------------------------------------------*/
void olNodeAllocMore(void)
{
    reallocNodes((uint32)(olAllocatedNode() + (olAllocatedNode() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Node.
----------------------------------------------------------------------------------------*/
void olNodeCopyProps(
    olNode oldNode,
    olNode newNode)
{
    olNodeSetKey(newNode, olNodeGetKey(oldNode));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void olShowNode(
    olNode Node)
{
    utDatabaseShowObject("ol", "Node", olNode2Index(Node));
}
#endif

/*----------------------------------------------------------------------------------------
  Free memory used by the ol database.
----------------------------------------------------------------------------------------*/
void olDatabaseStop(void)
{
    utFree(olRoots.RootNode);
    utFree(olNodes.Key);
    utFree(olNodes.Root);
    utFree(olNodes.ParentRootNode);
    utFree(olNodes.LeftRootNode);
    utFree(olNodes.RightRootNode);
    utFree(olNodes.PriorityRootNode);
    utUnregisterModule(olModuleID);
}

/*----------------------------------------------------------------------------------------
  Allocate memory used by the ol database.
----------------------------------------------------------------------------------------*/
void olDatabaseStart(void)
{
    if(!utInitialized()) {
        utStart();
    }
    olRootData.hash = 0x97abeb32;
    olModuleID = utRegisterModule("ol", false, olHash(), 2, 7, 0, sizeof(struct olRootType_),
        &olRootData, olDatabaseStart, olDatabaseStop);
    utRegisterClass("Root", 1, &olRootData.usedRoot, &olRootData.allocatedRoot,
        &olRootData.firstFreeRoot, 0, 4, allocRoot, destroyRoot);
    utRegisterField("RootNode", &olRoots.RootNode, sizeof(olNode), UT_POINTER, "Node");
    utRegisterClass("Node", 6, &olRootData.usedNode, &olRootData.allocatedNode,
        &olRootData.firstFreeNode, 2, 4, allocNode, destroyNode);
    utRegisterField("Key", &olNodes.Key, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Root", &olNodes.Root, sizeof(olRoot), UT_POINTER, "Root");
    utRegisterField("ParentRootNode", &olNodes.ParentRootNode, sizeof(olNode), UT_POINTER, "Node");
    utRegisterField("LeftRootNode", &olNodes.LeftRootNode, sizeof(olNode), UT_POINTER, "Node");
    utRegisterField("RightRootNode", &olNodes.RightRootNode, sizeof(olNode), UT_POINTER, "Node");
    utRegisterField("PriorityRootNode", &olNodes.PriorityRootNode, sizeof(uint32), UT_UINT, NULL);
    allocRoots();
    allocNodes();
}

#if defined(DD_DEBUG)
#undef olRootGetRootNode
olNode olRootGetRootNode(
    olRoot _Root)
{
    return olRoots.RootNode[olRoot2Index(_Root)];
}

#undef olRootSetRootNode
void olRootSetRootNode(
    olRoot _Root,
    olNode value)
{
    olRoots.RootNode[olRoot2Index(_Root)] = value;
}

#undef olNodeGetKey
uint32 olNodeGetKey(
    olNode _Node)
{
    return olNodes.Key[olNode2Index(_Node)];
}

#undef olNodeSetKey
void olNodeSetKey(
    olNode _Node,
    uint32 value)
{
    olNodes.Key[olNode2Index(_Node)] = value;
}

#undef olNodeGetRoot
olRoot olNodeGetRoot(
    olNode _Node)
{
    return olNodes.Root[olNode2Index(_Node)];
}

#undef olNodeSetRoot
void olNodeSetRoot(
    olNode _Node,
    olRoot value)
{
    olNodes.Root[olNode2Index(_Node)] = value;
}

#undef olNodeGetParentRootNode
olNode olNodeGetParentRootNode(
    olNode _Node)
{
    return olNodes.ParentRootNode[olNode2Index(_Node)];
}

#undef olNodeSetParentRootNode
void olNodeSetParentRootNode(
    olNode _Node,
    olNode value)
{
    olNodes.ParentRootNode[olNode2Index(_Node)] = value;
}

#undef olNodeGetLeftRootNode
olNode olNodeGetLeftRootNode(
    olNode _Node)
{
    return olNodes.LeftRootNode[olNode2Index(_Node)];
}

#undef olNodeSetLeftRootNode
void olNodeSetLeftRootNode(
    olNode _Node,
    olNode value)
{
    olNodes.LeftRootNode[olNode2Index(_Node)] = value;
}

#undef olNodeGetRightRootNode
olNode olNodeGetRightRootNode(
    olNode _Node)
{
    return olNodes.RightRootNode[olNode2Index(_Node)];
}

#undef olNodeSetRightRootNode
void olNodeSetRightRootNode(
    olNode _Node,
    olNode value)
{
    olNodes.RightRootNode[olNode2Index(_Node)] = value;
}

#undef olNodeGetPriorityRootNode
uint32 olNodeGetPriorityRootNode(
    olNode _Node)
{
    return olNodes.PriorityRootNode[olNode2Index(_Node)];
}

#undef olNodeSetPriorityRootNode
void olNodeSetPriorityRootNode(
    olNode _Node,
    uint32 value)
{
    olNodes.PriorityRootNode[olNode2Index(_Node)] = value;
}

#endif
