//
//  Node.h
//  FastGraph
//
//  Created by culter on 1/13/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef Node_h
#define Node_h

#include <array>
#include <iostream>

// The idea is to fit easily in the 32 KB L1 data cache on a Haswell core.

template<typename TIndex, typename TDegree, size_t MaxDegree>
class alignas(16) Node {
  TDegree out_degree;
  TDegree in_degree;
  std::array<TIndex, MaxDegree> neighbors;
  
public:
  Node(): out_degree(0), in_degree(0), neighbors{} {}
  
  constexpr const TDegree get_out_degree() const { return out_degree; }
  constexpr const TDegree get_in_degree() const { return in_degree; }
  
  constexpr const TIndex* succ_cbegin() const { return neighbors.cbegin(); }
  constexpr const TIndex* succ_cend() const { return neighbors.cbegin() + out_degree; }
  constexpr const TIndex* pred_cbegin() const { return neighbors.cbegin() + out_degree; }
  constexpr const TIndex* pred_cend() const { return neighbors.cbegin() + out_degree + in_degree; }
  
  TIndex* succ_begin() { return neighbors.begin(); }
  TIndex* succ_end() { return neighbors.begin() + out_degree; }
  TIndex* pred_begin() { return neighbors.begin() + out_degree; }
  TIndex* pred_end() { return neighbors.begin() + out_degree + in_degree; }
  
  void succ_push(TIndex new_index) {
    std::copy_backward(pred_cbegin(), pred_cend(), pred_end() + 1);
    *succ_end() = new_index;
    out_degree += 1;
  }
  void pred_push(TIndex new_index) {
    *pred_end() = new_index;
    in_degree += 1;
  }
  
  void remove_all_connections(TIndex target) {
    while (true) {
      auto place = std::find(succ_begin(), succ_end(), target);
      if (place != succ_end()) {
        std::copy(place + 1, pred_end(), place);
        out_degree -= 1;
      } else {
        break;
      }
    }
    for (auto n = succ_begin(); n != succ_end(); ++n) {
      if (*n == target) {
        std::cout << "Impossible!" << std::endl;
      }
      else if (*n > target) {
        *n -= 1;
      }
    }
    while (true) {
      auto place = std::find(pred_begin(), pred_end(), target);
      if (place != pred_end()) {
        std::copy(place + 1, pred_end(), place);
        in_degree -= 1;
      } else {
        break;
      }
    }
    for (auto n = pred_begin(); n != pred_end(); ++n) {
      if (*n == target) {
        std::cout << "Impossible!" << std::endl;
      }
      else if (*n > target) {
        *n -= 1;
      }
    }
  }
  
  void print() const {
    std::cout << "Node<" << MaxDegree << ">: out_degree=" << (int)out_degree << "(";
    for (auto in = succ_cbegin(); in != succ_cend(); ++in) {
      std::cout << *in << ",";
    }
    std::cout << ") in_degree=" << (int)in_degree << "(";
    for (auto in = pred_cbegin(); in != pred_cend(); ++in) {
      std::cout << *in << ",";
    }
    std::cout << ") neighbors=[";
    for (auto n: neighbors) {
      std::cout << n << ",";
    }
    std::cout << "].";
  }
  
  friend std::ostream& operator<<(std::ostream& os, const Node<TIndex, TDegree, MaxDegree>& node);
};

template<typename TIndex, typename TDegree, size_t MaxDegree>
std::ostream& operator<<(std::ostream& os, const Node<TIndex, TDegree, MaxDegree>& node) {
  os << "Node<" << MaxDegree << ">: out_degree=" << node.out_degree << "(";
  for (auto in = node.succ_cbegin(); in != node.succ_cend(); ++in) {
    os << *in << ",";
  }
  os << ") in_degree=" << node.in_degree << "(";
  for (auto in = node.pred_cbegin(); in != node.pred_cend(); ++in) {
    os << *in << ",";
  }
  os << ") neighbors=[";
  for (auto n: node.neighbors) {
    os << n << ",";
  }
  os << "].";
  return os;
}

#endif /* Node_h */
