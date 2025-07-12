#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <stdint.h>
#include <stdbool.h>
#include "graph.h"
#include "error_handling.h"

// =================
// Dijkstra's Algorithm Data Structures
// =================

typedef struct {
  double *distances;
  int *predecessors;
  bool *visited;
  int source_index;
  int target_index;
  int num_nodes;
  bool target_found;
} DijkstraResult;

typedef enum {
  DIJKSTRA_SHORTEST_DISTANCE = 1,
  DIJKSTRA_FASTEST_TIME = 2
} DijkstraMode;

// =================
// Dijkstra's Algorithm Function Prototypes
// =================

/**
 * Finds the shortest path between two nodes using Dijkstra's algorithm.
 * 
 * @param graph Pointer to the graph structure
 * @param source_node_id ID of the source node
 * @param target_node_id ID of the target node
 * @param mode Algorithm mode (shortest distance or fastest time)
 * @param result Pointer to store the algorithm results
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre All pointers must be non-NULL
 * @pre source_node_id and target_node_id must exist in the graph
 * @pre source_node_id != target_node_id
 * @pre mode must be either DIJKSTRA_SHORTEST_DISTANCE or DIJKSTRA_FASTEST_TIME
 * @post On success: result contains distances, predecessors, and visited arrays
 *       On failure: result content is undefined
 * @note For DIJKSTRA_FASTEST_TIME mode, edge speed_limit must be positive
 * @note The caller must call free_dijkstra_result() to free allocated memory
 */
error_code_t dijkstra_shortest_path(Graph *graph, uint32_t source_node_id, uint32_t target_node_id, DijkstraMode mode, DijkstraResult *result, error_info_t *err_info);

/**
 * Frees memory allocated for DijkstraResult structure.
 * 
 * @param result Pointer to DijkstraResult structure to free
 * 
 * @pre None
 * @post All allocated memory in result is freed
 * @note Safe to call with NULL pointer
 */
void free_dijkstra_result(DijkstraResult *result);

/**
 * Retrieves the shortest distance from Dijkstra algorithm result.
 * 
 * @param result Pointer to DijkstraResult structure
 * @param distance Pointer to store the shortest distance
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre result, distance, and err_info must be non-NULL
 * @pre result must be initialized by dijkstra_shortest_path()
 * @post On success: *distance contains the shortest distance or INFINITY if no path exists
 *       On failure: *distance is undefined
 * @note If target was not found, distance is set to INFINITY (DBL_MAX)
 */
error_code_t get_shortest_distance(DijkstraResult *result, double *distance, error_info_t *err_info);

/**
 * Retrieves the shortest path from Dijkstra algorithm result.
 * 
 * @param graph Pointer to the graph structure
 * @param result Pointer to DijkstraResult structure
 * @param path_length Pointer to store the path length
 * @param path Pointer to store the allocated path array
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre All pointers must be non-NULL
 * @pre result must be initialized by dijkstra_shortest_path()
 * @pre Target node must be reachable from source
 * @post On success: *path contains node indices array, *path_length contains array size
 *       On failure: *path_length is 0, *path is undefined
 * @note The caller must free the allocated path array
 * @note Path contains node indices in order from source to target
 */
error_code_t get_shortest_path(Graph *graph, DijkstraResult *result, int *path_length, int **path, error_info_t *err_info);

#endif // DIJKSTRA_H
