#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "utils.h"

/**
 * Print detailed information about a node including its ID and coordinates
 */
void print_node_info(Node *node) {
    if (!node) {
        log_error("Node pointer is null");
        return;
    }
    
    printf("Node %d: (%.6f, %.6f)\n", node->id, node->latitude, node->longitude);
}

/**
 * Print detailed information about an edge including all its properties
 */
void print_edge_info(Edge *edge) {
    if (!edge) {
        log_error("Edge pointer is null");
        return;
    }
    
    printf("Edge: %d -> %d, %s, %s, %.1fm, %s\n",
           edge->from_node, edge->to_node, edge->name, edge->highway_type,
           edge->length, edge->oneway ? "oneway" : "bidirectional");
}

/**
 * Compare two floating-point numbers with epsilon tolerance for precision
 */
int compare_doubles(double a, double b) {
    const double epsilon = 1e-9;
    
    if (fabs(a - b) < epsilon) {
        return 0;  // Equal within tolerance
    }
    
    return (a < b) ? -1 : 1;
}

/**
 * Format distance in meters to human-readable string (km or m)
 */
char* format_distance(double distance_meters) {
    static char buffer[64];
    
    // Convert to kilometers if distance is 1000m or more
    if (distance_meters >= 1000.0) {
        snprintf(buffer, sizeof(buffer), "%.2f km", distance_meters / 1000.0);
    } else {
        snprintf(buffer, sizeof(buffer), "%.0f m", distance_meters);
    }
    
    return buffer;
}

/**
 * Display program usage instructions to the user
 */
void print_usage(const char *program_name) {
    if (!program_name) {
        program_name = "dijkstra";
    }
    
    printf("Usage: %s <nodes.csv> <edges.csv> <source_node_id> [target_node_id] [output.gpx]\n", program_name);
    printf("Arguments:\n");
    printf("  nodes.csv       - CSV file containing node data (id, latitude, longitude)\n");
    printf("  edges.csv       - CSV file containing edge data\n");
    printf("  source_node_id  - Starting node ID for Dijkstra's algorithm\n");
    printf("  target_node_id  - Optional target node ID (shows path if specified)\n");
    printf("  output.gpx      - Optional GPX filename for path export\n");
}

/**
 * Log error messages with file location and system error information
 */
void log_error(const char *message) {
    if (!message) {
        message = "Unknown error";
    }
    
    fprintf(stderr, "Error in %s at line %d: %s", __FILE__, __LINE__, message);
    
    // Add system error description if available
    if (errno != 0) {
        fprintf(stderr, " - %s", strerror(errno));
    }
    
    fprintf(stderr, "\n");
}

/**
 * Export a path to GPX format file for GPS visualization
 */
int export_path_to_gpx(Graph *graph, int *path, int path_length, const char *filename) {
    // Validate input parameters
    if (!graph || !path || !filename) {
        log_error("Invalid parameters for GPX export");
        return -1;
    }
    
    if (path_length <= 0) {
        log_error("Path length must be positive for GPX export");
        return -1;
    }
    
    // Open output file for writing
    FILE *gpx_file = fopen(filename, "w");
    if (!gpx_file) {
        log_error("Failed to create GPX file");
        return -1;
    }
    
    // Get current timestamp for GPX metadata
    time_t raw_time;
    struct tm *time_info;
    char time_buffer[32];
    
    time(&raw_time);
    time_info = gmtime(&raw_time);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%SZ", time_info);
    
    // Write GPX file header with metadata
    fprintf(gpx_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(gpx_file, "<gpx version=\"1.1\" creator=\"Dijkstra Route Planner\" ");
    fprintf(gpx_file, "xmlns=\"http://www.topografix.com/gpx/1/1\">\n");
    fprintf(gpx_file, "  <metadata>\n");
    fprintf(gpx_file, "    <name>Shortest Path Route</name>\n");
    fprintf(gpx_file, "    <desc>Route calculated using Dijkstra's algorithm</desc>\n");
    fprintf(gpx_file, "    <time>%s</time>\n", time_buffer);
    fprintf(gpx_file, "  </metadata>\n");
    
    // Start track section
    fprintf(gpx_file, "  <trk>\n");
    fprintf(gpx_file, "    <name>Shortest Path</name>\n");
    fprintf(gpx_file, "    <trkseg>\n");
    
    // Write each waypoint in the path
    for (int i = 0; i < path_length; i++) {
        int node_index = path[i];
        
        // Validate node index bounds
        if (node_index < 0 || node_index >= graph->num_nodes) {
            log_error("Invalid node index in path");
            fclose(gpx_file);
            return -1;
        }
        
        Node *node = &graph->nodes[node_index];
        
        // Write track point with coordinates
        fprintf(gpx_file, "      <trkpt lat=\"%.6f\" lon=\"%.6f\">\n",
                node->latitude, node->longitude);
        fprintf(gpx_file, "        <name>Node %d</name>\n", node->id);
        fprintf(gpx_file, "      </trkpt>\n");
    }
    
    // Close track section and GPX file
    fprintf(gpx_file, "    </trkseg>\n");
    fprintf(gpx_file, "  </trk>\n");
    fprintf(gpx_file, "</gpx>\n");
    
    fclose(gpx_file);
    
    printf("Path exported to GPX file: %s\n", filename);
    return 0;
}
