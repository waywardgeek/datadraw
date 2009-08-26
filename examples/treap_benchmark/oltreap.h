/*----------------------------------------------------------------------------------------
  Module header file for: ol module
----------------------------------------------------------------------------------------*/
#ifndef OLDATABASE_H

#define OLDATABASE_H

#if defined __cplusplus
extern "C" {
#endif

#ifndef DD_UTIL_H
#include "ddutil.h"
#endif

/* Class reference definitions */

#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
typedef struct _struct_olRoot{char val;} *olRoot;
#define olRootNull ((olRoot)(UINT32_MAX))
#define olRoot2Index(Root) ((uint32)((Root) - (olRoot)(0)))
#define olRoot2ValidIndex(Root) ((uint32)(olValidRoot(Root) - (olRoot)(0)))
#define olIndex2Root(xRoot) ((olRoot)((xRoot) + (olRoot)(0)))
typedef struct _struct_olNode{char val;} *olNode;
#define olNodeNull ((olNode)(UINT32_MAX))
#define olNode2Index(Node) ((uint32)((Node) - (olNode)(0)))
#define olNode2ValidIndex(Node) ((uint32)(olValidNode(Node) - (olNode)(0)))
#define olIndex2Node(xNode) ((olNode)((xNode) + (olNode)(0)))
#else
typedef uint32 olRoot;
#define olRootNull UINT32_MAX
#define olRoot2Index(Root) (Root)
#define olRoot2ValidIndex(Root) (olValidRoot(Root))
#define olIndex2Root(xRoot) ((xRoot))
typedef uint32 olNode;
#define olNodeNull UINT32_MAX
#define olNode2Index(Node) (Node)
#define olNode2ValidIndex(Node) (olValidNode(Node))
#define olIndex2Node(xNode) ((xNode))
#endif

/* Validate macros */
#if defined(DD_DEBUG)
#define olValidRoot(Root) (utLikely((uint32)((Root) - (olRoot)0) < \
    olRootData.usedRoot)? (Root) : (utExit("Invalid Root"), (olRoot)0))
#define olValidNode(Node) (utLikely((uint32)((Node) - (olNode)0) < \
    olRootData.usedNode)? (Node) : (utExit("Invalid Node"), (olNode)0))
#else
#define olValidRoot(Root) (Root)
#define olValidNode(Node) (Node)
#endif

/*----------------------------------------------------------------------------------------
  Fields for class Root.
----------------------------------------------------------------------------------------*/
struct olRootFields {
    olNode *RootNode;
};
extern struct olRootFields olRoots;

#define olRootGetRootNode(_Root) (olRoots.RootNode[olRoot2ValidIndex(_Root)])
#define olRootSetRootNode(_Root, value) ((olRoots.RootNode)[olRoot2ValidIndex(_Root)] = (value))
olNode olRootFindNode(olRoot Root, uint32 Key);
olNode olRootFindPrevNode(olRoot Root, uint32 Key);
#define olForeachRootNode(pVar, cVar) \
    for(cVar = olRootGetFirstNode(pVar); cVar != olNodeNull; \
        cVar = olNodeGetNextRootNode(cVar))
#define olEndRootNode
#define olSafeForeachRootNode(pVar, cVar) { \
    olNode _nextNode; \
    for(cVar = olRootGetFirstNode(pVar); cVar != olNodeNull; cVar = _nextNode) { \
        _nextNode = olNodeGetNextRootNode(cVar);
#define olEndSafeRootNode }}
#define olRootSetConstructorCallback(func) (olRootConstructorCallback = (func))
#define olRootGetConstructorCallback() (olRootConstructorCallback)
#define olRootSetDestructorCallback(func) (olRootDestructorCallback = (func))
#define olRootGetDestructorCallback() (olRootDestructorCallback)
#define olRootNextFree(_Root) (((olRoot *)(void *)(olRoots.RootNode))[olRoot2ValidIndex(_Root)])
#define olRootSetNextFree(_Root, value) (((olRoot *)(void *)(olRoots.RootNode)) \
    [olRoot2ValidIndex(_Root)] = (value))
#define olRootAllocRaw() ( \
    olRootData.firstFreeRoot != olRootNull? \
        (olTemp_.Root = olRootData.firstFreeRoot, \
        olSetFirstFreeRoot(olRootNextFree(olTemp_.Root)), true) \
    : (olRootData.usedRoot == olRootData.allocatedRoot && (olRootAllocMore(), true), \
        olTemp_.Root = olIndex2Root(olRootData.usedRoot), \
        olSetUsedRoot(olUsedRoot() + 1)), \
    olTemp_.Root)
#define olRootAlloc() ( \
    olRootData.firstFreeRoot != olRootNull? \
        (olTemp_.Root = olRootData.firstFreeRoot, \
        olSetFirstFreeRoot(olRootNextFree(olTemp_.Root)), true) \
    : (olRootData.usedRoot == olRootData.allocatedRoot && (olRootAllocMore(), true), \
        olTemp_.Root = olIndex2Root(olRootData.usedRoot), \
        olSetUsedRoot(olUsedRoot() + 1)), \
    olRootSetRootNode(olTemp_.Root, olNodeNull), \
    olRootConstructorCallback != NULL && (olRootConstructorCallback(olTemp_.Root), true), \
    olTemp_.Root)

#define olRootFree(Root) (olRootSetNextFree(Root, olRootData.firstFreeRoot), \
    olSetFirstFreeRoot(Root))
void olRootDestroy(olRoot Root);
void olRootAllocMore(void);
void olRootCopyProps(olRoot olOldRoot, olRoot olNewRoot);
void olRootInsertNode(olRoot Root, olNode _Node);
void olRootRemoveNode(olRoot Root, olNode _Node);
olNode olRootGetFirstNode(olRoot Root);
olNode olNodeGetNextRootNode(olNode Node);
olNode olRootGetLastNode(olRoot Root);
olNode olNodeGetPrevRootNode(olNode Node);
int olRootCompareNode(olNode a, olNode b);

/*----------------------------------------------------------------------------------------
  Fields for class Node.
----------------------------------------------------------------------------------------*/
struct olNodeFields {
    uint32 *Key;
    olRoot *Root;
    olNode *ParentRootNode;
    olNode *LeftRootNode;
    olNode *RightRootNode;
    uint32 *PriorityRootNode;
};
extern struct olNodeFields olNodes;

#define olNodeGetKey(_Node) (olNodes.Key[olNode2ValidIndex(_Node)])
#define olNodeSetKey(_Node, value) ((olNodes.Key)[olNode2ValidIndex(_Node)] = (value))
#define olNodeGetRoot(_Node) (olNodes.Root[olNode2ValidIndex(_Node)])
#define olNodeSetRoot(_Node, value) ((olNodes.Root)[olNode2ValidIndex(_Node)] = (value))
#define olNodeGetParentRootNode(_Node) (olNodes.ParentRootNode[olNode2ValidIndex(_Node)])
#define olNodeSetParentRootNode(_Node, value) ((olNodes.ParentRootNode)[olNode2ValidIndex(_Node)] = (value))
#define olNodeGetLeftRootNode(_Node) (olNodes.LeftRootNode[olNode2ValidIndex(_Node)])
#define olNodeSetLeftRootNode(_Node, value) ((olNodes.LeftRootNode)[olNode2ValidIndex(_Node)] = (value))
#define olNodeGetRightRootNode(_Node) (olNodes.RightRootNode[olNode2ValidIndex(_Node)])
#define olNodeSetRightRootNode(_Node, value) ((olNodes.RightRootNode)[olNode2ValidIndex(_Node)] = (value))
#define olNodeGetPriorityRootNode(_Node) (olNodes.PriorityRootNode[olNode2ValidIndex(_Node)])
#define olNodeSetPriorityRootNode(_Node, value) ((olNodes.PriorityRootNode)[olNode2ValidIndex(_Node)] = (value))
#define olNodeSetConstructorCallback(func) (olNodeConstructorCallback = (func))
#define olNodeGetConstructorCallback() (olNodeConstructorCallback)
#define olNodeSetDestructorCallback(func) (olNodeDestructorCallback = (func))
#define olNodeGetDestructorCallback() (olNodeDestructorCallback)
#define olNodeNextFree(_Node) (((olNode *)(void *)(olNodes.Root))[olNode2ValidIndex(_Node)])
#define olNodeSetNextFree(_Node, value) (((olNode *)(void *)(olNodes.Root)) \
    [olNode2ValidIndex(_Node)] = (value))
#define olNodeAllocRaw() ( \
    olRootData.firstFreeNode != olNodeNull? \
        (olTemp_.Node = olRootData.firstFreeNode, \
        olSetFirstFreeNode(olNodeNextFree(olTemp_.Node)), true) \
    : (olRootData.usedNode == olRootData.allocatedNode && (olNodeAllocMore(), true), \
        olTemp_.Node = olIndex2Node(olRootData.usedNode), \
        olSetUsedNode(olUsedNode() + 1)), \
    olTemp_.Node)
#define olNodeAlloc() ( \
    olRootData.firstFreeNode != olNodeNull? \
        (olTemp_.Node = olRootData.firstFreeNode, \
        olSetFirstFreeNode(olNodeNextFree(olTemp_.Node)), true) \
    : (olRootData.usedNode == olRootData.allocatedNode && (olNodeAllocMore(), true), \
        olTemp_.Node = olIndex2Node(olRootData.usedNode), \
        olSetUsedNode(olUsedNode() + 1)), \
    olNodeSetKey(olTemp_.Node, 0), \
    olNodeSetRoot(olTemp_.Node, olRootNull), \
    olNodeSetParentRootNode(olTemp_.Node, olNodeNull), \
    olNodeSetLeftRootNode(olTemp_.Node, olNodeNull), \
    olNodeSetRightRootNode(olTemp_.Node, olNodeNull), \
    olNodeSetPriorityRootNode(olTemp_.Node, 0), \
    olNodeConstructorCallback != NULL && (olNodeConstructorCallback(olTemp_.Node), true), \
    olTemp_.Node)

#define olNodeFree(Node) (olNodeSetNextFree(Node, olRootData.firstFreeNode), \
    olSetFirstFreeNode(Node))
void olNodeDestroy(olNode Node);
void olNodeAllocMore(void);
void olNodeCopyProps(olNode olOldNode, olNode olNewNode);

/*----------------------------------------------------------------------------------------
  Temp Union structure - Macro temp variables, use only one
----------------------------------------------------------------------------------------*/
union olTempType_ {
    olRoot Root;
    olNode Node;
};

extern union olTempType_ olTemp_;

/*----------------------------------------------------------------------------------------
  Constructor/Destructor hooks.
----------------------------------------------------------------------------------------*/
extern void(*olRootConstructorCallback)(olRoot);
extern void(*olRootDestructorCallback)(olRoot);
extern void(*olNodeConstructorCallback)(olNode);
extern void(*olNodeDestructorCallback)(olNode);

/*----------------------------------------------------------------------------------------
  Root structure
----------------------------------------------------------------------------------------*/
struct olRootType_ {
    uint32 hash; /* This depends only on the structure of the database */
    olRoot firstFreeRoot;
    uint32 usedRoot, allocatedRoot;
    olNode firstFreeNode;
    uint32 usedNode, allocatedNode;
};
extern struct olRootType_ olRootData;

#define olHash() (olRootData.hash)
#define olFirstFreeRoot() olRootData.firstFreeRoot
#define olSetFirstFreeRoot(value) (olRootData.firstFreeRoot = (value))
#define olUsedRoot() olRootData.usedRoot
#define olAllocatedRoot() olRootData.allocatedRoot
#define olSetUsedRoot(value) (olRootData.usedRoot = (value))
#define olSetAllocatedRoot(value) (olRootData.allocatedRoot = (value))
#define olFirstFreeNode() olRootData.firstFreeNode
#define olSetFirstFreeNode(value) (olRootData.firstFreeNode = (value))
#define olUsedNode() olRootData.usedNode
#define olAllocatedNode() olRootData.allocatedNode
#define olSetUsedNode(value) (olRootData.usedNode = (value))
#define olSetAllocatedNode(value) (olRootData.allocatedNode = (value))

extern uint8 olModuleID;
void olDatabaseStart(void);
void olDatabaseStop(void);
#if defined __cplusplus
}
#endif

#endif
