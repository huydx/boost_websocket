#ifndef LOGRES_TOOL_MESSAGE_CLIENT_SYSTEM_CONFIG_HPP
#define LOGRES_TOOL_MESSAGE_CLIENT_SYSTEM_CONFIG_HPP

#include <vector>
#include <string>
#include <boost/function.hpp>
#include <boost/mpl/at.hpp>
#include "JSON_.h"
#include "message_server/container/uri.hpp"
#include "system/sequence_builder.hpp"

struct node_not_found {};
struct unable_to_open_config_file {};
struct invalid_config_file {};
struct invalid_uri {};
const common::json &get_child_node( const common::json &node, const std::u32string &name );
const common::json &get_child_node( const common::json &node, size_t index );
template< common::json_element_type type >
const typename boost::mpl::at_c< common::json::types, type >::type &
get_child_node_in_type( const common::json &node, const std::u32string &name ) {
  const auto &child = get_child_node( node, name );
  if( child.which() != type )
    throw node_not_found();
  return boost::get<
    typename boost::mpl::at_c< common::json::types, type >::type
  >( child );
}
template< common::json_element_type type >
const typename boost::mpl::at_c< common::json::types, type >::type &
get_child_node_in_type( const common::json &node, size_t index ) {
  const auto &child = get_child_node( node, index );
  if( child.which() != type )
    throw node_not_found();
  return boost::get<
    typename boost::mpl::at_c< common::json::types, type >::type
  >( child );
}

class config {
public:
  config( const std::string filename );
  const message_server::uri &get_server_addr() const { return server_addr; }
  const message_server::uri &get_client_addr() const { return client_addr; }
  const std::vector< unsigned int > &get_available_clients() const { return clients; }
  unsigned int get_server_number() const { return server_number; }
  unsigned int get_world_number() const { return world_number; }
  unsigned int get_tell_category_id() const { return tell_category_id; }
  const boost::function< void() > &get_next_task();
  size_t get_send_chunk_size() const { return send_chunk_size; }
private:
  static common::json load_json( const std::string filename );
  message_server::uri server_addr;
  message_server::uri client_addr;
  std::vector< unsigned int > clients;
  unsigned int server_number;
  unsigned int world_number;
  unsigned int tell_category_id;
  sequence_builder::task_queue_t sequence;
  sequence_builder::task_queue_t::const_iterator current_task;
  boost::mutex guard;
  size_t send_chunk_size;
};

#endif

