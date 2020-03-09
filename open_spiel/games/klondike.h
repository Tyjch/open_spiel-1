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

    class Card {
    public:
        std::string rank;
        std::string suit;
        bool hidden;

        Card(std::string rank, std::string suit, bool hidden);

        bool operator==(Card other_card);
    };

    class Deck {
    public:
        std::deque<Card> cards;
        std::deque<Card> waste;

        Deck();
        void shuffle(int seed);
        void draw(int num_cards);
        void rebuild();
        std::deque<Card> deal(unsigned long int num_cards);

    private:
        int  times_rebuilt = 0;
        bool is_shuffled = false;
        std::deque<Card> initial_order;
    };

    class Foundation {
    public:
        char suit;
        std::deque<Card> cards;
        explicit Foundation(char suit);
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
        explicit KlondikeState(std::shared_ptr<const Game> game);
        Player                  CurrentPlayer() const override;
        std::vector<Action>     LegalActions() const override;
        std::string             ActionToString(Player player, Action action_id) const override;
        std::string             ToString() const override;
        bool                    IsTerminal() const override;
        std::vector<double>     Returns() const override;
        std::unique_ptr<State>  Clone() const override;

    private:
        int  cur_player_;
        bool is_setup_;
        Deck deck;
        std::vector<Foundation> foundations;
        std::vector<Tableau>    tableaus;
    };

    class KlondikeGame : public Game {
    public:
        explicit KlondikeGame(const GameParameters& params);
        int      NumDistinctActions() const override;
        int      MaxGameLength() const override;
        int      NumPlayers() const override;
        double   MinUtility() const override;
        double   MaxUtility() const override;

        std::unique_ptr<State>      NewInitialState() const override;
        std::shared_ptr<const Game> Clone() const override;

    private:
        int num_players_;
    };
}

#endif //OPEN_SPIEL_KLONDIKE_H
