#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "utils.h"

/**
 * Create and initialize a new graph structure with specified capacity
 */
Graph* create_graph(int num_nodes, int num_edges) {
    // Validate input parameters
    if (num_nodes <= 0 || num_edges <= 0) {
        log_error("Number of nodes and edges must be positive");
        return NULL;
    }
    
    // Allocate memory for the main graph structure
    Graph *graph = malloc(sizeof(Graph));
    if (!graph) {
        log_error("Failed to allocate memory for graph structure");
        return NULL;
    }
    
    // Allocate memory for nodes array
    graph->nodes = malloc(num_nodes * sizeof(Node));
    if (!graph->nodes) {
        log_error("Failed to allocate memory for nodes array");
        free(graph);
        return NULL;
    }
    
    // Allocate memory for edges array
    graph->edges = malloc(num_edges * sizeof(Edge));
    if (!graph->edges) {
        log_error("Failed to allocate memory for edges array");
        free(graph->nodes);
        free(graph);
        return NULL;
    }
    
    // Allocate memory for adjacency list array
    graph->adj_list = malloc(num_nodes * sizeof(AdjListNode*));
    if (!graph->adj_list) {
        log_error("Failed to allocate memory for adjacency list");
        free(graph->edges);
        free(graph->nodes);
        free(graph);
        return NULL;
    }
    
    // Initialize adjacency list pointers to NULL
    for (int i = 0; i < num_nodes; i++) {
        graph->adj_list[i] = NULL;
    }
    
    // Set graph dimensions
    graph->num_nodes = num_nodes;
    graph->num_edges = num_edges;
    
    return graph;
}

/**
 * Free all memory allocated for the graph structure and its components
 */
void free_graph(Graph *graph) {
    if (!graph) {
        return;  // Nothing to free if graph is NULL
    }
    
    // Free all adjacency list nodes
    if (graph->adj_list) {
        for (int i = 0; i < graph->num_nodes; i++) {
            AdjListNode *current = graph->adj_list[i];
            
            // Free the linked list for this node
            while (current) {
                AdjListNode *temp = current;
                current = current->next;
                free(temp);
            }
        }
        free(graph->adj_list);
    }
    
    // Free nodes and edges arrays
    free(graph->nodes);
    free(graph->edges);
    
    // Free the main graph structure
    free(graph);
}

/**
 * Add a node to the graph at the specified index with given properties
 */
void add_node(Graph *graph, int index, int id, double lat, double lon) {
    // Validate graph pointer
    if (!graph) {
        log_error("Graph pointer is null");
        return;
    }
    
    // Validate index bounds
    if (index < 0 || index >= graph->num_nodes) {
        log_error("Node index out of bounds");
        return;
    }
    
    // Set node properties
    Node *node = &graph->nodes[index];
    node->id = id;
    node->latitude = lat;
    node->longitude = lon;
}

/**
 * Add an edge to the graph with all its properties and update adjacency lists
 */
void add_edge(Graph *graph, int index, int from, int to, const char *name,
              int speed_limit, const char *highway_type, 
              double length, bool oneway) {
    // Validate graph pointer
    if (!graph) {
        log_error("Graph pointer is null");
        return;
    }
    
    // Validate index bounds
    if (index < 0 || index >= graph->num_edges) {
        log_error("Edge index out of bounds");
        return;
    }
    
    // Get reference to the edge at specified index
    Edge *edge = &graph->edges[index];
    
    // Set basic edge properties
    edge->from_node = from;
    edge->to_node = to;
    edge->length = length;
    edge->speed_limit = speed_limit;
    edge->oneway = oneway;
    
    // Copy name string safely
    if (name) {
        strncpy(edge->name, name, sizeof(edge->name) - 1);
        edge->name[sizeof(edge->name) - 1] = '\0';
    } else {
        edge->name[0] = '\0';  // Empty string if name is NULL
    }
    
    // Copy highway type string safely
    if (highway_type) {
        strncpy(edge->highway_type, highway_type, sizeof(edge->highway_type) - 1);
        edge->highway_type[sizeof(edge->highway_type) - 1] = '\0';
    } else {
        strcpy(edge->highway_type, "unknown");
    }
    
    // Add edge to adjacency lists
    add_edge_to_adj_list(graph, from, index);
    
    // Add reverse direction if edge is bidirectional
    if (!oneway) {
        add_edge_to_adj_list(graph, to, index);
    }
}

/**
 * Add an edge reference to the adjacency list of a specific node
 */
void add_edge_to_adj_list(Graph *graph, int from_node, int edge_index) {
    // Validate graph pointer
    if (!graph) {
        log_error("Graph pointer is null");
        return;
    }
    
    // Validate node index bounds
    if (from_node < 0 || from_node >= graph->num_nodes) {
        log_error("Node index out of bounds for adjacency list");
        return;
    }
    
    // Allocate memory for new adjacency list node
    AdjListNode *new_adj_node = malloc(sizeof(AdjListNode));
    if (!new_adj_node) {
        log_error("Failed to allocate memory for adjacency list node");
        return;
    }
    
    // Set edge index and link to existing list
    new_adj_node->edge_index = edge_index;
    new_adj_node->next = graph->adj_list[from_node];
    
    // Update head of adjacency list
    graph->adj_list[from_node] = new_adj_node;
}

/**
 * Find the array index of a node given its unique ID
 */
int find_node_index(Graph *graph, int node_id) {
    // Validate graph pointer
    if (!graph) {
        log_error("Graph pointer is null");
        return -1;
    }
    
    // Linear search through nodes array
    for (int i = 0; i < graph->num_nodes; i++) {
        if (graph->nodes[i].id == node_id) {
            return i;  // Found the node
        }
    }
    
    return -1;  // Node not found
}

/**
 * Get the head of the adjacency list for a given node index
 */
AdjListNode* get_adjacent_edges(Graph *graph, int node_index) {
    // Validate graph pointer
    if (!graph) {
        log_error("Graph pointer is null");
        return NULL;
    }
    
    // Validate node index bounds
    if (node_index < 0 || node_index >= graph->num_nodes) {
        log_error("Node index out of bounds");
        return NULL;
    }
    
    return graph->adj_list[node_index];
}
