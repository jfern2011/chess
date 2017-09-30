#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <cstdlib>
#include <iostream>

#include "types.h"

#define FAIL_LO 1
#define FAIL_HI 2
#define PV_NODE 4

/**
 **********************************************************************
 *
 * @class HashEntry
 *
 * Represents an entry in the transposition table
 *
 **********************************************************************
 */
class HashEntry
{
	/*
	 * When it comes to replacement, we consider age, depth, and
	 * number of hash hits. Each of these is weighed
	 * differently, with the weightings determined through trial
	 * and error
	 */
	static const int age_weight   = 10;
	static const int depth_weight = 20;
	static const int hits_weight  = 1;

public:

	/**
	 * Default constructor
	 */
	HashEntry()
	{
	}

	/**
	 * Destructor
	 */
	~HashEntry()
	{
	}

	/**
	 * @brief
	 * Compare this hash entry with another, returning true if ours
	 * is better
	 *
	 * @details
	 * This is used to determine if an existing entry should be
	 * overwritten
	 *
	 * @param[in] rhs The entry to compare against
	 *
	 * @return True if our hash entry is better than the one passed
	 *         in
	 */
	inline bool operator>(const HashEntry& rhs) const
	{
		if (depth == rhs.depth)
			return (hits > rhs.hits);
		else
			return depth > rhs.depth;
	}

	/*
	 * The age increments for each increase in full move number
	 * at the root
	 */
	uint8 age;

	/*
	 * Depth to which this move was searched
	 */
	uint8  depth;
	bool   do_null;
	int32  hits;
	uint64 key;
	int    move;
	int8   node_type;
	int32  score;
};

/**
 **********************************************************************
 *
 * @class HashEntries
 *
 * Represents a collection of entries within a single slot of the
 * transposition table
 *
 **********************************************************************
 */
class HashEntries
{

public:

	static const int N_ENTRIES = 1;

	HashEntries()
		: _in_use(0)
	{
	}

	~HashEntries()
	{
	}

	inline const HashEntry& operator[](uint32 index) const
	{
		return _entries[index];
	}

	inline HashEntry& operator[](uint32 index)
	{
		return _entries[index];
	}

	inline void clear()
	{
		for (register int i = 0; i < N_ENTRIES; i++)
			_entries[i].node_type = 0;

		_in_use = 0;
	}

	inline bool in_use() const
	{
		return _in_use;
	}

	inline bool insert( const HashEntry& entry )
	{
		/*
		 * 1. Overwrite any entries with the same signature
		 */
		for (register int i = 0; i < N_ENTRIES; i++)
		{
			if (entry.key == _entries[i].key)
			{
					_entries[i] = entry; return true;
			}
		}

		/*
		 * 2. If there is another slot available, use it:
		 */
		if (_in_use < N_ENTRIES)
		{
			_entries[_in_use++] = entry;
			return true;
		}

		/*
		 * 3. No slots were available; overwrite the entry
		 *    that is "worst":
		 */
		HashEntry& worst = _entries[0];
		for (register int i = 1; i < N_ENTRIES; i++)
		{
			if (worst > _entries[i])
				worst = _entries[i];
		}

			worst = entry;
		return true;
	}

	/**
	 * Sort the slot entries. This only needs to be done once
	 * for each slot that fills up
	 *
	 * The elements are placed in descending order
	 */
	inline void sort()
	{
		int in_use = _in_use;

		HashEntry entry;

		bool swapped = true;
		while (swapped)
		{
			swapped = false;

			for (register int i = 1; i < in_use; i++)
			{
				if (_entries[i] > _entries[i-1])
				{
					entry = _entries[i-1];
					_entries[ i - 1 ] = _entries[ i ];
					_entries[i] = entry;
					swapped = true;
				}

				in_use--;
			}
		}
	}

private:

	HashEntry _entries[N_ENTRIES];
	int       _in_use;
};

/**
 **********************************************************************
 *
 * @class HashTable
 *
 * Implements the transposition table
 *
 **********************************************************************
 */
class HashTable
{

public:

	static const int TABLE_SIZE = 8192*1024;

	HashTable()
		: _mask(TABLE_SIZE-1)
	{
		const int nbytes = TABLE_SIZE * sizeof(HashEntries);

		_slots =
			static_cast<HashEntries* >(std::malloc(nbytes));


		std::cout << "Hash table is ";
		if (nbytes > 1e6)
			std::cout << int(nbytes/1e6) << " MB " << std::endl;
		else if (nbytes > 1000)
			std::cout << int(nbytes/1e3) << " KB " << std::endl;
		else
			std::cout << nbytes
				<< " bytes" << std::endl;
		clear();
	}

	~HashTable()
	{
		std::free(_slots);
	}

	inline const HashEntries& operator[](uint64 key) const
	{
		return _slots[key & _mask];
	}

	inline HashEntries& operator[](uint64 key)
	{
		return _slots[key & _mask];
	}

	inline void clear()
	{
		for (register int i = 0; i < TABLE_SIZE; i++)
		{
			_slots[i].clear();
		}
	}

	inline int in_use() const
	{
		int ans = 0;
		for ( register int i = 0; i < TABLE_SIZE; i++ )
		{
			if (_slots[i].in_use())
				ans++;
		}

		return ans;
	}

private:

	const int    _mask;
	HashEntries* _slots;

};

#endif
