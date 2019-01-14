/**
 *  \file   UdpConnection.cpp
 *  \author Jason Fernandez
 *  \date   4/11/2018
 *
 *  https://github.com/jfern2011/io_tools
 */

#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "abort.h"
#include "UdpConnection.h"

namespace net
{
	/**
	 * Constructor
	 */
	UdpConnection::UdpConnection()
		: _data(nullptr, 0), _fd(::socket(AF_INET, SOCK_DGRAM, 0)),
		  _is_connected(false),
		  _is_init(_fd),
		  _raw(nullptr),
		  _size(0)
	{
	}

	/**
	 * Destructor
	 */
	UdpConnection::~UdpConnection()
	{
		if (_raw) delete[] _raw;
	}

	/**
	 * Assign the port on which to listen for messages
	 *
	 * @param[in] port The port number
	 * @param[in] name A network interface name, e.g. "localhost", or
	 *                 an IP address, e.g. "127.0.0.1". This is
	 *                 optional; the default behavior is to bind() to
	 *                 all available interfaces
	 *
	 * @return True on success
	 */
	bool UdpConnection::bind(uint16 port, const std::string& name)
	{
		AbortIfNot(_is_init, false);

		struct sockaddr_in addr;
		AbortIfNot(_init_sockaddr(port, name, addr), false);

		AbortIf(::bind(_fd.get(), to_sockaddr(&addr), sizeof(addr)),
			false);

		return true;
	}

	/**
	 * Connect to a remote host
	 *
	 * @param[in] port The remote port number
	 * @param[in] host Connect to this host. This can be either an IP
	 *                 address or a host name
	 *
	 * @return True on success
	 */
	bool UdpConnection::connect(uint16 port, const std::string& host)
	{ 
		AbortIfNot(_is_init, false);

		AbortIfNot(_init_sockaddr(port, host, _remote_addr),
			false);

		socklen_t len = sizeof(_remote_addr);

		AbortIf(::connect(_fd.get(), to_sockaddr(&_remote_addr), len),
			false);

		_is_connected = true;
		return true;
	}

	/**
	 * Receive data from a remote node
	 *
	 * @param[out] buf     Place remote data here
	 * @param[in]  timeout The timeout (in milliseconds) after which
	 *                     this call will return, even if no data is
	 *                     available for reading. Specifying -1 may
	 *                     block indefinitely
	 * @param[in]  conn    If true, connect to the node that sent the
	 *                     data. Future calls to \ref send() will
	 *                     send to this node
	 *
	 * @return True on success
	 */
	bool UdpConnection::recv(DataBuffer& buf, int timeout, bool conn)
	{
		AbortIfNot(_is_init, false);

		struct sockaddr_in s_addr;
			socklen_t addrlen = sizeof(_remote_addr);

		if (!_fd.can_read(timeout))
		{
			buf.clear(); return true;
		}

		AbortIfNot(_handle_input(), false);

		int nbytes = ::recvfrom(_fd.get(), _data.get(), _data.size(),
			0, to_sockaddr(&s_addr), &addrlen);

		AbortIf(nbytes < 0, false);
		buf = _data;

		if (conn)
		{
			std::memcpy(&_remote_addr, &s_addr,
				addrlen);
			AbortIf( ::connect(_fd.get(), to_sockaddr(&_remote_addr),
				addrlen) < 0, false);

			_is_connected = true;
		}

		return true;
	}

	/**
	 * Send data to a remote node
	 *
	 * @param[in] buf     The buffer containing the data to send
	 * @param[in] timeout The maximum number of milliseconds to wait
	 *                    for space to become available for writing;
	 *                    specifying -1 may block indefinitely
	 *
	 * @return The number of bytes written, or -1 on error
	 */
	int UdpConnection::send(const DataBuffer& buf, int timeout) const
	{
		ConstDataBuffer const_buf(buf.get(), buf.size());
		int nbytes = send(const_buf, timeout);

		AbortIf(nbytes < 0, -1);
		return nbytes;
	}

	/**
	 * Send data to a remote node
	 *
	 * @param[in] buf     The buffer containing the data to send
	 * @param[in] timeout The maximum number of milliseconds to wait
	 *                    for space to become available for writing;
	 *                    specifying -1 may block indefinitely
	 *
	 * @return The number of bytes written, or -1 on error
	 */
	int UdpConnection::send(const ConstDataBuffer& buf, int timeout) const
	{
		AbortIfNot(_is_init, -1);
		AbortIfNot(buf, -1);
		AbortIfNot(_is_connected, -1,
				"send() is only allowed on connected sockets.");

		if (!_fd.can_write(timeout))
			return 0;

		int nbytes  = ::write(_fd.get(), buf.get(), buf.size());
		AbortIf(nbytes < 0, -1);

		return nbytes;
	}

	/**
	 * Handle an input message from a remote node. This preps
	 * the data buffer for reading
	 *
	 * @return True on success
	 */
	bool UdpConnection::_handle_input()
	{
		int fsize;
		AbortIf(::ioctl(_fd.get(), FIONREAD, &fsize) < 0, false);

		if ((size_t)fsize > _size)
		{
			if (_raw) delete[] _raw;

			_raw = new char[fsize];
			AbortIfNot(_raw, false, "requested %d bytes", fsize);
			_size = fsize;
		}

		_data.reset(_raw, fsize);

		return true;
	}

	/**
	 * Initialize a sockaddr_in struct
	 *
	 * @param[in]  port The port
	 * @param[in]  name A host name or IP address
	 * @oaram[out] addr The initialized struct
	 *
	 * @return True on success
	 */
	bool UdpConnection::_init_sockaddr(uint16 port,
		const std::string& name, struct sockaddr_in& addr) const
	{
		std::memset(reinterpret_cast<char*>(&addr), 0,
			sizeof(addr));

		addr.sin_family = AF_INET;
		addr.sin_port   = ::htons(port);

		if (name.empty())
		{
			addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
		}
		else
		{
			struct hostent *he = ::gethostbyname(name.c_str());
			AbortIfNot(he, false);

			std::memcpy(&addr.sin_addr, he->h_addr_list[0],
				he->h_length);
		}

		return true;
	}
}
