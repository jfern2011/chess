#include <cstring>
#include <utility>

#include "abort/abort.h"
#include "eval.h"
#include "MoveGen4.h"
#include "Search4.h"

namespace Chess
{
    Search4::Search4()
        : _aborted(false),
          _is_init(false),
          _max_iterations(0),
          _next_node_check(0),
          _node_count(0),
          _position(),
          _qnode_count(0),
          _start_time(),
          _stop_time()
    {
    }

    Search4::~Search4()
    {
    }

    bool Search4::init(Handle<Position> pos)
    {
        _is_init = false;

        AbortIfNot(pos, false);

        _position = pos;

        _aborted = false;
        _next_node_check = 100;
        _node_count  = 0;
        _qnode_count = 0;

        const auto size = sizeof( _pv[0][0] )
            * max_ply * max_ply;

        std::memset(_pv, 0, size);

        _is_init = true;
        return true;
    }

    int16 Search4::quiesce(int32 depth, int16 alpha, int16 beta)
    {
        Position& pos = *_position;

        const player_t to_move = pos.get_turn();
        const bool in_check    = pos.in_check(to_move);
        const auto& tables     = DataTables::get();

        BUFFER(int32, moves, max_moves);
        size_t n_moves;

        if (in_check)
        {
            n_moves = MoveGen::generate_check_evasions(
                pos, moves);

            if (n_moves == 0)
            {
                /*
                 * Mark the end of this variation:
                 */
                _save_pv(depth, 0);

                /*
                 * Add a small penalty to the mate score to encourage
                 * mates closer to the root:
                 */
                return depth - king_value;
            }
        }

        /*
         * Get an initial score for this position:
         */
        const int16 score =
            tables.sign[pos.get_turn()] * evaluate(pos);

        /*
         * Check if we can fail-high; not sure if this is correct for
         * zugzwang positions...
         */
        if (score >= beta)
            return beta;

        if (alpha < score) alpha = score;

        if (!in_check)
            n_moves = MoveGen::generate_captures(
                pos, moves);

        /*
         * Return the heuristic value of this position if
         * no captures are left:
         */
        if (n_moves == 0 || max_ply <= depth)
        {
            _save_pv(depth, 0);
            return score;
        }

        int32 best_move = 0;

        for (size_t i = 0; i < n_moves; i++)
        {
            _node_count++; _qnode_count++;

            const int32 move = moves[i];

            pos.make_move(move);

            const int16 score =
                -quiesce( depth+1, -beta, -alpha );

            pos.unmake_move(move);

            if (score >= beta)
                return beta;

            if ( score > alpha )
            {
                best_move = move;
                alpha = score;
            }
        }

        _save_pv(depth, best_move);

        return alpha;
    }

    int16 Search4::run(int32 depth, duration_t timeout)
    {
        AbortIfNot(_is_init , false);
        AbortIfNot(depth > 0, false);

        _max_iterations = depth;
        
        _start_time = std::chrono::steady_clock::now();
        _stop_time  = _start_time + timeout;

        int16 score = -king_value;

        for (int32 ply=0; ply < _max_iterations; ply++)
        {
            const int16 temp = search_root();

            if ( _aborted ) break;
            score = temp;
        }

        _is_init = false;
        return score;
    }

    int16 Search4::search(int32 depth, int16 alpha, int16 beta)
    {
        if (_next_node_check <= _node_count)
        {
            const auto now =
                        std::chrono::steady_clock::now();
            const auto dur = now - _start_time;

            if (_stop_time <= now)
            {
                _aborted = true; return beta;
            }

            // Check for timeouts once per second

            const int64 nps  = _node_count / dur.count();
            _next_node_check =
                _node_count + nps;
        }

        Position& pos = *_position;

        const player_t side = pos.get_turn();

        const bool in_check =
            pos.in_check( side );

        /*
         * Don't quiece() if we're in check:
         */
        if (_max_iterations <= depth && !in_check)
            return quiesce(depth, alpha, beta);

        BUFFER(int32, moves, max_moves);
        size_t n_moves;

        if (in_check)
        {
            n_moves = MoveGen::generate_check_evasions(
                pos, moves);

            if (n_moves == 0)
            {
                /*
                 * Mark the end of this variation:
                 */
                _save_pv(depth, 0);

                /*
                 * Add a small penalty to the mate score to encourage
                 * mates closer to the root:
                 */
                return depth - king_value;
            }
        }
        else
        {
            n_moves = MoveGen::generate_captures(
                pos, moves);

            n_moves += MoveGen::generate_noncaptures(
                pos, &moves[n_moves]);

            if (n_moves == 0)
            {
                /*
                 * Mark the end of this variation:
                 */
                _save_pv(depth, 0);

                return 0;
            }
        }

        for (size_t i = 0; i < n_moves; i++)
        {
            _node_count++;

            const int32 move = moves[i];

            pos.make_move( move );

            const int16 score = -search(depth+1, -beta, -alpha);

            pos.unmake_move(move);

            if (beta <= score)
                return beta;

            if ( score > alpha )
                alpha = score;
        }

        return alpha;
    }

    int16 Search4::search_root()
    {
        AbortIfNot( _position, false );

        Position& pos = *_position;

        const bool in_check =
            pos.in_check(pos.get_turn());

        BUFFER(int32, moves, max_moves );
        size_t n_moves;

        if (in_check)
        {
            n_moves = MoveGen::generate_check_evasions(
                pos, moves);

            if (n_moves == 0)
            {
                _save_pv(0, 0); return -king_value;
            }
        }
        else
        {
            n_moves = MoveGen::generate_captures(
                pos, moves);

            n_moves += MoveGen::generate_noncaptures(
                pos, &moves[n_moves]);
        }

        auto best = std::make_pair<int32,int16>(
            0, king_value+1);

        for (size_t i = 0; i < n_moves; i++)
        {
            _node_count++;

            const int32 move = moves[i];

            pos.make_move( move );

            const int16 score = search(1, -king_value,
                king_value);

            pos.unmake_move(move);

            if (score < best.second)
            {
                best.first = move; best.second
                    = score;
            }
        }

        return -best.second;
    }

    inline bool Search4::_out_of_time() const
    {
        return std::chrono::steady_clock::now() > _stop_time;
    }

    void Search4::_save_pv(int32 depth, int32 move)
    {
        if (depth < max_ply)
        {
            _pv[depth][depth] = move;

            // Null move signals the end of a variation:
            if (move == 0) return;
        }

        for  (int32 i = depth + 1; i < max_ply; i += 1 )
        {
            if ((_pv[depth][i] = _pv[depth+1][i])
                == 0) break;
        }
    }
}
