//
//  string_util.h
//  FastGraph
//
//  Created by culter on 1/13/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef string_util_h
#define string_util_h

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

#endif /* string_util_h */
