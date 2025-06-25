#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "csv_parser.h"
#include "utils.h"

/**
 * Parse CSV line into fields, handling empty fields properly
 * @param line Input line to parse
 * @param fields Array to store parsed field pointers
 * @param max_fields Maximum number of fields to parse
 * @return Number of fields parsed
 */
static int parse_csv_fields(char *line, char **fields, int max_fields) {
    int field_count = 0;
    char *start = line;
    char *current = line;
    
    while (*current != '\0' && field_count < max_fields) {
        // Find next comma or end of string
        while (*current != ',' && *current != '\0') {
            current++;
        }
        
        // Calculate field length
        int field_length = current - start;
        
        if (field_length == 0) {
            // Empty field - assign NULL
            fields[field_count] = NULL;
        } else {
            // Non-empty field - create a copy
            fields[field_count] = malloc(field_length + 1);
            if (fields[field_count] != NULL) {
                strncpy(fields[field_count], start, field_length);
                fields[field_count][field_length] = '\0';
                
                // Remove whitespace from field
                trim_whitespace(fields[field_count]);
                
                // If empty after trimming, free and set to NULL
                if (strlen(fields[field_count]) == 0) {
                    free(fields[field_count]);
                    fields[field_count] = NULL;
                }
            }
        }
        
        field_count++;
        
        // If we found a comma, move to next field
        if (*current == ',') {
            current++;
            start = current;
        }
    }
    
    return field_count;
}

/**
 * Free memory allocated for CSV fields
 * @param fields Array of field pointers to free
 * @param field_count Number of fields in the array
 */
static void free_csv_fields(char **fields, int field_count) {
    for (int i = 0; i < field_count; i++) {
        if (fields[i] != NULL) {
            free(fields[i]);
            fields[i] = NULL;
        }
    }
}

int count_csv_lines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    int line_count = 0;
    char buffer[1024];

    // Count all lines in file
    while (fgets(buffer, sizeof(buffer), file)) {
        line_count++;
    }

    fclose(file);
    
    // Exclude header line if file is not empty
    return line_count > 0 ? line_count - 1 : 0;
}

Graph* load_graph_from_csv(const char *nodes_file, const char *edges_file) {
    // Count lines in both CSV files
    int num_nodes = count_csv_lines(nodes_file);
    int num_edges = count_csv_lines(edges_file);

    if (num_nodes < 0 || num_edges < 0) {
        log_error("Failed to count lines in CSV files");
        return NULL;
    }

    // Create new graph structure
    Graph *graph = create_graph(num_nodes, num_edges);
    if (!graph) {
        log_error("Failed to create graph structure");
        return NULL;
    }

    // Parse nodes from CSV file
    if (parse_nodes_csv(nodes_file, graph->nodes, num_nodes) != num_nodes) {
        log_error("Failed to parse nodes from CSV file");
        free_graph(graph);
        return NULL;
    }

    // Parse edges from CSV file
    int actual_edges = parse_edges_csv(edges_file, graph->edges, num_edges);
    if (actual_edges < 0) {
        log_error("Failed to parse edges from CSV file");
        free_graph(graph);
        return NULL;
    }
    
    // Update actual number of edges parsed
    graph->num_edges = actual_edges;

    // Build adjacency list for efficient graph traversal
    for (int i = 0; i < graph->num_edges; i++) {
        Edge *edge = &graph->edges[i];
        
        // Find node indices from node IDs
        int from_index = find_node_index(graph, edge->from_node);
        int to_index = find_node_index(graph, edge->to_node);
        
        // Check if both nodes exist in the graph
        if (from_index == -1 || to_index == -1) {
            printf("Warning: Edge %d references non-existent nodes (%d -> %d)\n", 
                   i, edge->from_node, edge->to_node);
            continue;
        }
        
        // Update edge to use node indices instead of IDs
        edge->from_node = from_index;
        edge->to_node = to_index;
        
        // Add edge to adjacency list
        add_edge_to_adj_list(graph, from_index, i);
        if (!edge->oneway) {
            add_edge_to_adj_list(graph, to_index, i);
        }
    }

    return graph;
}

int parse_nodes_csv(const char *filename, Node *nodes, int max_nodes) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_error("Failed to open nodes CSV file");
        return -1;
    }

    char line[512];
    int count = 0;

    // Skip header line
    fgets(line, sizeof(line), file);

    // Parse each line of the CSV file
    while (fgets(line, sizeof(line), file) && count < max_nodes) {
        trim_whitespace(line);

        // Parse CSV fields from line
        char *fields[3];
        int field_count = parse_csv_fields(line, fields, 3);
        
        // Validate required fields are present
        if (field_count < 3 || !fields[0] || !fields[1] || !fields[2]) {
            printf("Warning: Invalid node line, skipping\n");
            free_csv_fields(fields, field_count);
            continue;
        }

        // Extract node data from fields
        int id = atoi(fields[0]);
        double latitude = atof(fields[1]);
        double longitude = atof(fields[2]);

        // Store node data
        nodes[count].id = id;
        nodes[count].latitude = latitude;
        nodes[count].longitude = longitude;
        count++;
        
        // Clean up allocated field memory
        free_csv_fields(fields, field_count);
    }

    fclose(file);
    return count;
}

int parse_edges_csv(const char *filename, Edge *edges, int max_edges) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_error("Failed to open edges CSV file");
        return -1;
    }
    
    char line[1024];
    int count = 0;
    int line_number = 0;
    
    // Skip header line
    if (fgets(line, sizeof(line), file)) {
        line_number++;
    }
    
    // Parse each line of the CSV file
    while (fgets(line, sizeof(line), file) && count < max_edges) {
        line_number++;
        
        trim_whitespace(line);
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Parse CSV fields from line
        char *fields[7];  // Array for 7 expected fields
        int field_count = parse_csv_fields(line, fields, 7);
        
        // Verify minimum required fields are present
        if (field_count < 6) {
            printf("Warning: Line %d has insufficient fields (%d), skipping\n", 
                   line_number, field_count);
            free_csv_fields(fields, field_count);
            continue;
        }
        
        // Parse and validate from_node (required field)
        if (!fields[0]) {
            printf("Warning: Line %d missing from_node, skipping\n", line_number);
            free_csv_fields(fields, field_count);
            continue;
        }
        int from_node = atoi(fields[0]);
        
        // Parse and validate to_node (required field)
        if (!fields[1]) {
            printf("Warning: Line %d missing to_node, skipping\n", line_number);
            free_csv_fields(fields, field_count);
            continue;
        }
        int to_node = atoi(fields[1]);
        
        // Parse and validate length (required field - field 5, index 5)
        if (!fields[5]) {
            printf("Warning: Line %d missing length, skipping\n", line_number);
            free_csv_fields(fields, field_count);
            continue;
        }
        double length = atof(fields[5]);
        if (length <= 0) {
            printf("Warning: Line %d has invalid length (%.2f), skipping\n", 
                   line_number, length);
            free_csv_fields(fields, field_count);
            continue;
        }
        
        // Assign required field values
        edges[count].from_node = from_node;
        edges[count].to_node = to_node;
        edges[count].length = length;
        
        // Parse optional name field (field 2)
        if (fields[2]) {
            strncpy(edges[count].name, fields[2], sizeof(edges[count].name) - 1);
            edges[count].name[sizeof(edges[count].name) - 1] = '\0';
        } else {
            strcpy(edges[count].name, "");
        }
        
        // Parse optional speed_limit field (field 3)
        if (fields[3]) {
            edges[count].speed_limit = parse_speed_limit(fields[3]);
        } else {
            edges[count].speed_limit = 50; // Default speed limit
        }
        
        // Parse optional highway_type field (field 4)
        if (fields[4]) {
            strncpy(edges[count].highway_type, fields[4], 
                   sizeof(edges[count].highway_type) - 1);
            edges[count].highway_type[sizeof(edges[count].highway_type) - 1] = '\0';
        } else {
            strcpy(edges[count].highway_type, "unknown");
        }
        
        // Parse optional oneway field (field 6)
        if (field_count > 6 && fields[6]) {
            edges[count].oneway = parse_oneway_field(fields[6]);
        } else {
            edges[count].oneway = false; // Default to bidirectional
        }
        
        count++;
        free_csv_fields(fields, field_count);
    }
    
    fclose(file);
    printf("Successfully parsed %d valid edges\n", count);
    return count;
}

int parse_speed_limit(const char *speed_str) {
    // Return default speed if string is null or empty
    if (!speed_str || strlen(speed_str) == 0) {
        return 50;
    }
    
    int speed = atoi(speed_str);
    return speed > 0 ? speed : 50; // Return default if invalid
}

bool parse_oneway_field(const char *oneway_str) {
    // Return false if string is null or empty
    if (!oneway_str || strlen(oneway_str) == 0) {
        return false;
    }
    
    // Check for various representations of "true"
    return (strcmp(oneway_str, "yes") == 0 || 
            strcmp(oneway_str, "true") == 0 ||
            strcmp(oneway_str, "1") == 0);
}

void trim_whitespace(char *str) {
    if (!str) return;
    
    char *end;

    // Remove leading whitespace
    while (isspace((unsigned char)*str)) str++;

    // Check if string is all spaces
    if (*str == '\0') return;

    // Remove trailing whitespace
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Null terminate after last non-whitespace character
    end[1] = '\0';
}
