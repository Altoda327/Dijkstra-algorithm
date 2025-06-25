#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

typedef struct DijkstraResult {
  double *distances;  // Array of distances from source node
  int *predecessors; // Array of predecessors for path reconstruction
  bool *visited;     // Array to track visited nodes
  int source; // Source node index
  int num_nodes; // Number of nodes in the graph
} DijkstraResult;

// Function declarations

/* 
 * Function to perform Dijkstra's algorithm on a graph.
 * @param graph Pointer to the graph
 * @param source_id ID of the source node
 * @return Pointer to a DijkstraResult structure containing distances and predecessors
 */
DijkstraResult* dijkstra(Graph *graph, int source_id);

/* 
 * Function to free the memory allocated for DijkstraResult.
 * @param result Pointer to the DijkstraResult to be freed
 */
void free_dijkstra_result(DijkstraResult *result);

/* 
 * Function to print the shortest path from the source to a target node.
 * @param graph Pointer to the graph
 * @param result Pointer to the DijkstraResult containing distances and predecessors
 * @param target_id ID of the target node
 */
void print_shortest_path(Graph *graph, DijkstraResult *result, int target_id);

/* 
 * Function to get the distance from the source node to a target node.
 * @param result Pointer to the DijkstraResult containing distances
 * @param target_id ID of the target node
 * @return Distance from source to target node
 */
double get_distance_to_node(Graph *graph, DijkstraResult *result, int target_id);

/* 
 * Function to get the path from the source node to a target node.
 * @param graph Pointer to the graph
 * @param result Pointer to the DijkstraResult containing predecessors
 * @param target_id ID of the target node
 * @param path_length Pointer to an integer to store the length of the path
 * @return Array of node IDs representing the path from source to target node
 */
int* get_path_to_node(Graph *graph, DijkstraResult *result, int target_id, int *path_length);

#endif
