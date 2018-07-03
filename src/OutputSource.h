#ifndef __OUTPUT_SOURCE_H__
#define __OUTPUT_SOURCE_H__

#include <ostream>
#include <string>

#include "chess4.h"

namespace Chess
{
	/**
	 * @class OutputSource
	 *
	 * Individual components (e.g. the search algorithm) act as output
	 * sources to a single stream object
	 */
	class OutputSource
	{

	public:

		OutputSource(const std::string& name,
			Handle<std::ostream> stream);

		~OutputSource();

		std::string get_name() const;

		bool write(const char* format, ...)
			const;

	private:

		/**
		 *  Identifies which module is writing to
		 *  the the stream
		 */
		const std::string _name;

		/**
		 * The stream object to write messages to
		 */
		Handle<std::ostream>
			_stream;
	};
}

#endif
