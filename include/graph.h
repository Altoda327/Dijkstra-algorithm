#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>
#include "error_handling.h"

// ==================
// Constants
// ==================

#define HASH_TABLE_SIZE 65536

// ==================
// Data Structures
// ==================

/**
 * Represents a node in the graph with geographical coordinates.
 */
typedef struct {
  uint32_t node_id;    // Unique identifier for the node
  double latitude;     // Latitude coordinate
  double longitude;    // Longitude coordinate
} Node;

/**
 * Represents an edge in the graph with routing information.
 */
typedef struct {
  uint32_t from_node;   // Source node identifier
  uint32_t to_node;     // Destination node identifier
  uint32_t length;      // Length of the edge in meters
  uint32_t reserved;    // Reserved field for future use
  uint16_t speed_limit; // Speed limit in km/h
  uint8_t highway_type; // Type of highway (0-255)
  uint8_t one_way;      // 1 if one-way, 0 if bidirectional
} Edge;

/**
 * Hash table entry for efficient node lookup by ID.
 */
typedef struct NodeHashEntry {
  uint32_t node_id;              // Node identifier
  int node_index;                // Index in the nodes array
  struct NodeHashEntry *next;    // Next entry in collision chain
} NodeHashEntry;

/**
 * Hash table for mapping node IDs to array indices.
 */
typedef struct {
  NodeHashEntry **buckets;  // Array of hash table buckets
  int size;                 // Number of buckets in the hash table
  int count;                // Number of entries in the hash table
} NodeHashTable;

/**
 * Graph structure with CSR representation for efficient adjacency queries.
 */
typedef struct {
  Node *nodes;              // Array of nodes
  Edge *edges;              // Array of edges

  // CSR (Compressed Sparse Row) representation
  int *adj_offsets;         // Offset array for adjacency list
  int *adj_indices;         // Edge indices for each node's adjacency list

  // Hash Table for node lookup
  NodeHashTable *node_hash; // Hash table for node ID to index mapping

  int num_nodes;            // Number of nodes in the graph
  int num_edges;            // Number of edges in the graph
} Graph;

// ==================
// Graph Function Prototypes
// ==================

/**
 * Creates and initializes a new graph structure.
 * 
 * @param graph Pointer to graph pointer to initialize
 * @param num_nodes Number of nodes in the graph
 * @param num_edges Number of edges in the graph
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre All pointers must be non-NULL, num_nodes and num_edges must be positive
 * @post On success: *graph points to a valid, initialized graph structure
 *       On failure: *graph is undefined and memory is cleaned up
 * @note Allocates memory for all graph components including CSR arrays and hash table
 */
error_code_t create_graph(Graph **graph, int num_nodes, int num_edges, error_info_t *err_info);

/**
 * Frees all memory associated with a graph structure.
 * 
 * @param graph Pointer to graph structure to free
 * 
 * @pre None (handles NULL gracefully)
 * @post All memory associated with the graph is freed
 * @note Safe to call with NULL pointer
 */
void free_graph(Graph *graph);

/**
 * Finds the array index of a node given its ID.
 * 
 * @param graph Pointer to graph structure
 * @param node_id Node identifier to search for
 * @param out_index Pointer to store the found index
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, ERR_NOT_FOUND if node not found
 * 
 * @pre graph, out_index, and err_info must be non-NULL
 * @post On success: *out_index contains the array index of the node
 *       On failure: *out_index is undefined
 * @note Uses hash table for O(1) average-case lookup performance
 */
error_code_t find_node_index(Graph *graph, uint32_t node_id, int *out_index, error_info_t *err_info);

// ==================
// CSR Function Prototypes
// ==================

/**
 * Builds the CSR (Compressed Sparse Row) representation for efficient adjacency queries.
 * 
 * @param graph Pointer to graph with nodes and edges already loaded
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre graph and err_info must be non-NULL, nodes and edges must be loaded
 * @post On success: CSR arrays (adj_offsets, adj_indices) are populated
 *       On failure: CSR arrays are undefined
 * @note Handles both directed and undirected edges based on one_way flag
 */
error_code_t build_csr_representation(Graph *graph, error_info_t *err_info);

/**
 * Gets the range of adjacent edges for a given node using CSR representation.
 * 
 * @param graph Pointer to graph with CSR representation built
 * @param node_index Index of the node in the nodes array
 * @param start_idx Pointer to store the starting index in adj_indices
 * @param end_idx Pointer to store the ending index in adj_indices
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre All pointers must be non-NULL, node_index must be valid
 * @post On success: *start_idx and *end_idx define the range [start_idx, end_idx)
 *       On failure: indices are undefined
 * @note Edge indices can be used to access edges in the edges array
 */
error_code_t get_adjacent_edges_csr(Graph *graph, int node_index, int *start_idx, int *end_idx, error_info_t *err_info);

// ==================
// Node Hash Table Function Prototypes
// ==================

/**
 * Creates and initializes a new node hash table.
 * 
 * @param node_hash Pointer to hash table pointer to initialize
 * @param size Number of buckets in the hash table
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre node_hash and err_info must be non-NULL, size must be positive
 * @post On success: *node_hash points to a valid, initialized hash table
 *       On failure: *node_hash is undefined and memory is cleaned up
 * @note Initializes all buckets to NULL for collision handling
 */
error_code_t create_node_hash_table(NodeHashTable **node_hash, int size, error_info_t *err_info);

/**
 * Frees all memory associated with a node hash table.
 * 
 * @param node_hash Pointer to hash table to free
 * 
 * @pre None (handles NULL gracefully)
 * @post All memory associated with the hash table is freed
 * @note Safe to call with NULL pointer, handles collision chains properly
 */
void free_node_hash_table(NodeHashTable *node_hash);

/**
 * Computes a 32-bit hash value using MurmurHash3 algorithm.
 * 
 * @param key Input key to hash
 * @return 32-bit hash value
 * 
 * @pre None
 * @post Returns a well-distributed hash value
 * @note Uses MurmurHash3 for good distribution and performance
 */
unsigned int hash_murmur3_32(unsigned int key);

/**
 * Inserts a node ID to index mapping into the hash table.
 * 
 * @param node_hash Pointer to hash table
 * @param node_id Node identifier to insert
 * @param node_index Array index corresponding to the node
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre node_hash and err_info must be non-NULL, node_index must be non-negative
 * @post On success: mapping is inserted into hash table
 *       On failure: hash table state is unchanged
 * @note Handles collisions using chaining, does not check for duplicates
 */
error_code_t insert_node_hash(NodeHashTable *node_hash, uint32_t node_id, int node_index, error_info_t *err_info);

/**
 * Looks up a node index by node ID in the hash table.
 * 
 * @param node_hash Pointer to hash table
 * @param node_id Node identifier to search for
 * @param out_index Pointer to store the found index
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, ERR_NOT_FOUND if not found
 * 
 * @pre All pointers must be non-NULL
 * @post On success: *out_index contains the array index of the node
 *       On failure: *out_index is undefined
 * @note Traverses collision chain if necessary for lookup
 */
error_code_t lookup_node_hash(NodeHashTable *node_hash, uint32_t node_id, int *out_index, error_info_t *err_info);

#endif // GRAPH_H
