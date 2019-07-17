#include <iterator>

#include "MoveGen4.h"
#include "multi_variation.h"
#include "chess_util4.h"

#include "util/str_util.h"

namespace Chess
{
    /**
     * Constructor
     *
     * @param[in] nLines Number of lines to store
     */
    MultiVariation::MultiVariation(size_t nLines)
        : _capacity(0), _lines()
    {
        resize(nLines);
    }

    /**
     *  Clear all variations (does not affect the
     *  \ref size())
     */
    void MultiVariation::clear()
    {
        _lines.clear();
    }

    /**
     * Get the line at the specified index. The best line is at
     * index 0, the 2nd best is at index 1, and so on
     *
     * @param[in] index The index of the line to retrieve
     *
     * @return  The line. The behavior is undefined if \a index
     *          is out of bounds
     */
    const std::vector<int32>& MultiVariation::operator[](
        size_t index) const
    {
        int16 dummy; return get(index, dummy);
    }

    /**
     * Get the line at the specified index. The best line is at
     * index 0, the 2nd best is at index 1, and so on
     *
     * @param[in]  index The index of the line to retrieve
     * @param[out] score The corresponding score
     *
     * @return  The line. The behavior is undefined if \a index
     *          is out of bounds
     */
    const std::vector<int32>& MultiVariation::get(
        size_t index, int16& score) const
    {
        auto iter = _lines.begin();

        AbortIf(index >= _lines.size(), iter->line);

        std::advance(iter, index);

        score = iter->score;
            return iter->line;
    }

    /**
     * Insert a new line. Lines are sorted in descending order,
     * starting with the best line
     *
     * @param [in] line   The line to insert
     * @param [in] score  The corresponding score for this line
     *
     * @return True if we successfully inserted \a line
     */
    bool MultiVariation::insert(const MoveList& line, int16 score)
    {
        const bool space_left = _lines.size() < _capacity;

        bool inserted = false;
        for (auto iter  =  _lines.begin(), end = _lines.end();
             iter != end; ++iter)
        {
            if (score > iter->score)
            {
                _lines.emplace( iter, line, score );
                inserted = true;
                break;
            }
        }

        /*
         * If this line is worse than all others and there is
         * no space left, don't do anything
         */
        if (!inserted && !space_left)
            return false;

        /*
         * If this line is worse than all others and there is
         * space left, place it at the end of the list
         */
        if (!inserted && space_left)
        {
            _lines.push_back(std::move(ListScore(
                line, score)));

            return true;
        }

        /*
         * We inserted the new line since it was better than
         * at least one of the others. However, if we exceeded
         * the limit set by resize(), then delete the last
         * (i.e. worst) line
         */
        if (!space_left)
            _lines.pop_back();

        return true;
    }

    /**
     * Reset the limit on the number of lines that are stored,
     * preserving all entries up to \a size
     *
     * @param[in] size The new size
     */
    void MultiVariation::resize(size_t size)
    {
        _capacity = size;

        if ( _capacity < _lines.size() )
            _lines.resize(_capacity);
    }

    /**
     * Get the number of lines currently stored
     *
     * @return The number of lines
     */
    size_t MultiVariation::size() const
    {
        return _lines.size();
    }

    /**
     * Format a line of moves in standard algebraic notation
     *
     * @param [in] line   The line of moves to format
     * @param [in] pos    The positiion from which \a line is
     *                    played
     * @param[in] moveNum The move number to start at 
     */
    std::string MultiVariation::format(
        const std::vector<int32>& line, Position& pos,
                size_t moveNum)
    {
        /*
         * Allow a move to take up to 5 characters, and center
         * it accordingly
         */
        auto pad = [](const std::string& mv)
        {
            if (mv.size() < 3) return mv + std::string(3, ' ');
            if (mv.size() < 4) return mv + std::string(2, ' ');
            if (mv.size() < 5) return mv + std::string(1, ' ');
            else return mv;
        };

        /*
         * Disambiguate between moves of the same piece and
         * destination square. These require prepending either
         * the file or rank prior to the destination square
         */
        auto file_or_rank = [&](bool check, int32 mv)
        {
            std::string out;

            BUFFER( int32, moves, max_moves );
            size_t n_moves;

            if (check)
            {
                n_moves = MoveGen::generate_check_evasions(
                    pos, moves);

                if (n_moves == 0) return out;
            }
            else
            {
                n_moves = MoveGen::generate_captures(
                    pos, moves);

                n_moves += MoveGen::generate_noncaptures(
                    pos, &moves[n_moves]);
            }

            for (size_t i = 0; i < n_moves; i++)
            {
                const int32 move = moves[i];
                if (move == mv) continue;

                if (extract_to(mv) == extract_to(move) &&
                     extract_moved(mv) == extract_moved(move))
                {
                    if (get_file(mv) != get_file(move))
                        Util::to_string( square_str[ extract_from(
                            mv)][0], out);
                    else // disambiguate by rank instead
                        Util::to_string( square_str[ extract_from(
                            mv)][1], out);
                    break;
                }
            }

            return out;
        };

        auto iter = line.begin();
        std::string out;

        /*
         * Handle where it's Black's turn to move
         */

        if (pos.get_turn() == player_t::black)
        {
            const int32 move = *iter++;

            Util::to_string(moveNum, out);

            out += ". "  + pad("... ");

            pos.make_move(move);

            const bool played_check = pos.in_check(
                pos.get_turn());

            out += pad(format_san(move,
                file_or_rank(played_check, move),
                    played_check)) + " ";

            moveNum++;
        }

        /*
         * Now handle the remaining moves
         */

        for (auto end = line.end(); iter != end; ++iter)
        {
            const int32 move = *iter;

            if (pos.get_turn() == player_t::white)
            {
                std::string num;
                Util::to_string( moveNum, num );
                out += num + ". ";
            }
            else
            {
                moveNum++;
            }

            pos.make_move(move);

            const bool played_check = pos.in_check(
                pos.get_turn());
            const bool mate = _is_mated(pos, pos.get_turn());

            out += pad(format_san(move,
                file_or_rank(played_check, move),
                    played_check, mate)) + " ";
        }
        
        return out;
    }

    /**
     * Determine if the given side is checkmated
     *
     * @param [in] pos     A Position
     * @param [in] to_move The player
     *
     * @return  True if \a to_move is checkmated
     */
    bool MultiVariation::_is_mated(const Position& pos,
                                   player_t to_move)
    {
        if (pos.in_check(to_move))
        {
            BUFFER( int32, moves, max_moves );

            return MoveGen::generate_check_evasions(
                    pos, moves) == 0;
        }

        return false;
    }
}
