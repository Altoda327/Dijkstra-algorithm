#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "utils.h"

// ================
// Distance Calculation Functions
// ================

/*
 * Haversine formula to calculate the great-circle distance 
 * between two points on the Earth's surface.
 *
 * Given:
 *   lat1, lon1 = coordinates of the first point (in degrees)
 *   lat2, lon2 = coordinates of the second point (in degrees)
 *   R = Earth's average radius (≈ 6371 km)
 *
 * Steps:
 *   1. Convert all coordinates from degrees to radians 
 *      (multiply by π/180).
 *   2. Compute the differences:
 *        Δlat = lat2 - lat1
 *        Δlon = lon2 - lon1
 *
 *   3. Apply the formula:
 *        a = sin²(Δlat / 2) +
 *            cos(lat1) * cos(lat2) * sin²(Δlon / 2)
 *
 *        c = 2 * atan2(√a, √(1 - a))
 *
 *        distance = R * c
 *
 * The resulting distance is in kilometers.
 *
 * Programming notes:
 *   - Use atan2() instead of asin() for better numerical stability
 *   - Handle edge cases: identical points (a = 0), antipodal points (a = 1)
 *   - For performance, precompute cos(lat1) and cos(lat2) if doing 
 *     multiple calculations
 *   - Consider using double precision for better accuracy
 */
double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371.0; // Earth's radius in kilometers

  // Convert degrees to radians
  double lat1_rad = lat1 * M_PI / 180.0;
  double lon1_rad = lon1 * M_PI / 180.0;
  double lat2_rad = lat2 * M_PI / 180.0;
  double lon2_rad = lon2 * M_PI / 180.0;

  // Calculate coordinate differences
  double dlat = lat2_rad - lat1_rad;
  double dlon = lon2_rad - lon1_rad;

  // Apply Haversine formula
  double a = sin(dlat / 2) * sin(dlat / 2) +
             cos(lat1_rad) * cos(lat2_rad) * 
             sin(dlon / 2) * sin(dlon / 2);

  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return R * c; // Distance in kilometers
}

int compare_node_distance(const void *a, const void *b) {
  NodeDistance *node_a = (NodeDistance *)a;
  NodeDistance *node_b = (NodeDistance *)b;

  if (node_a->distance_km < node_b->distance_km) return -1;
  if (node_a->distance_km > node_b->distance_km) return 1;
  return 0;
}

// ================
// Formatting and Display Functions
// ================

error_code_t format_distance(double distance, char *buffer, size_t buffer_size, DijkstraMode mode, error_info_t *err_info) {
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(buffer, err_info);

  if (buffer_size <= 0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Buffer size must be greater than zero.");
    return ERR_INVALID_ARGUMENT;
  }
  if (distance < 0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Distance cannot be negative.");
    return ERR_INVALID_ARGUMENT;
  }

  // Format distance based on the mode
  if (mode == DIJKSTRA_FASTEST_TIME) {
    // Time mode: display in minutes or hours
    if (distance >= 60) {
      snprintf(buffer, buffer_size, "%.2f Hours", distance / 60.0);
    } else {
      snprintf(buffer, buffer_size, "%.2f Minutes", distance);
    }
  } else {
    // Distance mode: display in meters or kilometers
    if (distance >= 1000) {
      snprintf(buffer, buffer_size, "%.2f km", distance / 1000.0);
    } else {
      snprintf(buffer, buffer_size, "%.2f m", distance);
    }
  }

  return ERR_SUCCESS;
}

void print_hash_table_stats(Graph *graph) {
  if (graph == NULL || graph->node_hash == NULL) {
    printf("No hash table available.\n");
    return;
  }

  printf("\n=== HASH TABLE STATS ===\n");
  printf("Hash table size: %d\n", graph->node_hash->size);
  printf("Hash table count: %d\n", graph->node_hash->count);
  printf("Load factor: %.2f\n", (double)graph->node_hash->count / graph->node_hash->size);

  // Count collisions and analyze chain lengths
  int used_buckets = 0;
  int max_chain_length = 0;
  int total_chain_length = 0;

  for (int i = 0; i < graph->node_hash->size; i++) {
    if (graph->node_hash->buckets[i] != NULL) {
      used_buckets++;
      int chain_length = 0;
      NodeHashEntry *entry = graph->node_hash->buckets[i];
      
      // Count entries in this bucket's chain
      while (entry) {
        chain_length++;
        entry = entry->next;
      }

      total_chain_length += chain_length;
      if (chain_length > max_chain_length) {
        max_chain_length = chain_length;
      }
    }
  }

  printf("Used buckets: %d / %d (%.2f%%)\n", 
         used_buckets, graph->node_hash->size, 
         (double)used_buckets / graph->node_hash->size * 100.0);
  printf("Max chain length: %d\n", max_chain_length);
  printf("Average chain length: %.2f\n", 
         used_buckets > 0 ? (double)total_chain_length / used_buckets : 0.0);
}

void print_usage(const char *program_name) {
  if (program_name == NULL) {
    program_name = "program";
  }

  printf("Usage:\n");
  printf("\nMode1:  %s <nodes.bin> <edges.bin> <source_node_id> <target_node_id> [output.gpx]\n", program_name);
  printf("  nodes.bin:  Binary file containing node data.\n");
  printf("  edges.bin:  Binary file containing edge data.\n");
  printf("  source_node_id:  ID of the source node (uint32_t).\n");
  printf("  target_node_id:  ID of the target node (uint32_t).\n");
  printf("  output.gpx:  Optional GPX file to save the path.\n");

  printf("\nMode2:  %s <nodes.bin> <edges.bin> -c [output.gpx]\n", program_name);
  printf("  -c:  Enter coordinate mode to select source and target nodes interactively.\n");
  printf("In coordinate mode, you input source and target coordinates and the program finds the 5 nearest nodes to each coordinate.\n");
}

// ================
// Node and Edge Functions
// ===============

error_code_t find_nearest_nodes(Graph *graph, double target_lat, double target_lon, int *count, NodeDistance **nodes, error_info_t *err_info) {
  CHECK_NULL(count, err_info);

  // Validate coordinate bounds
  if (target_lat < -90.0 || target_lat > 90.0 || target_lon < -180.0 || target_lon > 180.0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Coordinates out of bounds.");
    return ERR_INVALID_ARGUMENT;
  }

  // Allocate array for all nodes with distance calculations
  NodeDistance *distances = malloc(graph->num_nodes * sizeof(NodeDistance));
  CHECK_ALLOCATION(distances, err_info);

  // Calculate distance from target to each node
  for (int i = 0; i < graph->num_nodes; i++) {
    Node *node = &graph->nodes[i];
    distances[i].node_index = i;
    distances[i].node_id = node->node_id;
    distances[i].latitude = node->latitude;
    distances[i].longitude = node->longitude;
    distances[i].distance_km = haversine_distance(target_lat, target_lon, 
                                               node->latitude, node->longitude);
  }

  // Sort by distance to find nearest nodes
  qsort(distances, graph->num_nodes, sizeof(NodeDistance), compare_node_distance);

  // Return the 5 nearest nodes (or all nodes if less than 5)
  *count = graph->num_nodes < 5 ? graph->num_nodes : 5;
  *nodes = malloc(*count * sizeof(NodeDistance));
  if (*nodes == NULL) {
    free(distances);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "Failed to allocate memory for nearest nodes.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Copy nearest nodes to output array
  memcpy(*nodes, distances, *count * sizeof(NodeDistance));
  free(distances);

  return ERR_SUCCESS;
}

error_code_t select_node_from_list(NodeDistance *nodes, int count, const char *description, uint32_t *selected_id, error_info_t *err_info) {
  CHECK_NULL(nodes, err_info);
  CHECK_NULL(selected_id, err_info);
  CHECK_NULL(description, err_info);
  if (count <= 0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Count must be greater than zero.");
    return ERR_INVALID_ARGUMENT;
  }

  // Display available nodes
  printf("\n=== %s ===\n", description);
  printf("Nearest nodes:\n");

  for (int i = 0; i < count; i++) {
    printf("%d. Node ID %u - (%.6f, %.6f) - Distance: %.2f km\n", 
           i + 1, nodes[i].node_id, 
           nodes[i].latitude, nodes[i].longitude, nodes[i].distance_km);
  }

  // Get user selection with input validation
  char buffer[32];
  int choice;
  do {
    printf("Select a node (1-%d): ", count);
    if (fgets(buffer, sizeof(buffer), stdin)) {
      buffer[strcspn(buffer, "\n")] = 0; // Remove newline
      
      if (sscanf(buffer, "%d", &choice) != 1) {
        printf("Invalid input. Please enter a number between 1 and %d.\n", count);
        choice = 0;
      }
    } else {
      SET_ERROR(err_info, ERR_INPUT_ERROR, "Failed to read input.");
      return ERR_INPUT_ERROR;
    }

    if (choice < 1 || choice > count) {
      printf("Invalid choice. Please select a number between 1 and %d.\n", count);
    }
  } while (choice < 1 || choice > count);

  *selected_id = nodes[choice - 1].node_id;

  return ERR_SUCCESS;
}

// ================
// Interactive Functions
// ===============

error_code_t interactive_coordinate_mode(Graph *graph, uint32_t *source_id, uint32_t *target_id, error_info_t *err_info) {
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(graph, err_info);
  CHECK_NULL(source_id, err_info);
  CHECK_NULL(target_id, err_info);

  char buffer[128];
  double start_lat, start_lon, end_lat, end_lon;
  
  printf("\n=== COORDINATE MODE ===\n");
  
  // Get source coordinates
  printf("Enter source start coordinates (latitude,longitude): ");
  if (fgets(buffer, sizeof(buffer), stdin)) {
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
    if (sscanf(buffer, "%lf,%lf", &start_lat, &start_lon) != 2) {
      SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Invalid coordinates format.");
      return ERR_INVALID_ARGUMENT;
    }
  } else {
    SET_ERROR(err_info, ERR_INPUT_ERROR, "Failed to read input.");
    return ERR_INPUT_ERROR;
  }

  // Get target coordinates
  printf("Enter target end coordinates (latitude,longitude): ");
  if (fgets(buffer, sizeof(buffer), stdin)) {
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
    if (sscanf(buffer, "%lf,%lf", &end_lat, &end_lon) != 2) {
      SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Invalid coordinates format.");
      return ERR_INVALID_ARGUMENT;
    }
  } else {
    SET_ERROR(err_info, ERR_INPUT_ERROR, "Failed to read input.");
    return ERR_INPUT_ERROR;
  }

  error_code_t err_code;

  // Find and select source node
  int start_count;
  NodeDistance *start_nodes;
  err_code = find_nearest_nodes(graph, start_lat, start_lon, &start_count, &start_nodes, err_info);
  if (err_code != ERR_SUCCESS) return err_code;

  err_code = select_node_from_list(start_nodes, start_count, "Select Source Node", source_id, err_info);
  free(start_nodes);
  if (err_code != ERR_SUCCESS) return err_code;

  // Find and select target node
  int end_count;
  NodeDistance *end_nodes;
  err_code = find_nearest_nodes(graph, end_lat, end_lon, &end_count, &end_nodes, err_info);
  if (err_code != ERR_SUCCESS) return err_code;

  err_code = select_node_from_list(end_nodes, end_count, "Select Target Node", target_id, err_info);
  free(end_nodes);
  if (err_code != ERR_SUCCESS) return err_code;

  printf("Selected Source Node ID: %u\n", *source_id);
  printf("Selected Target Node ID: %u\n", *target_id);
  return ERR_SUCCESS;
}

// ================
// File Export Functions
// ===============

error_code_t export_path_to_gpx(Graph *graph, int *path, int path_length, const char *filename, DijkstraMode mode, DijkstraResult *result, error_info_t *err_info) {
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(graph, err_info);
  CHECK_NULL(path, err_info);
  CHECK_NULL(filename, err_info);
  CHECK_NULL(result, err_info);
  
  if (path_length <= 0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Path length must be greater than zero.");
    return ERR_INVALID_ARGUMENT;
  }

  // Open GPX file for writing
  FILE *gpx_file = fopen(filename, "w");
  if (gpx_file == NULL) {
    SET_ERROR(err_info, ERR_FILE_WRITE, "Failed to open GPX file for writing.");
    return ERR_FILE_WRITE;
  }

  // Get current timestamp for GPX metadata
  time_t raw_time;
  struct tm *time_info;
  char time_buffer[32];

  time(&raw_time);
  time_info = gmtime(&raw_time);
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%SZ", time_info);

  // Calculate total distance/time based on mode
  double total_value = 0.0;
  const char *mode_description;
  
  if (mode == DIJKSTRA_FASTEST_TIME) {
    // For time mode, use the result from Dijkstra which already calculated travel time
    total_value = result->distances[result->target_index];
    mode_description = "Fastest Time Route";
  } else {
    // For distance mode, calculate actual geographic distance using haversine
    for (int i = 0; i < path_length - 1; i++) {
      // Validate node indices
      if (path[i] < 0 || path[i] >= graph->num_nodes || 
          path[i + 1] < 0 || path[i + 1] >= graph->num_nodes) {
        fclose(gpx_file);
        SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Invalid node index in path.");
        return ERR_INVALID_ARGUMENT;
      }
      
      Node *from = &graph->nodes[path[i]];
      Node *to = &graph->nodes[path[i + 1]];
      
      // Use haversine distance for accurate geographic calculation
      double segment_distance = haversine_distance(from->latitude, from->longitude,
                                                  to->latitude, to->longitude);
      total_value += segment_distance * 1000.0; // Convert to meters
    }
    mode_description = "Shortest Distance Route";
  }

  // Format total value for display
  char value_buffer[64];
  error_code_t err_code = format_distance(total_value, value_buffer, sizeof(value_buffer), mode, err_info);
  if (err_code != ERR_SUCCESS) {
    fclose(gpx_file);
    return err_code;
  }

  // Write GPX file header with metadata
  fprintf(gpx_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(gpx_file, "<gpx version=\"1.1\" creator=\"Dijkstra Route Planner\" ");
  fprintf(gpx_file, "xmlns=\"http://www.topografix.com/gpx/1/1\">\n");
  fprintf(gpx_file, "  <metadata>\n");
  fprintf(gpx_file, "    <name>%s</name>\n", mode_description);
  fprintf(gpx_file, "    <desc>Route from node %u to node %u (%s) - Mode: %s</desc>\n", 
          graph->nodes[path[0]].node_id, 
          graph->nodes[path[path_length-1]].node_id,
          value_buffer,
          mode == DIJKSTRA_FASTEST_TIME ? "Fastest Time" : "Shortest Distance");
  fprintf(gpx_file, "    <time>%s</time>\n", time_buffer);
  fprintf(gpx_file, "  </metadata>\n");
  
  // Write waypoints for start and end
  fprintf(gpx_file, "  <wpt lat=\"%.6f\" lon=\"%.6f\">\n", 
          graph->nodes[path[0]].latitude, graph->nodes[path[0]].longitude);
  fprintf(gpx_file, "    <name>Start: Node %u</name>\n", graph->nodes[path[0]].node_id);
  fprintf(gpx_file, "    <desc>Route starting point</desc>\n");
  fprintf(gpx_file, "  </wpt>\n");
  
  fprintf(gpx_file, "  <wpt lat=\"%.6f\" lon=\"%.6f\">\n", 
          graph->nodes[path[path_length-1]].latitude, graph->nodes[path[path_length-1]].longitude);
  fprintf(gpx_file, "    <name>End: Node %u</name>\n", graph->nodes[path[path_length-1]].node_id);
  fprintf(gpx_file, "    <desc>Route destination</desc>\n");
  fprintf(gpx_file, "  </wpt>\n");
  
  // Start track section
  fprintf(gpx_file, "  <trk>\n");
  fprintf(gpx_file, "    <name>%s</name>\n", mode_description);
  fprintf(gpx_file, "    <desc>Calculated using Dijkstra's algorithm - %s</desc>\n", 
          mode == DIJKSTRA_FASTEST_TIME ? "Optimized for travel time" : "Optimized for distance");
  fprintf(gpx_file, "    <trkseg>\n");
  
  // Write each waypoint in the path
  for (int i = 0; i < path_length; i++) {
    int node_index = path[i];
    
    // Validate node index bounds
    if (node_index < 0 || node_index >= graph->num_nodes) {
      fclose(gpx_file);
      SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Invalid node index in path.");
      return ERR_INVALID_ARGUMENT;
    }
    
    Node *node = &graph->nodes[node_index];
    
    // Write track point with coordinates
    fprintf(gpx_file, "      <trkpt lat=\"%.6f\" lon=\"%.6f\">\n",
            node->latitude, node->longitude);
    fprintf(gpx_file, "        <name>Node %u</name>\n", node->node_id);
    
    // Add cumulative distance/time information for intermediate points
    if (i > 0) {
      double cumulative_value;
      if (mode == DIJKSTRA_FASTEST_TIME) {
        cumulative_value = result->distances[node_index];
      } else {
        // Calculate cumulative distance up to this point
        cumulative_value = 0.0;
        for (int j = 0; j < i; j++) {
          Node *from = &graph->nodes[path[j]];
          Node *to = &graph->nodes[path[j + 1]];
          cumulative_value += haversine_distance(from->latitude, from->longitude,
                                                to->latitude, to->longitude) * 1000.0;
        }
      }
      
      char cumulative_buffer[64];
      format_distance(cumulative_value, cumulative_buffer, sizeof(cumulative_buffer), mode, err_info);
      fprintf(gpx_file, "        <desc>Cumulative: %s</desc>\n", cumulative_buffer);
    }
    
    fprintf(gpx_file, "      </trkpt>\n");
  }
  
  // Close track section and GPX file
  fprintf(gpx_file, "    </trkseg>\n");
  fprintf(gpx_file, "  </trk>\n");
  fprintf(gpx_file, "</gpx>\n");
  
  fclose(gpx_file);
  
  // printf("Route contains %d waypoints\n", path_length);
  // printf("Total %s: %s\n", 
  //     mode == DIJKSTRA_FASTEST_TIME ? "travel time" : "distance", 
  //     value_buffer);
  
  return ERR_SUCCESS;
}
