#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

// ================
// Hash table functions
// ================

error_code_t create_node_hash_table(NodeHashTable **node_hash, int size, error_info_t *err_info) {
  // Null error check is already done in create_graph
  if (size <= 0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "hash table size must be positive.");
    return ERR_INVALID_ARGUMENT;
  }

  // Allocate memory for the hash table structure
  *node_hash = (NodeHashTable *)malloc(sizeof(NodeHashTable));
  CHECK_ALLOCATION(*node_hash, err_info);

  // Allocate and zero-initialize buckets array
  (*node_hash)->buckets = (NodeHashEntry **)calloc(size, sizeof(NodeHashEntry *));
  if ((*node_hash)->buckets == NULL) {
    free(*node_hash);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "failed to allocate memory for hash table buckets.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Initialize hash table properties
  (*node_hash)->size = size;
  (*node_hash)->count = 0;

  return ERR_SUCCESS;
}

void free_node_hash_table(NodeHashTable *node_hash) {
  if (node_hash == NULL) return;

  // Free all collision chains in each bucket
  for (int i = 0; i < node_hash->size; i++) {
    NodeHashEntry *entry = node_hash->buckets[i];
    while (entry != NULL) {
      NodeHashEntry *temp = entry;
      entry = entry->next;
      free(temp);
    }
  }

  // Free the buckets array and hash table structure
  free(node_hash->buckets);
  free(node_hash);
}

/**
 * MurmurHash3 32-bit hash function for node IDs.
 * Provides good distribution and performance for hash table operations.
 * 
 * @param key Input key to hash
 * @return 32-bit hash value
 * 
 * @note This is a simplified version of MurmurHash3 optimized for 32-bit integers
 */
unsigned int hash_murmur3_32(unsigned int key) {
    key ^= key >> 16;
    key *= 0x85ebca6b;
    key ^= key >> 13;
    key *= 0xc2b2ae35;
    key ^= key >> 16;
    return key;
}

error_code_t insert_node_hash(NodeHashTable *node_hash, uint32_t node_id, int node_index, error_info_t *err_info) {
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(node_hash, err_info);

  if (node_index < 0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "node id and index must be non-negative.");
    return ERR_INVALID_ARGUMENT;
  }

  // Calculate hash index using MurmurHash3
  unsigned int hash_index = hash_murmur3_32(node_id) % node_hash->size;

  // Create new hash entry
  NodeHashEntry *new_entry = (NodeHashEntry *)malloc(sizeof(NodeHashEntry));
  CHECK_ALLOCATION(new_entry, err_info);

  // Initialize entry and insert at head of collision chain
  new_entry->node_id = node_id;
  new_entry->node_index = node_index;
  new_entry->next = node_hash->buckets[hash_index];
  node_hash->buckets[hash_index] = new_entry;

  node_hash->count++;
  return ERR_SUCCESS;
}

error_code_t lookup_node_hash(NodeHashTable *node_hash, uint32_t node_id, int *outindex, error_info_t *err_info) {
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(node_hash, err_info);
  CHECK_NULL(outindex, err_info);

  // Calculate hash index for the node ID
  unsigned int hash_index = hash_murmur3_32(node_id) % node_hash->size;

  // Traverse collision chain to find matching node ID
  NodeHashEntry *entry = node_hash->buckets[hash_index];
  while (entry != NULL) {
    if (entry->node_id == node_id) {
      *outindex = entry->node_index;
      return ERR_SUCCESS;
    }
    entry = entry->next;
  }

  SET_ERROR(err_info, ERR_NOT_FOUND, "Node id not found in hash table.");
  return ERR_NOT_FOUND;
}

// ================
// Graph functions
// ================

error_code_t create_graph(Graph **graph, int num_nodes, int num_edges, error_info_t *err_info) {
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(graph, err_info);

  if (num_nodes <= 0 || num_edges <= 0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "number of nodes and edges must be positive.");
    return ERR_INVALID_ARGUMENT;
  }

  // Allocate memory for the graph structure
  *graph = malloc(sizeof(Graph));
  CHECK_ALLOCATION(*graph, err_info);

  // Allocate memory for nodes array
  (*graph)->nodes = (Node *)malloc(num_nodes * sizeof(Node));
  if ((*graph)->nodes == NULL) {
    free(*graph);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "failed to allocate memory for nodes.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Allocate memory for edges array
  (*graph)->edges = (Edge *)malloc(num_edges * sizeof(Edge));
  if ((*graph)->edges == NULL) {
    free((*graph)->nodes);
    free(*graph);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "failed to allocate memory for edges.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Allocate memory for CSR adjacency offsets (num_nodes + 1 for boundary)
  (*graph)->adj_offsets = (int *)malloc((num_nodes + 1) * sizeof(int));
  if ((*graph)->adj_offsets == NULL) {
    free((*graph)->edges);
    free((*graph)->nodes);
    free(*graph);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "failed to allocate memory for adjacency offsets.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Allocate memory for CSR adjacency indices (worst case: all edges bidirectional)
  (*graph)->adj_indices = (int *)malloc(num_edges * 2 * sizeof(int));
  if ((*graph)->adj_indices == NULL) {
    free((*graph)->adj_offsets);
    free((*graph)->edges);
    free((*graph)->nodes);
    free(*graph);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "failed to allocate memory for adjacency indices.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Create node hash table with load factor 0.50
  int hash_size = (num_nodes > HASH_TABLE_SIZE) ? num_nodes * 2 : HASH_TABLE_SIZE;
  error_code_t err_code = create_node_hash_table(&(*graph)->node_hash, hash_size, err_info);
  if (err_code != ERR_SUCCESS) {
    free((*graph)->adj_indices);
    free((*graph)->adj_offsets);
    free((*graph)->edges);
    free((*graph)->nodes);
    free(*graph);
    return err_code;
  }

  // Initialize CSR offsets array to zero
  memset((*graph)->adj_offsets, 0, (num_nodes + 1) * sizeof(int));

  // Set graph dimensions
  (*graph)->num_nodes = num_nodes;
  (*graph)->num_edges = num_edges;

  return ERR_SUCCESS;
}

void free_graph(Graph *graph) {
  if (graph == NULL) return;

  // Free all allocated memory components
  free(graph->nodes);
  free(graph->edges);
  free(graph->adj_offsets);
  free(graph->adj_indices);
  free_node_hash_table(graph->node_hash);
  free(graph);
}

error_code_t find_node_index(Graph *graph, uint32_t node_id, int *out_index, error_info_t *err_info) {
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(graph, err_info);
  CHECK_NULL(out_index, err_info);

  // Delegate to hash table lookup for O(1) average performance
  error_code_t err_code = lookup_node_hash(graph->node_hash, node_id, out_index, err_info);
  if (err_code != ERR_SUCCESS) return err_code;

  return ERR_SUCCESS;
}

// =================
// CSR representation functions
// =================

error_code_t build_csr_representation(Graph *graph, error_info_t *err_info) {
  // Null error check is already done in load_graph_from_binary
  
  // Allocate temporary array to count node degrees
  int *degree = (int *)calloc(graph->num_nodes, sizeof(int));
  CHECK_ALLOCATION(degree, err_info);

  error_code_t err_code = 0;

  // First pass: count degrees for each node
  for (int i = 0; i < graph->num_edges; i++) {
    Edge *edge = &graph->edges[i];

    // Get array indices for source and destination nodes
    int from_index, to_index;
    err_code = find_node_index(graph, edge->from_node, &from_index, err_info);
    if (err_code != ERR_SUCCESS) {
      free(degree);
      return err_code;
    }
    err_code = find_node_index(graph, edge->to_node, &to_index, err_info);
    if (err_code != ERR_SUCCESS) {
      free(degree);
      return err_code;
    }

    // Count outgoing edges for source node
    if (from_index >= 0) {
      degree[from_index] += 1;
    }
    // Count incoming edges for destination node (if bidirectional)
    if (!edge->one_way && to_index >= 0) {
      degree[to_index] += 1;
    }
  }

  // Build adjacency offsets using prefix sum
  graph->adj_offsets[0] = 0;
  for (int i = 0; i < graph->num_nodes; i++) {
    graph->adj_offsets[i + 1] = graph->adj_offsets[i] + degree[i];
  }

  // Reset degree array for second pass
  memset(degree, 0, graph->num_nodes * sizeof(int));

  // Second pass: populate adjacency indices
  for (int i = 0; i < graph->num_edges; i++) {
    Edge *edge = &graph->edges[i];

    // Get array indices for source and destination nodes
    int from_index, to_index;
    err_code = find_node_index(graph, edge->from_node, &from_index, err_info);
    if (err_code != ERR_SUCCESS) {
      free(degree);
      return err_code;
    }
    err_code = find_node_index(graph, edge->to_node, &to_index, err_info);
    if (err_code != ERR_SUCCESS) {
      free(degree);
      return err_code;
    }

    // Add edge index to source node's adjacency list
    if (from_index >= 0) {
      int pos = graph->adj_offsets[from_index] + degree[from_index];
      graph->adj_indices[pos] = i;
      degree[from_index] += 1;
    }
    // Add edge index to destination node's adjacency list (if bidirectional)
    if (!edge->one_way && to_index >= 0) {
      int pos = graph->adj_offsets[to_index] + degree[to_index];
      graph->adj_indices[pos] = i;
      degree[to_index] += 1;
    }
  }

  free(degree);
  return ERR_SUCCESS;
}

error_code_t get_adjacent_edges_csr(Graph *graph, int node_index, int *start_idx, int *end_idx, error_info_t *err_info) {
  // Null error check is already done in calling function
  
  // Validate node index bounds
  if (node_index < 0 || node_index >= graph->num_nodes) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Node index out of bounds.");
    return ERR_INVALID_ARGUMENT;
  }

  // Return range [start_idx, end_idx) for adjacency list
  *start_idx = graph->adj_offsets[node_index];
  *end_idx = graph->adj_offsets[node_index + 1];
  return ERR_SUCCESS;
}
