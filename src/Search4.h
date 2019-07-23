#ifndef __SEARCH4_H__
#define __SEARCH4_H__

#include <chrono>

#include "MoveList.h"
#include "multi_variation.h"
#include "Position4.h"

namespace Chess
{
    class Search4 final
    {

    public:

        class Statistics final
        {

        public:

            Statistics();

            Statistics(const Statistics& stats) = default;
            Statistics(Statistics&& stats) = default;

            Statistics& operator=(const Statistics& stats) = default;
            Statistics& operator=(Statistics&& stats) = default;

            ~Statistics() = default;

            void clear();

            uint64 lnode_count;
            uint64 node_count;
            uint64 qnode_count;
        };

        using duration_t =
            std::chrono::steady_clock::duration;

        Search4();

        ~Search4();

        void abort();

        std::vector<int32> get_pv(size_t index) const;

        Statistics get_stats() const;

        bool init(Handle<Position> pos);

        int16 quiesce(uint32 depth, int16 alpha, int16 beta);

        int16 run(uint32 depth,
                  duration_t timeout,
                  uint64 node_limit,
                  bool mate_search );

        int16 search (uint32 depth, int16 alpha, int16 beta);

        int16 search_root();

        bool setNumberOfLines(size_t size);

    private:

        bool _check_abort();

        MoveList _get_pv();

        void _save_pv(uint32 depth, int32 move);

        bool _aborted;

        bool _is_init;

        bool _mate_search;

        uint32 _max_depth;

        uint64 _max_nodes;

        uint64 _next_node_check;

        Handle<Position> _position;

        BUFFER(int32, _pv, max_ply,
            max_ply);

        MultiVariation _pv_set;

        std::chrono::steady_clock::time_point
            _start_time;

        Statistics _stats;

        std::chrono::steady_clock::time_point
            _stop_time;
    };
}

#endif
