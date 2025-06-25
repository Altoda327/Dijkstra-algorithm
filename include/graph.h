#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

typedef struct Node {
  int id;
  double latitude;
  double longitude;
} Node;

typedef struct Edge {
  int from_node;
  int to_node;
  char name[256];
  int speed_limit;
  char highway_type[32];
  double length;
  bool oneway;
} Edge;

typedef struct AdjListNode {
  int edge_index;
  struct AdjListNode *next;
} AdjListNode;

typedef struct Graph {
  Node *nodes;
  Edge *edges;
  AdjListNode **adj_list;
  int num_nodes;
  int num_edges;
} Graph;

// Function declarations

/*
 * Function to create a new graph with a specified number of nodes and edges.
 * @param num_nodes Number of nodes in the graph
 * @param num_edges Number of edges in the graph
 */
Graph* create_graph(int num_nodes, int num_edges);

/*
 * Function to free the memory allocated for the graph.
 * @param graph Pointer to the graph to be freed
 */
void free_graph(Graph *graph);

/*
 * Function to add a node to the graph.
 * @param graph Pointer to the graph
 * @param index Index of the node in the nodes array
 * @param id Unique identifier for the node
 * @param lat Latitude of the node
 * @param lon Longitude of the node
 */
void add_node(Graph *graph, int index, int id, double lat, double lon);

/*
 * Function to add an edge to the graph.
 * @param graph Pointer to the graph
 * @param index Index of the edge in the edges array
 * @param from Index of the starting node of the edge
 * @param to Index of the ending node of the edge
 * @param name Name of the edge
 * @param speed_limit Speed limit for the edge
 * @param highway_type Type of highway for the edge
 * @param length Length of the edge in meters
 * @param oneway Whether the edge is one-way or not
 */
void add_edge(Graph *graph, int index, int from, int to, const char *name,
              int speed_limit, const char *highway_type, 
              double length, bool oneway);

/*
 * Function to add an edge to the adjacency list of a node.
 * @param graph Pointer to the graph
 * @param from_node Index of the starting node of the edge
 * @param edge_index Index of the edge in the edges array
 */
void add_edge_to_adj_list(Graph *graph, int from_node, int edge_index);

/*
 * Function to get the number of nodes in the graph.
 * @param graph Pointer to the graph
 * @return Number of nodes in the graph
 */
int find_node_index(Graph *graph, int node_id);

/*
 * Function to get the adjacent edges for a given node index.
 * @param graph Pointer to the graph
 * @param node_index Index of the node in the nodes array
 * @return Pointer to the head of the adjacency list for the node
 */
AdjListNode* get_adjacent_edges(Graph *graph, int node_index);

#endif // GRAPH_H
