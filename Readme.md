# Dijkstra Pathfinding with Real OSM Data

A C implementation of Dijkstra's shortest path algorithm using real-world road network data from OpenStreetMap (OSM). This project loads geographic data from CSV files and finds optimal routes between nodes in a road network.

## Features

- **Real OSM Data**: Uses actual road network data converted from OpenStreetMap format
- **Dijkstra's Algorithm**: Implements the classic shortest path algorithm
- **CSV Data Loading**: Parses nodes and edges from CSV files
- **GPX Export**: Export calculated routes to GPX format for GPS visualization
- **Bidirectional Roads**: Supports both one-way and bidirectional road segments
- **Road Attributes**: Handles speed limits, highway types, and road names
- **Memory Management**: Proper memory allocation and cleanup

## Data Format

The project uses two CSV files containing road network data:

### nodes.csv
Contains geographical coordinates for each intersection/point:
```csv
node_id,latitude,longitude
0,44.8856563,7.295528
1,44.8856184,7.295695
2,44.8855765,7.2958571
```

### edges.csv
Contains road segments connecting the nodes:
```csv
from_node,to_node,name,speed_limit,highway_type,length,oneway
0,1,,,secondary,13.82,yes
1,2,,,secondary,13.59,yes
2,3,,,secondary,16.68,yes
```

**Field descriptions:**
- `from_node`/`to_node`: Node IDs connected by this road segment
- `name`: Street name (can be empty)
- `speed_limit`: Speed limit in km/h (optional, defaults to 50)
- `highway_type`: Road classification (e.g., secondary, primary, residential)
- `length`: Distance in meters
- `oneway`: Whether the road is one-way (`yes`/`true`/`1`) or bidirectional

## Data Source

The CSV data is derived from OpenStreetMap (OSM) files:
1. Extract road network data from OSM format
2. Convert nodes (intersections) to `nodes.csv`
3. Convert ways (road segments) to `edges.csv`
4. Use the CSV files as input to this pathfinding program

## Building the Project

### Prerequisites
- GCC compiler with C99 support
- Make build system
- Standard C libraries (math library for distance calculations)

### Compilation
```bash
# Clone the repository
git clone <repository-url>
cd dijkstra-osm-pathfinding

# Build the project
make all

# The executable will be created as bin/main
```

### Clean build files
```bash
make clean
```

## Usage

```bash
./bin/main <nodes.csv> <edges.csv> <source_node_id> [target_node_id] [output.gpx]
```

### Arguments

1. **nodes.csv**: Path to CSV file containing node coordinates
2. **edges.csv**: Path to CSV file containing road segments
3. **source_node_id**: Starting point for pathfinding (node ID)
4. **target_node_id** (optional): Destination node ID
5. **output.gpx** (optional): Output filename for GPX route export

### Examples

#### Find distances to all reachable nodes
```bash
./bin/main data/nodes.csv data/edges.csv 0
```

#### Find shortest path to specific target
```bash
./bin/main data/nodes.csv data/edges.csv 0 100
```

#### Find path and export to GPX
```bash
./bin/main data/nodes.csv data/edges.csv 0 100 route.gpx
```

## Output

### Distance Display
When no target is specified, the program shows distances from the source to all reachable nodes:
```
Distances from node 0 to all reachable nodes:
=================================================
Node 1: 13.82 m
Node 2: 27.41 m
Node 3: 44.09 m
...
Total reachable nodes: 1250 out of 1300
```

### Path Display
When a target is specified, the program shows the shortest path:
```
Shortest path to node 100:
========================
Shortest path to node 100: 2.45 km
Path: 0 -> 1 -> 15 -> 23 -> 100
```

### GPX Export
Routes can be exported to GPX format for visualization in GPS software or mapping applications.

## Project Structure

```
├── src/
│   ├── main.c          # Main program entry point
│   ├── graph.c         # Graph data structure implementation
│   ├── dijkstra.c      # Dijkstra's algorithm implementation
│   ├── csv_parser.c    # CSV file parsing utilities
│   └── utils.c         # Utility functions
├── include/
│   ├── graph.h         # Graph structure definitions
│   ├── dijkstra.h      # Algorithm function declarations
│   ├── csv_parser.h    # CSV parsing function declarations
│   └── utils.h         # Utility function declarations
├── bin/                # Compiled executable (created by make)
├── Makefile           # Build configuration
└── README.md          # This file
```

## Algorithm Details

The implementation uses:
- **Adjacency List**: Efficient graph representation for sparse road networks
- **Priority Queue**: Simple array-based implementation for node selection
- **Path Reconstruction**: Backtracking using predecessor array
- **Memory Management**: Proper allocation and cleanup of all data structures

## Data Validation

The parser includes robust error handling:
- Validates CSV format and required fields
- Handles missing or malformed data gracefully
- Reports parsing warnings and statistics
- Verifies node references in edges
