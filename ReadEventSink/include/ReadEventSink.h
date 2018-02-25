/**
 *  \file   ReadEventSink.h
 *  \author Jason Fernandez
 *  \date   10/15/2017
 *
 *  https://github.com/jfern2011/ReadEventSink
 */

#ifndef __READ_EVT_SINK_H__
#define __READ_EVT_SINK_H__

#include <cstdlib>
#include <cstring>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "Signal.h"
#include "util.h"

/**
 **********************************************************************
 *
 * @class ReadEventSink
 *
 * Associate file descriptor read events with a handler. When there is
 * data available for reading on a file descriptor, a read request
 * will either move the data into an internal buffer for later use or
 * immediately forward it to a user-defined handler (provided one
 * has been specified) for processing, depending on the type of read()
 * requested
 *
 **********************************************************************
 */
class ReadEventSink
{
	typedef Signal::Signal<bool,const char*,size_t> read_sig_t;

public:

	/**
	 * Return codes used for read requests:
	 */
	typedef enum
	{
		/** New data was read and processed successfully */
		RES_SUCCESS    = 0,

		/** No data available for reading on the
		    file descriptor */
		RES_NO_DATA    = 1,

		/** An error occured while attempting to read
		    from the file descriptor */
		RES_READ_ERR   = 2,

		/** Memory allocation error  */
		RES_MEMORY_ERR = 4,

		/** The reader returned "false" while processing
		    the data */
		RES_READER_ERR = 8,

		/** Either a reader was not properly attached or
		    the file descriptor is bad */
		RES_USIG_ERR   = 16

	} err_code_t;

	ReadEventSink();

	ReadEventSink(const ReadEventSink& other);

	ReadEventSink(ReadEventSink&& other);

	ReadEventSink(int fd);

	~ReadEventSink();

	ReadEventSink& operator=( const ReadEventSink& rhs );

	ReadEventSink& operator=(ReadEventSink&& rhs);

	bool assign_fd(int fd);

	bool attach_reader(bool(*func)(const char*, size_t));

	/**
	 * Attach a reader for processing input data. This routine will be
	 * invoked whenever data is read from the file descriptor
	 *
	 * @tparam C The class that implements the reader
	 *
	 * @param[in] obj  The object (of class C) through which to invoke
	 *                 the reader
	 * @param[in] func The class method that will process the data
	 *                 collected from each read
	 *
	 * @return True on success
	 */
	template <typename C>
	bool attach_reader(C& obj, bool(C::*func)(const char*,size_t))
	{
		_sig.attach<C>(obj, func);
			return _sig.is_connected();
	}

	/**
	 * Attach a reader for processing input data. This routine will be
	 * invoked whenever data is read from the file descriptor
	 *
	 * @tparam C The class that implements the reader
	 *
	 * @param[in] obj  The object (of class C) through which to invoke
	 *                 the reader
	 * @param[in] func A *const* class method that will process the
	 *                 data collected from each read
	 *
	 * @return True on success
	 */
	template <typename C>
	bool attach_reader(C& obj, bool(C::*func)(const char*,size_t) const)
	{
		_sig.attach<C>(obj, func);
			return _sig.is_connected();
	}

	const char* get_data(int& size) const;

	int get_fd() const;

	err_code_t poll(int64 timeout= 0) const;

	err_code_t read(const std::string& delim = "",
					bool clear = false,
					int64 timeout = 0);
	
	err_code_t read(size_t nbytes,
					bool clear = false,
					int64 timeout = 0);

	err_code_t read_until(const std::string& delim,
						  bool clear = false,
						  int64 timeout = 0);

	err_code_t read_until(size_t nbytes,
						  bool clear = false,
						  int64 timeout = 0);

private:

	err_code_t _read(bool clear, int64 timeout);

	char*       _buf;
	size_t      _buf_len;
	int         _fd;
	size_t      _in_use;
	bool        _is_init;
	std::string _saved;
	read_sig_t  _sig;
};

/**
 * Simplified return code type for convenience
 */
typedef ReadEventSink::err_code_t err_code_t;

#endif
