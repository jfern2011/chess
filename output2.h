#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "log.h"

/**
 * @class OutputWriter
 *
 * Sends messages from the chess engine to the GUI. Classes wishing
 * to write to the GUI inherit from this class so that all output
 * is done through here. This will also copy the logger on outgoing
 * messages
 */
class OutputWriter
{
	
public:

	OutputWriter(const std::string& name, Logger& logger);

	~OutputWriter();

	bool write(const char* format, ...) const;

private:

	/**
	 * Writes to the chess engine log file
	 */
	Logger& _logger;

protected:

	/**
	 * The name of this class (for logging
	 * purposes)
	 */
	const std::string
		_name;
};

#endif
