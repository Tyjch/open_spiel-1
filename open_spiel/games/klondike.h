#ifndef THIRD_PARTY_OPEN_SPIEL_GAMES_KLONDIKE_H_
#define  THIRD_PARTY_OPEN_SPIEL_GAMES_KLONDIKE_H_

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "open_spiel/spiel.h"

namespace open_spiel {
    namespace klondike {
        inline constexpr int kInvalidCard    = -10000;
        inline constexpr int kDefaultPlayers = 1;

        class KlondikeGame;

        enum ActionType {
            kSetup = 0,
            kEnd   = 1,
            kDraw  = 2,

            // TABLEAU MOVES
            // Spades
            k2sAh    = 3,
            k2sAd    = 4,
            k3s2h    = 5,
            k3s2d    = 6,
            k4s3h    = 7,
            k4s3d    = 8,
            k5s4h    = 9,
            k5s4d    = 10,
            k6s5h    = 11,
            k6s5d    = 12,
            k7s6h    = 13,
            k7s6d    = 14,
            k8s7h    = 15,
            k8s7d    = 16,
            k9s8h    = 17,
            k9s8d    = 18,
            kTs9h    = 19,
            kTs9d    = 20,
            kJsTh    = 21,
            kJsTd    = 22,
            kQsJh    = 23,
            kQsJd    = 24,
            kKsQh    = 25,
            kKsQd    = 26,
            kEmptyKs = 27,

            // Hearts
            k2hAs    = 28,
            k2hAc    = 29,
            k3h2s    = 30,
            k3h2c    = 31,
            k4h3s    = 32,
            k4h3c    = 33,
            k5h4s    = 34,
            k5h4c    = 35,
            k6h5s    = 36,
            k6h5c    = 37,
            k7h6s    = 38,
            k7h6c    = 39,
            k8h7s    = 40,
            k8h7c    = 41,
            k9h8s    = 42,
            k9h8c    = 43,
            kTh9s    = 44,
            kTh9c    = 45,
            kJhTs    = 46,
            kJhTc    = 47,
            kQhJs    = 48,
            kQhJc    = 49,
            kKhQs    = 50,
            kKhQc    = 51,
            kEmptyKh = 52,

            // Clubs
            k2cAh    = 53,
            k2cAd    = 54,
            k3c2h    = 55,
            k3c2d    = 56,
            k4c3h    = 57,
            k4c3d    = 58,
            k5c4h    = 59,
            k5c4d    = 60,
            k6c5h    = 61,
            k6c5d    = 62,
            k7c6h    = 63,
            k7c6d    = 64,
            k8c7h    = 65,
            k8c7d    = 66,
            k9c8h    = 67,
            k9c8d    = 68,
            kTc9h    = 69,
            kTc9d    = 70,
            kJcTh    = 71,
            kJcTd    = 72,
            kQcJh    = 73,
            kQcJd    = 74,
            kKcQh    = 75,
            kKcQd    = 76,
            kEmptyKc = 77,

            // Diamonds
            k2dAs    = 78,
            k2dAc    = 79,
            k3d2s    = 80,
            k3d2c    = 81,
            k4d3s    = 82,
            k4d3c    = 83,
            k5d4s    = 84,
            k5d4c    = 85,
            k6d5s    = 86,
            k6d5c    = 87,
            k7d6s    = 88,
            k7d6c    = 89,
            k8d7s    = 90,
            k8d7c    = 91,
            k9d8s    = 92,
            k9d8c    = 93,
            kTd9s    = 94,
            kTd9c    = 95,
            kJdTs    = 96,
            kJdTc    = 97,
            kQdJs    = 98,
            kQdJc    = 99,
            kKdQs    = 100,
            kKdQc    = 101,
            kEmptyKd = 102,

            // FOUNDATION MOVES
            // Spades
            kEmptyAs = 103,
            kAs2s    = 104,
            k2s3s    = 105,
            k3s4s    = 106,
            k4s5s    = 107,
            k5s6s    = 108,
            k6s7s    = 109,
            k7s8s    = 110,
            k8s9s    = 111,
            k9sTs    = 112,
            kTsJs    = 113,
            kJsQs    = 114,
            kQsKs    = 115,

            // Hearts
            kEmptyAh = 116,
            kAh2h    = 117,
            k2h3h    = 118,
            k3h4h    = 119,
            k4h5h    = 120,
            k5h6h    = 121,
            k6h7h    = 122,
            k7h8h    = 123,
            k8h9h    = 124,
            k9hTh    = 125,
            kThJh    = 126,
            kJhQh    = 127,
            kQhKh    = 128,

            // Clubs
            kEmptyAc = 129,
            kAc2c    = 130,
            k2c3c    = 131,
            k3c4c    = 132,
            k4c5c    = 133,
            k5c6c    = 134,
            k6c7c    = 135,
            k7c8c    = 136,
            k8c9c    = 137,
            k9cTc    = 138,
            kTcJc    = 139,
            kJcQc    = 140,
            kQcKc    = 141,

            // Diamonds
            kEmptyAd = 142,
            kAd2d    = 143,
            k2d3d    = 144,
            k3d4d    = 145,
            k4d5d    = 146,
            k5d6d    = 147,
            k6d7d    = 148,
            k7d8d    = 149,
            k8d9d    = 150,
            k9dTd    = 151,
            kTdJd    = 152,
            kJdQd    = 153,
            kQdKd    = 154

        };

        class KlondikeState : public State {
        public:
            explicit                KlondikeState(std::shared_ptr<const Game> game);
            Player                  CurrentPlayer()                                                     const override;
            std::string             ActionToString(Player player, Action move)                          const override;
            std::string             ToString()                                                          const override;
            std::string             InformationStateString(Player player)                               const override;
            std::string             ObservationString(Player player)                                    const override;
            std::unique_ptr<State>  Clone()                                                             const override;
            std::unique_ptr<State>  ResampleFromInfostate(int player_id, std::function<double()> rng)   const override;
            bool                    IsTerminal()                                                        const override;
            void                    InformationStateTensor(Player player, std::vector<double>* values)  const override;
            void                    ObservationTensor(Player player, std::vector<double>* values)       const override;

            std::vector<std::pair<Action, double>>  ChanceOutcomes()                                    const override;
            std::vector<double>                     Returns()                                           const override;
            std::vector<Action>                     LegalActions()                                      const override;

        protected:
            void DoApplyAction(Action move) override;

        private:
            int current_player;
        };

        class KlondikeGame  : public Game {
        public:
            explicit KlondikeGame(const GameParameters& params);

            int                     NumDistinctActions()    const override {
                return 155;
            }
            int                     MaxGameLength()         const override {
                return 5;
            }
            int                     MaxChanceOutcomes()     const override {
                return 1;
            }
            int                     NumPlayers()            const override {
                return num_players_;
            }
            double                  UtilitySum()            const override {
                return 0;
            }
            double                  MinUtility()            const override;
            double                  MaxUtility()            const override;

            std::unique_ptr<State>      NewInitialState()   const override;
            std::shared_ptr<const Game> Clone()             const override {
                return std::shared_ptr<const Game>(new KlondikeGame(*this));
            }
            std::vector<int> InformationStateTensorShape()  const override;
            std::vector<int> ObservationTensorShape()       const override;

        private:
            int num_players_;  // Number of players.
        };

    }  // namespace klondike
}  // namespace open_spiel

#endif  // THIRD_PARTY_OPEN_SPIEL_GAMES_KLONDIKE_H_
