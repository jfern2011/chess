# ReadEventSink
Associate file descriptor read events with handlers

## Overview

The purpose of this API is to abstract the handling of file descriptor
reads events by providing an interface that implements various ways of
reading from the file descriptor. You can read a specified number of
bytes at a time, read everything at once, perform a delimited read,
and so on. Anything that hasn't been read can optionally be saved off
for later.

Data read from the file descriptor is handled via a user-defined
function or class method. These must return a boolean equal to true to
indicate success, and take in a const char* and size_t to receive data
from the file descriptor and to get the number of bytes read,
respectively.

## Usage

The required two files are ReadEventSink.cpp and ReadEventSink.h. The
makefile I supplied is really only intended for compiling the unit
test, which you can use to get a feel for how the API works. In
addition, ReadEventSink.h requires abort.h, Signal.h, types.h, and
util.h, all which you'll need to pull from my other repos (sorry).
These are also listed in the Makefile.

## Examples

	#include <cstdio>
	#include "ReadEventSink.h"
     
    /*
     * The magnificent reader you wrote that will process data read
     * from the file descriptor
     */
	bool reader(const char* buf, size_t size)
	{
		std::printf("Received %s (%lu) bytes\n", buf, size);
		std::fflush(stdout);
		return true;
	}
     
	int main()
	{
		ReadEventSink res;
        
		/*
		 * Assign the file descriptor to read from. In this example,
		 * we'll read from stdin:
		 */
		res.assign_fd(0);
        
		/*
		 * Assign a reader to do something with the stuff we read:
		 */
		res.attach_reader(&reader);
        
		/*
		 * This is the delimiter, i.e. we read from the file descriptor
		 * until we hit this string. At that point, we invoke your
		 * reader() on the data. Then we continue reading until we hit
		 * the delimiter again, and forward the next set of data to
		 * reader(), and so on until there's no more data left to read.
		 *
		 * Note that by design, if the last chunk of data does not end
		 * with the delimiter, it will be stored until the next time
		 * the delimiter is read from the file descriptor. So, if we
		 * read the following from stdin:
		 *
		 * hey.there.bud
		 *
		 * Then reader() will process 'hey' and 'there' but not 'bud'
		 * because a trailing delimiter is missing. So, 'bud' is
		 * stored for now. If we next read:
		 *
		 * dy.
		 *
		 * Then reader() will process 'buddy'. Note that the delimiter
		 * is optional, and the default behavior is to read everything
		 * at once, i.e. 'hey.there.bud' will be passed to reader()
		 * in the default case, and 'dy.' will be passed by itself when
		 * it is written to stdin
		 */
		const std::string delimiter = "."; // can be any string
        
		/*
		 * This flag indicates whether or not to store data when we're
		 * performing a delimited read. As described above, we normally
		 * continue to buffer data from consecutive reads until we
		 * hit the delimiter, but in this case we clear the internal
		 * buffer on each read. Since 'bud' would normally be saved, it
		 * will be discarded
		 */
		const bool clear = true;
        
		/*
		 * The timeout specifies how long the read should block before
		 * returning if there's nothing in stdin, in nanoseconds
		 */
		const int64 timeout = 1000000000; // 1 second
        
		while (true)
		{
			if (!res.read(delimiter,
				          clear,
			              timeout))
			return -1;
		}
        
		return 0;
	}

This example goes through a use case of read() with three arguments.
These are all optional; in particular, read() defaults to
	
	read("", false, 0);

which does not delimit the data (i.e. everything is passed to the
reader at once), data is never stored from partial reads, and
the function returns immediately (no timeout).

An overloaded version of read() takes the number of bytes to read
instead of a delimiter, e.g.

	read(10, false, 0);

will read groups of 10 bytes at time, each group being passed to the
reader separately. Note that the first input is required in this case.

You can also choose to read up to a certain point. For example:

	read_until("..", false, 0);

As soon as '..' is read from the file descriptor, everything after that
is discarded and not passed to the reader. If we read from the file
descriptor and don't run into '..' then we'll buffer the data until the
next call to read_until() that picks up the delimiter, or until some
other type of read is performed.

Finally, read_until() can be called with an integral 1st argument
instead of a delimiter. In the following example, we won't pass
anything to the reader until we've read 100 bytes from the file
descriptor:

	read_until(100, false, 0);

Note that if instead we had

	read_until(100, true, 0);

Then we require that a single read picks up 100 bytes, because in this
case we're not buffering previous reads.

read_until() always requires the first argument. The default behavior
of read_until(100) is

	read_until(100, false, 0);

## Notes

1. Delimiters are never passed to your reader, just what's in between
   them.

## Contact

Feel free to email me with bug reports or suggestions:
jfernandez3@gatech.edu
