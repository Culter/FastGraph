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
#include <thread>

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
      
      // Push unexplored successors onto the stack.
      // Shuffle them, but only explore the connection directly to the front as a last resort.
      bool hits_front = false;
      temp.clear();
      for (auto s = g.nodes[n].succ_cbegin(); s != g.nodes[n].succ_cend(); ++s) {
        if (*s == gene.path.front()) {
          hits_front = true;
        }
        else if (!explored[*s]) {
          temp.push_back(*s);
        }
      }
      std::shuffle(temp.begin(), temp.end(), rng);
      
      if (hits_front) {
        to_explore.push_back(gene.path.front());
      }
      to_explore.insert(to_explore.end(), temp.cbegin(), temp.cend());
    }
  }
  
  std::cout << "Shouldn't have reached here!" << std::endl;
  assert(false);
}

template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
void optimize(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
              Gene<TIndex>& gene,
              TRng& rng) {
  for (int tried = 0; tried < gene.path.size(); ++tried) {
    auto baseline_size = gene.path.size();
    mutate_dfs(g, gene, rng);
    if (gene.path.size() > baseline_size) {
      // Reset! We want to keep doing this until there are no more expansions.
      tried = 0;
    }
    std::rotate(gene.path.begin(), gene.path.begin() + 1, gene.path.end());
  }
}

template<typename TIndex, typename TDegree, size_t MaxDegree, typename TRng>
void rotate(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
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

template<typename TIndex>
struct Evolver {
  std::mt19937 rng;
  std::vector<Gene<TIndex>> population;
  Gene<TIndex> longest;
  int age;
};

template<typename TIndex>
void merge(Evolver<TIndex>& target, Evolver<TIndex>& victim) {
  target.population.insert(target.population.end(), victim.population.cbegin(), victim.population.cend());
  victim.population.clear();
  
  if (victim.longest.path.size() > target.longest.path.size()) {
    target.longest = victim.longest;
  }
  victim.longest.path.clear();
  
  target.age = std::max(target.age, victim.age);
  victim.age = 0;
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
void evolve(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
            Evolver<TIndex>& evolver,
            uint32_t max_generations) {
  for (uint32_t generation = 1; generation <= max_generations; ++generation) {
    evolver.age += 1;
    
    // Refresh gene pool
    for (int y = 0; y < k_refresh_edge_count; ++y) {
      auto refresh = get_reversible_edges(g, evolver.rng);
      evolver.population.insert(evolver.population.end(), refresh.cbegin(), refresh.cend());
    }
    
    // Don't mind me, just testing the optimize() function...
//    std::cout << "Before optimization: ";
//    print(g, evolver.population[0]);
//    optimize(g, evolver.population[0], evolver.rng);
//    std::cout << "After optimization: ";
//    print(g, evolver.population[0]);
//    exit(0);
    
    // Pre-compute which sites are touched by which genes
    std::vector<std::vector<TIndex>> site_to_gene_pool(g.nodes.size());
    for (int igene = 0; igene < evolver.population.size(); ++igene) {
      for (const TIndex isite: evolver.population[igene].path) {
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
            Gene<TIndex>& mother = evolver.population[candidates[gene_chooser(evolver.rng)]];
            Gene<TIndex>& father = evolver.population[candidates[gene_chooser(evolver.rng)]];
            
            //          auto test1 = cross_reference(g, isite, mother, father);
            //          auto test2 = cross_faster(g, isite, mother, father);
            //
            //          assert(test1.path.size() == test2.path.size());
            //          for (int i = 0; i < test1.path.size(); ++i) {
            //            assert(test1.path[i] == test2.path[i]);
            //          }
            
            if (k_close_all_genes) {
              rotate(g, mother, evolver.rng);
              rotate(g, father, evolver.rng);
            }
            
            next_population.push_back(cross_faster(g, isite, mother, father));
          }
        }
      }
      evolver.population = std::move(next_population);
    }
    
    // Perform mutations
    for (auto& gene: evolver.population) {
      if (generation <= max_generations * 0.95 + 10) {
        if (k_close_all_genes) {
          mutate_dfs(g, gene, evolver.rng);
          rotate(g, gene, evolver.rng);
        }
        else {
          mutate_faster(g, gene, evolver.rng);
        }
      }
      else if (generation <= max_generations * 0.99 + 10) {
        if (k_close_after_one_third) {
          mutate_dfs(g, gene, evolver.rng);
          rotate(g, gene, evolver.rng);
        }
        else {
          mutate_faster(g, gene, evolver.rng);
        }
      }
      else {
        if (k_optimize_after_two_thirds) {
          optimize(g, gene, evolver.rng);
          rotate(g, gene, evolver.rng);
        }
        else {
          mutate_faster(g, gene, evolver.rng);
        }
      }
    }
    
    // Record keeping
    for (const auto& gene: evolver.population) {
      if (gene.path.size() > evolver.longest.path.size()) {
        evolver.longest = gene;
      }
    }
  }
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
void evolve_single(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g) {
  Evolver<TIndex> evolver;
  evolver.rng.seed(k_random_seed);
  
  if (k_refresh_edge_count < 1) {
    evolver.population = get_reversible_edges(g, evolver.rng);
  }
  
  size_t record = 0;
  for (uint32_t generation = 0; generation < k_max_generations; generation += k_report_record_period) {
    evolve(g, evolver, k_report_record_period);
    if (evolver.longest.path.size() > record) {
      if (k_print_records) print(g, evolver.longest);
      record = evolver.longest.path.size();
    }
    std::cout << "Generation " << generation << ": best length " << record << std::endl;
  }
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
void evolve(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g) {
  Evolver<TIndex> evolvers[num_evolvers];
  for (int i = 0; i < num_evolvers; ++i) {
    evolvers[i].rng.seed(k_random_seed + i);
    evolvers[i].age = 0;
  }

  if (k_refresh_edge_count < 1) {
    for (auto& evolver: evolvers) {
      evolver.population = get_reversible_edges(g, evolver.rng);
    }
  }
  
  size_t record = 0;
  uint32_t generation = 0;
  for (int multiplier = 1; generation <= k_max_generations; ++multiplier) {
    uint32_t generations_this_epoch = multiplier * k_report_record_period;
    generation += generations_this_epoch;
    std::vector<std::thread> threads;
    for (auto& evolver: evolvers) {
      threads.emplace_back([&]{ evolve(g, evolver, generations_this_epoch); });
    }
    for (auto& thread: threads) {
      thread.join();
    }
    
    for (auto& evolver: evolvers) {
      if (evolver.longest.path.size() > record) {
        if (k_print_records) print(g, evolver.longest);
        record = evolver.longest.path.size();
      }
    }
    
    std::cout << "Generation " << generation << ": best length " << record << std::endl;
    
    // Shuffle!
    for (int i = 0; 2*i+1 < num_evolvers; ++i) {
//      std::cout << "Merging " << 2*i+1 << " to " << 2*i << std::endl;
      merge(evolvers[2*i], evolvers[2*i+1]);
      if (k_refresh_edge_count < 1) {
        evolvers[2*i+1].population = get_reversible_edges(g, evolvers[2*i+1].rng);
      }
    }
    for (int i = 1; 2*i < num_evolvers; ++i) {
//      std::cout << "Swapping " << i << " with " << 2*i << std::endl;
      std::swap(evolvers[i], evolvers[2*i]);
    }
    for (int i = 0; i < num_evolvers; ++i) {
//      std::cout << "Pool " << i << " has " << evolvers[i].population.size() << " genes" << std::endl;
    }
  }
}

#endif /* evolution_h */
