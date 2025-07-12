#ifndef UTILS_H
#define UTILS_H
#include "graph.h"
#include "error_handling.h"
#include "dijkstra.h"
#include <stdint.h>

/**
 * Structure to hold node information with distance for sorting and selection.
 */
typedef struct {
    int node_index;          // Index in the graph's node array
    uint32_t node_id;        // Unique node identifier
    double latitude;         // Node latitude coordinate
    double longitude;        // Node longitude coordinate
    double distance_km;      // Distance from target point in kilometers
} NodeDistance;

// ================
// Distance Calculation Functions
// ================

/**
 * Calculates the great-circle distance between two points on Earth using the Haversine formula.
 * 
 * @param lat1 Latitude of the first point (in degrees)
 * @param lon1 Longitude of the first point (in degrees)
 * @param lat2 Latitude of the second point (in degrees)
 * @param lon2 Longitude of the second point (in degrees)
 * @return Distance in kilometers
 * 
 * @pre Latitude values must be in range [-90, 90], longitude values in range [-180, 180]
 * @post Returns accurate great-circle distance for geographical calculations
 * @note Uses Earth's radius of 6371 km for calculations
 */
double haversine_distance(double lat1, double lon1, double lat2, double lon2);

/**
 * Comparison function for sorting NodeDistance structures by distance.
 * 
 * @param a Pointer to first NodeDistance structure
 * @param b Pointer to second NodeDistance structure
 * @return -1 if a < b, 1 if a > b, 0 if equal
 * 
 * @pre Both pointers must be valid NodeDistance structures
 * @post Returns comparison result for qsort compatibility
 * @note Used with qsort() to sort nodes by distance from target point
 */
int compare_node_distance(const void *a, const void *b);

// ================
// Formatting and Display Functions
// ================

/**
 * Formats distance value according to Dijkstra mode and appropriate units.
 * 
 * @param distance Distance value to format
 * @param buffer Output buffer for formatted string
 * @param buffer_size Size of the output buffer
 * @param mode Dijkstra mode (distance or time)
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre buffer must be non-NULL and buffer_size > 0
 * @pre distance must be non-negative
 * @post On success: buffer contains formatted distance string
 *       On failure: buffer content is undefined
 * @note Automatically selects appropriate units (m/km for distance, min/hours for time)
 */
error_code_t format_distance(double distance, char *buffer, size_t buffer_size, DijkstraMode mode, error_info_t *err_info);

/**
 * Prints detailed statistics about the graph's hash table performance.
 * 
 * @param graph Pointer to the graph structure
 * 
 * @pre graph and graph->node_hash should be valid
 * @post Prints hash table statistics to stdout
 * @note Displays size, load factor, collision statistics, and chain length distribution
 */
void print_hash_table_stats(Graph *graph);

/**
 * Prints usage information for the program.
 * 
 * @param program_name Name of the program executable
 * 
 * @pre program_name can be NULL (defaults to "program")
 * @post Prints usage instructions to stdout
 * @note Shows both coordinate mode and direct node ID mode usage
 */
void print_usage(const char *program_name);

// ================
// Node and Edge Functions
// ===============

/**
 * Finds the nearest nodes to a given coordinate point.
 * 
 * @param graph Pointer to the graph structure
 * @param target_lat Target latitude coordinate
 * @param target_lon Target longitude coordinate
 * @param count Output parameter for number of nodes found
 * @param nodes Output parameter for array of nearest nodes
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre graph must be non-NULL with valid node data
 * @pre target coordinates must be within valid ranges
 * @pre count and nodes must be non-NULL
 * @post On success: *nodes contains up to 5 nearest nodes, *count set appropriately
 *       On failure: *nodes is NULL, *count is undefined
 * @note Caller is responsible for freeing the returned nodes array
 */
error_code_t find_nearest_nodes(Graph *graph, double target_lat, double target_lon, int *count, NodeDistance **nodes, error_info_t *err_info);

/**
 * Presents a list of nodes to the user and allows interactive selection.
 * 
 * @param nodes Array of NodeDistance structures to choose from
 * @param count Number of nodes in the array
 * @param description Description string for user prompt
 * @param selected_id Output parameter for selected node ID
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre nodes must be non-NULL and count > 0
 * @pre description and selected_id must be non-NULL
 * @post On success: *selected_id contains the chosen node ID
 *       On failure: *selected_id is undefined
 * @note Provides interactive menu and validates user input
 */
error_code_t select_node_from_list(NodeDistance *nodes, int count, const char *description, uint32_t *selected_id, error_info_t *err_info);

// ================
// Interactive Functions
// ===============

/**
 * Interactive coordinate mode for selecting source and target nodes by coordinates.
 * 
 * @param graph Pointer to the graph structure
 * @param source_id Output parameter for selected source node ID
 * @param target_id Output parameter for selected target node ID
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre All pointers must be non-NULL
 * @pre graph must contain valid node data
 * @post On success: *source_id and *target_id contain selected node IDs
 *       On failure: output parameters are undefined
 * @note Prompts user for coordinates and presents nearest nodes for selection
 */
error_code_t interactive_coordinate_mode(Graph *graph, uint32_t *source_id, uint32_t *target_id, error_info_t *err_info);

// ================
// File Export Functions
// ===============

/**
 * Exports a calculated path to a GPX file format.
 * 
 * @param graph Pointer to the graph structure
 * @param path Array of node indices representing the path
 * @param path_length Number of nodes in the path
 * @param filename Output GPX filename
 * @param mode Dijkstra mode used for calculation
 * @param result Dijkstra result structure with distance/time data
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre All pointers must be non-NULL and path_length > 0
 * @pre path must contain valid node indices
 * @pre filename must be a valid file path
 * @post On success: GPX file is created with route data
 *       On failure: file may be partially written
 * @note Creates GPX file with waypoints, track segments, and metadata
 */
error_code_t export_path_to_gpx(Graph *graph, int *path, int path_length, const char *filename, DijkstraMode mode, DijkstraResult *result, error_info_t *err_info);

#endif
