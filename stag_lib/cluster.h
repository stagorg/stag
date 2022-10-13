/**
 * Graph clustering algorithms based on spectral methods.
 *
 * Copyright 2022 Peter Macgregor
 */
#ifndef STAG_TEST_CLUSTER_H
#define STAG_TEST_CLUSTER_H

#include <vector>

#include <graph.h>

namespace stag {
  /**
   * Default local clustering algorithm. Given a graph and starting vertex,
   * return a cluster which is close to the starting vertex.
   *
   * This method defaults to use the ACL local clustering algorithm.
   *
   * [ACL] Andersen, Reid, Fan Chung, and Kevin Lang.
   * "Local graph partitioning using pagerank vectors." 2006
   * 47th Annual IEEE Symposium on Foundations of Computer Science (FOCS'06). IEEE, 2006.
   *
   * @param graph - a graph object implementing the LocalGraph interface
   * @param seed_vertex - the starting vertex in the graph
   * @return a vector containing the indices of vectors considered to be in the
   *         same cluster as the seed_vertex.
   */
  std::vector<int> local_cluster(stag::LocalGraph* graph, int seed_vertex);

  /**
   * The ACL local clustering algorithm. Given a graph and starting vertex,
   * returns a cluster close to the starting vertex, constructed in a local way.
   *
   * [ACL] Andersen, Reid, Fan Chung, and Kevin Lang.
   * "Local graph partitioning using pagerank vectors." 2006
   * 47th Annual IEEE Symposium on Foundations of Computer Science (FOCS'06). IEEE, 2006.
   *
   * @param graph - a graph object implementing the LocalGraph interface
   * @param seed_vertex - the starting vertex in the graph
   * @return a vector containing the indices of vectors considered to be in the
   *         same cluster as the seed_vertex.
   */
  std::vector<int> local_cluster_acl(stag::LocalGraph* graph, int seed_vertex);
}

#endif //STAG_TEST_CLUSTER_H
