#include <string>

namespace utility {
  struct invalid_ucs4_character {};
  std::string to_utf8( const std::u32string& );
}

