// requires c++23, but could be easily rewritten not to

#include <cstring>
#include <string>
#include <algorithm>

namespace rng = std::ranges;

/* in-place trim and squash spaces in a std::string
 * useful for parsing user input
 *
 * squash("  aa    bb c   ") -> "aa bb c"
 */
static void squash(std::string &str) {
        // trim
        str.erase(str.begin(), rng::find_if(str, isgraph));
        str.erase(rng::find_if(str.rbegin(), str.rend(), isgraph).base(), str.end());

        // replace different kinds of whitespace with 0x20
	rng::replace_if(str, isspace, ' ');

        // squash
        char *end = &*str.end();
        char *ss = const_cast<char *>(str.c_str());
        while (ss != end) {
                ss = std::find(ss, end, ' ');
                if (ss == end || *++ss != ' ')
                        continue;

                char *se = std::find_if(ss, end, isgraph);
                std::memmove(ss, se, (end - se) + 1);
                end -= se - ss;
        }
        str.resize(end - str.c_str());
}
