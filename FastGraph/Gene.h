//
//  Gene.h
//  FastGraph
//
//  Created by culter on 1/15/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef Gene_h
#define Gene_h

template<typename TIndex>
struct Gene {
  std::vector<TIndex> path;
  
  Gene() {}
  Gene(std::vector<TIndex> in_path): path(in_path) {}
  
  friend std::ostream& operator<<(std::ostream& os, const Gene<TIndex>& dt);
};


template<typename TIndex>
std::ostream& operator<<(std::ostream& os, const std::vector<TIndex>& path)
{
  os << "Gene len=" << path.size() << ":";
  for (TIndex x: path) {
    os << " " << x;
  }
  return os;
}

template<typename TIndex>
std::ostream& operator<<(std::ostream& os, const Gene<TIndex>& gene)
{
  return os << gene.path;
}



#endif /* Gene_h */
