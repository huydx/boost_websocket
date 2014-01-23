require 'sinatra'

set :public_folder, File.dirname(__FILE__) + 'static'
set :haml, format: :html5
set :bind, '0.0.0.0'

get '/' do
  haml :index 
end

post '/chat' do
  msg =  params['chat']
  haml :index
end
