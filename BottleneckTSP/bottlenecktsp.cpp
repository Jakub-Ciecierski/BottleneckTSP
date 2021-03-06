#include "bottlenecktsp.h"
#include <map>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

BottleneckTSP::BottleneckTSP()
{


}

Graph* BottleneckTSP::BTSPApprox(Graph *graph)
{
    Graph* packedMbst = MBST(graph);
    Graph* unpackedMbst = unpackGraph(packedMbst);
    Graph* standardTree = createStandardTree(unpackedMbst);
    Graph* btspCycle = createBTSPGraph(standardTree);
    return btspCycle;
}

Graph* BottleneckTSP::MBST(Graph* graph)
{
    if(graph->edgesVector.size() == 1)
    {
        return graph;
    }

    float median = computeMedianSTD(graph);

    vector<Edge*> vectorA, vectorB ;
    divideEdgesByMedian(&(graph->edgesVector),median,&vectorA,&vectorB);

    //move one edge from B to A if A is empty
    if(vectorB.size() == graph->edgesVector.size())
    {
        vectorA.push_back(vectorB.at(vectorB.size()-1));
        vectorB.pop_back();
    }

    Forest* forest = createForest(&vectorB, graph->nodeVector.size());

    if(forest->size == 1 && forest->spannedNodes == graph->nodeVector.size())
    {
        Graph* graphPrime = new Graph(&(graph->nodeVector),&vectorB);

        return MBST(graphPrime);
    }
    else{
        Graph* graphPrime = MBSTContract(forest,&vectorA,&(graph->nodeVector));
        return MBST(graphPrime);
    }
}

Graph* BottleneckTSP::MBSTContract(Forest *forest, vector<Edge *> *edges, vector<Node*>* allNodes)
{
    vector<Node*> nodeVector;
    vector<Edge*> edgeVector;
    //first add nodes representing component of F
    for(int i=0;i<forest->size;i++)
    {
        Node* node = new Node(forest->componentNodes.at(i),forest->componentEdges.at(i));
        nodeVector.push_back(node);
        //assingning parents to nodes in components so adding edges would be easier
        for(int j=0;j<forest->componentNodes.at(i).size();j++)
        {
            forest->componentNodes.at(i).at(j)->parent = node;
        }
    }

    for(int i=0;i<allNodes->size();i++)
    {
        if(allNodes->at(i)->parent == nullptr)
        {
            nodeVector.push_back(allNodes->at(i));
        }
    }
    /// for each node check if the edge was already added to to the set of contracted edges. If edge is connecting at least
    /// one component node than we have to move pointers from nodeVector to realNodesVector and change pointers in nodeVector
    /// to currently checked nodes. We also want to remember which nodes were already connected (new entity  in map node1-node2 and node2-node1

    map<Node*,vector<Node*>> edgeConnMap;
    //create vectors for each node to remeber all connections
    for(int i=0;i<nodeVector.size();i++)
    {
        vector<Node*> nodes;
        nodes.push_back(nodeVector.at(i));
        edgeConnMap.insert(std::pair<Node*,vector<Node*>>(nodeVector.at(i),nodes));

    }

    //TODO (BONUS): check for lower weight value
    for(int i=0;i<edges->size();i++){
        Edge* edge;
        Node* firstNode;
        Node* secondNode;
        // find potential nodes of checked edge in contracted graph
        if(edges->at(i)->nodes.at(0)->parent != NULL)
            firstNode = edges->at(i)->nodes.at(0)->parent;
        else
            firstNode = edges->at(i)->nodes.at(0);

        if(edges->at(i)->nodes.at(1)->parent != NULL)
            secondNode = edges->at(i)->nodes.at(1)->parent;
        else
            secondNode = edges->at(i)->nodes.at(1);

        vector<Node*> firstNodeEdgeVector = edgeConnMap[firstNode];

        bool isAlreadyConnected = false;
        for(int j=0;j<firstNodeEdgeVector.size();j++)
        {
            if(firstNodeEdgeVector.at(j) == secondNode)
            {
                isAlreadyConnected = true;
                break;
            }
        }

        if(!isAlreadyConnected)
        {
            edgeConnMap[firstNode].push_back(secondNode);
            edgeConnMap[secondNode].push_back(firstNode);
            edge = new Edge(firstNode,secondNode);
            edge->parent = edges->at(i);
            edgeVector.push_back(edge);
        }
    }
    return new Graph(nodeVector,edgeVector);

}

Forest* BottleneckTSP::createForest(vector<Edge *> *edgeVector, int graphSize)
{
    return new Forest(edgeVector,graphSize);
}

float BottleneckTSP::computeMedianSTD(Graph* graph){
    int size = graph->edgesVector.size();

    std::vector<float> weights(size);
    for(int i=0; i < size ;i++)
    {
        weights[i] = graph->edgesVector.at(i)->weight;
    }

    std::nth_element(weights.begin(), weights.begin() + weights.size()/2, weights.end());

    float median = 0;
    if(size % 2 == 0){
        int index = size/2;
        median += weights[index];
        median += weights[index - 1];
        median /= 2;
    }else{
        median = weights[size/2];
    }
    return median;
}

///Divide edges into two sets. Set B contains edges with weights smaller and equal than median,
/// set A contain edges bigger than median.
void BottleneckTSP::divideEdgesByMedian(vector<Edge *> *edgeVector, float median,
                                        vector<Edge *> *vectorA, vector<Edge *> *vectorB)
{
    for(int i=0;i<edgeVector->size();i++)
    {
        if(edgeVector->at(i)->weight > median)
            vectorA->push_back(edgeVector->at(i));
        else
            vectorB->push_back(edgeVector->at(i));
    }
}

Graph* BottleneckTSP::unpackGraph(Graph * packedGraph)
{
    vector<Node*> outNodeVector;
    vector<Edge*> outEdgeVector;
    for(int i=0;i<packedGraph->edgesVector.size();i++)
    {
    outEdgeVector.push_back(unpackEdge(packedGraph->edgesVector.at(i)));
    }
    for(int i=0;i<packedGraph->nodeVector.size();i++)
    {
        unpackNodes(&outNodeVector,&outEdgeVector,packedGraph->nodeVector.at(i));
    }

    Graph* outGraph = new Graph;
    //suspicious
    outGraph->nodeVector = outNodeVector;
    outGraph->edgesVector = outEdgeVector;
    return outGraph;
}

Edge* BottleneckTSP::unpackEdge(Edge * inputEdge)
{
    Edge* originalEdge = inputEdge;
    while(originalEdge->parent != nullptr)
    {
        originalEdge = originalEdge->parent;
    }
    return originalEdge;
}

void BottleneckTSP::unpackNodes(vector<Node *> *outNodeVector, vector<Edge *> *outEdgeVector, Node* packedNode)
{
    if(packedNode->reprNodes.size() == 0)
    {
        outNodeVector->push_back(packedNode);
        return;
    }
    for(int i=0;i<packedNode->reprEdges.size();i++)
    {
        outEdgeVector->push_back(
                    unpackEdge(packedNode->reprEdges.at(i))
                    );
    }
    for(int i=0;i<packedNode->reprNodes.size();i++)
    {
        unpackNodes(outNodeVector,outEdgeVector,packedNode->reprNodes.at(i));
    }
}

Graph* BottleneckTSP::createStandardTree(Graph * inputGraph)
{
    Graph* outputGraph = new Graph();
    outputGraph->size = inputGraph->nodeVector.size();
    outputGraph->root = inputGraph->nodeVector.at(0);
    findChildren(outputGraph->root,inputGraph);
    return outputGraph;
}

void BottleneckTSP::findChildren(Node* node,Graph* graph)
{
    for(int i=0;i<graph->edgesVector.size();i++)
    {
        if(graph->edgesVector.at(i)->nodes.at(0) == node)
        {
            graph->edgesVector.at(i)->nodes.at(1)->parent = node;
          graph->edgesVector.at(i)->nodes.at(1)->distanceToParent =
                  graph->edgesVector.at(i)->weight;
          node->children.push_back(graph->edgesVector.at(i)->nodes.at(1));
          graph->edgesVector.erase(graph->edgesVector.begin()+i);
          i--;
        }
        else if(graph->edgesVector.at(i)->nodes.at(1) == node)
        {
            graph->edgesVector.at(i)->nodes.at(0)->parent = node;
            graph->edgesVector.at(i)->nodes.at(0)->distanceToParent =
                    graph->edgesVector.at(i)->weight;
          node->children.push_back(graph->edgesVector.at(i)->nodes.at(0));
          graph->edgesVector.erase(graph->edgesVector.begin()+i);
          i--;
        }
    }
    for(int i=0;i<node->children.size();i++)
    {
        findChildren(node->children.at(i),graph);
    }
}

Graph* BottleneckTSP::createBTSPGraph(Graph* mbstTree)
{
    vector<Node*> btspSeq;
    btspSeq.push_back(mbstTree->root);
    Node* currentNode = mbstTree->root;
    currentNode->wasVisited = true;
    int currentDistance = 0;
    while(btspSeq.size() != mbstTree->size)
    {
        if(currentDistance == 3)
        {
            currentNode->wasVisited = true;
            btspSeq.push_back(currentNode);
            currentDistance = 0;
        }
        if(currentNode->children.size() == 0)
        {
            if(currentNode->wasVisited == false)
            {
                currentNode->wasVisited = true;
                btspSeq.push_back(currentNode);
                currentDistance = 0;
            }
            if(currentNode != mbstTree->root)
            {
                currentNode = currentNode->parent;
                currentNode->children.erase(currentNode->children.begin());
                currentDistance++;
            }
        }
        else{
            currentNode = currentNode->children.at(0);
            currentDistance++;
        }
    }

    Graph* btspGraph = new Graph();
    btspGraph->nodeVector = btspSeq;
    for(int i=0;i<btspSeq.size()-1;i++)
    {
        Edge* edge = new Edge(btspSeq.at(i),btspSeq.at(i+1));
        btspGraph->edgesVector.push_back(edge);
    }
    btspGraph->edgesVector.push_back(new Edge(btspSeq.at(btspSeq.size()-1),btspSeq.at(0)));
    btspGraph->isInit = true;

    btspGraph->root = mbstTree->root;
    return btspGraph;
}

