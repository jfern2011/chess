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
     * Allows individual components (e.g. the search algorithm) to write
     * to one or more shared stream objects
     */
    class OutputSource final
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
         * Name of the component handling this \ref
         * OutputSource
         */
        const std::string _name;

        /**
         * The underlying stream object to write to
         */
        Handle<std::ostream>
            _stream;
    };
}

#endif
