//
//  main.cpp
//  FastGraph
//
//  Created by culter on 1/9/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include <iostream>
#include <vector>
#include <unordered_set>
#include <random>

#define NDEBUG
#include <cassert>

constexpr static const uint8_t MaxDegree = 15;
constexpr static const uint16_t MaxNodes = 768;

// The idea is to fit easily in the 32 KB L1 data cache on a Haswell core.

struct Node {
  uint8_t out_degree;
  uint8_t in_degree;
  uint16_t neighbors[MaxDegree];
  
  constexpr const uint16_t* successors_cbegin() const { return neighbors; }
  constexpr const uint16_t* successors_cend() const { return neighbors + out_degree; }
  constexpr const uint16_t* predecessors_cbegin() const { return neighbors + out_degree; }
  constexpr const uint16_t* predecessors_cend() const { return neighbors + out_degree + in_degree; }
  
  void Read() {
    int input = 0;
    uint16_t i = 0;
    
    std::cin >> input; out_degree = input;
    if (out_degree > MaxDegree) {
      std::cerr << "Warning: out_degree " << out_degree << " is too large. Truncating to " << (int)MaxDegree << std::endl;
      out_degree = MaxDegree;
    }
    for (; i < out_degree; ++i) {
      std::cin >> neighbors[i];
    }
    
    std::cin >> input; in_degree = input;
    if (out_degree + in_degree > MaxDegree) {
      std::cerr << "Warning: out_degree + in_degree is too large. Truncating in_degree to " << (int)MaxDegree - out_degree << std::endl;
      in_degree = MaxDegree - out_degree;
    }
    for (; i < out_degree + in_degree; ++i) {
      std::cin >> neighbors[i];
    }
  }
};

struct FastGraph {
  std::vector<Node> nodes;
  
  void Read() {
    unsigned num_nodes = 0;
    std::cin >> num_nodes;
    nodes.resize(num_nodes);
    
    for (uint16_t i = 0; i < num_nodes; ++i) {
      nodes[i].Read();
    }
  }
  
  void create_skip_cycle(uint16_t num_nodes) {
    if (num_nodes < 4) num_nodes = 4;
    
    nodes.resize(num_nodes);
    for (uint16_t i = 0; i < num_nodes; ++i) {
      nodes[i].out_degree = 2;
      nodes[i].in_degree = 2;
      nodes[i].neighbors[0] = (i + 1) % num_nodes;
      nodes[i].neighbors[1] = (i + 2) % num_nodes;
      nodes[i].neighbors[2] = (i + num_nodes - 1) % num_nodes;
      nodes[i].neighbors[3] = (i + num_nodes - 2) % num_nodes;
    }
  }
  
  bool has_path(const uint16_t source,
                const uint16_t target,
                const std::bitset<MaxNodes>& forbidden_nodes) {
    if (source == target) return true;
    
    std::bitset<MaxNodes> forward_body;
    std::bitset<MaxNodes> reverse_body;
    std::vector<uint16_t> forward_fringe;
    std::vector<uint16_t> reverse_fringe;
    std::vector<uint16_t> this_level;
    
    forward_fringe.reserve(nodes.size() / 4);
    reverse_fringe.reserve(nodes.size() / 4);
    this_level.reserve(nodes.size() / 4);
    
    forward_body[source] = true;
    reverse_body[target] = true;
    forward_fringe.push_back(source);
    reverse_fringe.push_back(target);
    
    while (!forward_fringe.empty() && !reverse_fringe.empty()) {
      this_level.clear();
      if (forward_fringe.size() < reverse_fringe.size()) {
        std::swap(this_level, forward_fringe);
        for (const uint16_t v: this_level) {
          for (auto w = nodes[v].successors_cbegin(); w != nodes[v].successors_cend(); ++w) {
            if (!forbidden_nodes[*w]) {
              if (reverse_body[*w]) return true;
              else if (!forward_body[*w]) {
                forward_fringe.push_back(*w);
                forward_body[*w] = true;
              }
            }
          }
        }
      } else {
        std::swap(this_level, reverse_fringe);
        for (const uint16_t v: this_level) {
          for (auto w = nodes[v].predecessors_cbegin(); w != nodes[v].predecessors_cend(); ++w) {
            if (!forbidden_nodes[*w]) {
              if (forward_body[*w]) return true;
              else if (!reverse_body[*w]) {
                reverse_fringe.push_back(*w);
                reverse_body[*w] = true;
              }
            }
          }
        }
      }
    }
    
    return false;
  }
  
  template<typename InputIterator>
  std::bitset<MaxNodes> flatten(InputIterator begin, InputIterator end) {
    std::bitset<MaxNodes> out;
    for (auto in = begin; in != end; ++in) {
      out[*in] = true;
    }
    return out;
  }
  
  std::vector<uint16_t> cross(uint16_t site,
                              std::vector<uint16_t> mother,
                              std::vector<uint16_t> father) {
    //return mother;
    auto m1 = mother.cbegin();
    const auto m2 = std::find(mother.cbegin(), mother.cend(), site);
    const auto f1 = std::find(father.cbegin(), father.cend(), site) + 1;
    auto f2 = father.cend();
    
    assert(std::find(mother.cbegin(), mother.cend(), site) != mother.cend());
    assert(std::find(father.cbegin(), father.cend(), site) != father.cend());
    
    auto sa = flatten(m1, m2);
    auto sb = flatten(f1, f2);
    
    // Start shortening m and f until we can return m + site + f
    while ((sa & sb).any()) {
      if (m1 != m2) {
        sa[*m1] = false;
        m1++;
      }
      if (f1 != f2) {
        f2--;
        sb[*f2] = false;
      }
      
      assert(m1 != mother.cend());
      assert(f2 != father.cbegin());
    }
    
    // Now sa represents all nodes in the prospective path.
    sa &= sb;
    sa[site] = true;
    
    while (!has_path(*(f2 - 1), *m1, sa)) {
      if (m1 != m2) {
        sa[*m1] = false;
        m1++;
      }
      if (f1 != f2) {
        f2--;
        sa[*f2] = false;
      }
      
      assert(m1 != mother.cend());
      assert(f2 != father.cbegin());
    }
    
    //std::vector<uint16_t> out(m1, m2);
    std::vector<uint16_t> out;
    out.reserve((m2 - m1) + 1 + (f2 - f1));
    out.insert(out.cend(), m1, m2);
    out.push_back(site);
    out.insert(out.cend(), f1, f2);
    
    return out;
  }
  
  void evolve() {
    std::mt19937 random_engine(123456);
    
    // Pre-populate using all edges in the graph
    std::vector<std::vector<uint16_t>> population;
    for (uint16_t i = 0; i < nodes.size(); ++i) {
      for (auto pj = nodes[i].successors_cbegin(); pj != nodes[i].successors_cend(); ++pj) {
        auto edge = {i, *pj};
        population.emplace_back(edge);
      }
    }
    
    while (true) {
      // Pre-compute which sites are touched by which genes
      std::vector<std::vector<uint16_t>> site_to_gene_pool(nodes.size());
      for (int igene = 0; igene < population.size(); ++igene) {
        for (const uint16_t isite: population[igene]) {
          site_to_gene_pool[isite].push_back(igene);
        }
      }
      
      // Perform per-site crossover
      std::vector<std::vector<uint16_t>> next_population;
      for (uint16_t isite = 0; isite < nodes.size(); ++isite) {
        const std::vector<uint16_t>& candidates = site_to_gene_pool[isite];
        std::uniform_int_distribution<uint16_t> gene_chooser(0, candidates.size() - 1);
        
        const std::vector<uint16_t>& mother = population[candidates[gene_chooser(random_engine)]];
        const std::vector<uint16_t>& father = population[candidates[gene_chooser(random_engine)]];
        
        next_population.push_back(cross(isite, mother, father));
      }
      population = std::move(next_population);
      
      // Try adding random edges
      for (std::vector<uint16_t>& gene: population) {
        std::vector<uint16_t> potential_additions;
        
        // TODO: This only adds forward. Also add backward at random.
        const uint16_t i = gene.back();
        auto flat = flatten(gene.cbegin(), gene.cend());
        for (auto pj = nodes[i].successors_cbegin(); pj != nodes[i].successors_cend(); ++pj) {
          if (!flat[*pj] && has_path(*pj, gene.front(), flatten(gene.cbegin() + 1, gene.cend()))) {
            potential_additions.push_back(*pj);
          }
        }
        if (!potential_additions.empty()) {
          std::uniform_int_distribution<uint16_t> chooser(0, potential_additions.size() - 1);
          gene.push_back(potential_additions[chooser(random_engine)]);
        }
      }
      
      // Dump output.
      static int generation = 0;
      generation += 1;
      
      size_t record = 0;
      for (auto gene: population) {
        record = std::max(record, gene.size());
        if (gene.size() > nodes.size()) {
          std::cout << "Gene of length " << gene.size() << ": ";
          for (auto node: gene) {
            std::cout << node << ' ';
          }
          std::cout << std::endl;
        }
      }
      std::cout << "Generation " << generation << ": best length " << record << std::endl;
      
      /*
      std::cout << "Generation " << generation << std::endl;
      for (auto gene: population) {
        std::cout << "Gene of length " << gene.size() << ": ";
        for (auto node: gene) {
          std::cout << node << ' ';
        }
        std::cout << std::endl;
      }
       */
      
      if (generation >= 2000) break;
    }
  }
};

int main(int argc, const char * argv[]) {
  FastGraph g;
  //g.Read();
  g.create_skip_cycle(512);
  g.evolve();
  
  return 0;
}
