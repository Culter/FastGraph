//
//  io_util.hpp
//  FastGraph
//
//  Created by culter on 1/13/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef io_util_hpp
#define io_util_hpp

#include <fstream>
#include <iostream>
#include <string>

#include "FastGraph.h"
#include "io_util.hpp"
#include "string_util.h"

template<typename TNode>
void read_massey(FastGraph<TNode>& g) {
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
    
    auto node_a = std::find(g.names.cbegin(), g.names.cend(), team_a);
    if (node_a == g.names.cend())
    {
      g.names.push_back(team_a);
      g.nodes.emplace_back();
    }
    auto node_b = std::find(g.names.cbegin(), g.names.cend(), team_b);
    if (node_b == g.names.cend())
    {
      g.names.push_back(team_b);
      g.nodes.emplace_back();
    }
    
    uint16_t node_index_a = std::find(g.names.cbegin(), g.names.cend(), team_a) - g.names.cbegin();
    uint16_t node_index_b = std::find(g.names.cbegin(), g.names.cend(), team_b) - g.names.cbegin();
    
//    std::cout << team_a << " (index " << node_index_a << ") beat " << team_b << " (index " << node_index_b << ")" << std::endl;
    
    g.nodes[node_index_a].succ_push(node_index_b);
    g.nodes[node_index_b].pred_push(node_index_a);
    g.check();
  }
  
  //    int x = 0;
  //    for (auto node: nodes) {
  //      std::cout << x << " " << team_names[x] << ": ";
  //      node.print();
  //      x += 1;
  //    }
}

#endif /* io_util_hpp */
