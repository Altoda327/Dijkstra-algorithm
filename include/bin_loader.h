#ifndef BIN_LOADER_H
#define BIN_LOADER_H

#include <stdint.h>
#include "graph.h"
#include "error_handling.h"

// ==================
// Binary Loader Function Prototypes
// ==================

/**
 * Loads a graph in CSR format from binary files.
 * 
 * @param graph Pointer to graph pointer to initialize
 * @param nodes_filename Path to node data file
 * @param edges_filename Path to edge data file
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre All pointers must be non-NULL
 * @post On success: *graph points to a valid CSR graph. On failure: *graph == NULL
 * @note The function opens both files, reads their contents, creates the graph structure,
 *       and builds the CSR (Compressed Sparse Row) representation for efficient access 
 */
error_code_t load_graph_from_binary(Graph **graph, const char *nodes_filename, const char *edges_filename, error_info_t *err_info);

/**
 * Loads node data into the graph structure from an open binary file.
 * 
 * @param graph Pointer to an allocated Graph structure
 * @param file  Open file pointer at the node data section (binary mode)
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, ERR_FILE_READ on failure
 * 
 * @pre graph, file, and err_info must be non-NULL
 * @pre graph->nodes must be allocated for graph->num_nodes elements
 * @post On success: graph->nodes is filled and node hash table is populated
 *       On failure: graph content is undefined
 * @note This function also populates the node hash table for efficient node lookup 
 */
error_code_t load_nodes_from_binary(Graph *graph, FILE *file, error_info_t *err_info);


/**
 * Loads edge data into the graph structure from an open binary file.
 * 
 * @param graph Pointer to a Graph with nodes already loaded
 * @param file  Open file pointer at the edge data section (binary mode)
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, ERR_FILE_READ on failure
 * 
 * @pre graph, file, and err_info must be non-NULL
 * @pre graph->edges must be allocated for graph->num_edges elements
 * @pre Nodes and node hash table must already be initialized
 * @post On success: graph->edges is filled and validated
 *       On failure: graph content is undefined
 * @note This function validates that all referenced nodes in edges exist in the graph 
 */
error_code_t load_edges_from_binary(Graph *graph, FILE *file, error_info_t *err_info);

#endif // BIN_LOADER_H
