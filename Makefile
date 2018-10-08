producent_consument: main.cpp remote_server.cpp monitor.cpp sender.cpp receiver.cpp conditional.cpp message.cpp token.cpp push_socket.cpp pull_socket.cpp
	clang++ -o producent_consument -std=c++1z -lzmq -lThorSerialize17 -Wall -Wextra -pedantic main.cpp remote_server.cpp monitor.cpp sender.cpp receiver.cpp conditional.cpp message.cpp token.cpp push_socket.cpp pull_socket.cpp 
