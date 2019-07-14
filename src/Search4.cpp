#include <cstring>
#include <utility>

#include "abort/abort.h"
#include "chess_util4.h"
#include "eval.h"
#include "MoveGen4.h"
#include "Search4.h"

namespace Chess
{
    Search4::Statistics::Statistics()
        : lnode_count(0),
          node_count(0),
          qnode_count(0)
    {
    }

    void Search4::Statistics::clear()
    {
        lnode_count = 0;
        node_count  = 0;
        qnode_count = 0;
    }

    Search4::Search4()
        : _aborted(false),
          _is_init(false),
          _max_depth(1),
          _next_node_check(0),
          _position(),
          _pv_set(),
          _start_time(),
          _stop_time()
    {
    }

    Search4::~Search4()
    {
    }

    std::vector<int32>
    Search4::get_pv(size_t index) const
    {
        AbortIfNot(index < _pv_set.size(), std::vector<int32>());

        return _pv_set[index];
    }

    auto Search4::get_stats() const -> Statistics
    {
        return _stats;
    }

    bool Search4::init(Handle<Position> pos)
    {
        _is_init = false;

        AbortIfNot(pos, false);

        _position = pos;

        _aborted = false;
        _next_node_check = 0;
        
        _stats.clear();

        const auto size = sizeof( _pv[0][0] )
            * max_ply * max_ply;

        std::memset(_pv, 0, size);

        _is_init = true;
        return true;
    }

    int16 Search4::quiesce(uint32 depth, int16 alpha, int16 beta)
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
                _stats.lnode_count++;

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
        }

        /*
         * Get an initial score for this position:
         */
        const int16 score =
            tables.sign[ pos.get_turn() ] * evaluate(pos);

        if (alpha < score) alpha = score;

        /*
         * Return the heuristic value of this position if
         * no captures are left:
         */
        if (n_moves == 0 || max_ply <= depth)
        {
            _save_pv(depth, 0);

            _stats.lnode_count++;
            return score;
        }

        /*
         * Check if we can fail-high; not sure if this is
         * correct for zugzwang positions...
         */
        if (score >= beta)
        {
            _stats.lnode_count++;
            return beta;
        }

        int32 best_move = 0;

        for (size_t i = 0; i < n_moves; i++)
        {
            _stats.node_count++; _stats.qnode_count++;

            const int32 move = moves[i];

            pos.make_move(move);

            const int16 score
                = -quiesce(depth + 1, -beta, -alpha );

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

    int16 Search4::run(uint32 depth, duration_t timeout)
    {
        AbortIfNot(_is_init , -king_value);
        AbortIfNot(depth > 0, -king_value);
        
        _start_time  =  std::chrono::steady_clock::now();
        _stop_time   =  _start_time + timeout;

        int16 score = -king_value;

        for (_max_depth =  1; _max_depth <= depth;
             _max_depth++)
        {
            _pv_set.clear();
            
            const int16 tmp_score = search_root();

            if ( _aborted ) break;
            score = tmp_score;

            // Display the principal variation
            // TODO Dump this to a stream configured
            // for UCI or human-readable format

            Position temp(*_position);
            const std::string line =
                MultiVariation::format( get_pv(0), temp);

            std::printf("[%2u]: %5hd --> %s\n",
                        _max_depth, score, line.c_str());
            std::fflush(stdout);
        }

        _is_init = false;
        return score;
    }

    int16 Search4::search(uint32 depth, int16 alpha, int16 beta)
    {
        if (_next_node_check <= _stats.node_count
            && _check_timeout())
        {
            _aborted = true; _stats.lnode_count++;
            return beta;
        }

        Position& pos = *_position;

        const player_t side = pos.get_turn();

        const bool in_check =
            pos.in_check( side );

        /*
         * Don't quiece() if we're in check:
         */
        if (_max_depth <= depth && !in_check)
            return quiesce( depth, alpha, beta );

        BUFFER(int32, moves, max_moves);
        size_t n_moves;

        if (in_check)
        {
            n_moves = MoveGen::generate_check_evasions(
                pos, moves);

            if (n_moves == 0)
            {
                _stats.lnode_count++;

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
                _stats.lnode_count++;

                /*
                 * Mark the end of this variation:
                 */
                _save_pv(depth, 0);

                return 0;
            }
        }

        int32 best_move = 0;
        for (size_t i = 0; i < n_moves; i++)
        {
            _stats.node_count++;

            const int32 move = moves[i];

            pos.make_move( move );

            const int16 score = -search(depth+1, -beta, -alpha);

            pos.unmake_move(move);

            if (beta <= score)
                return beta;

            if ( score > alpha )
            {
                best_move = move;
                alpha = score;
            }
        }

        if (best_move)
            _save_pv(depth, best_move);

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
            _stats.node_count++;

            const int32 move = moves[i];

            pos.make_move( move );

            const int16 score = search(1, -king_value,
                king_value);

            pos.unmake_move(move);

            if (score < best.second)
            {
                _save_pv(0, move);

                // Insert the newly computed variation

                _pv_set.insert(_get_pv(),
                    -best.second);

                best.first = move; best.second
                    = score;
            }
        }

        return -best.second;
    }

    bool Search4::setNumberOfLines(size_t size)
    {
        AbortIfNot(size < max_moves, false);
        _pv_set.resize(size);

        return true;
    }

    bool Search4::_check_timeout()
    {
        const auto now = std::chrono::steady_clock::now();
        const auto dur = now - _start_time;

        if (_stop_time <= now)
            return true;

        // Check for timeouts once per second

        const auto nps =
            _stats.node_count * decltype(dur)::period::den /
                dur.count();

        _next_node_check =
            _stats.node_count + nps;

        return false;
    }

    MoveList Search4::_get_pv()
    {
        MoveList list; list.init(_pv[0], 0);

        for (size_t i = 0;
             _pv[0][i] && i < max_ply; i++ )
            list.size++;

        return list;
    }

    void Search4::_save_pv(uint32 depth, int32 move)
    {
        if (depth < max_ply)
        {
            _pv[depth][depth] = move;

            // Null move signals the end of a variation:
            if (move == 0) return;

            for  (uint32 i = depth + 1; i < max_ply; i += 1)
            {
                if ((_pv[depth][i] = _pv[depth+1][i])
                    == 0) break;
            }
        }
    }
}
