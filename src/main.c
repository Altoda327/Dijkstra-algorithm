#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "csv_parser.h"
#include "dijkstra.h"
#include "utils.h"

#define INFINITY_DBL DBL_MAX

/**
 * Display distances from source to all reachable nodes
 */
static void display_all_distances(Graph *graph, DijkstraResult *result, int source_id) {
    printf("\nDistances from node %d to all reachable nodes:\n", source_id);
    printf("=================================================\n");
    
    int reachable_count = 0;
    
    for (int i = 0; i < graph->num_nodes; i++) {
        // Skip unreachable nodes (infinite distance)
        if (result->distances[i] == INFINITY_DBL) {
            continue;
        }
        
        Node *node = &graph->nodes[i];
        printf("Node %d: %s\n", node->id, format_distance(result->distances[i]));
        reachable_count++;
    }
    
    printf("\nTotal reachable nodes: %d out of %d\n", reachable_count, graph->num_nodes);
}

/**
 * Display shortest path and optionally export to GPX file
 */
static void display_and_export_path(Graph *graph, DijkstraResult *result, 
                                  int target_id, const char *gpx_filename) {
    printf("\nShortest path to node %d:\n", target_id);
    printf("========================\n");
    
    // Display path information
    print_shortest_path(graph, result, target_id);
    
    // Export to GPX if filename is provided
    if (gpx_filename) {
        int path_length;
        int *path = get_path_to_node(graph, result, target_id, &path_length);
        
        if (path && path_length > 0) {
            if (export_path_to_gpx(graph, path, path_length, gpx_filename) == 0) {
                printf("Route successfully exported to: %s\n", gpx_filename);
            } else {
                fprintf(stderr, "Warning: Failed to export route to GPX file\n");
            }
            free(path);
        } else {
            fprintf(stderr, "Warning: Cannot export empty or invalid path\n");
        }
    }
}

/**
 * Main program entry point - handles command line arguments and orchestrates execution
 */
int main(int argc, char *argv[]) {
    // Check minimum required arguments
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse command line arguments
    const char *nodes_file = argv[1];
    const char *edges_file = argv[2];
    int source_id = atoi(argv[3]);
    int target_id = (argc > 4) ? atoi(argv[4]) : -1;
    const char *gpx_filename = (argc > 5) ? argv[5] : NULL;
    
    // Validate source node ID
    if (source_id <= 0) {
        fprintf(stderr, "Error: Source node ID must be positive\n");
        return 1;
    }
    
    // Load graph from CSV files
    printf("Loading graph from CSV files...\n");
    printf("Nodes file: %s\n", nodes_file);
    printf("Edges file: %s\n", edges_file);
    
    Graph *graph = load_graph_from_csv(nodes_file, edges_file);
    if (!graph) {
        fprintf(stderr, "Error: Failed to load graph from CSV files\n");
        return 1;
    }
    
    printf("Graph loaded successfully:\n");
    printf("- Nodes: %d\n", graph->num_nodes);
    printf("- Edges: %d\n", graph->num_edges);
    
    // Execute Dijkstra's algorithm
    printf("\nRunning Dijkstra's algorithm from node %d...\n", source_id);
    
    DijkstraResult *result = dijkstra(graph, source_id);
    if (!result) {
        fprintf(stderr, "Error: Failed to execute Dijkstra's algorithm\n");
        free_graph(graph);
        return 1;
    }
    
    printf("Algorithm completed successfully.\n");
    
    // Display results based on whether target is specified
    if (target_id != -1) {
        // Show path to specific target node
        display_and_export_path(graph, result, target_id, gpx_filename);
    } else {
        // Show distances to all reachable nodes
        display_all_distances(graph, result, source_id);
        
        if (gpx_filename) {
            printf("\nNote: GPX export requires a target node. Specify target_node_id to export route.\n");
        }
    }
    
    // Clean up allocated memory
    free_dijkstra_result(result);
    free_graph(graph);
    
    printf("\nProgram completed successfully.\n");
    return 0;
}
