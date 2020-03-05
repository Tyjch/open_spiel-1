#include "open_spiel/games/klondike.h"

#include <algorithm>
#include <array>
#include <numeric>
#include <utility>

#include "open_spiel/abseil-cpp/absl/strings/str_format.h"
#include "open_spiel/abseil-cpp/absl/strings/str_join.h"
#include "open_spiel/game_parameters.h"
#include "open_spiel/spiel_utils.h"

namespace open_spiel {
    namespace klondike {
        namespace {
            const GameType kGameType{
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
                {{"players", GameParameter(kDefaultPlayers)}}
            };

            std::shared_ptr<const Game> Factory(const GameParameters& params) {
                std::cout << "Factory" << std::endl;
                return std::shared_ptr<const Game>(new KlondikeGame(params));
            }

            REGISTER_SPIEL_GAME(kGameType, Factory);

        } // namespace

        // KlondikeState Methods =======================================================================================

        KlondikeState::KlondikeState(std::shared_ptr<const Game> game) :
            State(game),
            cur_player_(kChancePlayerId) {
            std::cout << "KlondikeState()" << std::endl;
        };

        int                 KlondikeState::CurrentPlayer() const {
            // IsTerminal -> kTerminalPlayerId
            // !is_setup  -> kChancePlayerId
            // else       -> player 0
            std::cout << "CurrentPlayer()" << std::endl;
            return 0;
        }

        std::string         KlondikeState::ActionToString(Player player, Action move) const {
            std::cout << "ActionToString()" << std::endl;
            std::string result = "ActionToString()";
            return result;
        }

        std::string         KlondikeState::ToString() const {
            std::cout << "ToString()" << std::endl;

            std::string result = "To String";

            /*
            // Deck
            absl::StrAppend(&result, "DECK: ");
            for (auto card : deck.cards) {
                absl::StrAppend(&result, card.rank, card.suit);
            }
            absl::StrAppend(&result, "\n");

            // Foundations
            absl::StrAppend(&result, "FOUNDATIONS: ");
            for (auto foundation : foundations) {
                for (auto card : foundation.cards) {
                    absl::StrAppend(&result, card.rank, card.suit);
                }
                absl::StrAppend(&result, "\n");
            }

            // Tableaus
            absl::StrAppend(&result, "TABLEAUS: ");
            for (auto tableau : tableaus) {
                for (auto card : tableau.cards) {
                    absl::StrAppend(&result, card.rank, card.suit);
                }
                absl::StrAppend(&result, "\n");
            }
            */

            return result;
        }

        std::string         KlondikeState::InformationStateString(Player player) const {
            // Information state is card then bets.
            std::cout << "InformationStateString()" << std::endl;
            std::string result = "InformationStateString()";
            return result;
        }

        std::string         KlondikeState::ObservationString(Player player) const {
            // Observation is card then contribution of each players to the pot.
            std::cout << "ObservationString()" << std::endl;
            std::string result = "ObservationString";
            return result;
        }

        void                KlondikeState::DoApplyAction(Action move) {
            // In a chance node, `move` should be the card to deal to the current underlying player.
            // On a player node, it should be ActionType::{kFold, kCall, kRaise}
            std::cout << "DoApplyAction()" << std::endl;
        }

        void                KlondikeState::InformationStateTensor(Player player, std::vector<double>* values) const {
            std::cout << "InformationStateTensor()" << std::endl;
        }

        void                KlondikeState::ObservationTensor(Player player, std::vector<double>* values) const {
            std::cout << "ObservationTensor()" << std::endl;
        }

        bool                KlondikeState::IsTerminal() const {
            std::cout << "IsTerminal()" << std::endl;
            return false;
        }

        std::unique_ptr<State>                  KlondikeState::Clone() const {
            std::cout << "Clone()" << std::endl;
            return std::unique_ptr<State>(new KlondikeState(*this));
        }

        std::unique_ptr<State>                  KlondikeState::ResampleFromInfostate(int player_id, std::function<double()> rng) const {
            std::cout << "ResampleFromInfostate()" << std::endl;
            std::unique_ptr<KlondikeState> clone = std::make_unique<KlondikeState>(game_);
            return clone;
        }

        std::vector<Action>                     KlondikeState::LegalActions() const {
            std::cout << "LegalActions()" << std::endl;
            return {1};
        }

        std::vector<double>                     KlondikeState::Returns() const {
            std::cout << "Returns()" << std::endl;
            return {};
        }

        std::vector<std::pair<Action, double>>  KlondikeState::ChanceOutcomes() const {
            std::cout << "ChanceOutcomes()" << std::endl;
            std::vector<std::pair<Action, double>> outcomes;
            return outcomes;
        }

        // KlondikeGame Methods ========================================================================================

        KlondikeGame::KlondikeGame(const GameParameters& params) :
            Game(kGameType, params),
            num_players_(ParameterValue<int>("players")) {
                std::cout << "KlondikeGame()" << std::endl;
                SPIEL_CHECK_GE(num_players_, kGameType.min_num_players);
                SPIEL_CHECK_LE(num_players_, kGameType.max_num_players);
            }

        std::unique_ptr<State> KlondikeGame::NewInitialState() const {
            std::cout << "NewInitialState()" << std::endl;
            return std::unique_ptr<State>(new KlondikeState(shared_from_this()));
        }

        std::vector<int> KlondikeGame::InformationStateTensorShape() const {
            // Usually `HistoryString` padded by max game length
            // However, max game length is unknown and potentially infinite if loops are allowed
            std::cout << "InformationStateTensorShape()" << std::endl;
            return {};
        }

        std::vector<int> KlondikeGame::ObservationTensorShape() const {
            // Deck, Waste, Foundations, Tableaus
            // 24 + 24 + 52 + 133 = 233
            std::cout << "ObservationTensorShape()" << std::endl;
            return {233};
        }

        double KlondikeGame::MaxUtility() const {
            // Sums of the total points that can be gained by three methods:
            // - Moving cards to foundation (10 - 100 pts, depending on rank)
            // - Moving cards from the waste (20 pts)
            // - Uncovering hidden cards in the tableau (20 pts)
            std::cout << "MaxUtility()" << std::endl;
            return 3220.0;
        }

        double KlondikeGame::MinUtility() const {
            // Going through the deck gives -20 each time past 3 rebuilds
            // However, total points have a floor at 0 to prevent going to negative infinity
            std::cout << "MinUtility()" << std::endl;
            return 0.0;
        }

    }  // namespace klondike
}  // namespace open_spiel


int main () {
    game = KlondikeGame();
    return 0;
}