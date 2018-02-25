/**
 *  \file   util.h
 *  \author Jason Fernandez
 *  \date   10/18/2017
 *
 *  https://github.com/jfern2011/util
 */

#ifndef __UTIL__
#define __UTIL__

#include <cstdlib>
#include <fstream>

#include <sys/stat.h>

#include <string>
#include <vector>

#include "abort.h"
#include "types.h"

namespace Util
{
	/**
	 * A traits class that determines whether the given type is
	 * a C++ boolean
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_bool
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template<>
	struct is_bool<bool>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * a C++ char
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_char
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template <>
	struct is_char<char>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * a signed 16-bit integer
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_int16
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template <>
	struct is_int16<short>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * a signed 32-bit integer
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_int32
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template <>
	struct is_int32<int>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * a signed 64-bit integer
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_int64
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template <>
	struct is_int64<long long>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * a C++ unsigned char
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_uchar
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template <>
	struct is_uchar<unsigned char>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * an unsigned 16-bit integer
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_uint16
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template <>
	struct is_uint16<unsigned short>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * an unsigned 32-bit integer
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_uint32
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template <>
	struct is_uint32<unsigned int>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * an unsigned 64-bit integer
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_uint64
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template <>
	struct is_uint64<unsigned long long>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * a C++ float
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_float
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template<>
	struct is_float<float>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * a C++ double
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_double
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template<>
	struct is_double<double>
	{
	    static const bool value = true;
	};

	/**
	 * A traits class that determines whether the given type is
	 * a std::string
	 *
	 * @tparam The data type
	 */
	template <typename T>
	struct is_string
	{
		/**
		 * True or false
		 */
	    static const bool value = false;
	};

	template<>
	struct is_string<std::string>
	{
	    static const bool value = true;
	};

	/**
	 * A vector of strings
	 */
	typedef std::vector<std::string> str_v;

	/**
	 **********************************************************************
	 *
	 * Count the number of bits set in a word in O(n) time
	 *
	 * @param [in] word An n-bit word
	 *
	 * @return The number of bits set in the word
	 *
	 **********************************************************************
	 */
	template<typename T> inline int bitCount(T word)
	{
		int count = 0;

		for (count = 0; word; count++)
			word &= word - 1;

		return count;
	}

	/**
	 **********************************************************************
	 *
	 *  Combine a vector of std::strings into a single string. This is
	 *  nearly the opposite of split()
	 *
	 * @param[in] tokens The strings to combine
	 * @param[in] sep    An optional separator that will get placed in
	 *                   between tokens
	 *
	 * @return The combined string
	 *
	 **********************************************************************
	 */
	inline std::string build_string(const str_v& tokens,
					const std::string& sep = "")
	{
		std::string out = "";

		for (size_t i = 0; i < tokens.size(); i++)
		{
			out += tokens[i] + sep;
		}

		if (sep != "")
		{
			return
				out.substr(0,out.size() - sep.size());
		}
		else
			return out;
	}

	/**
	 **********************************************************************
	 *
	 * Clear the specified bit within a word
	 *
	 * @param [in]     bit  The bit to clear
	 * @param [in,out] word An n-bit word
	 *
	 **********************************************************************
	 */
	template<typename T> inline void clearBit(int bit, T& word)
	{
		word ^= (word & ((T)1 << bit));
	}

	/**
	 **********************************************************************
	 *
	 * Clear the specified bits of a word
	 *
	 * @param [in]     mask Specifies the bits within \a word to clear
	 * @param [in,out] word An n-bit word
	 *
	 **********************************************************************
	 */
	template<typename T> inline void clearBits(T mask, T& word)
	{
		word ^= (word & mask);
	}

	/**
     **********************************************************************
     *
     * Determine if a file or directory exists
     *
     * @param [in] dir The full (absolute) path to the file or directory
     *
     * @return True if it exists
     *
     **********************************************************************
     */
    inline bool dirExists(const std::string& dir)
    {
        struct stat info;

        return(!stat(dir.c_str(), &info));
    }

	/**
	 **********************************************************************
	 *
	 * Check if a string ends with another string
	 *
	 * @param [in] str    String to scan
	 * @param [in] suffix The ending to search for
	 *
	 **********************************************************************
	 */
	inline bool ends_with(const std::string& str,const std::string& suffix)
	{
		size_t start = str.size() - suffix.size();
		return str.size() >= suffix.size() &&
				str.compare(start, suffix.size(), suffix) == 0;
	}

	/**
     **********************************************************************
     *
     * Get the size of a regular file. See stat(2) man page for details
     *
     * @param [in] dir The full (absolute) path to the file
     *
     * @return The file size in bytes or -1 on error, including if this
     *         is a dirctory
     *
     **********************************************************************
     */
	inline int64 file_size(const std::string& dir)
	{
		struct stat st;
		AbortIf(stat(dir.c_str(), &st) == -1 || S_ISDIR(st.st_mode), -1);

		return
			st.st_size;
	}

	/*
	 * Forward declarations:
	 */
	std::string to_lower(const std::string& str);
	std::string trim(const std::string& str);

	/**
	 **********************************************************************
	 *
	 * Convert from a std::string back to a C++ basic data type
	 *
	 * @tparam The C++ data type
	 *
	 * @param[in]  str The string to convert
	 * @param[out] val The value represented by \a str
	 *
	 * @return True on success
	 *
	 **********************************************************************
	 */
	template <typename T>
	bool from_string(const std::string& str, T& val)
	{
		return false;
	}

	template<>
	inline bool from_string<bool>(const std::string& str, bool& val)
	{
		const std::string temp = trim(to_lower(str));

		if (temp == "false" || temp == "0")
			val = false;
		else if (temp == "true" || temp == "1")
			val = true;
		else
			return false;

		return true;
	}

	template<>
	inline bool from_string<char>(const std::string& str, char& val)
	{
		const std::string _str = trim(str);

		if (_str.empty())
			return false;
		else
			val = _str[0];

		return true;
	}

	template<>
	inline bool from_string<int16>(const std::string& str, int16& val)
	{
		int temp, ans = 
			std::sscanf(str.c_str(), "%d", &temp) == 1;

		val = temp;
			return(ans == 1);
	}

	template<>
	inline bool from_string<int32>(const std::string& str, int32& val)
	{
		return std::sscanf( str.c_str(), "%d", &val ) == 1;
	}

	template<>
	inline bool from_string<int64>(const std::string& str, int64& val)
	{
		return std::sscanf(str.c_str(), "%lld", &val) == 1;
	}

	template<>
	inline bool from_string<unsigned char>(const std::string& str,
		unsigned char& val)
	{
		return std::sscanf(str.c_str(), "%hhu", &val) == 1;
	}

	template<>
	inline bool from_string<uint16>(const std::string& str, uint16& val)
	{
		unsigned int temp;
		int ans = 
			std::sscanf(str.c_str(), "%u", &temp) == 1;

		val = temp;
			return(ans == 1);
	}

	template<>
	inline bool from_string<uint32>(const std::string& str, uint32& val)
	{
		return std::sscanf( str.c_str(), "%u", &val ) == 1;
	}

	template<>
	inline bool from_string<uint64>(const std::string& str, uint64& val)
	{
		return std::sscanf(str.c_str(), "%llu", &val) == 1;
	}

	template<>
	inline bool from_string<float>(const std::string& str, float& val)
	{
		errno = 0;
		val = std::strtof(str.c_str(), nullptr);
		
		return (errno == 0);
	}

	template<>
	inline bool from_string<double>(const std::string& str, double& val)
	{
		errno = 0;
		val = std::strtod(str.c_str(), nullptr);
		
		return (errno == 0);
	}

	template<>
	inline bool from_string<std::string>(const std::string& str,
		std::string& val)
	{
		val = str;
		return true;
	}

	/**
	 **********************************************************************
	 *
	 * Retrieve a bitmask with only the specified bit set
	 *
	 * @param [in] bit The desired bit, indexed from 0
	 *
	 * @return A power of 2 whose base-2 logarithm = bit. If bit exceeds
	 *         the size of T (in bits), -1 is returned, i.e. the maximum
	 *         value of the unsigned type T
	 *
	 **********************************************************************
	 */
	template<typename T> inline T getBit(uint32 bit)
	{
		AbortIfNot(bit < 8*sizeof(T), (T)~0);
		return ((T)1) << bit;
	}

	/**
	 **********************************************************************
	 *
	 * Get the index of the least significant bit set. This uses an O(n)
	 * algorithm, and should not be used beyond initialization for speed
	 * considerations
	 *
	 * @param [in] word The word to scan
	 *
	 * @return The LSB, or -1 if no bits are set
	 *
	 **********************************************************************
	 */
	template<typename T> inline int getLSB(T word)
	{
		int bit = 0;
		T mask = 1;

		while (mask)
		{
			if (mask & word) return bit;
			mask <<= 1; bit += 1;
		}

		return -1;
	}

	/**
	 **********************************************************************
	 *
	 * Get the index of the most significant bit set. This uses an O(n)
	 * algorithm and should not be used beyond initialization for speed
	 * considerations
	 *
	 * @param [in] word The word to scan
	 *
	 * @return The MSB, or -1 if no bits are set
	 *
	 **********************************************************************
	 */
	template<typename T> inline int getMSB(T word)
	{
		int bit = (8 * sizeof(T) - 1 );
		T mask = ((T)1) << bit;

		while (mask)
		{
			if (mask & word) return bit;
			mask >>= 1; bit -= 1;
		}

		return -1;
	}

	/**
	 **********************************************************************
	 *
	 * Returns the indexes of all bits set in a word
	 *
	 * @param [in]  word    The word to parse
	 * @param [out] indexes A list of bit indexes set
	 *
	 * @return True on success
	 *
	 **********************************************************************
	 */
	template<typename T> inline bool getSetBits(T word, uint32_v& indexes)
	{
		AbortIfNot(indexes.empty(), false);

		while (word)
		{
			int lsb = getLSB(word);
			indexes.push_back(lsb);

			clearBit(lsb, word);
		}

		return true;
	}

	/**
	 **********************************************************************
	 *
	 * Generate a pseudo-random 64-bit unsigned integer
	 *
	 * @return A random 64-bit integer
	 *
	 **********************************************************************
	 */
	inline uint64 rand64()
	{
		uint64 n1 = std::rand() & 0xFFFF;
		uint64 n2 = std::rand() & 0xFFFF;
		uint64 n3 = std::rand() & 0xFFFF;
		uint64 n4 = std::rand() & 0xFFFF;

		return (n1 << 48) |
			   (n2 << 32) |
			   (n3 << 16) |
			   (n4 << 0 );
	}

	/**
	 **********************************************************************
	 *
	 * Read all lines in a file, storing them in a vector of strings. Note
	 * blank (whitespace only) lines are discarded
	 *
	 * @param[in]  filename The file to read
	 * @param[out] lines    All lines in the file
	 *
	 * @return True on success
	 *
	 **********************************************************************
	 */
	inline
	bool readlines(const std::string& filename, str_v& lines)
	{
		lines.clear();

		AbortIfNot(dirExists(filename), false);

		std::ifstream infile(filename);
		AbortIfNot(  infile.is_open() , false);

		std::string line;
		while (std::getline(infile, line))
		{
			if (trim(line).size() > 0) lines.push_back(line);
		}

		return true;
	}

	/**
	 **********************************************************************
	 *
	 * Set the specified bit in a word
	 *
	 * @param [in]     bit  The desired bit, indexed from 0
	 * @param [in,out] word The word to modify
	 *
	 **********************************************************************
	 */
	template<typename T> inline void setBit(uint32 bit, T& word)
	{
		word |= ((T)1) << bit;
	}
	
	/**
	 **********************************************************************
	 *
	 * Split a string into tokens. A delimiter is specified to indicate
	 * how to slice the string
	 *
	 * @param [in] str     The string to split
	 * @param [out] tokens A vector of the elements of the split string
	 * @param [in] delim   A delimiter (char)
	 *
	 **********************************************************************
	 */
	inline void split(const std::string& str, str_v& tokens, char delim=' ')
	{
		std::string line = str;
		size_t ind, start = 0;

		while (true)
		{
			if ((ind = line.find(delim, start)) != std::string::npos)
			{
				if (ind - start > 0)
					tokens.push_back(line.substr(start,ind - start));
				start = ind+1;
			}
			else
			{
				if (start < line.size())
					tokens.push_back(line.substr(start,std::string::npos));
				break;
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Split a string into tokens. A delimiter is specified to indicate
	 * how to slice the string
	 *
	 * @param [in] str     The string to split
	 * @param [out] tokens A vector of the elements of the split string
	 * @param [in] delim   A delimiter (string)
	 *
	 **********************************************************************
	 */
	inline void split(
		const std::string& str, str_v& tokens, const std::string& delim)
	{
		std::string line = str;
		size_t ind, start = 0;

		while (true)
		{
			if ((ind = line.find(delim, start)) != std::string::npos)
			{
				if (ind - start > 0)
					tokens.push_back(line.substr(start,ind - start));
				start = ind + delim.size();
			}
			else
			{
				if (start < line.size())
					tokens.push_back(line.substr(start,std::string::npos));
				break;
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Split a string into evenly sized tokens, each one of size \a size
	 * characters. The final token contains characters remaining after
	 * dividing up the string 
	 *
	 * @param [in] str     The string to split
	 * @param [out] tokens A vector of the elements of the split string
	 * @param [in] size    The size of each token
	 *
	 **********************************************************************
	 */
	inline void split(const std::string& str,
		              str_v& tokens, size_t size)
	{

		size_t start = 0;

		if (size == 0)
			return;
		else
			tokens.clear();

		do
		{
			tokens.push_back(str.substr(start,size));
			start += size;

		} while (start < str.size());
	}

	/**
	 **********************************************************************
	 *
	 * Get the double representation of a string
	 *
	 * @param [in]  str  The string to convert
	 * @param [out] out  The result
	 *
	 * @return True on success, or false on errno
	 *
	 **********************************************************************
	 */
	inline bool str_to_f64(const std::string& str, double& out)
	{
		if (errno) errno = 0;
		out = std::strtod(str.c_str(), NULL);
		
		return errno == 0;
	}

	/**
	 **********************************************************************
	 *
	 * Get the integer representation of a string
	 *
	 * @param [in]  str  The string to convert
	 * @param [in]  base Radix (see strtol())
	 * @param [out] out  The result
	 *
	 * @return True on success, or false on errno
	 *
	 **********************************************************************
	 */
	inline bool str_to_i32(const std::string& str, int base, int& out)
	{
		if (errno) errno = 0;
		out = std::strtol(str.c_str(), NULL, base);
		
		return errno == 0;
	}

	/**
	 **********************************************************************
	 *
	 * Swap two generic data items
	 *
	 * @tparam T The item type
	 *
	 * @param[in,out] val1 Item 1
	 * @param[in,out] val2 Item 2
	 *
	 **********************************************************************
	 */
	template <typename T>
	inline void swap(T& val1, T& val2)
	{
		T temp = val1; val1 = val2;
		val2 = temp;
	}

	/**
	 **********************************************************************
	 *
	 * Get the string representation of a value. This operates on only
	 * fundamental C++ types
	 *
	 * @tparam T The type of \a val

	 * @param[in]  val The value to convert
	 * @param[out] str The string representation of \a val
	 *
	 * @return True on success
	 *
	 **********************************************************************
	 */
	template <typename T>
	bool to_string(const T& val, std::string& str)
	{
		return false;
	}

	template <>
	inline bool to_string<bool>(const bool& val, std::string& str)
	{
		str = val ? "true" : "false";
		return true;
	}

	template <>
	inline bool to_string<char>(const char& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%c", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<int16>(const int16& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%d", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<int32>(const int32& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%d", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<int64>(const int64& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%lld", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<unsigned char>(const unsigned char& val,
										 std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%hhu", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<uint16>(const uint16& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%u", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<uint32>(const uint32& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%u", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<uint64>(const uint64& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%llu", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<float>(const float& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%f", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<double>(const double& val, std::string& str)
	{
		char buf[64];

		if (std::snprintf(buf, 64, "%f", val) < 0)
			return false;

		str = std::string(buf);
		return true;
	}

	template <>
	inline bool to_string<std::string>(const std::string& val,
		std::string& str)
	{
		str = val;
		return true;
	}

	/**
	 **********************************************************************
	 *
	 * @brief
	 * Convert a character to lower case. Only when using the default C
	 * locale is this equivalent to ::tolower()
	 *
	 * @details
	 * Converts the given character in the range A-Z to lower case. If
	 * it is already in lower case or if there is no lower case 
	 * equivalent (e.g. a digit), the original character is returned
	 *
	 * @param [in] c The character to convert to lower case
	 *
	 * @return The lower case equivalent (if it exists) of the character
	 *
	 **********************************************************************
	 */
	inline char to_lower(char c)
	{
		if (c >= 65 && c <= 90)
			c += 32;

		return c;
	}

	/**
	 **********************************************************************
	 *
	 * Converts a string to lower case. See to_lower(char) for details
	 *
	 * @param[in] str The string to convert
	 *
	 * @return The lower case equivalent of the string
	 *
	 **********************************************************************
	 */
	inline std::string to_lower(const std::string& str)
	{
		std::string res = str;
		for (size_t i = 0; i < str.size(); i++)
			res[i] = to_lower(res[i]);

		return res;
	}

	/**
	 **********************************************************************
	 *
	 * @brief
	 * Convert a character to upper case. Only when using the default C
	 * locale is this equivalent to ::toupper()
	 *
	 * @details
	 * Converts the given character in the range A-Z to upper case. If
	 * it is already in upper case or if there is no upper case 
	 * equivalent (e.g. a digit), the original character is returned
	 *
	 * @param [in] c The character to convert to upper case
	 *
	 * @return The upper case equivalent (if it exists) of the character
	 *
	 **********************************************************************
	 */
	inline char to_upper(char c)
	{
		if (c >= 97 && c <= 122)
			c -= 32;

		return c;
	}

	/**
	 **********************************************************************
	 *
	 * Remove leading and trailing whitespace from a string. This includes
	 * the character set " \t\n\v\f\r"
	 *
	 * @param[in] str Input string
	 *
	 * @return   The input string with all leading and trailing whitespace
	 *           removed
	 *
	 **********************************************************************
	 */
	inline std::string trim(const std::string& str)
	{
		if (str.empty()) return str;

		const std::string space = "\t\n\v\f\r ";

		size_t start =
			str.find_first_not_of(space);

		if (start == std::string::npos)
			return "";

		size_t stop = str.find_last_not_of(space);

		return
			str.substr(start,stop-start+1);
	}

	/**
	 **********************************************************************
	 *
	 * Swap two data elements using the classic XOR swap algorithm
	 *
	 * @param[out] a The 1st element
	 * @param[out] b The 2nd element
	 *
	 **********************************************************************
	 */
	template <typename T>
	inline void xor_swap(T& a, T& b)
	{
		a = a ^ b;
		b = a ^ b;
		a = a ^ b;
	}
}

#endif
