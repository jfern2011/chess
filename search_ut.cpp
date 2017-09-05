#include "search.h"

class SEE_UT
{

public:

	SEE_UT()
	{
	}

	~SEE_UT()
	{
	}

	/**
	 * Test the SEE algorithm. The first two positions were taken from here:
	 *
	 *     https://chessprogramming.wikispaces.com/SEE+-+The+Swap+Algorithm
	 */
	bool run() const
	{
		bool xboard = false;

		DataTables tables; MoveGen movegen(tables);

		Node node(tables, xboard);

		Position pos(tables, xboard);

		if (pos.reset("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/8/3p4/4p3/3P4/8/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2B3b1/3p1p2/4p3/3P4/8/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2B3b1/5p2/4p3/3P4/8/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2B3b1/3p4/4p3/3P4/8/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2B5/3p4/4p3/3P4/8/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("1q2k3/2B5/3p4/4p3/3P4/8/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/8/3p1p2/4p3/3P1P2/8/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/8/3p1p2/4p3/3P1P2/3N4/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/8/3p4/4p3/8/2Q5/1B6/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/8/3p1p2/4p3/8/2Q5/1B6/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/4r3/4q3/4r3/4R3/8/4R3/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2q5/8/4b3/8/8/4R3/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2q5/8/4b3/8/6Q1/7B/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2q5/3p4/4b3/8/6B1/7Q/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2q5/3B4/4b3/8/6B1/8/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2q5/3b4/4b3/5B2/6B1/7Q/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/2q5/3b1p2/4b3/5B2/6B1/7Q/4K3 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/6b1/5p2/3Kb3/5B2/6B1/8/8 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/6b1/5p2/3Kb3/8/6B1/8/8 w - - 0 1",
					  false))
		{
			int score = node.see(pos, E5, WHITE);
			std::cout << "see(E5) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("2n1k3/1P5p/8/1N3pP1/8/2P5/P2r4/4K2R w K - 0 1",
					  false))
		{
			int score = node.see(pos, D6, WHITE);
			std::cout << "see(D6) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("2n1k3/1P5p/8/1NP2pP1/8/8/P2r4/4K2R w K - 0 1",
					  false))
		{
			int score = node.see(pos, D6, WHITE);
			std::cout << "see(D6) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("2n1k3/1P5p/8/1NP1PpP1/8/8/P2r4/4K2R w K - 0 1",
					  false))
		{
			int score = node.see(pos, D6, WHITE);
			std::cout << "see(D6) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("2n1k3/1P5p/8/1NP1PpP1/8/8/P2r4/4K2R w K - 0 1",
					  false))
		{
			int score = node.see(pos, A3, WHITE);
			std::cout << "see(A3) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/1P5p/8/1nP1PpP1/8/8/P2r4/4K2R w K - 0 1",
					  false))
		{
			int score = node.see(pos, H2, WHITE);
			std::cout << "see(H2) = " << score
				<< std::endl;
		}
		else
			return false;

		if (pos.reset("4k3/1P5p/8/1nP1PpP1/8/8/P2r4/4K2R w K - 0 1",
					  false))
		{
			int score = node.see(pos, H3, WHITE);
			std::cout << "see(H3) = " << score
				<< std::endl;
		}
		else
			return false;

		return true;
	}
};

int main()
{
	SEE_UT ut; ut.run();
	return 0;
}