//
//  graph_operations.h
//  FastGraph
//
//  Created by culter on 1/13/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef graph_operations_h
#define graph_operations_h

#include "NodeSet.h"
#include "Gene.h"

template<typename TIndex, typename TDegree, size_t MaxDegree>
bool has_path(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
              const TIndex source,
              const TIndex target,
              const NodeSet& forbidden_nodes) {
//  std::cout << "has_path: g.nodes.size()=" << g.nodes.size() << "; source=" << source << "; target=" << target << "; forbidden_nodes=";
//  print(forbidden_nodes);
//  std::cout << std::endl;
  
  if (source == target) return true;
  
  NodeSet forward_body = {};
  NodeSet reverse_body = {};
  forward_body[source] = true;
  reverse_body[target] = true;
  
  TIndex forward_fringe[MaxNodes] = {};
  TIndex reverse_fringe[MaxNodes] = {};
  TIndex forward_fringe_size = 1;
  TIndex reverse_fringe_size = 1;
  forward_fringe[0] = source;
  reverse_fringe[0] = target;
  
  while (forward_fringe_size && reverse_fringe_size) {
    if (forward_fringe_size < reverse_fringe_size) {
      TIndex this_level[MaxNodes] = {};
      TIndex this_level_size = forward_fringe_size;
      memcpy(this_level, forward_fringe, forward_fringe_size * sizeof(forward_fringe[0]));
      forward_fringe_size = 0;
      
      for (TIndex iv = 0; iv < this_level_size; ++iv) {
        TIndex v = this_level[iv];
//        std::cout << "Expanding " << v << " forward" << std::endl;
        for (auto w = g.nodes[v].succ_cbegin(); w != g.nodes[v].succ_cend(); ++w) {
          if (forbidden_nodes[*w]) {
//            std::cout << "  " << *w << " is forbidden" << std::endl;
          } else {
            if (reverse_body[*w]) {
//              std::cout << "  Found connection! reverse_body[" << *w << "]" << std::endl;
              return true;
            }
            else if (!forward_body[*w]) {
//              std::cout << "  Pushing " << *w << " forward" << std::endl;
              forward_fringe[forward_fringe_size] = *w;
              forward_fringe_size++;
              
              forward_body[*w] = true;
            }
          }
        }
      }
    } else {
      TIndex this_level[MaxNodes] = {};
      TIndex this_level_size = reverse_fringe_size;
      memcpy(this_level, reverse_fringe, reverse_fringe_size * sizeof(reverse_fringe[0]));
      reverse_fringe_size = 0;
      
      for (TIndex iv = 0; iv < this_level_size; ++iv) {
        TIndex v = this_level[iv];
//        std::cout << "Expanding " << v << " backward" << std::endl;
        for (auto w = g.nodes[v].pred_cbegin(); w != g.nodes[v].pred_cend(); ++w) {
          if (forbidden_nodes[*w]) {
//            std::cout << "  " << *w << " is forbidden" << std::endl;
          } else {
            if (forward_body[*w]) {
//              std::cout << "  Found connection! forward_body[" << *w << "]" << std::endl;
              return true;
            }
            else if (!reverse_body[*w]) {
//              std::cout << "  Pushing " << *w << " backward" << std::endl;
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


template<typename TIndex, typename TDegree, size_t MaxDegree>
bool has_path(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
              std::vector<TIndex> base_path) {
  NodeSet forbidden = {};
  if (base_path.size() > 2) {
    flatten(forbidden, base_path.cbegin() + 1, base_path.cend() - 1);
  }
  return has_path(g, base_path.back(), base_path.front(), forbidden);
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
bool has_path(const FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
              const TIndex source,
              const TIndex target) {
  return has_path(g, source, target, NodeSet{});
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
void remove_node(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                 const TIndex target) {
//  std::cout << "Removing unreachable node: " << target << " (" << g.names[target] << ")" << std::endl;
  g.nodes.erase(g.nodes.begin() + target);
  g.names.erase(g.names.begin() + target);
  for (auto& node: g.nodes) {
    node.remove_all_connections(target);
  }
}

template<typename TIndex, typename TDegree, size_t MaxDegree>
void restrict_to_scc(FastGraph<Node<TIndex, TDegree, MaxDegree>>& g,
                     const TIndex source) {
  bool found_outlier = true;
  while (found_outlier) {
    found_outlier = false;
    for (TIndex i = 0; i < g.nodes.size(); ++i) {
      if (!has_path(g, source, i) || !has_path(g, i, source)) {
        remove_node(g, i);
        --i;
        found_outlier = true;
        break;
      }
    }
  }
}

#endif /* graph_operations_h */
