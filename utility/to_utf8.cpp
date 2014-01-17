#include <fstream>
#include <string>
#include <iterator>
#include <boost/thread.hpp>
#include "grammar/UTF8.h"
#include "utility/to_utf8.hpp"

namespace utility {
  std::string to_utf8( const std::u32string &str ) {
    common::detail::karma::utf8< std::back_insert_iterator< std::string > > rule;
    namespace karma = boost::spirit::karma;
    std::string result;
    if( karma::generate( std::back_inserter( result ), *rule, str ) ) {
      return result;
    }
    else
      throw invalid_ucs4_character();
  }
}
