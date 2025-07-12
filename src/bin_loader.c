#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bin_loader.h"

error_code_t load_nodes_from_binary(Graph *graph, FILE *file, error_info_t *err_info) {
  // Null error check is already done in load_graph_from_binary
  
  // Read all nodes from binary file in one operation
  size_t nodes_read = fread(graph->nodes, sizeof(Node), graph->num_nodes, file);
  if (nodes_read != (size_t)graph->num_nodes) {
    SET_ERROR(err_info, ERR_FILE_READ, "Failed to read nodes from binary file.");
    return ERR_FILE_READ;
  }

  // Initialize node hash table for efficient node lookup
  error_code_t err_code;
  for (int i = 0; i < graph->num_nodes; i++) {
    // Insert each node into hash table mapping node_id to array index
    err_code = insert_node_hash(graph->node_hash, graph->nodes[i].node_id, i, err_info);
    if (err_code != ERR_SUCCESS) return err_code;
  }

  // Debug output for successful loading
  printf("Debug: Loaded %d nodes from binary file.\n", graph->num_nodes);
  
  return ERR_SUCCESS;
}

error_code_t load_edges_from_binary(Graph *graph, FILE *file, error_info_t *err_info) {
  // Null error check is already done in load_graph_from_binary
  
  // Read all edges from binary file in one operation
  size_t edges_read = fread(graph->edges, sizeof(Edge), graph->num_edges, file);
  if (edges_read != (size_t)graph->num_edges) {
    SET_ERROR(err_info, ERR_FILE_READ, "Failed to read edges from binary file.");
    return ERR_FILE_READ;
  }
  
  // Validate that all node_id references in edges exist in the graph
  error_code_t err_code;
  for (int i = 0; i < graph->num_edges; i++) {
    Edge *edge = &graph->edges[i];
    int out_index;
    
    // Validate source node exists
    err_code = find_node_index(graph, edge->from_node, &out_index, err_info);
    if (err_code != ERR_SUCCESS) return err_code;
    
    // Validate destination node exists
    err_code = find_node_index(graph, edge->to_node, &out_index, err_info);
    if (err_code != ERR_SUCCESS) return err_code;
  }
  
  // NOTE: Do not close the file here - caller will handle file closure
  return ERR_SUCCESS;
}

error_code_t load_graph_from_binary(Graph **graph, const char *nodes_filename, const char *edges_filename, error_info_t *err_info) {
  // Input validation - ensure all required parameters are provided
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(graph, err_info);
  CHECK_NULL(nodes_filename, err_info);
  CHECK_NULL(edges_filename, err_info);
  
  // Open nodes binary file for reading
  FILE *nodes_file = fopen(nodes_filename, "rb");
  if (nodes_file == NULL) {
    SET_ERROR(err_info, ERR_FILE_NOT_FOUND, "Failed to open nodes binary file.");
    return ERR_FILE_NOT_FOUND;
  }
  
  // Read number of nodes from file header
  uint32_t num_nodes;
  if (fread(&num_nodes, sizeof(uint32_t), 1, nodes_file) != 1) {
    SET_ERROR(err_info, ERR_FILE_READ, "Failed to read number of nodes from binary file.");
    fclose(nodes_file);
    return ERR_FILE_READ;
  }
  
  // Open edges binary file for reading
  FILE *edges_file = fopen(edges_filename, "rb");
  if (edges_file == NULL) {
    SET_ERROR(err_info, ERR_FILE_NOT_FOUND, "Failed to open edges binary file.");
    fclose(nodes_file);
    return ERR_FILE_NOT_FOUND;
  }
  
  // Read number of edges from file header
  uint32_t num_edges;
  if (fread(&num_edges, sizeof(uint32_t), 1, edges_file) != 1) {
    SET_ERROR(err_info, ERR_FILE_READ, "Failed to read number of edges from binary file.");
    fclose(nodes_file);
    fclose(edges_file);
    return ERR_FILE_READ;
  }
  
  // Create graph structure with the read dimensions
  error_code_t err_code = create_graph(graph, (int)num_nodes, (int)num_edges, err_info);
  if (err_code != ERR_SUCCESS) {
    fclose(nodes_file);
    fclose(edges_file);
    return err_code;
  }

  // Load node data from binary file
  err_code = load_nodes_from_binary(*graph, nodes_file, err_info);
  if (err_code != ERR_SUCCESS) {
    free_graph(*graph);
    fclose(nodes_file);
    fclose(edges_file);
    return err_code;
  }
 
  // Load edge data from binary file
  err_code = load_edges_from_binary(*graph, edges_file, err_info);
  if (err_code != ERR_SUCCESS) {
    free_graph(*graph);
    fclose(nodes_file);
    fclose(edges_file);
    return err_code;
  }
  
  // Close both files after successful loading
  fclose(nodes_file);
  fclose(edges_file);
 
  // Build CSR (Compressed Sparse Row) representation for efficient graph operations
  err_code = build_csr_representation(*graph, err_info);
  if (err_code != ERR_SUCCESS) {
    free_graph(*graph);
    return err_code;
  }
  
  return ERR_SUCCESS;
}
