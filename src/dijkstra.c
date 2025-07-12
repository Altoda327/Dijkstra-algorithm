#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "dijkstra.h"

#define INFINITY_DBL DBL_MAX

// =================
// MinHeap Data Structures
// =================

typedef struct {
  int node_index;
  double distance;
} HeapNode;

typedef struct {
  HeapNode *nodes;
  int size;
  int capacity;
} MinHeap;

// =================
// MinHeap Function Prototypes
// =================

/**
 * Creates a new min-heap with specified capacity.
 * 
 * @param heap Pointer to heap pointer to initialize
 * @param capacity Maximum number of elements the heap can hold
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre heap and err_info must be non-NULL
 * @pre capacity must be positive
 * @post On success: *heap points to a valid MinHeap structure
 *       On failure: *heap is undefined
 * @note The caller must call free_heap() to free allocated memory
 */
static error_code_t create_heap(MinHeap **heap, int capacity, error_info_t *err_info) {
  // Null check for err_info is already done in the caller function
  CHECK_NULL(heap, err_info);

  if (capacity <= 0) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Heap capacity must be positive.");
    return ERR_INVALID_ARGUMENT;
  }

  *heap = (MinHeap *)malloc(sizeof(MinHeap));
  CHECK_ALLOCATION(*heap, err_info);

  (*heap)->nodes = (HeapNode *)malloc(capacity * sizeof(HeapNode));
  if ((*heap)->nodes == NULL) {
    free(*heap);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "Failed to allocate memory for heap nodes.");
    return ERR_MEMORY_ALLOCATION;
  }

  (*heap)->size = 0;
  (*heap)->capacity = capacity;

  return ERR_SUCCESS;
}

/**
 * Frees memory allocated for MinHeap structure.
 * 
 * @param heap Pointer to MinHeap structure to free
 * 
 * @pre None
 * @post All allocated memory in heap is freed
 * @note Safe to call with NULL pointer
 */
static void free_heap(MinHeap *heap) {
  if (heap == NULL) return;
  free(heap->nodes);
  free(heap);
}

/**
 * Swaps two heap nodes.
 * 
 * @param a Pointer to first heap node
 * @param b Pointer to second heap node
 * 
 * @pre a and b must be non-NULL
 * @post Contents of a and b are swapped
 */
static void swap_heap_nodes(HeapNode *a, HeapNode *b) {
  HeapNode tmp = *a;
  *a = *b;
  *b = tmp;
}

/**
 * Checks if the heap is empty.
 * 
 * @param heap Pointer to MinHeap structure
 * @return true if heap is empty, false otherwise
 * 
 * @pre heap must be non-NULL
 */
static bool is_heap_empty(MinHeap *heap) {
  return (heap->size == 0);
}

/**
 * Maintains the min-heap property by bubbling down from given index.
 * 
 * @param heap Pointer to MinHeap structure
 * @param idx Index to start heapify from
 * 
 * @pre heap must be non-NULL and valid
 * @pre idx must be within valid range
 * @post Min-heap property is maintained
 */
static void min_heapify(MinHeap *heap, int idx) {
  int smallest = idx;
  int left = 2 * idx + 1;
  int right = 2 * idx + 2;

  if (left < heap->size && heap->nodes[left].distance < heap->nodes[smallest].distance) {
    smallest = left;
  }
  if (right < heap->size && heap->nodes[right].distance < heap->nodes[smallest].distance) {
    smallest = right;
  }
  
  if (smallest != idx) {
    swap_heap_nodes(&heap->nodes[idx], &heap->nodes[smallest]);
    min_heapify(heap, smallest);
  }
}

/**
 * Inserts a new node into the min-heap.
 * 
 * @param heap Pointer to MinHeap structure
 * @param node_index Index of the node to insert
 * @param distance Distance value for the node
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre heap and err_info must be non-NULL
 * @pre heap must have available capacity
 * @post Node is inserted and min-heap property is maintained
 */
static error_code_t insert_heap(MinHeap *heap, int node_index, double distance, error_info_t *err_info) {
  // Null check for err_info is already done in the caller function
  CHECK_NULL(heap, err_info);

  if (heap->size >= heap->capacity) {
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "Heap is full, cannot insert new node.");
    return ERR_MEMORY_ALLOCATION;
  }

  int i = heap->size++;
  heap->nodes[i].node_index = node_index;
  heap->nodes[i].distance = distance;

  while (i != 0 && heap->nodes[(i - 1) / 2].distance > heap->nodes[i].distance) {
    swap_heap_nodes(&heap->nodes[i], &heap->nodes[(i - 1) / 2]);
    i = (i - 1) / 2;
  }

  return ERR_SUCCESS;
}

/**
 * Extracts the minimum node from the heap.
 * 
 * @param heap Pointer to MinHeap structure
 * @param min_node Pointer to store the extracted minimum node
 * @param err_info Error reporting structure
 * @return ERR_SUCCESS on success, error code otherwise
 * 
 * @pre heap, min_node, and err_info must be non-NULL
 * @post Minimum node is extracted and min-heap property is maintained
 * @note If heap is empty, min_node is set to invalid values
 */
static error_code_t extract_min(MinHeap *heap, HeapNode *min_node, error_info_t *err_info) {
  // Null check for err_info is already done in the caller function
  CHECK_NULL(heap, err_info);
  CHECK_NULL(min_node, err_info);

  if (is_heap_empty(heap)) {
    min_node->node_index = -1;
    min_node->distance = INFINITY_DBL;
    return ERR_SUCCESS;
  }

  if (heap->size == 1) {
    *min_node = heap->nodes[0];
    heap->size--;
    return ERR_SUCCESS;
  }

  *min_node = heap->nodes[0];
  heap->nodes[0] = heap->nodes[heap->size - 1];
  heap->size--;
  
  min_heapify(heap, 0);
  return ERR_SUCCESS;
}

// =================
// Dijkstra's Algorithm Implementation
// =================

error_code_t dijkstra_shortest_path(Graph *graph, uint32_t source_node_id, uint32_t target_node_id, DijkstraMode mode, DijkstraResult *result, error_info_t *err_info) {
  // Input validation - ensure all required parameters are provided
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(graph, err_info);
  CHECK_NULL(result, err_info);

  if (mode != DIJKSTRA_SHORTEST_DISTANCE && mode != DIJKSTRA_FASTEST_TIME) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Invalid Dijkstra mode.");
    return ERR_INVALID_ARGUMENT;
  }
  if (source_node_id == target_node_id) {
    SET_ERROR(err_info, ERR_INVALID_ARGUMENT, "Source and target node IDs cannot be the same.");
    return ERR_INVALID_ARGUMENT;
  }

  // Find node indices in the graph
  int source_index, target_index;
  error_code_t err_code;
  err_code = find_node_index(graph, source_node_id, &source_index, err_info);
  if (err_code != ERR_SUCCESS) return err_code;
  err_code = find_node_index(graph, target_node_id, &target_index, err_info);
  if (err_code != ERR_SUCCESS) return err_code;

  // Initialize distance array
  result->distances = (double *)malloc(graph->num_nodes * sizeof(double));
  if (result->distances == NULL) {
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "Failed to allocate memory for distances.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Initialize predecessor array
  result->predecessors = (int *)malloc(graph->num_nodes * sizeof(int));
  if (result->predecessors == NULL) {
    free(result->distances);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "Failed to allocate memory for predecessors.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Initialize visited array
  result->visited = (bool *)malloc(graph->num_nodes * sizeof(bool));
  if (result->visited == NULL) {
    free(result->distances);
    free(result->predecessors);
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "Failed to allocate memory for visited nodes.");
    return ERR_MEMORY_ALLOCATION;
  }

  // Initialize all arrays with default values
  for (int i = 0; i < graph->num_nodes; i++) {
    result->distances[i] = INFINITY_DBL;
    result->predecessors[i] = -1;
    result->visited[i] = false;
  }

  // Set source node distance to zero and initialize result structure
  result->distances[source_index] = 0.0;
  result->source_index = source_index;
  result->target_index = target_index;
  result->num_nodes = graph->num_nodes;
  result->target_found = false;

  // Create and initialize priority queue (min-heap)
  MinHeap *heap;
  err_code = create_heap(&heap, graph->num_nodes, err_info);
  if (err_code != ERR_SUCCESS) {
    free_dijkstra_result(result);
    return err_code;
  }

  err_code = insert_heap(heap, source_index, 0.0, err_info);
  if (err_code != ERR_SUCCESS) {
    free_heap(heap);
    free_dijkstra_result(result);
    return err_code;
  }

  // Main Dijkstra algorithm loop
  while (!is_heap_empty(heap)) {
    HeapNode min_node;
    err_code = extract_min(heap, &min_node, err_info);
    if (err_code != ERR_SUCCESS) {
      free_heap(heap);
      free_dijkstra_result(result);
      return err_code;
    }

    int current_index = min_node.node_index;

    // Skip if node already visited
    if (result->visited[current_index]) continue;
    result->visited[current_index] = true;

    // Check if target node is reached
    if (current_index == target_index) {
      result->target_found = true;
      break;
    }

    // Process all adjacent edges using CSR representation
    int start_idx, end_idx;
    err_code = get_adjacent_edges_csr(graph, current_index, &start_idx, &end_idx, err_info);
    if (err_code != ERR_SUCCESS) {
      free_heap(heap);
      free_dijkstra_result(result);
      return err_code;
    }

    for (int i = start_idx; i < end_idx; i++) {
      int edge_idx = graph->adj_indices[i];
      Edge *edge = &graph->edges[edge_idx];

      // Determine neighbor node index
      int neighbor = -1;
      int from_index, to_index;
      err_code = find_node_index(graph, edge->from_node, &from_index, err_info);
      if (err_code != ERR_SUCCESS) {
        free_heap(heap);
        free_dijkstra_result(result);
        return err_code;
      }
      err_code = find_node_index(graph, edge->to_node, &to_index, err_info);
      if (err_code != ERR_SUCCESS) {
        free_heap(heap);
        free_dijkstra_result(result);
        return err_code;
      }

      if (from_index == current_index) {
        neighbor = to_index;
      } else if (to_index == current_index && !edge->one_way) {
        neighbor = from_index;
      }

      // Skip invalid or already visited neighbors
      if (neighbor == -1 || neighbor < 0 || neighbor >= graph->num_nodes) continue;
      if (result->visited[neighbor]) continue;

      // Calculate new distance based on selected mode
      double new_distance;
      if (mode == DIJKSTRA_FASTEST_TIME) {
        // Calculate travel time in minutes
        if (edge->speed_limit <= 0) {
          SET_ERROR(err_info, ERR_INVALID_DATA, "Edge speed must be positive for travel time calculation.");
          free_heap(heap);
          free_dijkstra_result(result);
          return ERR_INVALID_DATA;
        }
        double length_km = edge->length / 1000.0; // Convert length to kilometers
        double travel_time = (length_km / edge->speed_limit) * 60.0; // Convert to minutes
        new_distance = result->distances[current_index] + travel_time;
      } else {
        new_distance = result->distances[current_index] + edge->length;
      }

      // Update distance if a shorter path is found
      if (new_distance < result->distances[neighbor]) {
        result->distances[neighbor] = new_distance;
        result->predecessors[neighbor] = current_index;
        err_code = insert_heap(heap, neighbor, new_distance, err_info);
        if (err_code != ERR_SUCCESS) {
          free_heap(heap);
          free_dijkstra_result(result);
          return err_code;
        }
      }
    }
  }

  free_heap(heap);
  return ERR_SUCCESS;
}

void free_dijkstra_result(DijkstraResult *result) {
  // Safe to call with NULL pointer
  if (result == NULL) return;

  free(result->distances);
  free(result->predecessors);
  free(result->visited);
}

error_code_t get_shortest_distance(DijkstraResult *result, double *distance, error_info_t *err_info) {
  // Input validation - ensure all required parameters are provided
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(result, err_info);
  CHECK_NULL(distance, err_info);

  if (!result->target_found) {
    *distance = INFINITY_DBL;
    return ERR_SUCCESS;
  }

  *distance = result->distances[result->target_index];
  return ERR_SUCCESS;
}

error_code_t get_shortest_path(Graph *graph, DijkstraResult *result, int *path_length, int **path, error_info_t *err_info) {
  // Input validation - ensure all required parameters are provided
  CHECK_NULL(err_info, err_info);
  CHECK_NULL(graph, err_info);
  CHECK_NULL(result, err_info);
  CHECK_NULL(path_length, err_info);

  if (!result->target_found) {
    SET_ERROR(err_info, ERR_NOT_FOUND, "Target node not found in Dijkstra result.");
    *path_length = 0;
    return ERR_NOT_FOUND;
  }

  // Calculate path length by backtracking from target to source
  int length = 0;
  int current_index = result->target_index;
  while (current_index != result->source_index) {
    length++;
    current_index = result->predecessors[current_index];
    if (current_index == -1) {
      SET_ERROR(err_info, ERR_NOT_FOUND, "Path to source node not found in Dijkstra result.");
      *path_length = 0;
      return ERR_NOT_FOUND;
    }
  }
  length++; // Include the source node

  // Allocate memory for path array
  *path = (int *)malloc(length * sizeof(int));
  if (*path == NULL) {
    SET_ERROR(err_info, ERR_MEMORY_ALLOCATION, "Failed to allocate memory for path.");
    *path_length = 0;
    return ERR_MEMORY_ALLOCATION;
  }

  // Build path array by backtracking from target to source
  current_index = result->target_index;
  for (int i = length - 1; i >= 0; i--) {
    (*path)[i] = current_index;
    if (i > 0) {
      current_index = result->predecessors[current_index];
    }
  }

  *path_length = length;
  return ERR_SUCCESS;
}
