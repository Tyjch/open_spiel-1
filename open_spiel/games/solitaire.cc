#include "solitaire.h"

#include <deque>
#include <numeric>
#include <random>
#include <algorithm>
#include <math.h>

#include "open_spiel/abseil-cpp/absl/strings/str_format.h"
#include "open_spiel/abseil-cpp/absl/strings/str_join.h"
#include "open_spiel/game_parameters.h"
#include "open_spiel/spiel_utils.h"

#define RED    "\033[31m"
#define WHITE  "\033[37m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

namespace open_spiel::solitaire {

    namespace {
        const GameType kGameType {
                "solitaire",
                "Solitaire",
                GameType::Dynamics::kSequential,
                GameType::ChanceMode::kSampledStochastic,   // TODO: Should this be explicit stochastic?
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

        std::shared_ptr<const Game> Factory(const GameParameters & params) {
            return std::shared_ptr<const Game>(new SolitaireGame(params));
        }

        REGISTER_SPIEL_GAME(kGameType, Factory);
    }

    // Miscellaneous Functions =========================================================================================

    std::vector<std::string> GetOppositeSuits(const std::string & suit) {
        if (suit == "s" or suit == "c") {
            return {"h", "d"};
        } else if (suit == "h" or suit == "d") {
            return {"s", "c"};
        } else {
            // TODO: Raise an error here
        }
    }

    // Card Methods ====================================================================================================

    Card::Card() : rank(""), suit(""), hidden(true), location(kMissing) {};

    Card::Card(std::string rank, std::string suit) : rank(rank), suit(suit), hidden(true), location(kMissing) {};

    Card::Card(int index) : hidden(false), location(kMissing) {
        int rank_value = index % 13;
        int suit_value = floor(index / 13);
        rank = RANKS.at(rank_value);
        suit = SUITS.at(suit_value);
    }

    Card::operator int() const {
        int rank_value = GetIndex(RANKS, rank);
        int suit_value = GetIndex(SUITS, suit);
        return 13 * suit_value + rank_value;
    }

    bool Card::operator==(Card & other_card) const {
        return rank == other_card.rank and suit == other_card.suit;
    }

    bool Card::operator==(const Card & other_card) const {
        return rank == other_card.rank and suit == other_card.suit;
    }

    std::vector<Card> Card::LegalChildren() const {
        std::vector<Card>        legal_children = {};
        std::string              child_rank;
        std::vector<std::string> child_suits;

        switch (location) {
            case kTableau:
                if (rank != "A") {
                    child_rank  = RANKS.at(GetIndex(RANKS, rank) - 1);
                    child_suits = GetOppositeSuits(suit);
                }
                break;

            case kFoundation:
                if (rank != "K") {
                    child_rank  = RANKS.at(GetIndex(RANKS, rank) + 1);
                    child_suits = {suit};
                }
                break;

            default:
                return legal_children;
        }

        for (auto child_suit : child_suits) {
            legal_children.emplace_back(Card(child_rank, child_suit));
        }

        return legal_children;
    }

    std::string Card::ToString() const {
        if (hidden) {
            return "[] ";
        } else {
            std::string result;
            if (suit == "s" or suit == "c") {
                absl::StrAppend(&result, WHITE, rank, suit, RESET, " ");
            } else if (suit == "h" or suit == "d") {
                absl::StrAppend(&result, RED, rank, suit, RESET, " ");
            }
            return result;
        }
    }


    // Deck Methods ====================================================================================================

    Deck::Deck() {
        for (int i = 1; i <= 24; i++) {
            cards.emplace_back();
        }
        for (auto & card : cards) {
            card.location = kDeck;
        }
    }

    void Deck::draw(unsigned long num_cards) {
        std::deque<Card> drawn_cards;
        num_cards = std::min(num_cards, cards.size());

        int i = 1;
        while (i <= num_cards) {
            auto card = cards.front();
            card.location = kWaste;
            drawn_cards.push_back(card);
            cards.pop_front();
            i++;
        }

        waste.insert(waste.begin(), drawn_cards.begin(), drawn_cards.end());
    }

    void Deck::rebuild() {
        if (cards.empty()) {
            for (const Card & card : initial_order) {
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

    // Foundation Methods ==============================================================================================

    Foundation::Foundation(std::string suit) : suit(suit) {
        cards = {};
    };

    std::vector<Card> Foundation::Sources() const {
        if (not cards.empty()) {
            return {cards.back()};
        } else {
            return {};
        }
    }

    std::vector<Card> Foundation::Targets() const {
        if (not cards.empty()) {
            return {cards.back()};
        } else {
            return {};
        }
    }

    std::vector<Card> Foundation::Split(Card card) {
        std::vector<Card> split_cards;
        if (cards.back() == card) {
            split_cards = {cards.back()};
            cards.pop_back();
            return split_cards;
        }
    }

    void Foundation::Extend(std::vector<Card> source_cards) {
        for (auto card : source_cards) {
            cards.push_back(card);
        }
    }

    // Tableau Methods =================================================================================================

    Tableau::Tableau(int num_cards) {

        for (int i = 1; i <= num_cards; i++) {
            cards.emplace_back();
        }
        for (auto & card : cards) {
            card.location = kTableau;
        }
    };

    std::vector<Card> Tableau::Sources() const {
        if (not cards.empty()) {
            std::vector<Card> sources;
            for (auto & card : cards) {
                if (not card.hidden) {
                    sources.push_back(card);
                }
            }
            return sources;
        } else {
            return {};
        }
    }

    std::vector<Card> Tableau::Targets() const {
        if (not cards.empty()) {
            return {cards.back()};
        } else {
            return {};
        }
    }

    std::vector<Card> Tableau::Split(Card card) {
        std::vector<Card> split_cards;
        if (not cards.empty()) {
            bool split_flag = false;
            for (auto it = cards.begin(); it != cards.end(); it++) {
                if (*it == card) {
                    split_flag = true;
                }
                if (split_flag) {
                    split_cards.push_back(*it);
                    it = cards.erase(it);
                }
            }
        }
        return split_cards;
    }

    void Tableau::Extend(std::vector<Card> source_cards) {
        for (auto card : source_cards) {
            cards.push_back(card);
        }
    }

    // SolitaireState Methods ==========================================================================================

    SolitaireState::SolitaireState(std::shared_ptr<const Game> game) :
        State(game),
        deck(),
        foundations(),
        tableaus() {
            is_setup = false;
        };

    // Overriden Methods -----------------------------------------------------------------------------------------------

    Player                  SolitaireState::CurrentPlayer() const {
        // NOTE: 2nd method implemented

        // There are only two players in this game: chance and player 1.
        if (IsChanceNode()) {
            // Index of the chance player
            return kChancePlayerId;
        } else {
            // Index of the player
            return 0;
        }
    }

    std::unique_ptr<State>  SolitaireState::Clone() const {
        return std::unique_ptr<State>(new SolitaireState(*this));
    }

    bool                    SolitaireState::IsTerminal() const {
        return false;
    }

    bool                    SolitaireState::IsChanceNode() const {
        // NOTE: 1st method implemented

        if (not is_setup) {
            // If setup is not started, this is a chance node
            return true;
        } else {
            // If there is a hidden card on the top of a tableau, this is a chance ndoe
            for (auto & tableau : tableaus) {
                if (tableau.cards.back().hidden) {
                    return true;
                }
            }
            // If any card in the waste is hidden, this is a chance node
            for (auto & card : deck.waste) {
                if (card.hidden) {
                    return true;
                }
            }
            // Otherwise, this is node a chance node; it's a decision node
            return false;
        }
    }

    std::string             SolitaireState::ToString() const {
        // NOTE: 3rd method implemented
        // TODO: Create methods that allow casting to std::string for card, deck, waste, foundation, and tableau

        std::string result;

        absl::StrAppend(&result, "DECK        : ");
        for (const Card & card : deck.cards) {
            absl::StrAppend(&result, card.ToString());
        }

        absl::StrAppend(&result, "\nWASTE       : ");
        for (const Card & card : deck.waste) {
            absl::StrAppend(&result, card.ToString());
        }

        absl::StrAppend(&result, "\nORDER       : ");
        for (const Card & card : deck.initial_order) {
            absl::StrAppend(&result, card.ToString());
        }

        absl::StrAppend(&result, "\nFOUNDATIONS : ");
        for (const Foundation & foundation : foundations) {
            if (not foundation.cards.empty()) {
                absl::StrAppend(&result, "\n");
                for (const Card & card : foundation.cards) {
                    absl::StrAppend(&result, card.ToString());
                }
            }
        }

        absl::StrAppend(&result, "\nTABLEAUS    : ");
        for (const Tableau & tableau : tableaus) {
            if (not tableau.cards.empty()) {
                absl::StrAppend(&result, "\n");
                for (const Card & card : tableau.cards) {
                    absl::StrAppend(&result, card.ToString());
                }
            }
        }

        return result;
    }

    std::string             SolitaireState::ActionToString(Player player, Action action_id) const {
        switch (action_id) {
            case (0) : {
                return "kStartSetup";
                break;
            }
            case 1 ... 52 : {
                return "kReveal";
                break;
            }
            case (53) : {
                return "kDraw";
                break;
            }
            default : {
                return "kMissingAction";
                break;
            }
        }


    }

    std::string             SolitaireState::InformationStateString(Player player) const {
        return "Information State String";
    }

    std::string             SolitaireState::ObservationString(Player player) const {
        return "Observation String";
    }

    void                    SolitaireState::InformationStateTensor(Player player, std::vector<double> *values) const {

    }

    void                    SolitaireState::ObservationTensor(Player player, std::vector<double> *values) const {

    }

    void                    SolitaireState::DoApplyAction(Action move) {
        std::cout << "Inside DoApplyAction()" << std::endl;
        std::cout << "Action = " << move << std::endl;

        // Handles kSetup
        if (move == kSetup) {
            for (int i = 1; i <= 7; i++) {
                tableaus.emplace_back(i);
            }
            is_setup = true;
        }

        // Handles kReveal
        else if (1 <= move and move <= 52) {
            // Get the rank and suit to reveal, cards start at 0 instead of 1
            // which is why we subtract 1 to move here.

            Card revealed_card = Card(move - 1);
            bool found_hidden_card = false;

            // Find the first hidden card in the tableaus
            for (auto & tableau : tableaus) {
                if (tableau.cards.back().hidden) {
                    tableau.cards.back().rank = revealed_card.rank;
                    tableau.cards.back().suit = revealed_card.suit;
                    tableau.cards.back().hidden = false;
                    found_hidden_card = true;
                    break;
                }
            }

            // Find the first hidden card in the waste
            if (not found_hidden_card) {
                for (auto & card : deck.waste) {
                    if (card.hidden) {
                        card.rank = revealed_card.rank;
                        card.suit = revealed_card.suit;
                        card.hidden = false;
                        deck.initial_order.push_back(card);
                        break;
                    }
                }
            }

            // Add move to revealed cards so we don't try to reveal it again
            revealed_cards.push_back(move);
        }

        // Handles kDraw
        else if (move == kDraw) {
            if (deck.cards.empty()) {
                deck.rebuild();
            }
            deck.draw(3);
        }
    }

    std::vector<double>     SolitaireState::Returns() const {
        return {0.0};
    }

    std::vector<double>     SolitaireState::Rewards() const {
        return {0.0};
    }

    std::vector<Action>     SolitaireState::LegalActions() const {
        return {kDraw};
    }

    std::vector<std::pair<Action, double>> SolitaireState::ChanceOutcomes() const {
        if (!is_setup) {
            return {{kSetup, 1.0}};
        } else {
            std::vector<std::pair<Action, double>> outcomes;
            const double p = 1.0 / (52 - revealed_cards.size());

            for (int i = 1; i <= 52; i++) {
                if (std::find(revealed_cards.begin(), revealed_cards.end(), i) != revealed_cards.end()) {
                    continue;
                } else {
                    outcomes.emplace_back(i, p);
                }
            }
            return outcomes;
        }
    }

    // Other Methods ---------------------------------------------------------------------------------------------------

    std::vector<Card>       SolitaireState::Targets(const std::string & location) const {
        // TODO
    }

    std::vector<Card>       SolitaireState::Sources(const std::string & location) const {
        // TODO
    }

    std::vector<Action>     SolitaireState::CandidateActions() const {
        // TODO
    }

    void                    SolitaireState::MoveCards(Move & move) {

    }


















    // SolitaireGame Methods ===========================================================================================

    SolitaireGame::SolitaireGame(const GameParameters & params) :
        Game(kGameType, params),
        num_players_(ParameterValue<int>("players")) {

    };

    int     SolitaireGame::NumDistinctActions() const {
        return 155;
    }

    int     SolitaireGame::MaxGameLength() const {
        return 5;
    }

    int     SolitaireGame::NumPlayers() const {
        return 1;
    }

    double  SolitaireGame::MinUtility() const {
        return 0.0;
    }

    double  SolitaireGame::MaxUtility() const {
        return 3220.0;
    }

    std::vector<int> SolitaireGame::InformationStateTensorShape() const {
        return {200};
    }

    std::vector<int> SolitaireGame::ObservationTensorShape() const {
        return {233};
    }

    std::unique_ptr<State> SolitaireGame::NewInitialState() const {
        return std::unique_ptr<State>(new SolitaireState(shared_from_this()));
    }

    std::shared_ptr<const Game> SolitaireGame::Clone() const {
        return std::shared_ptr<const Game>(new SolitaireGame(*this));
    }


} // namespace open_spiel::solitaire

