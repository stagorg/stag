/**
 * Copyright 2022 Peter Macgregor
 */
#include <stdexcept>
#include "graph.h"
#include "utility.h"

//------------------------------------------------------------------------------
// Graph Object Constructors
//------------------------------------------------------------------------------

stag::Graph::Graph(const SprsMat& adjacency_matrix) {
  // Load the adjacency matrix into this object.
  adjacency_matrix_ = adjacency_matrix;
  adjacency_matrix_.makeCompressed();

  // The number of vertices is the dimensions of the adjacency matrix
  number_of_vertices_ = adjacency_matrix_.outerSize();

  // Set the flags to indicate which matrices have been initialised.
  adj_init_ = true;
  lap_init_ = false;
  deg_init_ = false;
  norm_lap_init_ = false;

  // Check that the graph is configured correctly
  self_test_();
}

stag::Graph::Graph(std::vector<int> &outerStarts, std::vector<int> &innerIndices,
                   std::vector<double> &values) {
  // Map the provided data vectors to the sparse matrix type.
  adjacency_matrix_ = Eigen::Map<SprsMat>(long(outerStarts.size()) - 1,
                                          long(outerStarts.size()) - 1,
                                          long(values.size()),
                                          outerStarts.data(),
                                          innerIndices.data(),
                                          values.data());
  adjacency_matrix_.makeCompressed();

  // The number of vertices is the dimensions of the adjacency matrix
  number_of_vertices_ = adjacency_matrix_.outerSize();

  // Set the flags to indicate which matrices have been initialised.
  adj_init_ = true;
  lap_init_ = false;
  deg_init_ = false;
  norm_lap_init_ = false;

  // Check that the graph is configured correctly
  self_test_();
}

//------------------------------------------------------------------------------
// Graph Object Public Methods
//------------------------------------------------------------------------------

const SprsMat* stag::Graph::adjacency() const {
  return &adjacency_matrix_;
}

const SprsMat* stag::Graph::laplacian() {
  initialise_laplacian_();
  return &laplacian_matrix_;
}

const SprsMat* stag::Graph::normalised_laplacian() {
  initialise_normalised_laplacian_();
  return &normalised_laplacian_matrix_;
}

const SprsMat* stag::Graph::degree_matrix() {
  initialise_degree_matrix_();
  return &degree_matrix_;
}

double stag::Graph::total_volume() {
  Eigen::VectorXd degrees = adjacency_matrix_ * Eigen::VectorXd::Ones(adjacency_matrix_.cols());
  return degrees.sum();
}

long stag::Graph::number_of_vertices() const {
  return number_of_vertices_;
}

long stag::Graph::number_of_edges() const {
  return adjacency_matrix_.nonZeros() / 2;
}

double stag::Graph::degree(int v) {
  // For now, we can be a little lazy and use the degree matrix. Once this is
  // initialised, then checking degree is constant time.
  initialise_degree_matrix_();

  // If the vertex number is larger than the number of vertices in the graph,
  // then we say that the degree is 0.
  if (v < number_of_vertices_) {
    return degree_matrix_.coeff(v, v);
  } else {
    return 0;
  }
}

int stag::Graph::degree_unweighted(int v) {
  // If the requested vertex number is greater than the number of vertices, then
  // return a degree of 0.
  if (v >= number_of_vertices_) {
    return 0;
  }

  // The combinatorical degree of a vertex is equal to the number of non-zero
  // entries in its adjacency matrix row.
  const int *indexPtr = adjacency_matrix_.outerIndexPtr();
  int rowStart = *(indexPtr + v);
  int nextRowStart = *(indexPtr + v + 1);
  return nextRowStart - rowStart;
}

std::vector<stag::edge> stag::Graph::neighbors(int v) {
  // If the vertex v does not exist, return the empty vector.
  if (v >= number_of_vertices_) {
    return {};
  }

  std::vector<stag::edge> edges;

  // Iterate through the non-zero entries in the vth row of the adjacency matrix
  const double *weights = adjacency_matrix_.valuePtr();
  const int *innerIndices = adjacency_matrix_.innerIndexPtr();
  const int *rowStarts = adjacency_matrix_.outerIndexPtr();
  int vRowStart = *(rowStarts + v);
  int degree_unw = degree_unweighted(v);

  for (int i = 0; i < degree_unw; i++) {
    edges.push_back({v, *(innerIndices + vRowStart + i), *(weights + vRowStart + i)});
  }

  return edges;
}

std::vector<int> stag::Graph::neighbors_unweighted(int v) {
  // If the vertex v does not exist, return the empty vector.
  if (v >= number_of_vertices_) {
    return {};
  }

  // Return the non-zero indices in the vth row of the adjacency matrix
  const int *innerIndices = adjacency_matrix_.innerIndexPtr();
  const int *rowStarts = adjacency_matrix_.outerIndexPtr();
  int vRowStart = *(rowStarts + v);
  int degree = degree_unweighted(v);
  return {innerIndices + vRowStart, innerIndices + vRowStart + degree};
}

//------------------------------------------------------------------------------
// Graph Object Private Methods
//------------------------------------------------------------------------------

void stag::Graph::self_test_() {
  // Check that the adjacency matrix is symmetric.
  if (!stag::isSymmetric(&adjacency_matrix_)) {
    throw std::domain_error("Graph adjacency matrix must be symmetric.");
  }
}

void stag::Graph::initialise_laplacian_() {
  // If the laplacian matrix has already been initialised, then we do not
  // initialise it again.
  if (lap_init_) return;

  // Ensure that the degree matrix is initialised
  initialise_degree_matrix_();

  // Construct and return the laplacian matrix.
  laplacian_matrix_ = degree_matrix_ - adjacency_matrix_;
  laplacian_matrix_.makeCompressed();

  // We have now initialised the laplacian.
  lap_init_ = true;
}

void stag::Graph::initialise_normalised_laplacian_() {
  // If the normalised laplacian matrix has already been initialised, then we
  // do not initialise it again.
  if (norm_lap_init_) return;

  // Ensure that the degree matrix is initialised
  initialise_degree_matrix_();

  // Construct the inverse degree matrix
  SprsMat sqrt_inv_deg_mat(number_of_vertices_, number_of_vertices_);
  std::vector<Eigen::Triplet<double>> non_zero_entries;
  for (int i = 0; i < number_of_vertices_; i++) {
    non_zero_entries.emplace_back(i, i, 1 / sqrt(degree_matrix_.coeff(i, i)));
  }
  sqrt_inv_deg_mat.setFromTriplets(non_zero_entries.begin(), non_zero_entries.end());

  // The normalised laplacian is defined by I - D^{-1/2} A D^{-1/2}
  SprsMat identity_matrix(number_of_vertices_, number_of_vertices_);
  identity_matrix.setIdentity();
  normalised_laplacian_matrix_ = identity_matrix - sqrt_inv_deg_mat * adjacency_matrix_ * sqrt_inv_deg_mat;
  normalised_laplacian_matrix_.makeCompressed();

  // We have now initialised the normalised laplacian matrix.
  norm_lap_init_ = true;
}

void stag::Graph::initialise_degree_matrix_() {
  // If the degree matrix has already been initialised, then we do not
  // initialise it again.
  if (deg_init_) return;

  // Construct the vertex degrees.
  Eigen::VectorXd degrees = adjacency_matrix_ * Eigen::VectorXd::Ones(adjacency_matrix_.cols());
  degree_matrix_ = SprsMat(adjacency_matrix_.cols(), adjacency_matrix_.cols());
  for (int i = 0; i < adjacency_matrix_.cols(); i++) {
    degree_matrix_.insert(i, i) = degrees[i];
  }

  // Compress the degree matrix storage, and set the initialised flag
  degree_matrix_.makeCompressed();
  deg_init_ = true;
}

//------------------------------------------------------------------------------
// Equality Operators
//------------------------------------------------------------------------------
bool stag::operator==(const stag::Graph& lhs, const stag::Graph& rhs) {
  bool outerIndicesEqual = stag::sprsMatOuterStarts(lhs.adjacency()) == stag::sprsMatOuterStarts(rhs.adjacency());
  bool innerIndicesEqual = stag::sprsMatInnerIndices(lhs.adjacency()) == stag::sprsMatInnerIndices(rhs.adjacency());
  bool valuesEqual = stag::sprsMatValues(lhs.adjacency()) == stag::sprsMatValues(rhs.adjacency());
  return (outerIndicesEqual && innerIndicesEqual) && valuesEqual;
}

bool stag::operator!=(const stag::Graph &lhs, const stag::Graph &rhs) {
  return !(lhs == rhs);
}

bool stag::operator==(const stag::edge &lhs, const stag::edge &rhs) {
  return lhs.u == rhs.u && lhs.v == rhs.v && lhs.weight == rhs.weight;
}

bool stag::operator!=(const stag::edge &lhs, const stag::edge &rhs) {
  return !(lhs == rhs);
}

//------------------------------------------------------------------------------
// Standard Graph Constructors
//------------------------------------------------------------------------------
stag::Graph stag::cycle_graph(int n) {
  SprsMat adj_mat(n, n);
  std::vector<Eigen::Triplet<double>> non_zero_entries;
  for (int i = 0; i < n; i++) {
    non_zero_entries.emplace_back(i, (i + n + 1) % n, 1);
    non_zero_entries.emplace_back(i, (i + n - 1) % n, 1);
  }
  adj_mat.setFromTriplets(non_zero_entries.begin(), non_zero_entries.end());
  return stag::Graph(adj_mat);
}

stag::Graph stag::complete_graph(int n) {
  SprsMat adj_mat(n, n);
  std::vector<Eigen::Triplet<double>> non_zero_entries;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (i != j) {
        non_zero_entries.emplace_back(i, j, 1);
      }
    }
  }
  adj_mat.setFromTriplets(non_zero_entries.begin(), non_zero_entries.end());
  return stag::Graph(adj_mat);
}
