#include "UdpConnection.h"
#include "abort.h"

#include <cstring>
#include <iostream>
#include <string>

bool run()
{
	net::UdpConnection conn1, conn2;
	AbortIfNot(conn1.bind(12345, "localhost"), false);
	AbortIfNot(conn2.connect(12345, "localhost"), false);

	std::string msg;
	char remote_input[1024];

	net::DataBuffer send_buf;
	net::DataBuffer recv_buf;

	char output[1024];

	while (true)
	{
		std::getline(std::cin, msg);

		if (msg == "quit") break;

		std::memset(output, 0, 1024);
		std::memcpy(output, msg.c_str(), msg.size());

		send_buf.reset(output, msg.size());

		int sent = conn2.send(send_buf);
		AbortIf(sent < 0, false);

		std::printf("sent     '%s' (%lu bytes)\n",
			send_buf.get(), send_buf.size());

		AbortIfNot(conn1.recv(recv_buf, 2000), false);
		int recvd = recv_buf.size();

		if (recvd == 0)
			std::printf("nothing received.\n");
		else
		{
			std::memset(remote_input, 0, 1024);
			std::memcpy(remote_input, recv_buf.get(), recv_buf.size());

			std::printf("received '%s' (%d bytes)\n",
				remote_input, recvd);
		}

		std::fflush(stdout);
	}

	return true;
}

int main()
{
	run();
	return 0;
}
