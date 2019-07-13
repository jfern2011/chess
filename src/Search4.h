#ifndef __SEARCH4_H__
#define __SEARCH4_H__

#include <chrono>

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

            int64 node_count;
            int64 qnode_count;
        };

        using duration_t =
            std::chrono::steady_clock::duration;

        Search4();

        ~Search4();

        Statistics get_stats() const;

        bool init(Handle<Position> pos);

        int16 quiesce(int32 depth, int16 alpha, int16 beta);

        int16 run(int32 depth, duration_t timeout);

        int16 search (int32 depth, int16 alpha, int16 beta);

        int16 search_root();

    private:

        bool _check_timeout();

        void _save_pv(int32 depth, int32 move);

        bool _aborted;

        bool _is_init;

        int32 _max_depth;

        int64 _next_node_check;

        Handle<Position> _position;

        BUFFER(int32, _pv, max_ply,
            max_ply);

        std::chrono::steady_clock::time_point
            _start_time;

        Statistics _stats;

        std::chrono::steady_clock::time_point
            _stop_time;
    };
}

#endif
