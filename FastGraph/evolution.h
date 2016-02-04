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

#include "Config.h"
#include "gene_operations.h"
#include "graph_operations.h"

template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
void mutate(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
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
void mutate_faster(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                   Gene<TIndex>& gene,
                   TRng& rng) {
  // TODO: This only adds forward. Also add backward at random.
  const TIndex i = gene.path.back();
  std::vector<TIndex> successors_shuffled(g.nodes[i].succ_cbegin(), g.nodes[i].succ_cend());
  std::shuffle(successors_shuffled.begin(), successors_shuffled.end(), rng);
  
  NodeSet flat = {};
  flatten(flat, gene.path.cbegin() + 1, gene.path.cend());
  for (const TIndex& j: successors_shuffled) {
    if (j != gene.path.front() && !flat[j]) {
      if (has_path(g, j, gene.path.front(), flat)) {
        gene.path.push_back(j);
        return;
      }
    }
  }
}

template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
void mutate_better(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                   Gene<TIndex>& gene,
                   TRng& rng) {
  if (rng() % 2) {
    const TIndex i = gene.path.back();
    std::vector<TIndex> candidates(g.nodes[i].succ_cbegin(), g.nodes[i].succ_cend());
    std::shuffle(candidates.begin(), candidates.end(), rng);
    
    NodeSet flat = {};
    flatten(flat, gene.path.cbegin() + 1, gene.path.cend());
    for (const TIndex& j: candidates) {
      if (j != gene.path.front() && !flat[j]) {
        if (has_path(g, j, gene.path.front(), flat)) {
          gene.path.push_back(j);
          return;
        }
      }
    }
  } else {
    const TIndex i = gene.path.front();
    std::vector<TIndex> candidates(g.nodes[i].pred_cbegin(), g.nodes[i].pred_cend());
    std::shuffle(candidates.begin(), candidates.end(), rng);
    
    NodeSet flat = {};
    flatten(flat, gene.path.cbegin(), gene.path.cend() - 1);
    for (const TIndex& j: candidates) {
      if (j != gene.path.back() && !flat[j]) {
        if (has_path(g, gene.path.back(), j, flat)) {
          gene.path.insert(gene.path.begin(), j);
          return;
        }
      }
    }
  }
}

template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
void mutate_dfs(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
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

template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
std::vector<Gene<TIndex>> get_reversible_edges(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                                               TRng& rng) {
  std::vector<Gene<TIndex>> population;
  NodeSet empty = {};
  for (TIndex i = 0; i < g.nodes.size(); ++i) {
    for (auto pj = g.nodes[i].succ_cbegin(); pj != g.nodes[i].succ_cend(); ++pj) {
      if (has_path(g, *pj, i, empty)) {
        population.emplace_back();
        population.back().path = std::vector<TIndex>{i, *pj};
        
        if (k_close_all_genes) {
          mutate_dfs(g, population.back(), rng);
        }
      }
    }
  }
  return population;
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
void evolve(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g) {
  std::mt19937 random_engine(k_random_seed);
  
  std::vector<Gene<TIndex>> population;
  if (k_refresh_edge_count < 1) {
    population = get_reversible_edges(g, random_engine);
  }
  
  while (true) {
    // Refresh gene pool
    for (int y = 0; y < k_refresh_edge_count; ++y) {
      auto refresh = get_reversible_edges(g, random_engine);
      population.insert(population.end(), refresh.cbegin(), refresh.cend());
    }
    
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
          
          for (int x = 0; x < k_population_multiplier; ++x) {
            Gene<TIndex>& mother = population[candidates[gene_chooser(random_engine)]];
            Gene<TIndex>& father = population[candidates[gene_chooser(random_engine)]];
            
            //          auto test1 = cross_reference(g, isite, mother, father);
            //          auto test2 = cross_faster(g, isite, mother, father);
            //
            //          assert(test1.path.size() == test2.path.size());
            //          for (int i = 0; i < test1.path.size(); ++i) {
            //            assert(test1.path[i] == test2.path[i]);
            //          }
            
            if (k_close_all_genes) {
              rotate(g, mother, random_engine);
              rotate(g, father, random_engine);
            }
            
            next_population.push_back(cross_faster(g, isite, mother, father));
          }
        }
      }
      population = std::move(next_population);
    }
    
    // Perform mutations
    for (auto& gene: population) {
      if (k_close_all_genes) {
        mutate_dfs(g, gene, random_engine);
        rotate(g, gene, random_engine);
      }
      else {
        mutate_faster(g, gene, random_engine);
      }
    }
    
    // Dump output.
    static size_t record = 0;
    for (const auto& gene: population) {
      if (gene.path.size() > record) {
        if (k_print_records) print(g, gene);
        record = gene.path.size();
      }
    }
    
    static uint32_t generation = 0;
    generation += 1;
    if (generation % k_report_record_period == 0) {
      std::cout << "Generation " << generation << ": best length " << record << std::endl;
    }
    if (generation >= k_max_generations) break;
  }
}

#endif /* evolution_h */
