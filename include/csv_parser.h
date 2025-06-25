#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include "graph.h"

// Function declarations

/*
 * Function to count the number of lines in a CSV file.
 * @param filename Name of the CSV file
 * @return Number of lines in the file, or -1 on error
 */
int count_csv_lines(const char *filename);

/*
 * Function to load a graph from CSV files containing nodes and edges.
 * @param nodes_file Name of the CSV file containing nodes
 * @param edges_file Name of the CSV file containing edges
 * @return Pointer to the created Graph structure, or NULL on error
 */
Graph* load_graph_from_csv(const char *nodes_file, const char *edges_file);

/*
 * Function to parse a CSV file containing nodes.
 * @param filename Name of the CSV file containing nodes
 * @param nodes Pointer to an array of Node structures
 * @param max_nodes Maximum number of nodes to parse
 * @return Number of nodes parsed, or -1 on error
 */
int parse_nodes_csv(const char *filename, Node *nodes, int max_nodes);

/*
 * Function to parse a CSV file containing edges.
 * @param filename Name of the CSV file containing edges
 * @param edges Pointer to an array of Edge structures
 * @param max_edges Maximum number of edges to parse
 * @return Number of edges parsed, or -1 on error
 */
int parse_edges_csv(const char *filename, Edge *edges, int max_edges);

/*
 * Function to parse a speed limit string and return the speed limit in km/h.
 * @param speed_str String containing the speed limit
 * @return Speed limit in km/h, or -1 on error
 */
int parse_speed_limit(const char *speed_str);

/*
 * Function to parse a highway type string and return the highway type.
 * @param highway_str String containing the highway type
 * @return Pointer to a string representing the highway type, or NULL on error
 */
bool parse_oneway_field(const char *oneway_str);

/*
 * Function to trim leading and trailing whitespace from a string.
 * @param str Pointer to the string to be trimmed
 */
void trim_whitespace(char *str);

#endif
