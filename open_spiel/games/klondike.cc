#include "klondike.h"

#include <algorithm>
#include <array>
#include <numeric>
#include <utility>

#include "open_spiel/abseil-cpp/absl/strings/str_format.h"
#include "open_spiel/abseil-cpp/absl/strings/str_join.h"
#include "open_spiel/game_parameters.h"
#include "open_spiel/spiel_utils.h"

namespace open_spiel::klondike {
    namespace {
        const GameType kGameType {
            "klondike",
            "Klondike Solitaire",
            GameType::Dynamics::kSequential,
            GameType::ChanceMode::kSampledStochastic,
            GameType::Information::kImperfectInformation,
            GameType::Utility::kConstantSum,
            GameType::RewardModel::kRewards,
            1,
            1,
            true,
            true,
            true,
            true,
            {{"players", GameParameter(kDefaultPlayers)}},
        };

        std::shared_ptr<const Game> Factory(const GameParameters& params) {
            return std::shared_ptr<const Game>(new KlondikeGame(params));
        }

        REGISTER_SPIEL_GAME(kGameType, Factory);
    }

    // KlondikeState Methods ===========================================================================================

    KlondikeState::KlondikeState(std::shared_ptr<const Game> game) :
        State(game),
        cur_player_(kChancePlayerId) {
        // Empty
    }

    Player                  KlondikeState::CurrentPlayer() const {
        return 0;
    }

    std::vector<Action>     KlondikeState::LegalActions() const {
        return std::vector<Action>();
    }

    std::string             KlondikeState::ActionToString(Player player, Action action_id) const {
        return std::string();
    };

    std::string             KlondikeState::ToString() const {}

    bool                    KlondikeState::IsTerminal() const {}

    std::vector<double>     KlondikeState::Returns() const {}

    std::unique_ptr<State>  KlondikeState::Clone() const {
        return std::unique_ptr<State>(new KlondikeState(*this));
    }


    // KlondikeGame Methods ============================================================================================

    KlondikeGame::KlondikeGame(const GameParameters &params) :
        Game(kGameType, params),
        num_players_(ParameterValue<int>("players")) {
        std::cout << 'KlondikeGame()' << std::endl;
    };

    int    KlondikeGame::NumDistinctActions() const {
        return 155;
    }

    int    KlondikeGame::MaxGameLength() const {
        return 5;
    }

    int    KlondikeGame::NumPlayers() const {
        return 1;
    }

    double KlondikeGame::MinUtility() const {
        return 0.0;
    }

    double KlondikeGame::MaxUtility() const {
        return 3220.0;
    }

    std::unique_ptr<State>      KlondikeGame::NewInitialState() const {
        return std::unique_ptr<State>(new KlondikeState(shared_from_this()));
    }

    std::shared_ptr<const Game> KlondikeGame::Clone() const {
        return std::shared_ptr<const Game>(new KlondikeGame(*this));
    }
}