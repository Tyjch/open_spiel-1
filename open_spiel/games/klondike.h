#ifndef THIRD_PARTY_OPEN_SPIEL_GAMES_KLONDIKE_H_
#define THIRD_PARTY_OPEN_SPIEL_GAMES_KLONDIKE_H_

#include <array>
#include <memory>
#include <string>
#include <vector>
#include "open_spiel/spiel.h"

namespace open_spiel::klondike {
    inline constexpr int kDefaultPlayers = 1;

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
        int cur_player_;
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
