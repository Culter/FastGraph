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
#include <fstream>
#include <string>

// Trim code stolen from
// http://stackoverflow.com/a/217605/2611123
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

// trim from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}

#define NDEBUG
#include <cassert>

constexpr static const uint8_t MaxDegree = 15;
constexpr static const uint16_t MaxNodes = 768;

// The idea is to fit easily in the 32 KB L1 data cache on a Haswell core.

/*
 TODOs:
 1. Ingest the actual NCAA2015 Massey data. This is its own best test case for profiling. It will probably be different from my experiments so far.
 2. For gene mutation, replace the series of has_path calls with a single enumeration from target to potential sources
 3. For gene crossover, dynamic programming: keep the reachable sets from previous calls to be used in subsequent calls, after shortening the gene to consider sibling of the previous edges
 4. Look up best (Boost graph lib?) C++ breadth-first search implementations. Maybe networkx's core strategy is inherently bad? Consider just going one way, for example.
 5. Implement remaining missing features: (A) extending front instead of back (B) local optimization of best/all genes
 6. Do keep testing the skip-1,2 circle graph, though, to make sure we optimize it
 */

struct Node {
  uint8_t out_degree;
  uint8_t in_degree;
  uint16_t neighbors[MaxDegree];
  
  constexpr const uint16_t* successors_cbegin() const { return neighbors; }
  constexpr const uint16_t* successors_cend() const { return neighbors + out_degree; }
  constexpr const uint16_t* predecessors_cbegin() const { return neighbors + out_degree; }
  constexpr const uint16_t* predecessors_cend() const { return neighbors + out_degree + in_degree; }
  uint16_t* predecessors_end() { return neighbors + out_degree + in_degree; }
  
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
  
  void print() {
    std::cout << "out_degree " << (int)out_degree << ", in_degree " << (int)in_degree;
    for (uint8_t i = 0; i < in_degree + out_degree; ++i) {
      std::cout << " " << neighbors[i];
    }
    std::cout << std::endl;
  }
};

struct FastGraph {
  std::vector<Node> nodes;
  
//  void Read() {
//    unsigned num_nodes = 0;
//    std::cin >> num_nodes;
//    nodes.resize(num_nodes);
//    
//    for (uint16_t i = 0; i < num_nodes; ++i) {
//      nodes[i].Read();
//    }
//  }
  
  void read_massey() {
    std::vector<std::string> team_names;
    
    std::ifstream infile("/Users/culter/Documents/Python/ncaa_final.txt");
    std::string line;
    while (std::getline(infile, line))
    {
      std::string team_a(line.cbegin() + 12, line.cbegin() + 37);
      //std::string score_a(line.cbegin() + 37, line.cbegin() + 39);
      std::string team_b(line.cbegin() + 41, line.cbegin() + 66);
      //std::string score_b(line.cbegin() + 66, line.cbegin() + 68);
      
      team_a = trim(team_a);
      team_b = trim(team_b);
      //int nscore_a = atoi(score_a.c_str());
      //int nscore_b = atoi(score_b.c_str());
      
      auto node_a = std::find(team_names.cbegin(), team_names.cend(), team_a);
      if (node_a == team_names.cend())
      {
        team_names.push_back(team_a);
        nodes.emplace_back();
      }
      auto node_b = std::find(team_names.cbegin(), team_names.cend(), team_b);
      if (node_b == team_names.cend())
      {
        team_names.push_back(team_b);
        nodes.emplace_back();
      }
      
      uint16_t node_index_a = std::find(team_names.cbegin(), team_names.cend(), team_a) - team_names.cbegin();
      uint16_t node_index_b = std::find(team_names.cbegin(), team_names.cend(), team_b) - team_names.cbegin();
      
      nodes[node_index_a].neighbors[nodes[node_index_a].out_degree] = node_index_b;
      nodes[node_index_a].out_degree += 1;
    }
    
    // Fill in predecessor links
    for (uint16_t i = 0; i < nodes.size(); ++i) {
      for (uint16_t j = 0; j < nodes[i].out_degree; ++j) {
        auto pj = nodes[i].neighbors[j];
        *nodes[pj].predecessors_end() = i;
        nodes[pj].in_degree += 1;
      }
    }
    
//    int x = 0;
//    for (auto node: nodes) {
//      std::cout << x << " " << team_names[x] << ": ";
//      node.print();
//      x += 1;
//    }
    
    // exit(0);
  }
  
//  void create_skip_cycle(uint16_t num_nodes) {
//    if (num_nodes < 4) num_nodes = 4;
//    
//    nodes.resize(num_nodes);
//    for (uint16_t i = 0; i < num_nodes; ++i) {
//      nodes[i].out_degree = 2;
//      nodes[i].in_degree = 2;
//      nodes[i].neighbors[0] = (i + 1) % num_nodes;
//      nodes[i].neighbors[1] = (i + 2) % num_nodes;
//      nodes[i].neighbors[2] = (i + num_nodes - 1) % num_nodes;
//      nodes[i].neighbors[3] = (i + num_nodes - 2) % num_nodes;
//    }
//  }
  
  bool has_path(const uint16_t source,
                const uint16_t target,
                const std::bitset<MaxNodes>& forbidden_nodes) {
    if (source == target) return true;
    
    std::bitset<MaxNodes> forward_body;
    std::bitset<MaxNodes> reverse_body;
    forward_body[source] = true;
    reverse_body[target] = true;
    
    uint16_t forward_fringe[MaxNodes];
    uint16_t reverse_fringe[MaxNodes];
    uint16_t forward_fringe_size = 1;
    uint16_t reverse_fringe_size = 1;
    forward_fringe[0] = source;
    reverse_fringe[0] = target;
    
    while (forward_fringe_size && reverse_fringe_size) {
      if (forward_fringe_size < reverse_fringe_size) {
        uint16_t this_level[MaxNodes];
        uint16_t this_level_size = forward_fringe_size;
        memcpy(this_level, forward_fringe, forward_fringe_size * sizeof(forward_fringe[0]));
        forward_fringe_size = 0;
        
        for (uint16_t iv = 0; iv < this_level_size; ++iv) {
          uint16_t v = this_level[iv];
          for (auto w = nodes[v].successors_cbegin(); w != nodes[v].successors_cend(); ++w) {
            if (!forbidden_nodes[*w]) {
              if (reverse_body[*w]) return true;
              else if (!forward_body[*w]) {
                forward_fringe[forward_fringe_size] = *w;
                forward_fringe_size++;
                
                forward_body[*w] = true;
              }
            }
          }
        }
      } else {
        uint16_t this_level[MaxNodes];
        uint16_t this_level_size = reverse_fringe_size;
        memcpy(this_level, reverse_fringe, reverse_fringe_size * sizeof(reverse_fringe[0]));
        reverse_fringe_size = 0;
        
        for (uint16_t iv = 0; iv < this_level_size; ++iv) {
          uint16_t v = this_level[iv];
          for (auto w = nodes[v].predecessors_cbegin(); w != nodes[v].predecessors_cend(); ++w) {
            if (!forbidden_nodes[*w]) {
              if (forward_body[*w]) return true;
              else if (!reverse_body[*w]) {
                reverse_fringe[reverse_fringe_size] = *w;
                reverse_fringe_size++;
                
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
    out.reserve((m2 - m1) + 1 + (f2 - f1) + 1); // The last +1 is to accomodate future mutations.
    out.insert(out.cend(), m1, m2);
    out.push_back(site);
    out.insert(out.cend(), f1, f2);
    
    return out;
  }
  
  void evolve() {
    std::mt19937 random_engine(123456);
    
    // Pre-populate using all edges in the graph that have back-paths
    std::vector<std::vector<uint16_t>> population;
    for (uint16_t i = 0; i < nodes.size(); ++i) {
      for (auto pj = nodes[i].successors_cbegin(); pj != nodes[i].successors_cend(); ++pj) {
        if (has_path(*pj, i, std::bitset<MaxNodes>())) {
          auto edge = {i, *pj};
          population.emplace_back(edge);
        }
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
        if (!candidates.empty()) {
          std::uniform_int_distribution<uint16_t> gene_chooser(0, candidates.size() - 1);
          
          const std::vector<uint16_t>& mother = population[candidates[gene_chooser(random_engine)]];
          const std::vector<uint16_t>& father = population[candidates[gene_chooser(random_engine)]];
          
          next_population.push_back(cross(isite, mother, father));
        }
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
  g.read_massey();
  //g.create_skip_cycle(512);
  g.evolve();
  
  return 0;
}
