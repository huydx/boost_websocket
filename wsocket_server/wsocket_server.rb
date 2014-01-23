require 'em-websocket'
require 'socket'
require 'debugger'

class TCPSocket
  def open
    begin
      super
    rescue Exception=>e
      STDERR.puts caller.join(", ")
    end
  end
end

begin
  socket = TCPSocket.open("0.0.0.0", 12345)
rescue Exception=>e
end

EM.run {
  EM::WebSocket.run(:host => "0.0.0.0", :port => 1234) do |ws|
    ws.onopen { |handshake|
      puts "WebSocket connection open"
      ws.send "Hello Client, you connected to #{handshake.path}"
    }

    ws.onclose { puts "Connection closed" }

    ws.onmessage { |msg|
      puts "Recieved message: #{msg}"
      ws.send "Pong: #{msg}"
      socket.write(msg)
    }
  end
}
