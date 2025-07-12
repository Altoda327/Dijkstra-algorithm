# Dijkstra Pathfinding with Real OSM Data

A high-performance C implementation of Dijkstra's shortest path algorithm using real-world road network data from OpenStreetMap (OSM). This project loads geographic data from optimized binary files and finds optimal routes between nodes in a road network.

## Features

- **Real OSM Data**: Uses actual road network data converted from OpenStreetMap format
- **Optimized Dijkstra's Algorithm**: Implements the classic shortest path algorithm with MinHeap for O(log n) performance
- **Binary Data Loading**: Fast loading from optimized binary files with CSR (Compressed Sparse Row) representation
- **Dual Mode Operation**: Supports both shortest distance and fastest time routing
- **Interactive Coordinate Mode**: Find routes by specifying GPS coordinates
- **GPX Export**: Export calculated routes to GPX format for GPS visualization
- **Hash Table Optimization**: O(1) node lookup using MurmurHash3 algorithm
- **Bidirectional Roads**: Supports both one-way and bidirectional road segments
- **Advanced Error Handling**: Comprehensive error reporting and recovery
- **Memory Management**: Proper memory allocation and cleanup with detailed tracking

## Data Format

The project uses two optimized binary files containing road network data:

### nodes.bin
Contains geographical coordinates for each intersection/point in binary format:
- **node_id** (uint32_t): Unique identifier for the node
- **latitude** (double): Latitude coordinate
- **longitude** (double): Longitude coordinate

### edges.bin
Contains road segments connecting the nodes in binary format:
- **from_node** (uint32_t): Source node ID
- **to_node** (uint32_t): Destination node ID
- **length** (uint32_t): Distance in meters
- **reserved** (uint32_t): Reserved field for future use
- **speed_limit** (uint16_t): Speed limit in km/h
- **highway_type** (uint8_t): Road classification (0-255)
- **one_way** (uint8_t): 1 if one-way, 0 if bidirectional

## Data Source

The binary data is derived from OpenStreetMap (OSM) files:
1. Extract road network data from OSM format
2. Convert nodes (intersections) to `nodes.bin`
3. Convert ways (road segments) to `edges.bin`
4. Use the binary files as input to this pathfinding program

**Performance Benefits:**
- **Fast Loading**: Binary format loads 10-100x faster than CSV
- **Compact Storage**: Reduced file size compared to text formats
- **Memory Efficient**: Direct memory mapping possible
- **Type Safety**: Fixed data types prevent parsing errors

## Building the Project

### Prerequisites
- Make build system
- Standard C libraries (math library for distance calculations)

### Compilation
```bash
# Clone the repository
git clone <repository-url>
cd Dijkstra-algorithm

# Build the project
make all

# The executable will be created as bin/main
```

### Clean build files
```bash
make clean
```

## Usage

The program supports two main modes of operation:

### 1. Direct Node ID Mode
```bash
./bin/main <nodes.bin> <edges.bin> <source_node_id> <target_node_id> [output.gpx]
```

### 2. Interactive Coordinate Mode
```bash
./bin/main <nodes.bin> <edges.bin> -c [output.gpx]
```

The <> brackets indicate required parameters, while square brackets [] indicate optional parameters.

### Arguments

1. **nodes.bin**: Path to binary file containing node coordinates
2. **edges.bin**: Path to binary file containing road segments
3. **source_node_id**: Starting point for pathfinding (node ID)
4. **target_node_id**: Destination node ID
6. **output.gpx** (optional): Output filename for GPX route export
7. **-c**: Enable interactive coordinate mode

### Examples

#### Find path
```bash
./bin/main data/nodes.bin data/edges.bin 0 100
```

#### Find path and export to GPX
```bash
./bin/main data/nodes.bin data/edges.bin 0 100 route.gpx
```

#### Interactive coordinate mode
```bash
./bin/main data/nodes.bin data/edges.bin -c route.gpx
```

## Output

Example output when running the program:

```bash
=== GRAPH LOADER ===
Loading graph from files:
  Nodes: data/nodes.bin
  Edges: data/edges.bin
Debug: Loaded 7217651 nodes from binary file.

=== GRAPH SUMMARY ===
Total nodes: 7217651
Total edges: 7504582
Memory usage:
  Nodes: 165.20 MB
  Edges: 143.14 MB
  CSR: 56.16 MB
  Hash Table: 110.13 MB

=== HASH TABLE STATS ===
Hash table size: 14435302
Hash table count: 7217651
Load factor: 0.50
Used buckets: 5682362 / 14435302 (39.36%)
Max chain length: 8
Average chain length: 1.27

Choose Dijkstra mode:
  1. Dijkstra shortest distance
  2. Dijkstra fastest path
Enter choice (1 or 2): 1

=== RUNNING DIJKSTRA ===
Computing shortest path from node 519945 to node 925900...
Path found from node 519945 to node 925900:
Path contains 924 nodes.
Total distance: 34.38 Km
Path exported to GPX file: output.gpx

=== ANALYSIS COMPLETE ===
```

### GPX Export
Routes can be exported to GPX format for visualization in GPS software or mapping applications.

## Performance Optimizations

### MinHeap Implementation
- **O(log n)** node extraction instead of O(n) linear search
- **Custom MinHeap**: Optimized for pathfinding with distance keys
- **Memory Pool**: Efficient memory allocation for heap operations

### CSR (Compressed Sparse Row) Representation
- **Fast Adjacency Queries**: O(1) access to node neighbors
- **Memory Efficient**: Compact storage for sparse road networks
- **Cache Friendly**: Sequential memory access patterns

### Hash Table Optimization
- **O(1) Node Lookup**: MurmurHash3 for fast node ID to index mapping
- **Collision Handling**: Efficient chaining for hash collisions
- **Load Factor Management**: Optimized hash table size

### Binary File Format
- **Direct Memory Access**: Minimal parsing overhead
- **Batch Loading**: Efficient bulk data operations
- **Type Safety**: Fixed-size data structures

### Memory capacity and Performance
For typical road network graphs (like OpenStreetMap data):

- **With 4GB RAM: ~5-10 million nodes, ~15-25 million edges
- **With 8GB RAM: ~15-20 million nodes, ~40-50 million edges
- **With 16GB RAM: ~30-40 million nodes, ~80-100 million edges

These estimates are based on the memory footprint of the graph data structures, CSR representation, hash tables, and Dijkstra's algorithm arrays. The actual limits may vary depending on system architecture and available memory.

## Project Structure

```
├── src/
│   ├── main.c          # Main program entry point
│   ├── graph.c         # Graph data structure with CSR implementation
│   ├── dijkstra.c      # Dijkstra's algorithm with MinHeap
│   ├── bin_loader.c    # Binary file loading utilities
│   ├── utils.c         # Utility functions and coordinate mode
│   └── error_handling.c # Comprehensive error handling
├── include/
│   ├── graph.h         # Graph structure and CSR definitions
│   ├── dijkstra.h      # Algorithm and MinHeap declarations
│   ├── bin_loader.h    # Binary loading function declarations
│   ├── utils.h         # Utility function declarations
│   └── error_handling.h # Error handling macros and types
├── data/              # Sample data files (nodes.bin, edges.bin)
├── bin/                # Compiled executable (created by make)
├── output.gpx          # GPX output files
├── Makefile           # Build configuration
└── README.md          # This file
```

## Algorithm Details

The implementation uses advanced data structures and algorithms:

### Dijkstra's Algorithm
- **MinHeap Priority Queue**: O(log n) node selection
- **Visited Array**: O(1) visited node checking
- **Predecessor Array**: Efficient path reconstruction
- **Distance Array**: Maintains shortest distances

### Graph Representation
- **CSR Format**: Compressed Sparse Row for efficient adjacency queries
- **Hash Table**: MurmurHash3 for O(1) node ID lookup
- **Memory Pool**: Efficient allocation strategies

### Distance Calculation
- **Haversine Formula**: Accurate great-circle distance calculation
- **Speed-based Routing**: Time calculation using speed limits
- **Coordinate Utilities**: GPS coordinate to node mapping

## Error Handling

The system includes comprehensive error handling:
- **Structured Error Reporting**: Detailed error messages with location
- **Graceful Degradation**: Continues operation when possible
- **Input Validation**: Validates all user inputs and file formats

## Data Validation

The binary loader includes robust validation:
- Validates binary file format and magic numbers
- Handles corrupted or incomplete files gracefully
- Verifies node references in edges
- Reports loading statistics and warnings
- Ensures data consistency across files

## Visualization

The GPX export creates industry-standard GPS files that can be used with:
- **Web Tools**: gpx.studio, GPS Visualizer
- **Desktop Software**: JOSM, QGIS, Google Earth
- **Mobile Apps**: Most GPS and mapping applications
- **GPS Devices**: Garmin, TomTom, and other GPS units

## Memory Management

The implementation features advanced memory management:
- **Automatic Cleanup**: All allocated memory is properly freed
- **Error Recovery**: Memory cleanup on error conditions
- **Pool Allocation**: Efficient memory pools for performance
- **Leak Detection**: Debug builds include leak detection
