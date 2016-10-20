//
//  gene_operations.h
//  FastGraph
//
//  Created by culter on 1/13/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef gene_operations_h
#define gene_operations_h

#include <iostream>
#include <unordered_set>

#include "NodeSet.h"
#include "Gene.h"
#include "graph_operations.h"

template<typename TIndex, typename TDegree, size_t MaxDegree>
Gene<TIndex> cross_reference(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                             TIndex site,
                             const Gene<TIndex>& mother,
                             const Gene<TIndex>& father) {
  // Form the longest conceivable cross
  std::vector<TIndex> m(mother.path.cbegin(), std::find(mother.path.cbegin(), mother.path.cend(), site));
  std::vector<TIndex> f(std::find(father.path.cbegin(), father.path.cend(), site)+1, father.path.cend());
  std::vector<TIndex> child;
  
//  std::cout << "cross_reference===========================================================================================" << std::endl;
  
  // Whittle it down until it no longer self-intersects
  while (true) {
    child = m;
    child.push_back(site);
    child.insert(child.end(), f.cbegin(), f.cend());
    
//    std::cout << "m = " << m << std::endl;
//    std::cout << "f = " << f << std::endl;
//    std::cout << "child = " << child << std::endl;
    
    std::unordered_set<TIndex> nodes(child.cbegin(), child.cend());
    if (nodes.size() == child.size()) {
      // Hooray! Candidate does not repeat any nodes.
      if (has_path(g, child)) {
        // Hooray! Path is closable.
        break;
      }
      else {
//        std::cout << "Vector child is not closable. Trimming..." << std::endl;
      }
    }
    else {
//      std::cout << "Vector child overlaps itself. Trimming..." << std::endl;
    }
    
    if (!m.empty()) {
      m.erase(m.begin());
    }
    if (!f.empty()) {
      f.pop_back();
    }
  }
  
  return child;
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
Gene<TIndex> cross_2(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                        TIndex site,
                        const Gene<TIndex>& mother,
                        const Gene<TIndex>& father) {
  const auto m2 = std::find(mother.path.cbegin(), mother.path.cend(), site);
  const auto f1 = std::find(father.path.cbegin(), father.path.cend(), site) + 1;
  
  auto m1 = m2;
  auto f2 = f1;
  NodeSet sa = {};
  sa[site] = true;
  
  // Lengthen the shorter of m and f until the remainders are equal
  int mother_more_room = (int)(m1 - mother.path.cbegin()) - (int)(father.path.cend() - f2);
  while (mother_more_room < 0) {
    assert(f2 != father.path.cend());
    sa[*f2] = true;
    f2++;
    mother_more_room++;
  }
  while (mother_more_room > 0) {
    assert(m1 != mother.path.cbegin());
    m1--;
    sa[*m1] = true;
    mother_more_room--;
  }
  
  // Lengthen both m and f until they collide
  while (m1 != mother.path.cbegin()) {
    assert(f2 != father.path.cend());
    auto mn = *(m1 - 1);
    auto fn = *f2;
    if (mn == fn || sa[mn] || sa[fn]) break;
    sa[mn] = true;
    sa[fn] = true;
    m1--;
    f2++;
  }
  
  // Now m + site + f is as long as possible without intersecting itself.
  // But we may need to shorten it again to be closable.
  // Form the longest conceivable cross
  std::vector<TIndex> m(m1, m2);
  std::vector<TIndex> f(f1, f2);
  std::vector<TIndex> child;
  
//  std::cout << "cross_2===========================================================================================" << std::endl;
  
  // Whittle it down until it no longer self-intersects
  while (true) {
    child = m;
    child.push_back(site);
    child.insert(child.end(), f.cbegin(), f.cend());
    
//    std::cout << "m = " << m << std::endl;
//    std::cout << "f = " << f << std::endl;
//    std::cout << "child = " << child << std::endl;
    
    std::unordered_set<TIndex> nodes(child.cbegin(), child.cend());
    if (nodes.size() == child.size()) {
      // Hooray! Candidate does not repeat any nodes.
      if (has_path(g, child)) {
        // Hooray! Path is closable.
        break;
      }
      else {
//        std::cout << "Vector child is not closable. Trimming..." << std::endl;
      }
    }
    else {
//      std::cout << "Vector child overlaps itself. Trimming..." << std::endl;
    }
    
    if (!m.empty()) {
      m.erase(m.begin());
    }
    if (!f.empty()) {
      f.pop_back();
    }
  }
  
  return child;
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
Gene<TIndex> cross_fast(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                   TIndex site,
                   const Gene<TIndex>& mother,
                   const Gene<TIndex>& father) {
  const auto m2 = std::find(mother.path.cbegin(), mother.path.cend(), site);
  const auto f1 = std::find(father.path.cbegin(), father.path.cend(), site) + 1;
  
  auto m1 = m2;
  auto f2 = f1;
  NodeSet sa = {};
  sa[site] = true;
  
  // Lengthen the shorter of m and f until the remainders are equal
  int mother_more_room = (int)(m1 - mother.path.cbegin()) - (int)(father.path.cend() - f2);
  while (mother_more_room < 0) {
    assert(f2 != father.path.cend());
    sa[*f2] = true;
    f2++;
    mother_more_room++;
  }
  while (mother_more_room > 0) {
    assert(m1 != mother.path.cbegin());
    m1--;
    sa[*m1] = true;
    mother_more_room--;
  }
  
  // Lengthen both m and f until they collide
  while (m1 != mother.path.cbegin()) {
    assert(f2 != father.path.cend());
    auto mn = *(m1 - 1);
    auto fn = *f2;
    if (mn == fn || sa[mn] || sa[fn]) break;
    sa[mn] = true;
    sa[fn] = true;
    m1--;
    f2++;
  }
  
  // Now m + site + f is as long as possible without intersecting itself.
  // But we may need to shorten it again to be closable.
  
  // Copy-paste job from has_path
  {
    NodeSet forward_body = {};
    NodeSet reverse_body = {};
    TIndex forward_fringe[MaxNodes];
    TIndex reverse_fringe[MaxNodes];
    TIndex forward_fringe_size = 0;
    TIndex reverse_fringe_size = 0;
    
    while (true) {
      auto source = *(f2 - 1);
      auto target = *m1;
      auto& forbidden_nodes = sa;
      
      forbidden_nodes[source] = false;
      forbidden_nodes[target] = false;
      
      forward_body[source] = true;
      reverse_body[target] = true;
      forward_fringe[forward_fringe_size] = source;
      reverse_fringe[reverse_fringe_size] = target;
      forward_fringe_size += 1;
      reverse_fringe_size += 1;
      
      while (forward_fringe_size && reverse_fringe_size) {
        // Regardless of the lengths of the fringes, at every step at this loop level, we must explore both at least once.
        TIndex this_level[MaxNodes];
        TIndex this_level_size = forward_fringe_size;
        memcpy(this_level, forward_fringe, forward_fringe_size * sizeof(forward_fringe[0]));
        forward_fringe_size = 0;
        
        for (TIndex iv = 0; iv < this_level_size; ++iv) {
          TIndex v = this_level[iv];
          for (auto w = g.nodes[v].succ_cbegin(); w != g.nodes[v].succ_cend(); ++w) {
            if (!forbidden_nodes[*w]) {
              if (reverse_body[*w]) goto found_path;
              else if (!forward_body[*w]) {
                forward_fringe[forward_fringe_size] = *w;
                forward_fringe_size++;
                
                forward_body[*w] = true;
              }
            }
          }
        }
        
        this_level_size = reverse_fringe_size;
        memcpy(this_level, reverse_fringe, reverse_fringe_size * sizeof(reverse_fringe[0]));
        reverse_fringe_size = 0;
        
        for (TIndex iv = 0; iv < this_level_size; ++iv) {
          TIndex v = this_level[iv];
          for (auto w = g.nodes[v].pred_cbegin(); w != g.nodes[v].pred_cend(); ++w) {
            if (!forbidden_nodes[*w]) {
              if (forward_body[*w]) goto found_path;
              else if (!reverse_body[*w]) {
                reverse_fringe[reverse_fringe_size] = *w;
                reverse_fringe_size++;
                
                reverse_body[*w] = true;
              }
            }
          }
        }
      }
      
      assert(m1 + 1 <= m2);
      assert(f1 <= f2 - 1);
      
      //Exhausted fringes but didn't find path. Better rolllll back.
      m1++;
      f2--;
      assert(mother.path.cbegin() <= m1);
      assert(m1 <= m2);
      assert(m2 <= mother.path.cend());
      assert(father.path.cbegin() <= f1);
      assert(f1 <= f2);
      assert(f2 <= father.path.cend());
    }
  }
  
found_path:
  assert(mother.path.cbegin() <= m1);
  assert(m1 <= m2);
  assert(m2 <= mother.path.cend());
  assert(father.path.cbegin() <= f1);
  assert(f1 <= f2);
  assert(f2 <= father.path.cend());
  
  Gene<TIndex> out;
  out.path.reserve((m2 - m1) + 1 + (f2 - f1) + 1); // The last +1 is to accomodate future mutations.
  out.path.insert(out.path.cend(), m1, m2);
  out.path.push_back(site);
  out.path.insert(out.path.cend(), f1, f2);
  
  assert((out.path.size() == 1) || (out.path.front() != out.path.back()));
  
  return out;
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
Gene<TIndex> cross_faster(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                          TIndex site,
                          const Gene<TIndex>& mother,
                          const Gene<TIndex>& father) {
  const auto m2 = std::find(mother.path.cbegin(), mother.path.cend(), site);
  const auto f1 = std::find(father.path.cbegin(), father.path.cend(), site) + 1;
  
  auto m1 = m2;
  auto f2 = f1;
  NodeSet sa = {};
  sa[site] = true;
  
  // Lengthen the shorter of m and f until the remainders are equal
  int mother_more_room = (int)(m1 - mother.path.cbegin()) - (int)(father.path.cend() - f2);
  while (mother_more_room < 0) {
    assert(f2 != father.path.cend());
    sa[*f2] = true;
    f2++;
    mother_more_room++;
  }
  while (mother_more_room > 0) {
    assert(m1 != mother.path.cbegin());
    m1--;
    sa[*m1] = true;
    mother_more_room--;
  }
  
  // Lengthen both m and f until they collide
  while (m1 != mother.path.cbegin()) {
    assert(f2 != father.path.cend());
    auto mn = *(m1 - 1);
    auto fn = *f2;
    if (mn == fn || sa[mn] || sa[fn]) break;
    sa[mn] = true;
    sa[fn] = true;
    m1--;
    f2++;
  }
  
  // Now m + site + f is as long as possible without intersecting itself.
  // But we may need to shorten it again to be closable.
  
  // Copy-paste job from has_path
  {
    NodeSet forward_body = {};
    NodeSet reverse_body = {};
    TIndex forward_fringe[MaxNodes];
    TIndex reverse_fringe[MaxNodes];
    TIndex forward_fringe_size = 0;
    TIndex reverse_fringe_size = 0;
    
    bool must_expand_both = false;
    
    while (true) {
      auto source = *(f2 - 1);
      auto target = *m1;
      auto& forbidden_nodes = sa;
      
      forbidden_nodes[source] = false;
      forbidden_nodes[target] = false;
      
      forward_body[source] = true;
      reverse_body[target] = true;
      forward_fringe[forward_fringe_size] = source;
      reverse_fringe[reverse_fringe_size] = target;
      forward_fringe_size += 1;
      reverse_fringe_size += 1;
      
      while (forward_fringe_size && reverse_fringe_size) {
        TIndex this_level[MaxNodes];
        
        if (must_expand_both || forward_fringe_size < reverse_fringe_size) {
          TIndex this_level_size = forward_fringe_size;
          memcpy(this_level, forward_fringe, forward_fringe_size * sizeof(forward_fringe[0]));
          forward_fringe_size = 0;
          
          for (TIndex iv = 0; iv < this_level_size; ++iv) {
            TIndex v = this_level[iv];
            for (auto w = g.nodes[v].succ_cbegin(); w != g.nodes[v].succ_cend(); ++w) {
              if (!forbidden_nodes[*w]) {
                if (reverse_body[*w]) goto found_path;
                else if (!forward_body[*w]) {
                  forward_fringe[forward_fringe_size] = *w;
                  forward_fringe_size++;
                  
                  forward_body[*w] = true;
                }
              }
            }
          }
        }
        
        if (must_expand_both || forward_fringe_size >= reverse_fringe_size) {
          TIndex this_level_size = reverse_fringe_size;
          memcpy(this_level, reverse_fringe, reverse_fringe_size * sizeof(reverse_fringe[0]));
          reverse_fringe_size = 0;
          
          for (TIndex iv = 0; iv < this_level_size; ++iv) {
            TIndex v = this_level[iv];
            for (auto w = g.nodes[v].pred_cbegin(); w != g.nodes[v].pred_cend(); ++w) {
              if (!forbidden_nodes[*w]) {
                if (forward_body[*w]) goto found_path;
                else if (!reverse_body[*w]) {
                  reverse_fringe[reverse_fringe_size] = *w;
                  reverse_fringe_size++;
                  
                  reverse_body[*w] = true;
                }
              }
            }
          }
        }
        
        // Okay, one force-expand pair is enough.
        must_expand_both = false;
      }
      
      assert(m1 + 1 <= m2);
      assert(f1 <= f2 - 1);
      
      //Exhausted fringes but didn't find path. Better rolllll back.
      m1++;
      f2--;
      assert(mother.path.cbegin() <= m1);
      assert(m1 <= m2);
      assert(m2 <= mother.path.cend());
      assert(father.path.cbegin() <= f1);
      assert(f1 <= f2);
      assert(f2 <= father.path.cend());
      
      must_expand_both = true;
    }
  }
  
found_path:
  assert(mother.path.cbegin() <= m1);
  assert(m1 <= m2);
  assert(m2 <= mother.path.cend());
  assert(father.path.cbegin() <= f1);
  assert(f1 <= f2);
  assert(f2 <= father.path.cend());
  
  Gene<TIndex> out;
  out.path.reserve((m2 - m1) + 1 + (f2 - f1) + 1); // The last +1 is to accomodate future mutations.
  out.path.insert(out.path.cend(), m1, m2);
  out.path.push_back(site);
  out.path.insert(out.path.cend(), f1, f2);
  
  assert((out.path.size() == 1) || (out.path.front() != out.path.back()));
  
  return out;
}

template<typename TIndex>
void print(const Gene<TIndex>& gene) {
  std::cout << "Gene of length " << gene.path.size() << ": ";
  for (auto node: gene.path) {
    std::cout << node << ' ';
  }
  std::cout << std::endl;
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
void print(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g, const Gene<TIndex>& gene) {
  std::cout << "Gene of length " << gene.path.size() << ": ";
  for (int i = 0; i < gene.path.size(); ++i) {
//  for (auto node: gene.path) {
    auto node = gene.path[i];
    auto next = gene.path[(i+1) % gene.path.size()];
    //    std::cout << node << " (" << g.names[node] << ") ";
        std::cout << g.names[node] << " ";
    bool forward = (std::find(g.nodes[node].succ_cbegin(), g.nodes[node].succ_cend(), next) != g.nodes[node].succ_cend());
    bool reverse = (std::find(g.nodes[node].pred_cbegin(), g.nodes[node].pred_cend(), next) != g.nodes[node].pred_cend());
//    if (!forward) {
//      throw 79;
//    }
    if (forward && !reverse) {
      std::cout << "> ";
    }
    else if (forward && reverse) {
      std::cout << "? ";
    }
  }
  std::cout << std::endl;
}

#endif /* gene_operations_h */
