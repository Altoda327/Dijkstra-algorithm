#ifndef UTILS_H
#define UTILS_H

#include "graph.h"

// Function declarations

/* 
 * Function to print information about a node.
 * @param node Pointer to the Node structure
 */
void print_node_info(Node *node);

/* 
 * Function to print information about an edge.
 * @param edge Pointer to the Edge structure
 */
void print_edge_info(Edge *edge);

/* 
 * Function to compare two double values with epsilon tolerance.
 * @param a First double value
 * @param b Second double value
 * @return 0 if equal, -1 if a < b, 1 if a > b
 */
int compare_doubles(double a, double b);

/*
 * Function to format a distance in meters into a human-readable string.
 * @param distance_meters Distance in meters
 * @return Pointer to a string representing the formatted distance
 */
char* format_distance(double distance_meters);

/* 
 * Function to print the usage of the program.
 * @param program_name Name of the program
 */
void print_usage(const char *program_name);

/* 
 * Function to log an error message.
 * @param message Error message to log
 */
void log_error(const char *message);

/*
 * Function to export a path to a GPX file.
 * @param graph Pointer to the graph
 * @param path Array of node indices representing the path
 * @param path_length Length of the path array
 * @param filename Name of the GPX file to create
 * @return 0 on success, -1 on error
 */
int export_path_to_gpx(Graph *graph, int *path, int path_length, const char *filename);

#endif
