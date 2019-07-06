#ifndef __SEARCH4_H__
#define __SEARCH4_H__

#include <chrono>

#include "Position4.h"

namespace Chess
{
    class Search4 final
    {

    public:

        using duration_t =
            std::chrono::steady_clock::duration;

        Search4();

        ~Search4();

        bool init(Handle< Position > pos);

        int16 quiesce(int32 depth, int16 alpha, int16 beta);

        int16 run(int32 depth, duration_t timeout);

        int16 search (int32 depth, int16 alpha, int16 beta);

        int16 search_root();

    private:

        bool _out_of_time() const;

        void _save_pv(int32 depth, int32 move);

        bool _aborted;

        bool _is_init;

        int32 _max_iterations;

        int64 _next_node_check;

        int64 _node_count;

        Handle<Position> _position;

        BUFFER(int32, _pv, max_ply,
            max_ply);

        int64 _qnode_count;

        std::chrono::steady_clock::time_point
            _start_time;

        std::chrono::steady_clock::time_point
            _stop_time;
    };
}

#endif
