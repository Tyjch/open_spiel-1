#include "klondike.h"

#include <deque>
#include <numeric>
#include <random>
#include <algorithm>
// #include <array>
// #include <utility>

#include "open_spiel/abseil-cpp/absl/strings/str_format.h"
#include "open_spiel/abseil-cpp/absl/strings/str_join.h"
#include "open_spiel/game_parameters.h"
#include "open_spiel/spiel_utils.h"

#define RED    "\033[31m"
#define WHITE  "\033[37m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

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

    // Card Methods ====================================================================================================

    Card::Card(std::string rank, std::string suit, bool hidden) : rank(rank), suit(suit), hidden(hidden) {}

    bool Card::operator==(Card other_card) {
        return rank == other_card.rank and suit == other_card.suit;
    }

    // Deck Methods ====================================================================================================

    Deck::Deck() {
        for (auto suit : SUITS) {
            for (auto rank : RANKS) {
                Card current_card = Card {rank, suit, true};
                cards.push_back(current_card);
            }
        }
    }

    void Deck::shuffle(int seed) {
        if (!is_shuffled) {
            std::shuffle(cards.begin(), cards.end(), std::default_random_engine(seed));
            is_shuffled = true;
            initial_order = cards;
        } else {
            std::cout << YELLOW << "WARNING: Deck is already shuffled" << RESET << std::endl;
        }
    }

    void Deck::draw(int num_cards=3) {
        std::deque<Card> dealt_cards = deal(num_cards);
        for (Card & card : dealt_cards) {
            card.hidden = false;
        }
        waste.insert(waste.begin(), dealt_cards.begin(), dealt_cards.end());
    }

    void Deck::rebuild() {
        if (cards.empty()) {
            for (Card card : initial_order) {
                if (std::find(waste.begin(), waste.end(), card) != waste.end()) {
                    cards.push_back(card);
                }
            }
            waste.clear();
            times_rebuilt += 1;
        } else {
            std::cout << YELLOW << "WARNING: Cannot rebuild a non-empty deck" << RESET << std::endl;
        }
    }

    std::deque<Card> Deck::deal (unsigned long int num_cards) {
        std::deque<Card> dealt_cards;
        num_cards = std::min(num_cards, cards.size());
        int i = 0;
        while (i < num_cards) {
            Card card = cards.front();
            dealt_cards.push_back(card);
            cards.pop_front();
            i++;
        }
        return dealt_cards;
    }

    // Foundation Methods ==============================================================================================

    Foundation::Foundation(char suit) : suit(suit) {};

    // Tableau Methods =================================================================================================

    Tableau::Tableau() {
        cards = {};
    }

    Tableau::Tableau(std::deque<Card> provided_cards) {
        cards = provided_cards;
        for (Card & card : cards) {
            card.hidden = true;
        }
        cards.back().hidden = false;
    }

    // KlondikeState Methods ===========================================================================================

    KlondikeState::KlondikeState(std::shared_ptr<const Game> game) :
        State(game),
        cur_player_(kChancePlayerId),
        is_setup_(false),
        deck(),
        foundations(),
        tableaus() {
        // Empty
    };

    Player                  KlondikeState::CurrentPlayer() const {
        if (!is_setup_) {
            return kChancePlayerId;
        } else {
            return 0;
        }
    }

    std::vector<Action>     KlondikeState::LegalActions() const {
        return std::vector<Action>();
    }

    std::string             KlondikeState::ActionToString(Player player, Action action_id) const {
        return std::string();
    };

    std::string             KlondikeState::ToString() const {
        std::string result;
        absl::StrAppend(&result, "Deck:");
        for (Card card : deck.cards) {
            absl::StrAppend(&result, card.rank, card.suit);
        }

    }

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