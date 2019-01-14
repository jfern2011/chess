/**
 *  \file   UdpConnection.h
 *  \author Jason Fernandez
 *  \date   4/11/2018
 *
 *  https://github.com/jfern2011/io_tools
 */

#ifndef __UDP_CONNECTION_H__
#define __UDP_CONNECTION_H__

#include <cstdint>
#include <string>

#include "net.h"
#include "DataBuffer.h"
#include "ConstDataBuffer.h"
#include "Fd.h"

namespace net
{
	using uint16 = std::uint16_t;

	/**
	 * Manages an IPv4 UDP connection. The underlying socket is
	 * said to be connected when it establishes a dedicated
	 * connection to a remote node via \ref connect(). In this
	 * case, all correspondence will occur exclusively with
	 * the remote node. An unconnected socket on the other hand
	 * cannot send, but can still receive messages
	 */
	class UdpConnection
	{

	public:

		UdpConnection();

		~UdpConnection();

		bool bind(uint16 port, const std::string& name="");

		bool connect(uint16 port, const std::string& host);

		bool recv(DataBuffer& buf, int timeout = -1, bool conn = false);

		int send(const DataBuffer& buf, int timeout = -1) const;

		int send(const ConstDataBuffer& buf, int timeout = -1) const;

	private:

		bool _handle_input();

		bool _init_sockaddr(uint16 port, const std::string& name,
			struct sockaddr_in& addr) const;

		/**
		 * The DataBuffer emitted on each \ref
		 * recv()
		 */
		DataBuffer _data;

		/**
		 * The file descriptor to send and
		 * receive on
		 */
		Fd _fd;

		/**
		 * True if this is a connected socket
		 */
		bool _is_connected;

		/**
		 * True if ::socket() succeeded
		 */
		bool _is_init;

		/**
		 * The raw data read from the socket
		 */
		char* _raw;

		/**
		 * The size of the raw buffer
		 */
		size_t _size;

		/**
		 * The remote node address; applies to
		 * connected sockets only
		 */
		struct sockaddr_in
			_remote_addr;
	};
}

#endif
