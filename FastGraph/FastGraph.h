//
//  FastGraph.h
//  FastGraph
//
//  Created by culter on 1/13/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef FastGraph_h
#define FastGraph_h

#include <vector>

template<typename TNode>
struct FastGraph {
  std::vector<TNode> nodes;
  std::vector<std::string> names;
  
  void check() const {
    for (const auto& n: nodes) {
      for (auto im = n.succ_cbegin(); im != n.succ_cend(); ++im) {
        assert(0 <= *im);
        assert(*im < nodes.size());
      }
      for (auto im = n.pred_cbegin(); im != n.pred_cend(); ++im) {
        assert(0 <= *im);
        if (*im >= nodes.size()) {
          n.print();
        }
        assert(*im < nodes.size());
      }
    }
  }
};

template<typename TIndex, typename TDegree, size_t MaxDegree>
FastGraph<Node<TIndex, TDegree, MaxDegree>> cycle(int length) {
  FastGraph<Node<TIndex, TDegree, MaxDegree>> answer;
  answer.nodes.resize(length);
  answer.names.resize(length);
  for (int i = 0; i < length; ++i) {
    answer.nodes[i].succ_push((i + 1) % length);
    answer.nodes[i].pred_push((i + length - 1) % length);
  }
  return answer;
}

#endif /* FastGraph_h */
