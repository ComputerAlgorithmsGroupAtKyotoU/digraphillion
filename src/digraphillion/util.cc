#include "digraphillion/util.h"

#ifdef WIN32
#include "../mingw32/strtok_r.hpp"
#else
#ifdef _WIN32
#define strtok_r strtok_s
#endif
#endif

namespace digraphillion {

using std::string;
using std::vector;

vector<string> split(const string& str, const string sep) {
  vector<char> buf;
  for (string::const_iterator c = str.begin(); c != str.end(); ++c)
    buf.push_back(*c);
  buf.push_back('\0');
  vector<string> v;
  char* last;
  char* p = strtok_r(buf.data(), sep.c_str(), &last);
  while (p != NULL) {
    v.push_back(p);
    p = strtok_r(NULL, sep.c_str(), &last);
  }
  return v;
}

}  // namespace digraphillion
