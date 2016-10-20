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
#include <unordered_map>

#include "FastGraph.h"
#include "io_util.hpp"
#include "string_util.h"

std::unordered_map<std::string, std::string> load_csv() {
  std::unordered_map<std::string, std::string> map;
  
  std::ifstream infile;
  infile.open("names.csv");
  
  std::string line;
  while (std::getline(infile, line))
  {
    auto pos = line.find_first_of(',');
    std::string key(line.cbegin(), line.cbegin() + pos);
    std::string value(line.cbegin() + pos + 1, line.cend() - 1);
    map[key] = value;
    
//    std::cout << key << " will be renamed as " << value << std::endl;
  }
  
  return map;
}

template<typename TNode>
void read_massey(FastGraph<TNode>& g) {
  auto name_map = load_csv();
  
  std::ifstream infile;
//  infile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
  infile.open("edges.txt");
//  std::cout << "Hello???" << std::endl;
  std::string line;
  while (std::getline(infile, line))
  {
//    std::cout << line << std::endl;
    std::string node_name_a(line.cbegin() + 12, line.cbegin() + 37);
    std::string score_a(line.cbegin() + 37, line.cbegin() + 39);
    std::string node_name_b(line.cbegin() + 41, line.cbegin() + 66);
    std::string score_b(line.cbegin() + 66, line.cbegin() + 68);
    
    node_name_a = name_map[trim(node_name_a)];
    node_name_b = name_map[trim(node_name_b)];
    int nscore_a = atoi(score_a.c_str());
    int nscore_b = atoi(score_b.c_str());
    
    auto node_a = std::find(g.names.cbegin(), g.names.cend(), node_name_a);
    if (node_a == g.names.cend())
    {
      g.names.push_back(node_name_a);
      g.nodes.emplace_back();
    }
    auto node_b = std::find(g.names.cbegin(), g.names.cend(), node_name_b);
    if (node_b == g.names.cend())
    {
      g.names.push_back(node_name_b);
      g.nodes.emplace_back();
    }
    
    uint16_t node_index_a = std::find(g.names.cbegin(), g.names.cend(), node_name_a) - g.names.cbegin();
    uint16_t node_index_b = std::find(g.names.cbegin(), g.names.cend(), node_name_b) - g.names.cbegin();
    
//    std::cout << node_name_a << " (index " << node_index_a << ") beat " << node_name_b << " (index " << node_index_b << ")" << std::endl;
    
    g.nodes[node_index_a].succ_push(node_index_b);
    g.nodes[node_index_b].pred_push(node_index_a);
    
    if (nscore_a == 0 && nscore_b == 0) {
      g.nodes[node_index_b].succ_push(node_index_a);
      g.nodes[node_index_a].pred_push(node_index_b);
    }
    
    g.check();
  }
}

#endif /* io_util_hpp */
