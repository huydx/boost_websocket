#include <fstream>
#include <iterator>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/thread.hpp>
#include "grammar/JSON__.h"
#include "grammar/UTF8.h"
#include "message_server/grammar/uri.hpp"
#include "system/config.hpp"
#include "system/sequence_builder.hpp"
#include "utility/to_utf8.hpp"

#include "network/connect.hpp"
#include "network/done.hpp"

const common::json &get_child_node( const common::json &node, const std::u32string &name ) {
  if( node.which() != common::json_struct )
    throw node_not_found();
  const auto &node_ = boost::get<
    boost::mpl::at_c< common::json::types, common::json_struct >::type
  >( node );
  const auto search_result = node_.find( name );
  if( search_result == node_.end() )
    throw node_not_found();
  return search_result->second;
}
const common::json &get_child_node( const common::json &node, size_t index ) {
  if( node.which() != common::json_array )
    throw node_not_found();
  const auto &node_ = boost::get<
    boost::mpl::at_c< common::json::types, common::json_array >::type
  >( node );
  if( node_.size() <= index )
    throw node_not_found();
  return node_[ index ];
}

config::config( const std::string filename ) {
  const common::json json = load_json( filename );
  const auto server_location = get_child_node_in_type< common::json_string >( json, U"接続先" );
  server_addr = message_server::load_uri( utility::to_utf8( server_location ) );
  if( !server_addr.authority )
    throw invalid_uri();
  else if( !server_addr.authority->port )
    throw invalid_uri();
  else if( server_addr.scheme != "mrs" )
    throw invalid_uri();
  const auto client_location = get_child_node_in_type< common::json_string >( json, U"自己紹介" );
  client_addr = message_server::load_uri( utility::to_utf8( client_location ) );
  if( !client_addr.authority )
    throw invalid_uri();
  else if( !client_addr.authority->port )
    throw invalid_uri();
  else if( client_addr.scheme != "gmsv" )
    throw invalid_uri();
  const auto clients_ = get_child_node( json, U"利用可能なクライアント" );
  if( clients_.which() != common::json_array )
    throw node_not_found();
  const auto &clients_in_array = boost::get<
    boost::mpl::at_c< common::json::types, common::json_array >::type
  >( clients_ );
  BOOST_FOREACH( const auto elem, clients_in_array )
    clients.push_back( boost::get<
      boost::mpl::at_c< common::json::types, common::json_number >::type
    >( elem ) );
  server_number = get_child_node_in_type< common::json_number >( json, U"サーバ番号" );
  world_number = get_child_node_in_type< common::json_number >( json, U"ワールド番号" );
  tell_category_id = get_child_node_in_type< common::json_number >( json, U"ダイレクトチャットのカテゴリID" );
  send_chunk_size = get_child_node_in_type< common::json_number >( json, U"送信ウィンドウサイズ" );
  const auto script = get_child_node_in_type< common::json_string >( json, U"スクリプト" );
  sequence = sequence_builder::run_script( utility::to_utf8( script ) );
  sequence.push_back( &done );
  current_task = sequence.begin();
}

common::json config::load_json( const std::string filename ) {
  std::ifstream in( filename );
  if( !in.is_open() )
    throw unable_to_open_config_file();
  typedef std::istreambuf_iterator<char> base_iterator;
  typedef boost::spirit::multi_pass< base_iterator > multi_pass_iterator;
  multi_pass_iterator begin =
    boost::spirit::make_default_multi_pass( base_iterator( in ) );
  multi_pass_iterator cur = begin;
  multi_pass_iterator end =
    boost::spirit::make_default_multi_pass( base_iterator() );
  namespace qi = boost::spirit::qi;
  common::detail::qi::json< multi_pass_iterator > rule;
  common::json result;
  if( qi::parse( cur, end, rule, result ) )
    return result;
  else {
    std::cerr << "error at " << std::distance( begin, cur );
    std::cerr << " : " << std::string( begin, cur ) << "<here>" << std::string( cur, end ) << std::endl;
    throw invalid_config_file();
  }
}

const boost::function< void() > &config::get_next_task() {
  boost::mutex::scoped_lock lock( guard );
  const auto &task = *current_task;
  if( std::distance( current_task, static_cast< sequence_builder::task_queue_t::const_iterator >( sequence.end() ) ) != 1u )
    ++current_task;
  return task;
}

