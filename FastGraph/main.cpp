//
//  main.cpp
//  FastGraph
//
//  Created by culter on 1/9/16.
//  Copyright © 2016 culter. All rights reserved.
//

#define NDEBUG
#include <cassert>

#include "Node.h"
#include "FastGraph.h"
#include "io_util.hpp"
#include "graph_operations.h"
#include "evolution.h"

/*
 TODOs:
 2. For gene mutation, replace the series of has_path calls with a single enumeration from target to potential sources
 3. For gene crossover, dynamic programming: keep the reachable sets from previous calls to be used in subsequent calls, after shortening the gene to consider sibling of the previous edges
 5. Implement remaining missing features: (A) extending front instead of back (B) local optimization of best/all genes
 */

void test_cross() {
  auto g = cycle<uint16_t, uint8_t, 15>(16);
  Gene<uint16_t> mother = std::vector<uint16_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  Gene<uint16_t> father = std::vector<uint16_t>{7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5};
  uint16_t site = 9;
  
  // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 0 1 2 3 4 5
  //   1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 0 1 2 3 4
  //     2 3 4 5 6 7 8 9 10 11 12 13 14 15 0 1 2 3
  //       3 4 5 6 7 8 9 10 11 12 13 14 15 0 1 2
  
  auto t1 = cross_reference(g, site, mother, father);
  auto t2 = cross_2(g, site, mother, father);
  
  assert(t1.path.size() == t2.path.size());
  for (int i = 0; i < t1.path.size(); ++i) {
    assert(t1.path[i] == t2.path[i]);
  }
  
  t2 = cross_fast(g, site, mother, father);
  
  assert(t1.path.size() == t2.path.size());
  for (int i = 0; i < t1.path.size(); ++i) {
    assert(t1.path[i] == t2.path[i]);
  }
  
  t2 = cross_faster(g, site, mother, father);
  
  assert(t1.path.size() == t2.path.size());
  for (int i = 0; i < t1.path.size(); ++i) {
    assert(t1.path[i] == t2.path[i]);
  }
}

void test_has_path() {
  FastGraph<Node<uint16_t, uint8_t, 15>> g;
  read_massey(g);
  Gene<uint16_t> mother = std::vector<uint16_t>{328, 125, 446, 412, 369, 165, 420, 171, 406, 345, 496, 424, 405, 53, 41, 442, 97, 528, 401, 402, 427, 485, 168, 169, 404, 347, 403, 436, 459, 497, 349, 350, 473, 385, 415, 522, 363, 122, 123, 348, 499, 572, 613, 636, 545, 483, 158, 159};
  Gene<uint16_t> father = std::vector<uint16_t>{328, 125, 446, 412, 369, 165, 420, 171, 406, 345, 496, 424, 405, 53, 41, 442, 97, 528, 401, 402, 427, 485, 168, 169, 404, 347, 403, 436, 459, 497, 349, 350, 473, 385, 415, 522, 363, 122, 123, 348, 499, 572, 613, 636, 545, 483, 158, 159};
  uint16_t site = 168;
  
  std::cout << has_path(g, mother.path) << std::endl;
//  exit(0);
  
//  for (int i = 0; i < 10; ++i) {
//    std::cout << has_path(g, mother.path) << std::endl;
//    std::cout << has_path(g, father.path) << std::endl;
//  }
  
  auto t1 = cross_reference(g, site, mother, father);
//  for (int i = 0; i < 10; ++i) {
//    std::cout << has_path(g, mother.path) << std::endl;
//    std::cout << has_path(g, father.path) << std::endl;
//  }
  auto t2 = cross_2(g, site, mother, father);
//  for (int i = 0; i < 10; ++i) {
//    std::cout << has_path(g, mother.path) << std::endl;
//    std::cout << has_path(g, father.path) << std::endl;
//  }
  
  assert(t1.path.size() == t2.path.size());
  for (int i = 0; i < t1.path.size(); ++i) {
    assert(t1.path[i] == t2.path[i]);
  }
  
  t2 = cross_fast(g, site, mother, father);
  
  assert(t1.path.size() == t2.path.size());
  for (int i = 0; i < t1.path.size(); ++i) {
    assert(t1.path[i] == t2.path[i]);
  }
}

int main() {
//  std::cout << alignof(std::max_align_t) << '\n'; exit(0);
  
//  test_has_path();
//  exit(0);
  
//  test_cross();
//  exit(0);
  
  FastGraph<Node<uint16_t, uint8_t, 15>> g;
  read_massey(g);
  std::cout << g.nodes.size() << " teams in input" << std::endl;
  restrict_to_scc(g, (uint16_t)0);
  std::cout << g.nodes.size() << " teams in SCC" << std::endl;
//  exit(0);
  evolve(g);
}