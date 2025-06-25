#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "dijkstra.h"
#include "utils.h"

#define INFINITY_DBL DBL_MAX

DijkstraResult* dijkstra(Graph *graph, int source_id) {
    // Validate input parameters
    if (!graph) {
        log_error("Graph pointer is null");
        return NULL;
    }

    // Find the index of the source node in the graph
    int source_index = find_node_index(graph, source_id);
    if (source_index == -1) {
        log_error("Source node ID not found in graph");
        return NULL;
    }

    // Allocate memory for the result structure
    DijkstraResult *result = malloc(sizeof(DijkstraResult));
    if (!result) {
        log_error("Failed to allocate memory for Dijkstra result");
        return NULL;
    }

    // Allocate arrays for distances, predecessors, and visited status
    result->distances = malloc(graph->num_nodes * sizeof(double));
    result->predecessors = malloc(graph->num_nodes * sizeof(int));
    result->visited = malloc(graph->num_nodes * sizeof(bool));

    // Check if all allocations were successful
    if (!result->distances || !result->predecessors || !result->visited) {
        log_error("Failed to allocate memory for Dijkstra arrays");
        free_dijkstra_result(result);
        return NULL;
    }

    // Initialize all arrays with default values
    for (int i = 0; i < graph->num_nodes; i++) {
        result->distances[i] = INFINITY_DBL;    // Initially infinite distance
        result->predecessors[i] = -1;           // No predecessor initially
        result->visited[i] = false;             // Not visited initially
    }

    // Set distance to source node as 0 and initialize result metadata
    result->distances[source_index] = 0.0;
    result->source = source_index;
    result->num_nodes = graph->num_nodes;

    // Main Dijkstra's algorithm loop
    for (int i = 0; i < graph->num_nodes; i++) {
        // Find the unvisited node with minimum distance
        int current_node = -1;
        double min_distance = INFINITY_DBL;

        for (int v = 0; v < graph->num_nodes; v++) {
            if (!result->visited[v] && result->distances[v] < min_distance) {
                min_distance = result->distances[v];
                current_node = v;
            }
        }

        // No more reachable nodes found
        if (current_node == -1) break;

        // Mark current node as visited
        result->visited[current_node] = true;

        // Update distances to all adjacent nodes
        AdjListNode *adjacent = graph->adj_list[current_node];
        while (adjacent) {
            Edge *edge = &graph->edges[adjacent->edge_index];
            int neighbor;

            // Determine which node is the neighbor based on edge direction
            // Note: edges now store node indices, not original IDs
            if (edge->from_node == current_node) {
                neighbor = edge->to_node;
            } else if (edge->to_node == current_node && !edge->oneway) {
                // Can traverse backward only if edge is bidirectional
                neighbor = edge->from_node;
            } else {
                // Skip this edge if it cannot be traversed
                adjacent = adjacent->next;
                continue;
            }

            // Update neighbor's distance if a shorter path is found
            if (!result->visited[neighbor]) {
                double new_distance = result->distances[current_node] + edge->length;

                if (new_distance < result->distances[neighbor]) {
                    result->distances[neighbor] = new_distance;
                    result->predecessors[neighbor] = current_node;
                }
            }

            adjacent = adjacent->next;
        }
    }

    return result;
}

void free_dijkstra_result(DijkstraResult *result) {
    if (!result) {
        log_error("Dijkstra result pointer is null");
        return;
    }

    // Free all allocated arrays
    free(result->distances);
    free(result->predecessors);
    free(result->visited);
    free(result);
}

void print_shortest_path(Graph *graph, DijkstraResult *result, int target_id) {
    // Validate input parameters
    if (!graph || !result) {
        log_error("Graph or result pointer is null");
        return;
    }

    // Find the index of the target node
    int target_index = find_node_index(graph, target_id);
    if (target_index == -1) {
        log_error("Target node not found in graph");
        return;
    }

    // Check if target is reachable from source
    if (result->distances[target_index] == INFINITY_DBL) {
        printf("No path exists to node %d\n", target_id);
        return;
    }

    // Get the complete path from source to target
    int path_length;
    int *path = get_path_to_node(graph, result, target_id, &path_length);
    if (!path) {
        log_error("Failed to reconstruct path to target node");
        return;
    }

    // Display path information
    printf("Shortest path to node %d: %s\n", target_id, 
           format_distance(result->distances[target_index]));
    printf("Path: ");
    for (int i = 0; i < path_length; i++) {
        printf("%d", graph->nodes[path[i]].id);
        if (i < path_length - 1) printf(" -> ");
    }
    printf("\n");

    // Clean up allocated memory
    free(path);
}

double get_distance_to_node(Graph *graph, DijkstraResult *result, int target_id) {
    // Validate input parameters
    if (!graph || !result) {
        log_error("Graph or result pointer is null");
        return INFINITY_DBL;
    }

    // Find the index of the target node
    int target_index = find_node_index(graph, target_id);
    if (target_index == -1) {
        log_error("Target node not found in graph");
        return INFINITY_DBL;
    }

    return result->distances[target_index];
}

int* get_path_to_node(Graph *graph, DijkstraResult *result, int target_id, int *path_length) {
    // Validate input parameters
    if (!graph || !result) {
        log_error("Graph or result pointer is null");
        return NULL;
    }

    // Find the index of the target node
    int target_index = find_node_index(graph, target_id);
    if (target_index == -1) {
        log_error("Target node not found in graph");
        *path_length = 0;
        return NULL;
    }

    // Check if target is reachable from source
    if (result->distances[target_index] == INFINITY_DBL) {
        log_error("No path exists to target node");
        *path_length = 0;
        return NULL;
    }

    // Count the number of nodes in the path by backtracking
    int length = 0;
    int current = target_index;
    while (current != result->source) {
        length++;
        current = result->predecessors[current];
    }

    // Allocate memory for the path array
    int *path = malloc(length * sizeof(int));
    if (!path) {
        log_error("Failed to allocate memory for path array");
        *path_length = 0;
        return NULL;
    }

    // Reconstruct the path by backtracking from target to source
    current = target_index;
    for (int i = length - 1; i >= 0; i--) {
        path[i] = current;
        current = result->predecessors[current];
    }

    *path_length = length;
    return path;
}
