// Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
                GameType::ChanceMode::kExplicitStochastic,
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

        KlondikeState::KlondikeState(std::shared_ptr<const Game> game) : State(game), current_player(kChancePlayerId) {};

        int                 KlondikeState::CurrentPlayer() const {
            // IsTerminal -> kTerminalPlayerId
            // !is_setup  -> kChancePlayerId
            // else       -> player 0
            return 0;
        }

        std::string         KlondikeState::ActionToString(Player player, Action move) const {
            std::string result = "ActionToString()";
            return result;
        }

        std::string         KlondikeState::ToString() const {
            std::string result = "ToString()";
            return result;
        }

        std::string         KlondikeState::InformationStateString(Player player) const {
            // Information state is card then bets.
            std::string result = "InformationStateString()";
            return result;
        }

        std::string         KlondikeState::ObservationString(Player player) const {
            // Observation is card then contribution of each players to the pot.
            std::string result = "ObservationString";
            return result;
        }

        void                KlondikeState::DoApplyAction(Action move) {
            // In a chance node, `move` should be the card to deal to the current underlying player.
            // On a player node, it should be ActionType::{kFold, kCall, kRaise}
            std::cout << "KlondikeState::DoApplyAction()" << std::endl;
        }

        void                KlondikeState::InformationStateTensor(Player player, std::vector<double>* values) const {
            std::cout << "KlondikeState::InformationStateTensor()" << std::endl;
        }

        void                KlondikeState::ObservationTensor(Player player, std::vector<double>* values) const {
            std::cout << "KlondikeState::ObservationTensor()" << std::endl;
            return {1}
        }

        bool                KlondikeState::IsTerminal() const {
            std::cout << "KlondikeState::IsTerminal()" << std::endl;
            return false;
        }

        std::unique_ptr<State>                  KlondikeState::Clone() const {
            std::cout << "KlondikeState::Clone()" << std::endl;
            return std::unique_ptr<State>(new KlondikeState(*this));
        }

        std::unique_ptr<State>                  KlondikeState::ResampleFromInfostate(int player_id, std::function<double()> rng) const {
            std::cout << "KlondikeState::ResampleFromInfostate()" << std::endl;
            std::unique_ptr<KlondikeState> clone = std::make_unique<KlondikeState>(game_);
            return clone;
        }

        std::vector<Action>                     KlondikeState::LegalActions() const {
            std::cout << "KlondikeState::LegalActions()" << std::endl;
            return {1};
        }

        std::vector<double>                     KlondikeState::Returns() const {
            std::cout << "KlondikeState::Returns()" << std::endl;
            return {};
        }

        std::vector<std::pair<Action, double>>  KlondikeState::ChanceOutcomes() const {
            std::cout << "KlondikeState::ChanceOutcomes()" << std::endl;
            std::vector<std::pair<Action, double>> outcomes;
            return outcomes;
        }

        // KlondikeGame Methods ========================================================================================

        KlondikeGame::KlondikeGame(const GameParameters& params) :
            Game(kGameType, params),
            num_players_(ParameterValue<int>("players")) {
            // Empty
        }

        std::unique_ptr<State> KlondikeGame::NewInitialState() const {
            std::cout << "KlondikeGame::NewInitialState()" << std::endl;
            return std::unique_ptr<State>(new KlondikeState(shared_from_this()));
        }

        std::vector<int> KlondikeGame::InformationStateTensorShape() const {
            // Usually `HistoryString` padded by max game length
            // However, max game length is unknown and potentially infinite if loops are allowed
            return {};
        }

        std::vector<int> KlondikeGame::ObservationTensorShape() const {
            // Deck, Waste, Foundations, Tableaus
            // 24 + 24 + 52 + 133 = 233
            std::cout << "KlondikeGame::ObservationTensorShape()" << std::endl;
            return {233};
        }

        double KlondikeGame::MaxUtility() const {
            // Sums of the total points that can be gained by three methods:
            // - Moving cards to foundation (10 - 100 pts, depending on rank)
            // - Moving cards from the waste (20 pts)
            // - Uncovering hidden cards in the tableau (20 pts)
            return 3220.0;
        }

        double KlondikeGame::MinUtility() const {
            // Going through the deck gives -20 each time past 3 rebuilds
            // However, total points have a floor at 0 to prevent going to negative infinity
            return 1.0;
        }

    }  // namespace klondike
}  // namespace open_spiel
