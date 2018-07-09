#include <cstdarg>

#include "OutputSource.h"
#include "util/abort/abort.h"
#include "util/Buffer.h"

namespace Chess
{
	/**
	 * Constructor
	 *
	 * @param[in] name   The name of the module writing to \a stream
	 * @param[in] stream The stream object to write to
	 */
	OutputSource::OutputSource(
		const std::string& name, Handle<std::ostream> stream)
		: _name(name), _stream(stream)
	{
	}

	/**
	 * Destructor
	 */
	OutputSource::~OutputSource()
	{
	}

	/**
	 * Get the name of the module writing to the stream
	 *
	 * @return The name of the module
	 */
	std::string OutputSource::get_name() const
	{
		return _name;
	}

	/**
	 * Attempt to write to the stream
	 *
	 * @param[in] format  This follows the style for std::printf()
	 *
	 * @return True on success
	 */
	bool OutputSource::write(const char* format, ...) const
	{
		const int buf_size = 1024;

		Buffer<char,buf_size> buf;

		std::string format_s = _name + ": " + format;

		va_list args;

		va_start(args, format);

		int nchars =
			std::vsnprintf(buf, buf_size, format_s.c_str(),
				args);

		AbortIf(nchars < 0, false);

		va_end(args);

		AbortIfNot(_stream, false);

			_stream->write(buf, nchars);
		
		_stream->flush();
		return (true);
	}
}
