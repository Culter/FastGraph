//
//  evolution.h
//  FastGraph
//
//  Created by culter on 1/15/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef evolution_h
#define evolution_h

#include <iostream>
#include <random>

#include "gene_operations.h"
#include "graph_operations.h"

template<typename TIndex, typename TDegree, size_t MaxDegree>
std::vector<Gene<TIndex>> get_reversible_edges(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g) {
  std::vector<Gene<TIndex>> population;
  NodeSet empty = {};
  for (TIndex i = 0; i < g.nodes.size(); ++i) {
    for (auto pj = g.nodes[i].succ_cbegin(); pj != g.nodes[i].succ_cend(); ++pj) {
      if (has_path(g, *pj, i, empty)) {
        population.emplace_back();
        population.back().path = std::vector<TIndex>{i, *pj};
      }
    }
  }
  return population;
}


template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
void mutate(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
            Gene<TIndex>& gene,
            TRng& rng) {
//  while (true) {
    std::vector<TIndex> potential_additions;
    
    // TODO: This only adds forward. Also add backward at random.
    const TIndex i = gene.path.back();
    NodeSet flat = {};
    flatten(flat, gene.path.cbegin() + 1, gene.path.cend());
    for (auto pj = g.nodes[i].succ_cbegin(); pj != g.nodes[i].succ_cend(); ++pj) {
      if (*pj != gene.path.front() && !flat[*pj]) {
        if (has_path(g, *pj, gene.path.front(), flat)) {
          potential_additions.push_back(*pj);
        }
      }
    }
    if (!potential_additions.empty()) {
      std::uniform_int_distribution<TIndex> chooser(0, potential_additions.size() - 1);
      gene.path.push_back(potential_additions[chooser(rng)]);
    }
    else {
//      break;
    }
//  }
}

template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
void mutate_dfs(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                Gene<TIndex>& gene,
                TRng& rng) {
  constexpr TIndex marker = -1;
  
  if (gene.path.size() < 2) {
    // TODO: something more graceful?
    return;
  }
  
  // DFS traversal with random choices until a solution is found
  NodeSet explored = {};
  std::vector<TIndex> to_explore;
  to_explore.reserve(MaxDegree);
  std::vector<TIndex> temp;
  temp.reserve(MaxDegree);
  
  // Explore forward
  flatten(explored, gene.path.cbegin(), gene.path.cend() - 1);
  
  to_explore.push_back(gene.path.back());
  gene.path.pop_back();
  
  while (!to_explore.empty()) {
    TIndex n = to_explore.back();
    to_explore.pop_back();
    
    if (n == marker) {
      gene.path.pop_back();
    }
    else if (n == gene.path.front()) {
      // Success!
      return;
    }
    else if (!explored[n]) {
      explored[n] = true;
      
      gene.path.push_back(n);
      to_explore.push_back(marker);
      
//      to_explore.insert(to_explore.end(), g.nodes[n].succ_cbegin(), g.nodes[n].succ_cend());
//      std::shuffle(to_explore.end() - g.nodes[n].get_out_degree(), to_explore.end(), rng);
      temp.clear();
      for (auto s = g.nodes[n].succ_cbegin(); s != g.nodes[n].succ_cend(); ++s) {
        if (*s == gene.path.front() || !explored[*s]) {
          temp.push_back(*s);
        }
      }
      std::shuffle(temp.begin(), temp.end(), rng);
      to_explore.insert(to_explore.end(), temp.cbegin(), temp.cend());
    }
  }
  
  std::cout << "Shouldn't have reached here!" << std::endl;
  assert(false);
}

template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
void rotate(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
            Gene<TIndex>& gene,
            TRng& rng) {
  std::uniform_int_distribution<TIndex> rotor(0, gene.path.size() - 1);
  std::rotate(gene.path.begin(), gene.path.begin() + rotor(rng), gene.path.end());
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
void evolve(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g) {
  std::mt19937 random_engine(123456);
  
  std::vector<Gene<TIndex>> population = get_reversible_edges(g);
  
  while (true) {
    // Pre-compute which sites are touched by which genes
    std::vector<std::vector<TIndex>> site_to_gene_pool(g.nodes.size());
    for (int igene = 0; igene < population.size(); ++igene) {
      for (const TIndex isite: population[igene].path) {
        site_to_gene_pool[isite].push_back(igene);
      }
    }
    
    // Perform per-site crossover
    {
      std::vector<Gene<TIndex>> next_population;
      for (TIndex isite = 0; isite < g.nodes.size(); ++isite) {
        const std::vector<TIndex>& candidates = site_to_gene_pool[isite];
        if (!candidates.empty()) {
          // Pick from 0 to size - 1, inclusive
          std::uniform_int_distribution<TIndex> gene_chooser(0, candidates.size() - 1);
          
          const Gene<TIndex>& mother = population[candidates[gene_chooser(random_engine)]];
          const Gene<TIndex>& father = population[candidates[gene_chooser(random_engine)]];
          
//          auto test1 = cross_reference(g, isite, mother, father);
//          auto test2 = cross_faster(g, isite, mother, father);
//          
//          assert(test1.path.size() == test2.path.size());
//          for (int i = 0; i < test1.path.size(); ++i) {
//            assert(test1.path[i] == test2.path[i]);
//          }
          
          next_population.push_back(cross_faster(g, isite, mother, father));
        }
      }
      population = std::move(next_population);
    }
    
    // Perform mutations
    for (auto& gene: population) {
      mutate_dfs(g, gene, random_engine);
      rotate(g, gene, random_engine);
    }
    
    // Dump output.
    static size_t record = 0;
    for (const auto& gene: population) {
      if (gene.path.size() > record) {
        print(g, gene);
        record = gene.path.size();
      }
    }
    
    static int generation = 0;
    generation += 1;
    if (generation % 100 == 0) {
      std::cout << "Generation " << generation << ": best length " << record << std::endl;
    }
//    if (generation >= 2000) break;
  }
}

#endif /* evolution_h */
