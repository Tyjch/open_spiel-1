#ifndef THIRD_PARTY_OPEN_SPIEL_GAMES_KLONDIKE_H_
#define THIRD_PARTY_OPEN_SPIEL_GAMES_KLONDIKE_H_

#include <array>
#include <memory>
#include <string>
#include <vector>
#include "open_spiel/spiel.h"

namespace open_spiel::klondike {

    inline constexpr int kDefaultPlayers = 1;

    const std::vector<std::string> SUITS = {"s", "h", "c", "d"};
    const std::vector<std::string> RANKS = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K"};

    const std::map<std::string, int> SUIT_VALUES = {
            {"s", 0},
            {"h", 1},
            {"c", 2},
            {"d", 3}
    };

    const std::map<std::string, int> RANK_VALUES = {
            {"A", 0},
            {"2", 1},
            {"3", 2},
            {"4", 3},
            {"5", 4},
            {"6", 5},
            {"7", 6},
            {"8", 7},
            {"9", 8},
            {"T", 9},
            {"J", 10},
            {"Q", 11},
            {"K", 12}
    };

    const std::map<int, std::string> VALUES_TO_SUIT = {
            {0, "s"},
            {1, "h"},
            {2, "c"},
            {3, "d"}
    };

    const std::map<int, std::string> VALUES_TO_RANK = {
            {0, "A"},
            {1, "2"},
            {2, "3"},
            {3, "4"},
            {4, "5"},
            {5, "6"},
            {6, "7"},
            {7, "8"},
            {8, "9"},
            {9, "T"},
            {10, "J"},
            {11, "Q"},
            {12, "K"},
    };

    const std::map<std::string, std::string> TABLEAU_CHILD_RANK = {
            {"2", "A"},
            {"3", "2"},
            {"4", "3"},
            {"5", "4"},
            {"6", "5"},
            {"7", "6"},
            {"8", "7"},
            {"9", "8"},
            {"T", "9"},
            {"J", "T"},
            {"Q", "J"},
            {"K", "Q"}
    };

    const std::map<std::string, std::string> FOUNDATION_CHILD_RANK = {
            {"A", "2"},
            {"2", "3"},
            {"3", "4"},
            {"4", "5"},
            {"5", "6"},
            {"6", "7"},
            {"7", "8"},
            {"8", "9"},
            {"9", "T"},
            {"T", "J"},
            {"J", "Q"},
            {"Q", "K"}
    };

    // kInvalidAction == -1
    enum ActionType {

        // Meta Actions ================================================================================================
        kSetup = -4,
        kEnd   = -3,
        kDraw  = -2,

        // Pre-game Actions ============================================================================================
        
        // Set Spades
        kSetAs = 0,
        kSet2s = 1,
        kSet3s = 2,
        kSet4s = 3,
        kSet5s = 4,
        kSet6s = 5,
        kSet7s = 6,
        kSet8s = 7,
        kSet9s = 8,
        kSetTs = 9,
        kSetJs = 10,
        kSetQs = 11,
        kSetKs = 12,

        // Set Hearts
        kSetAh = 13,
        kSet2h = 14,
        kSet3h = 15,
        kSet4h = 16,
        kSet5h = 17,
        kSet6h = 18,
        kSet7h = 19,
        kSet8h = 20,
        kSet9h = 21,
        kSetTh = 22,
        kSetJh = 23,
        kSetQh = 24,
        kSetKh = 25,

        // Set Clubs
        kSetAc = 26,
        kSet2c = 27,
        kSet3c = 28,
        kSet4c = 29,
        kSet5c = 30,
        kSet6c = 31,
        kSet7c = 32,
        kSet8c = 33,
        kSet9c = 34,
        kSetTc = 35,
        kSetJc = 36,
        kSetQc = 37,
        kSetKc = 38,

        // Set Diamonds
        kSetAd = 39,
        kSet2d = 40,
        kSet3d = 41,
        kSet4d = 42,
        kSet5d = 43,
        kSet6d = 44,
        kSet7d = 45,
        kSet8d = 46,
        kSet9d = 47,
        kSetTd = 48,
        kSetJd = 49,
        kSetQd = 50,
        kSetKd = 51,
        
        // Card Moves ==================================================================================================
        kMoveAs2s = 53,
        kMove2s3s = 154,
        kMove3s4s = 255,
        kMove4s5s = 356,
        kMove5s6s = 457,
        kMove6s7s = 558,
        kMove7s8s = 659,
        kMove8s9s = 760,
        kMove9sTs = 861,
        kMoveTsJs = 962,
        kMoveJsQs = 1063,
        kMoveQsKs = 1164,
        kMoveKs   = 1252,
        kMoveAh2h = 1366,
        kMove2h3h = 1467,
        kMove3h4h = 1568,
        kMove4h5h = 1669,
        kMove5h6h = 1770,
        kMove6h7h = 1871,
        kMove7h8h = 1972,
        kMove8h9h = 2073,
        kMove9hTh = 2174,
        kMoveThJh = 2275,
        kMoveJhQh = 2376,
        kMoveQhKh = 2477,
        kMoveKh   = 2552,
        kMoveAc2c = 2679,
        kMove2c3c = 2780,
        kMove3c4c = 2881,
        kMove4c5c = 2982,
        kMove5c6c = 3083,
        kMove6c7c = 3184,
        kMove7c8c = 3285,
        kMove8c9c = 3386,
        kMove9cTc = 3487,
        kMoveTcJc = 3588,
        kMoveJcQc = 3689,
        kMoveQcKc = 3790,
        kMoveKc   = 3852,
        kMoveAd2d = 3992,
        kMove2d3d = 4093,
        kMove3d4d = 4194,
        kMove4d5d = 4295,
        kMove5d6d = 4396,
        kMove6d7d = 4497,
        kMove7d8d = 4598,
        kMove8d9d = 4699,
        kMove9dTd = 4800,
        kMoveTdJd = 4901,
        kMoveJdQd = 5002,
        kMoveQdKd = 5103,
        kMoveKd   = 5152,
        kMoveAs   = 52,
        kMove2sAh = 165,
        kMove3s2h = 266,
        kMove4s3h = 367,
        kMove5s4h = 468,
        kMove6s5h = 569,
        kMove7s6h = 670,
        kMove8s7h = 771,
        kMove9s8h = 872,
        kMoveTs9h = 973,
        kMoveJsTh = 1074,
        kMoveQsJh = 1175,
        kMoveKsQh = 1276,
        kMoveAh   = 1352,
        kMove2hAs = 1452,
        kMove3h2s = 1553,
        kMove4h3s = 1654,
        kMove5h4s = 1755,
        kMove6h5s = 1856,
        kMove7h6s = 1957,
        kMove8h7s = 2058,
        kMove9h8s = 2159,
        kMoveTh9s = 2260,
        kMoveJhTs = 2361,
        kMoveQhJs = 2462,
        kMoveKhQs = 2563,
        kMoveAc   = 2652,
        kMove2cAh = 2765,
        kMove3c2h = 2866,
        kMove4c3h = 2967,
        kMove5c4h = 3068,
        kMove6c5h = 3169,
        kMove7c6h = 3270,
        kMove8c7h = 3371,
        kMove9c8h = 3472,
        kMoveTc9h = 3573,
        kMoveJcTh = 3674,
        kMoveQcJh = 3775,
        kMoveKcQh = 3876,
        kMoveAd   = 3952,
        kMove2dAs = 4052,
        kMove3d2s = 4153,
        kMove4d3s = 4254,
        kMove5d4s = 4355,
        kMove6d5s = 4456,
        kMove7d6s = 4557,
        kMove8d7s = 4658,
        kMove9d8s = 4759,
        kMoveTd9s = 4860,
        kMoveJdTs = 4961,
        kMoveQdJs = 5062,
        kMoveKdQs = 5163,
        kMove2sAd = 191,
        kMove3s2d = 292,
        kMove4s3d = 393,
        kMove5s4d = 494,
        kMove6s5d = 595,
        kMove7s6d = 696,
        kMove8s7d = 797,
        kMove9s8d = 898,
        kMoveTs9d = 999,
        kMoveJsTd = 1100,
        kMoveQsJd = 1201,
        kMoveKsQd = 1302,
        kMove2hAc = 1478,
        kMove3h2c = 1579,
        kMove4h3c = 1680,
        kMove5h4c = 1781,
        kMove6h5c = 1882,
        kMove7h6c = 1983,
        kMove8h7c = 2084,
        kMove9h8c = 2185,
        kMoveTh9c = 2286,
        kMoveJhTc = 2387,
        kMoveQhJc = 2488,
        kMoveKhQc = 2589,
        kMove2cAd = 2791,
        kMove3c2d = 2892,
        kMove4c3d = 2993,
        kMove5c4d = 3094,
        kMove6c5d = 3195,
        kMove7c6d = 3296,
        kMove8c7d = 3397,
        kMove9c8d = 3498,
        kMoveTc9d = 3599,
        kMoveJcTd = 3700,
        kMoveQcJd = 3801,
        kMoveKcQd = 3902,
        kMove2dAc = 4078,
        kMove3d2c = 4179,
        kMove4d3c = 4280,
        kMove5d4c = 4381,
        kMove6d5c = 4482,
        kMove7d6c = 4583,
        kMove8d7c = 4684,
        kMove9d8c = 4785,
        kMoveTd9c = 4886,
        kMoveJdTc = 4987,
        kMoveQdJc = 5088,
        kMoveKdQc = 5189,
    };

    const std::set<Action> special_moves = {1252, 2552, 3852, 5152, 52, 1352, 2652, 3952};

    class Card {
    public:
        std::string rank;
        std::string suit;
        bool hidden;

        Card(std::string rank, std::string suit, bool hidden);

        explicit Card(int card_index);

        explicit operator int() const;

        bool operator==(const Card & other_card) const;

        bool operator==(Card & other_card) const;

    };

    class Deck {
    public:
        std::deque<Card> cards;
        std::deque<Card> waste;
        std::deque<Card> initial_order;
        int              times_rebuilt = 0;

        Deck();
        void shuffle(int seed);
        void draw(int num_cards);
        void rebuild();
        std::deque<Card> deal(unsigned long int num_cards);

    private:
        bool is_shuffled = false;
    };

    class Foundation {
    public:
        std::string suit;
        std::deque<Card> cards;
        explicit Foundation(std::string suit);
        bool operator==(Foundation & other_foundation) const;
    };

    class Tableau {
    public:
        std::deque<Card> cards;

        Tableau();
        explicit Tableau(std::deque<Card> provided_cards);
    };

    class KlondikeGame;

    class KlondikeState : public State {
    public:
        Deck deck;
        std::vector<Foundation> foundations;
        std::vector<Tableau>    tableaus;
        std::string             previous_string;
        double                  previous_score;
        bool                    last_move_was_reversible;
        bool                    is_terminal;

        explicit                KlondikeState(std::shared_ptr<const Game> game);
        Player                  CurrentPlayer() const override;
        std::vector<Action>     LegalActions() const override;
        std::string             ActionToString(Player player, Action action_id) const override;
        std::string             ToString() const override;
        std::string             GetContainerType(Card card) const;
        bool                    IsTerminal() const override;
        std::vector<double>     Returns() const override;
        std::unique_ptr<State>  Clone() const override;
        void                    DoApplyAction(Action move) override;
        bool                    IsChanceNode() const override;
        std::vector<Card>       Targets() const;
        std::vector<Card>       Sources() const;
        std::vector<Card>       Sources(const std::string& location) const;
        std::vector<Card>       Targets(const std::string& location) const;
        void                    MoveCards(const std::pair<Card, Card>& move);
        std::vector<double>     Rewards() const override;
        bool                    IsReversible(Action action) const;
        std::vector<Action>     CandidateActions() const;

        std::string InformationStateString(Player player) const override;
        std::string ObservationString(Player player) const override;

        void InformationStateTensor(Player player, std::vector<double> * values) const override;
        void ObservationTensor(Player player, std::vector<double> * values) const override;

        const std::deque<Card> * GetContainer(Card card_to_find) const;
        std::vector<std::pair<Action, double>> ChanceOutcomes() const override;

    private:
        int    cur_player_;
        int    setup_counter_{};
        bool   is_setup_;
        double score_;
        double CurrentScore() const;
    };

    class KlondikeGame : public Game {
    public:
        explicit KlondikeGame(const GameParameters& params);
        int      NumDistinctActions() const override;
        int      MaxGameLength() const override;
        int      NumPlayers() const override;
        double   MinUtility() const override;
        double   MaxUtility() const override;

        std::vector<int> InformationStateTensorShape() const override;
        std::vector<int> ObservationTensorShape() const override;

        std::unique_ptr<State>      NewInitialState() const override;
        std::shared_ptr<const Game> Clone() const override;

    private:
        int num_players_;
    };

}

#endif // THIRD_PARTY_OPEN_SPIEL_GAMES_KLONDIKE_H_
