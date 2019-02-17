#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <limits>

#include "abort/abort.h"
#include "chess4.h"

namespace Chess
{
	const int EMPTY   = 0; // Reset between searches
	const int FAIL_HI = 1;
	const int FAIL_LO = 2;
	const int EXACT   = 3;

	struct HashEntry
	{
		uint8 age;
		uint8 draft;
		int16 score;
		int32 type_move;
		uint64 key;

        /**
         * Limit the age to prevent integer roll-over, making
         * entries appear younger than they are
         */
        static const auto age_limit = 0;
            //std::numeric_limits<decltype(age)>::max();

		int32 move() const
		{
			return ( type_move & 0x1fffff);
		}

        uint8 ripen()
        {
            if (age < age_limit) age++;
            return age;
        }

		void set_move(int32 move)
		{
			type_move &= (~0x1fffff);
			type_move |= (move & 0x1fffff);
		}

		void set_type(int type)
		{
			type_move &= 0x1fffff;
			type_move |= (type << 21);
		}

		int type() const
		{
			return (type_move >> 21) & 0x3;
		}
	};

	template <size_t N>
	struct HashBucket
	{
        static_assert(N > 0, "");

        static const size_t size = N ;

        void clear()
        {
            for (size_t i = 0; i < N; i++)
                entries[i].set_type(EMPTY);
        }

		BUFFER(HashEntry, entries, N);
	};

	template <size_t N>
	class HashTable
	{

	public:

        static const size_t bucket_size = N;

		HashTable()
			: _buckets(nullptr), _mask(0)
		{
		}

		~HashTable()
		{
			if (_buckets) delete [] _buckets;
		}

        void clear()
        {
            if (_buckets)
            {
                std::printf("clearing hash entries...\n\n");
                std::fflush(stdout);

                for (size_t i = 0; i < size(); i++)
                    _buckets[i].clear();
            }
        }

		HashBucket<N>& operator[]( uint64 key )
		{
			return _buckets[key & _mask];
		}

		bool resize(size_t numel)
		{
			AbortIfNot((numel > 0), false);
			AbortIfNot((numel & (numel-1)) == 0,
				false);

			auto temp= new HashBucket<N>[numel];
			AbortIfNot(temp, false);

			if (_buckets) delete [] _buckets;

			_mask    = numel-1;
			_buckets = temp;

			return true;
		}

        size_t usage() const
        {
            size_t used = 0;
            for (size_t i = 0; i < size(); i++)
            {
                for (size_t j = 0; j < bucket_size; j++)
                {
                    if (_buckets[i].entries[j].type() != EMPTY)
                        used++;
                }
            }

            return used;
        }

		size_t size() const
		{
			return _mask + 1;
		}

	private:

		HashBucket<N>* _buckets;

		uint64 _mask;
	};
}

#endif
