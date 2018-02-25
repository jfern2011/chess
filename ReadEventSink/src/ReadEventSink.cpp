/**
 *  \file   ReadEventSink.cpp
 *  \author Jason Fernandez
 *  \date   10/15/2017
 *
 *  https://github.com/jfern2011/ReadEventSink
 */

#include "ReadEventSink.h"

/**
 * Default constructor
 */
ReadEventSink::ReadEventSink()
	: _buf(NULL),
	  _buf_len(0), _fd(-1), _in_use(0), _saved(), _sig()
{
	_is_init = false;
}

/**
 * Constructor
 *
 * @param[in] fd The file descriptor to read from
 */
ReadEventSink::ReadEventSink(int fd)
	: _buf(NULL),
	  _buf_len(0), _fd(fd), _in_use(0), _saved(), _sig()
{
	_is_init = (0 <= _fd);
}

/**
 * Copy constructor
 *
 * @param[in] other The ReadEventSink that we'll make a copy of
 */
ReadEventSink::ReadEventSink(const ReadEventSink& other)
	: _buf(NULL), _saved(), _sig()
{
	*this = other;
}

/**
 * Move constructor
 *
 * @param[in] other The ReadEventSink that we'll move into ours
 */
ReadEventSink::ReadEventSink(ReadEventSink&& other)
	: _buf(NULL), _saved(), _sig()
{
	*this = std::move(other);
}


/**
 * Destructor
 */
ReadEventSink::~ReadEventSink()
{
	if (_buf) std::free(_buf);
}

/**
 * Copy assignment operator
 *
 * @param [in] rhs The ReadEventSink to make a copy of
 *
 * @return *this
 */
ReadEventSink& ReadEventSink::operator=(const ReadEventSink& rhs)
{
	if (this != &rhs)
	{
		if (_buf) std::free(_buf);

		if (rhs._buf_len > 0)
			_buf = static_cast<char*>(std::malloc(rhs._buf_len));
		else
			_buf = NULL;

		std::memcpy(_buf, rhs._buf,
			rhs._buf_len);

		_buf_len = rhs._buf_len;
		_fd      = rhs._fd;
		_in_use  = rhs._in_use;
		_is_init = rhs._is_init;
		_saved   = rhs._saved;
		_sig     = rhs._sig;
	}

	return *this;
}

/**
 * Move assignment operator
 *
 * @param [in] rhs The ReadEventSink that we'll move into ours
 *
 * @return *this
 */
ReadEventSink& ReadEventSink::operator=( ReadEventSink&& rhs )
{
	if (this != &rhs)
	{
		if (_buf) std::free(_buf);

		_buf     = rhs._buf;
		_buf_len = rhs._buf_len;
		_fd      = rhs._fd;
		_in_use  = rhs._in_use;
		_is_init = rhs._is_init;
		_saved   =
			std::move(rhs._saved);
		_sig     =
			std::move( rhs._sig );

		rhs._buf     = NULL;
		rhs._buf_len =  0;
		rhs._fd      = -1;
		rhs._in_use  =  0;
		rhs._is_init =
			false;
	}

	return *this;
}

/**
 * Assign a new file descriptor to read from, which will be used
 * for future reads. If this is the same file descriptor
 * currently being used or if the new file descriptor is invalid,
 * nothing is done. Otherwise, all buffered data from previous
 * reads is discarded
 *
 * @param[in] fd The file descriptor to read from
 *
 * @return True on success
 */
bool ReadEventSink::assign_fd(int fd)
{
	if (fd < 0)
		return false;
	
	if (_fd == fd)
		return true;

	_fd = fd;

	_saved.clear();
	_in_use = 0;

	_is_init = true;
	return true;
}

/**
 * Attach a reader for processing input data. This function will be
 * invoked whenever data is read from the file descriptor
 *
 * @param[in] func Pointer to the static function that will process
 *                 the data collected from each read
 *
 * @return True on success
 */
bool ReadEventSink::attach_reader(bool(*func)(const char*,size_t))
{
	_sig.attach(func);
		return _sig.is_connected();
}

/**
 * Retrieve the data currently stored from previous reads
 *
 * @param[out] size The number of bytes of data available
 *
 * @return A pointer to the buffered data
 */
const char* ReadEventSink::get_data(int& size) const
{
	size = _in_use;
	return _buf;
}

/**
 * Get the current active file descriptor
 *
 * @return   The internal file descriptor
 */
int ReadEventSink::get_fd() const
{
	return _fd;
}

/**
 * Poll for new data. Note this isn't necessary before invoking any
 * of the read functions, which indicate whether or not any data
 * is indeed available for reading. See the read* documentation for
 * usage details
 *
 * @param[in] timeout Block for at most this many nanoseconds. When
 *                    negative, this blocks indefinitely
 *
 * @return One of the defined error codes
 */
err_code_t ReadEventSink::poll(int64 timeout) const
{
	if (!_is_init)
		return RES_USIG_ERR;

	struct pollfd pfd; pfd.events = POLLIN;
	pfd.fd = _fd;

	int retval = 0;

	if (0 <= timeout)
	{
		static const int NPS  = 1000000000;

		struct timespec ts;

		ts.tv_sec  = timeout / NPS;
		ts.tv_nsec = timeout % NPS;

		retval = ::ppoll(&pfd, 1, &ts,  0);
	}
	else
	{
		retval = ::ppoll(&pfd, 1, NULL, 0);
	}

	switch (retval)
	{
	case  0:
		return RES_NO_DATA ;
	case -1:
		return RES_READ_ERR;
	}

	AbortIf(pfd.revents != POLLIN,
		RES_READ_ERR);

	return RES_SUCCESS;
}

/**
 *  Attempt a read, forwarding the contents to the reader (provided
 *  one has been supplied)
 *
 * @param[in] delim A delimiter used to split read data. Each token
 *                  is then passed to the reader iteratively.
 *                  If the last byte string is not the delimiter,
 *                  then read() will assume the last write to
 *                  the file descriptor was incomplete and will not
 *                  pass the last token to the reader. Instead,
 *                  data remaining after the last delimiter will
 *                  be stored and appended to on the following read
 *                  request
 *
 *                  If no delimiter is provided, everything gets
 *                  read and passed to the reader
 *
 * @param[in] clear Flag indicates the internal read buffer should
 *                  be cleared prior to consuming new data.
 *                  If false, buffered data from previous reads is
 *                  forwarded to the reader
 *
 * @param[in] timeout Block for at most this many nanoseconds. When
 *                    negative, this blocks indefinitely
 *
 * @return One of the defined return codes
 */
err_code_t ReadEventSink::read(const std::string& delim, bool clear,
							   int64 timeout)
{
	err_code_t code = _read(clear, timeout);

	AbortIf(code != RES_SUCCESS && code != RES_NO_DATA,
		    code);
	if (code == RES_NO_DATA) return code;

	Util::str_v tokens;
	std::string data( _buf,_in_use );
	if (delim != "")
	{
		Util::split( data, tokens, delim );

		/*
		 * If the input did not end with a delimiter, then we have
		 * only partially read the last chunk of data. Save what
		 * we've read so far, and on the next read(), we'll append
		 * to it:
		 */
		if (!Util::ends_with(data, delim))
		{
			_saved = tokens.back(); tokens.pop_back();
		}
		else
			_saved.clear();
	}
	else
		tokens.push_back(data);

	bool success = true;
	if (_sig.is_connected())
	{
		for (size_t i = 0; i < tokens.size(); i++)
			success =
				success && _sig.raise(tokens[i].c_str(),
					                  tokens[i].size());
	}
	else
		return RES_USIG_ERR;

	return success ?
			RES_SUCCESS : RES_READER_ERR;
}

/**
 * Attempt a read, forwarding the contents to the reader (provided
 * one has been supplied)
 *
 * @param[in] nbytes Send data to the reader this many bytes at a
 *                   time. Any leftover bytes are saved and
 *                   prepended to data received on future calls
 *                   to read()
 * @param[in] clear  If true, the read buffer is cleared prior to
 *                   consumption
 *
 * @param[in] timeout Block for at most this many nanoseconds. If
 *                    negative, block indefinitely
 *
 * @return One of the defined return codes
 */
err_code_t ReadEventSink::read(size_t nbytes, bool clear,
							   int64 timeout)
{
	err_code_t code = _read(clear, timeout);

	AbortIf(code != RES_SUCCESS && code != RES_NO_DATA,
		    code);
	if (code == RES_NO_DATA) return code;

	std::string data(_buf, _in_use );
	Util::str_v tokens;
		Util::split(data, tokens, nbytes);

	/*
	 * If nbytes bytes does not fit evently into the total number
	 * of bytes available, save the remainder and we'll prepend
	 * those bytes to the data received on the next read() unless
	 * the clear flag is set:
	 */
	if (tokens.back().size() < nbytes)
	{
		_saved = tokens.back(); tokens.pop_back();
	}

	bool success = true;
	if (_sig.is_connected())
	{
		for (size_t i = 0; i < tokens.size(); i++)
			success =
				success && _sig.raise(tokens[i].c_str(),
					                  tokens[i].size());
	}
	else
		return RES_USIG_ERR;

	return success ?
			RES_SUCCESS : RES_READER_ERR;
}

/**
 *  Attempt a read, forwarding the contents to the reader (provided
 *  one has been supplied)
 *
 * @param[in] delim Process all data up to (but not including) this
 *                  byte signature. All data past that point will
 *                  be discarded. If the signature cannot be found,
 *                  everything read from the file descriptor is
 *                  saved and prepended to the data obtained from
 *                  the next read. This continues until the desired
 *                  byte signature is found, when everything
 *                  is pushed to the reader
 *
 * @param[in] clear If true, the read buffer is cleared prior to
 *                  consumption
 *
 * @param[in] timeout Block for at most this many nanoseconds. When
 *                    negative, this blocks indefinitely
 *
 * @return One of the defined return codes
 */
err_code_t ReadEventSink::read_until(const std::string& delim,
									 bool clear,
									 int64 timeout)
{
	err_code_t code = _read(clear, timeout);

	AbortIf(code != RES_SUCCESS && code != RES_NO_DATA,
		    code);
	if (code == RES_NO_DATA) return code;

	std::string data(_buf, _in_use );

	/*
	 * Search for the byte sequence, and if not found then save off
	 * what we've read so far:
	 */
	const size_t ind = data.find(delim);

	if (ind == std::string::npos)
	{
		_saved = std::move(data);
		return RES_SUCCESS;
	}
	else if ( _sig.is_connected() )
	{
		return
			_sig.raise(data.substr(0,ind).c_str(), ind)
				? RES_SUCCESS:RES_READER_ERR;
	}

	return RES_USIG_ERR;
}

/**
 * Attempt a read, forwarding the contents to the reader (provided
 * one has been supplied)
 *
 * @param[in] nbytes The number of bytes to read. Any remaining
 *                   bytes are discarded. If there are fewer
 *                   than \a nbytes available, they'll be saved
 *                   and added to on the following read
 *
 * @param[in] clear  If true, the read buffer is cleared prior to
 *                   consumption
 *
 * @param[in] timeout Block for at most this many nanoseconds. If
 *                    negative, block indefinitely
 *
 * @return One of the defined return codes
 */
err_code_t ReadEventSink::read_until(size_t nbytes, bool clear,
									 int64 timeout)
{
	err_code_t code = _read(clear, timeout);

	AbortIf(code != RES_SUCCESS && code != RES_NO_DATA,
		    code);
	if (code == RES_NO_DATA) return code;

	std::string data(_buf,_in_use);

	if (nbytes > _in_use)
	{
		_saved = std::move(data);
		return RES_SUCCESS;
	}
	else if ( _sig.is_connected() )
	{
		return
			_sig.raise(data.substr(0,nbytes).c_str(), nbytes)
				? RES_SUCCESS:RES_READER_ERR;
			
	}

	return RES_USIG_ERR;
}

/**
 * Reads from the file descriptor. On success, returns SUCCESS or
 * or NO_DATA
 *
 * The buffer is resized as necessary to accomodate the new data,
 * plus any data saved from previous reads. 
 *
 * @param [in] clear  Flag that specifies whether or not to clear
 *                    the buffer before reading
 *
 * @param[in] timeout Block for at most this many nanoseconds. If
 *                    negative, block indefinitely
 *
 * @return One of the defined return codes
 */
err_code_t ReadEventSink::_read(bool clear, int64 timeout)
{
	AbortIfNot(_is_init,RES_USIG_ERR);

	err_code_t code = poll( timeout );
	if (code != RES_SUCCESS)
		return code;

	int fsize;
	::ioctl(_fd, FIONREAD, &fsize );

	if (clear)
		_saved.clear();

	/*
	 * The maximum number of bytes we need to read into the buffer
	 */
	size_t new_size = fsize + _saved.size();

	if (new_size > _buf_len)
	{
		char* tmp = (char*)std::realloc(_buf, new_size);
		AbortIf(!tmp, RES_MEMORY_ERR);
		_buf=tmp; _buf_len = new_size;
	}

	/*
	 * Update the number of bytes available to send
	 */
	_in_use = _saved.size() + fsize;

	/*
	 * If there was any data left over from the last read, move it
	 * to the front of the buffer:
	 */
	if (_saved.size())
		std::memcpy(_buf, _saved.c_str(), _saved.size());

	/*
	 * Read raw data:
	 */
	AbortIf(::read(_fd, _buf + _saved.size(),
		           fsize) != fsize,
		RES_READ_ERR);

	_saved.clear();

#if defined(CONSOLE_TEST)
	// Discard newline character when reading
	// from standard input:
	if (_buf[_in_use-1] == '\n')
		_in_use--;
#endif

	return RES_SUCCESS;
}
