//
//  NodeSet.h
//  FastGraph
//
//  Created by culter on 1/13/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef NodeSet_h
#define NodeSet_h

// Next multiple of 64 greater than 671
constexpr static const uint16_t MaxNodes = 704;

// Two bytes per slot seems to be the fastest, at least in the tests I've run.
// It's also faster than std::bitset<MaxNodes>.
typedef uint16_t NodeSet[MaxNodes];

template<typename InputIterator>
void flatten(NodeSet& out, InputIterator begin, InputIterator end) {
  for (auto in = begin; in != end; ++in) {
    out[*in] = true;
  }
}

void print(const NodeSet& node_set) {
  std::cout << "NodeSet {array_size=" << MaxNodes << "; nonzero=[";
  for (uint16_t i = 0; i < MaxNodes; ++i) {
    if (node_set[i]) {
      std::cout << i << ",";
    }
  }
  std::cout << "]}";
}

#endif /* NodeSet_h */
