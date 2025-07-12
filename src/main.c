#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "bin_loader.h"
#include "dijkstra.h"
#include "graph.h"
#include "utils.h"

// =================
// Main function
// =================

int main(int argc, char *argv[]) {
  // Check command line arguments - minimum required: program nodes_file edges_file
  if (argc < 3) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  // Parse command line arguments
  const char *nodes_file = argv[1];
  const char *edges_file = argv[2];

  // Initialize variables for different execution modes
  bool coordinate_mode = false;
  uint32_t source_id = 0;
  uint32_t target_id = 0;
  const char *gpx_file = NULL;

  // Parse optional arguments to determine execution mode
  if (argc >= 4 && strcmp(argv[3], "-c") == 0) {
    // Coordinate mode: user will input coordinates interactively
    coordinate_mode = true;
    gpx_file = (argc >= 5) ? argv[4] : NULL;
  } else if (argc >= 5) {
    // Direct node ID mode: source and target specified as arguments
    source_id = (uint32_t)atoi(argv[3]);
    target_id = (uint32_t)atoi(argv[4]);
    gpx_file = (argc >= 6) ? argv[5] : NULL;
  } else {
    // Invalid arguments provided
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  // Display program header and file information
  printf("\n=== GRAPH LOADER ===\n");
  printf("Loading graph from files:\n");
  printf("  Nodes: %s\n", nodes_file);
  printf("  Edges: %s\n", edges_file);

  // Initialize error handling system
  error_info_t err_info;
  error_code_t err_code;

  // Load the graph from binary files into memory
  Graph *graph = NULL;
  err_code = load_graph_from_binary(&graph, nodes_file, edges_file, &err_info);
  if (err_code != ERR_SUCCESS) {
    print_error(&err_info);
    return EXIT_FAILURE;
  }

  // Display comprehensive graph statistics and memory usage
  printf("\n=== GRAPH SUMMARY ===\n");
  printf("Total nodes: %d\n", graph->num_nodes);
  printf("Total edges: %d\n", graph->num_edges);
  printf("Memory usage:\n");
  printf("  Nodes: %.2f MB\n", (double)(graph->num_nodes * 
        sizeof(Node)) / (1024 * 1024));
  printf("  Edges: %.2f MB\n", (double)(graph->num_edges *
        sizeof(Edge)) / (1024 * 1024));
  printf("  CSR: %.2f MB\n", (double)((graph->num_nodes + graph->num_edges) *
        sizeof(int)) / (1024 * 1024));
  printf("  Hash Table: %.2f MB\n", (double)(graph->node_hash->size *
        sizeof(NodeHashEntry *)) / (1024 * 1024));

  // Display hash table performance statistics
  print_hash_table_stats(graph);

  // Handle coordinate mode if enabled - allows user to input lat/lon coordinates
  if (coordinate_mode) {
    err_code = interactive_coordinate_mode(graph, &source_id, &target_id, &err_info);
    if (err_code != ERR_SUCCESS) {
      print_error(&err_info);
      free_graph(graph);
      return EXIT_FAILURE;
    }
  }

  // Prompt user to choose Dijkstra algorithm mode
  char buffer[32];
  int dijkstra_mode;
  printf("\nChoose Dijkstra mode:\n");
  printf("  1. Dijkstra shortest distance\n");
  printf("  2. Dijkstra fastest path\n");
  printf("Enter choice (1 or 2): ");

  // Read and validate user input for Dijkstra mode
  if (fgets(buffer, sizeof(buffer), stdin)) {
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline character
    if (sscanf(buffer, "%d", &dijkstra_mode) != 1 || 
        (dijkstra_mode != 1 && dijkstra_mode != 2)) {
      fprintf(stderr, "Invalid choice. Please enter 1 or 2.\n");
      free_graph(graph);
      return EXIT_FAILURE;
    }
  } else {
    fprintf(stderr, "Error reading input.\n");
    free_graph(graph);
    return EXIT_FAILURE;
  }
  DijkstraMode mode = (DijkstraMode)dijkstra_mode;

  // Execute Dijkstra's algorithm to find shortest path
  printf("\n=== RUNNING DIJKSTRA ===\n");
  printf("Computing shortest path from node %d to node %d...\n", 
      source_id, target_id);

  DijkstraResult result;
  err_code = dijkstra_shortest_path(graph, source_id, target_id, mode, &result, &err_info);
  if (err_code != ERR_SUCCESS) {
    print_error(&err_info);
    free_graph(graph);
    return EXIT_FAILURE;
  }

  // Process and display results
  if (!result.target_found) {
    printf("No path found from node %d to node %d.\n", source_id, target_id);
  } else {
    printf("Path found from node %d to node %d:\n", 
        source_id, target_id);

    // Extract the shortest distance/time from results
    double distance;
    err_code = get_shortest_distance(&result, &distance, &err_info);
    if (err_code != ERR_SUCCESS) {
      print_error(&err_info);
      free_dijkstra_result(&result);
      free_graph(graph);
      return EXIT_FAILURE;
    }

    // Extract the actual path (sequence of nodes)
    int path_length;
    int *path;
    err_code = get_shortest_path(graph, &result, &path_length, &path, &err_info);
    if (err_code != ERR_SUCCESS) {
      print_error(&err_info);
      free_dijkstra_result(&result);
      free_graph(graph);
      return EXIT_FAILURE;
    }

    // Display path information with appropriate units
    if (path && path_length > 0) {
      printf("Path contains %d nodes.\n", path_length);

      // Display results with appropriate units based on mode
      if (mode == DIJKSTRA_FASTEST_TIME) {
        if (distance >= 60) {
          printf("Total time: %.2f Hours\n", distance / 60.0);
        } else {
          printf("Total time: %.2f Minutes\n", distance);
        }
      } else {
        if (distance >= 1000) {
          printf("Total distance: %.2f Km\n", distance / 1000.0);
        } else {
          printf("Total distance: %.2f Meters\n", distance);
        }
      }

      // Export path to GPX file if filename was provided
      if (gpx_file) {
        err_code = export_path_to_gpx(graph, path, path_length, gpx_file, mode, &result, &err_info);
        if (err_code != ERR_SUCCESS) {
          print_error(&err_info);
          free(path);
          free_dijkstra_result(&result);
          free_graph(graph);
          return EXIT_FAILURE;
        }
        printf("Path exported to GPX file: %s\n", gpx_file);
      }
    }
  }

  // Clean up all allocated resources
  free_dijkstra_result(&result);
  free_graph(graph);

  printf("\n=== ANALYSIS COMPLETE ===\n");
  return EXIT_SUCCESS;
}
