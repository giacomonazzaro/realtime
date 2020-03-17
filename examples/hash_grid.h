#include "../source/yocto_math.h"
using namespace yocto;

// A sparse grid of cells, containing list of points. Cells are stored in
// a dictionary to get sparsity. Helpful for nearest neighboor lookups.
struct hash_grid {
  float                             cell_size     = 0;
  float                             cell_inv_size = 0;
  vector<vec2f>                     positions     = {};
  unordered_map<vec2i, vector<int>> cells         = {};
};

// Gets the cell index
vec2i get_cell_index(const hash_grid& grid, const vec2f& position) {
  auto scaledpos = position * grid.cell_inv_size;
  return vec2i{(int)scaledpos.x, (int)scaledpos.y};
}

// Create a hash_grid
inline hash_grid make_hash_grid(float cell_size) {
  auto grid          = hash_grid{};
  grid.cell_size     = cell_size;
  grid.cell_inv_size = 1 / cell_size;
  return grid;
}
inline hash_grid make_hash_grid(
    const std::vector<vec2f>& positions, float cell_size) {
  auto grid          = hash_grid{};
  grid.cell_size     = cell_size;
  grid.cell_inv_size = 1 / cell_size;
  for (auto& position : positions) insert_vertex(grid, position);
  return grid;
}
// Inserts a point into the grid
inline int insert_vertex(hash_grid& grid, const vec2f& position) {
  auto vertex_id = (int)grid.positions.size();
  auto cell      = get_cell_index(grid, position);
  grid.cells[cell].push_back(vertex_id);
  grid.positions.push_back(position);
  return vertex_id;
}
// Finds the nearest neighbors within a given radius
inline void find_neighbors(const hash_grid& grid, std::vector<int>& neighbors,
    const vec2f& position, float max_radius, int skip_id) {
  auto cell        = get_cell_index(grid, position);
  auto cell_radius = (int)(max_radius * grid.cell_inv_size) + 1;
  neighbors.clear();
  auto max_radius_squared = max_radius * max_radius;
  for (auto j = -cell_radius; j <= cell_radius; j++) {
    for (auto i = -cell_radius; i <= cell_radius; i++) {
      auto ncell         = cell + vec2i{i, j};
      auto cell_iterator = grid.cells.find(ncell);
      if (cell_iterator == grid.cells.end()) continue;
      auto& ncell_vertices = cell_iterator->second;
      for (auto vertex_id : ncell_vertices) {
        if (distance_squared(grid.positions[vertex_id], position) >
            max_radius_squared)
          continue;
        if (vertex_id == skip_id) continue;
        neighbors.push_back(vertex_id);
      }
    }
  }
}
inline void find_neighbors(const hash_grid& grid, std::vector<int>& neighbors,
    const vec2f& position, float max_radius) {
  find_neighbors(grid, neighbors, position, max_radius, -1);
}
inline void find_neighbors(const hash_grid& grid, std::vector<int>& neighbors,
    int vertex, float max_radius) {
  find_neighbors(grid, neighbors, grid.positions[vertex], max_radius, vertex);
}
