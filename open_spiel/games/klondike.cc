#include "klondike.h"

#include <deque>
#include <numeric>
#include <random>
#include <algorithm>
#include <math.h>
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


// TODO: Handle moves to empty piles for aces and kings
// TODO: Implement draw moves


// TODO: I left off by "fixing" LegalActions, trying to prevent loops by implementing
// IsReversible. Still have loops though.


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

    // Miscellaneous Methods ===========================================================================================

    std::vector<std::string>    GetOppositeSuits(const std::string & suit) {
        if (suit == "s" or suit == "c") {
            return {"h", "d"};
        } else if (suit == "h" or suit == "d") {
            return {"s", "c"};
        }
    }

    std::vector<Card>           GetAllowableChildren(const Card & card, const std::string & location) {

        std::string child_rank;
        std::vector<std::string> child_suits;
        std::vector<Card> allowable_children;

        if (location == "tableau") {
            if (card.rank == "A") {
                return {};  // In a tableau, aces have no children
            } else {
                // In a tableau, children have the opposite color suit and are one rank lower
                child_rank  = TABLEAU_CHILD_RANK.at(card.rank);
                child_suits = GetOppositeSuits(card.suit);
            }

        } else if (location == "foundation") {
            if (card.rank == "K") {
                return {};  // In a foundation, kings have no children
            } else {
                // In a foundation, children have the same suit and are one rank higher
                child_rank  = FOUNDATION_CHILD_RANK.at(card.rank);
                child_suits = {card.suit};
            }
        }

        for (const auto & suit : child_suits) {
            allowable_children.push_back(Card {child_rank, suit, false});
        }

        return allowable_children;

    }

    Action                      GetActionFromMove(const Card & target, const Card & source) {
        // TODO: Handle special moves
        int target_int = (int) target;
        int source_int = (int) source;
        return target_int * 100 + source_int + 52;
    }

    std::pair<Card, Card>       GetMoveFromAction(const Action & action) {
        // TODO: Handle special moves -> {1252, 2552, 3852, 5152, 52, 1352, 2652, 3952}
        int source_int = (action - 52) % 100;
        int target_int = (action - source_int) / 100;
        return std::make_pair(Card(target_int), Card(source_int));
    }

    // Rendering Methods ===============================================================================================

    void RenderPile(std::deque<Card> pile) {
        for (Card & card : pile) {
            std::cout << card.rank << card.suit << " ";
        }
        std::cout << std::endl;
    }

    void RenderPile(std::vector<Card> pile) {
        for (Card & card : pile) {
            std::cout << card.rank << card.suit << " ";
        }
        std::cout << std::endl;
    }

    void RenderPair(const std::pair<Card, Card> & pair) {
        std::cout << "(" << pair.first.rank << pair.first.suit << ", ";
        std::cout << pair.second.rank << pair.second.suit << ")" << std::endl;
    }

    void RenderCard(Card card) {
        std::cout << card.rank << card.suit << " ";
    }

    // Card Methods ====================================================================================================

    Card::Card(std::string rank, std::string suit, bool hidden) : rank(rank), suit(suit), hidden(hidden) {}

    Card::Card(int card_index) : hidden(false) {
        int rank_value = card_index % 13;
        int suit_value = floor((card_index / 13));
        rank = VALUES_TO_RANK.at(rank_value);
        suit = VALUES_TO_SUIT.at(suit_value);
    }

    Card::operator int() const {
        // Gets an integer representing the card
        int rank_value = RANK_VALUES.at(rank);
        int suit_value = SUIT_VALUES.at(suit);
        return 13 * suit_value + rank_value;
    }

    bool Card::operator==(const Card & other_card) const {
        return rank == other_card.rank and suit == other_card.suit;
    }

    bool Card::operator==(Card & other_card) const {
        return rank == other_card.rank and suit == other_card.suit;
    }

    // Deck Methods ====================================================================================================

    Deck::Deck() {
        for (const auto & suit : SUITS) {
            for (const auto & rank : RANKS) {
                Card current_card = Card {rank, suit, true};
                cards.push_back(current_card);
            }
        }
    }

    void Deck::draw(int num_cards=3) {
        std::deque<Card> dealt_cards = deal(num_cards);
        for (Card & card : dealt_cards) {card.hidden = false;}
        waste.insert(waste.begin(), dealt_cards.begin(), dealt_cards.end());
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

    Foundation::Foundation(std::string suit) : suit(suit) {};

    bool Foundation::operator==(Foundation & other_foundation) const {
        return cards == other_foundation.cards;
    }

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
        setup_counter_(0),
        score_(0.0),
        previous_score(0.0),
        deck(),
        foundations(),
        tableaus(),
        last_move_was_reversible(false) {};

    Player                  KlondikeState::CurrentPlayer() const {

        // Chance Player handles choosing order of deck and dealing it into the initial game state
        if (!is_setup_) { return kChancePlayerId; }

        // Player 0 handles all decision nodes after initial state has been set
        else { return 0; }

    }

    std::string             KlondikeState::ActionToString(Player player, Action action_id) const {

        std::string result;

        if (0 <= action_id and action_id <= 51) {
            Card card = Card {action_id};
            absl::StrAppend(&result, "kSet", card.rank, card.suit);
        }
        else if (!is_setup_) {
            absl::StrAppend(&result, "kSetup");
        }
        else if (action_id == kDraw) {
            absl::StrAppend(&result, "kDraw");
        }
        else if (special_moves.count(action_id) == 1) {
            int card_index = (action_id - 52) / 100;
            Card special_card = Card(card_index);
            absl::StrAppend(&result, "kMove", special_card.rank, special_card.suit);
        }
        else {
            std::pair<Card, Card> move = GetMoveFromAction(action_id);
            absl::StrAppend(&result, "kMove", move.first.rank, move.first.suit, move.second.rank, move.second.suit);
        }

        return result;
    };

    std::string             KlondikeState::ToString() const {

        std::string result;

        absl::StrAppend(&result, "DECK  : ");
        for (const Card & card : deck.cards) {
            if (card.hidden) {
                absl::StrAppend(&result, "[] ");
            } else {
                if (card.suit == "s" or card.suit == "c") {
                    absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
                } else if (card.suit == "h" or card.suit == "d") {
                    absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
                }
            }
        }

        absl::StrAppend(&result, "\nWASTE : ");
        for (const Card & card : deck.waste) {
            if (card.suit == "s" or card.suit == "c") {
                absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
            } else if (card.suit == "h" or card.suit == "d") {
                absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
            }
        }

        absl::StrAppend(&result, "\n\nFOUNDATIONS : ");
        for (const Foundation & foundation : foundations) {
            if (!foundation.cards.empty()) { absl::StrAppend(&result, "\n"); }
            for (const Card & card : foundation.cards) {
                if (card.suit == "s" or card.suit == "c") {
                    absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
                } else if (card.suit == "h" or card.suit == "d") {
                    absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
                }
            }
        }

        absl::StrAppend(&result, "\n\nTABLEAUS : ");
        for (const Tableau & tableau : tableaus) {
            if (!tableau.cards.empty()) { absl::StrAppend(&result, "\n"); }
            for (const Card & card : tableau.cards) {
                if (card.hidden) {
                    absl::StrAppend(&result, "[] ");
                } else {
                    if (card.suit == "s" or card.suit == "c") {
                        absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
                    } else if (card.suit == "h" or card.suit == "d") {
                        absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
                    }
                }
            }
        }

        absl::StrAppend(&result, "\n\nTARGETS : ");
        for (const Card & card : Targets()) {
            if (card.suit == "s" or card.suit == "c") {
                absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
            } else if (card.suit == "h" or card.suit == "d") {
                absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
            }
        }

        absl::StrAppend(&result, "\nSOURCES : ");
        for (const Card & card : Sources()) {
            if (card.suit == "s" or card.suit == "c") {
                absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
            } else if (card.suit == "h" or card.suit == "d") {
                absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
            }
        }

        return result;
    }

    std::string             KlondikeState::InformationStateString(Player player) const {
        // TODO: I don't know if this is correct
        // Sequence of actions that generate the current information state
        return HistoryString();
    }

    std::string             KlondikeState::ObservationString(Player player) const {
        // TODO: I don't know if this is correct
        //SPIEL_CHECK_GE(player, 0);
        //SPIEL_CHECK_LT(player, num_players_);

        std::string result;

        absl::StrAppend(&result, "DECK  : ");
        for (const Card & card : deck.cards) {
            if (card.hidden) {
                absl::StrAppend(&result, "[] ");
            } else {
                if (card.suit == "s" or card.suit == "c") {
                    absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
                } else if (card.suit == "h" or card.suit == "d") {
                    absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
                }
            }
        }

        absl::StrAppend(&result, "\nWASTE : ");
        for (const Card & card : deck.waste) {
            if (card.suit == "s" or card.suit == "c") {
                absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
            } else if (card.suit == "h" or card.suit == "d") {
                absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
            }
        }

        absl::StrAppend(&result, "\n\nFOUNDATIONS : ");
        for (const Foundation & foundation : foundations) {
            if (!foundation.cards.empty()) { absl::StrAppend(&result, "\n"); }
            for (const Card & card : foundation.cards) {
                if (card.suit == "s" or card.suit == "c") {
                    absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
                } else if (card.suit == "h" or card.suit == "d") {
                    absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
                }
            }
        }

        absl::StrAppend(&result, "\n\nTABLEAUS : ");
        for (const Tableau & tableau : tableaus) {
            if (!tableau.cards.empty()) { absl::StrAppend(&result, "\n"); }
            for (const Card & card : tableau.cards) {
                if (card.hidden) {
                    absl::StrAppend(&result, "[] ");
                } else {
                    if (card.suit == "s" or card.suit == "c") {
                        absl::StrAppend(&result, WHITE, card.rank, card.suit, RESET, " ");
                    } else if (card.suit == "h" or card.suit == "d") {
                        absl::StrAppend(&result, RED, card.rank, card.suit, RESET, " ");
                    }
                }
            }
        }

        return result;
    }

    std::string             KlondikeState::GetContainerType(Card card_to_find) const {

        std::string location = "missing";

        for (auto & tableau : tableaus) {
            if (!tableau.cards.empty()) {
                if (std::find(tableau.cards.begin(), tableau.cards.end(), card_to_find) != tableau.cards.end()) {
                    location = "tableau";
                }
            }
        }

        for (auto & foundation : foundations) {
            if (!foundation.cards.empty()) {
                if (std::find(foundation.cards.begin(), foundation.cards.end(), card_to_find) != foundation.cards.end()) {
                    location = "foundation";
                }
            }
        }

        if (!deck.waste.empty()) {
            location = "waste";
        }

        return location;
    }

    bool                    KlondikeState::IsTerminal() const {

        // =============================================================================================================
        // This section checks is the game has even been setup yet.
        // =============================================================================================================

        if (IsChanceNode()) {return false;}
        else {

            // =============================================================================================================
            // This section checks if the game has been won by checking if the top
            // card of each foundation has the rank "K" (King).
            // =============================================================================================================

            for (const auto & foundation : foundations) {
                if (!foundation.cards.empty()) {
                    if (foundation.cards.back().rank != "K") {
                        break;
                    } else {
                        return true;
                    }
                }
            }

            // =============================================================================================================
            // This section checks if there have been any moves in the last 12 turns
            // besides drawing from the deck. 12 turns is the maximum number of draws
            // before the deck is emptied and must be rebuilt.
            //
            // TODO: Technically we should check if there have been any moves besides
            // TODO: kDraw and kEnd since the last time the deck has been rebuilt.
            // =============================================================================================================

            int i = 0;
            std::vector<Action> history = History();

            // For action in history, starting from most recent ...
            for (auto rit = history.rbegin(); rit != history.rend(); ++rit) {
                // If there is a move other than draw ...
                if (*rit != kDraw) {
                    return false;
                } else if (i >= 11) {
                    return true;
                }

            }
        }
    }

    bool                    KlondikeState::IsChanceNode() const {
        return setup_counter_ <= 51;
    }

    bool                    KlondikeState::IsReversible(Action action) const {
        /*
         This function determines if an action would produce a child state that has at least one action
         that would reverse the initial action.

         - Tableau-to-tableau moves:
         In general, these are reversible. Exceptions are:
            - When the source card is on top of a hidden card, this will reveal the hidden card
            - When the source card is the top card of the tableau (If the source card is a king, technically it
              makes the move reversible, but we filter out these king to empty tableau moves in CandidateMoves()
              if the king is already the top card).

         - Tableau-to-foundation moves
         In general, these are reversible too. The exceptions are:
            - When the source card is on top of a hidden card, this will reveal the hidden card
            - When the source card is the only card in a tableau, except if it is a king (then it's reversible)

         - Foundation-to-tableau moves
         These moves are always reversible because they leave a compatible target in the foundation when they
         are moved to the tableau.

         - Waste-to-tableau moves
           All moves from the waste are irreversible

         - Waste-to-foundation moves
           All moves from the waste are irreversible
        */

        if (action == kSetup or action == kDraw or !is_setup_) {
            return false;
        }

        Card target = Card("0", "0", false);
        Card source = Card("0", "0", false);

        // Extract move from action ====================================================================================
        if (special_moves.count(action) == 0) {
            std::pair<Card, Card> move = GetMoveFromAction(action);
            target = move.first;
            source = move.second;
        }
        else {
            int card_index = (action - 52) / 100;
            source = Card(card_index);
        }

        // Check if reversible =========================================================================================
        std::string source_type = GetContainerType(source);

        if (source_type == "waste") {
            return false;
        }
        else if (source_type == "foundation") {
            return true;
        }
        else if (source_type == "tableau") {
            bool last_card_hidden = false;
            std::deque<Card> * source_container = const_cast<std::deque<Card> *>(GetContainer(source));

            for (auto & card : *source_container) {
                if (card == source) {
                    if (last_card_hidden) {
                        return false;
                    } else {
                        break;
                    }
                } else {
                    last_card_hidden = card.hidden;
                }
            }

            if (source_container->front() == source) {
                if (source.rank == "K") {
                    return true;
                } else {
                    return false;
                }
            }
        }
        else {
            return false;
        }
    }

    void                    KlondikeState::DoApplyAction(Action move) {

        // Change to previous state instead of string
        previous_string = ToString();
        bool action_reversible = IsReversible(move);

        if (CurrentPlayer() != kChancePlayerId) {
            previous_score = CurrentScore();
        }

        if (setup_counter_ <= 51) {
            // This branch handles choosing the order of the deck
            Card selected_card = Card {move};
            deck.initial_order.push_front(selected_card);
            auto it = deck.cards.begin();
            while (it != deck.cards.end()) {
                if (*it == selected_card) {
                    deck.cards.erase(it);
                    break;
                } else {
                    ++it;
                }
            }
            setup_counter_ += 1;
        }

        else if (move == -4 and deck.initial_order.size() == 52 and !is_setup_) {

            // This branch sets up the initial state of the game given the order of the deck

            // Copy initial order to deck cards
            deck.cards = deck.initial_order;

            // Place cards into tableaus
            for (int i = 1; i <= 7; i++) {
                tableaus.emplace_back(deck.deal(i));
            }

            // Initialize foundations
            for (const auto & suit : SUITS) {
                foundations.emplace_back(Foundation(suit));
            }

            // Set setup flag to true
            is_setup_ = true;
        }

        else if (move == kDraw) {
            if (!deck.cards.empty()) {
                deck.draw();
            } else {
                deck.rebuild();
                deck.draw();
            }
        }

        else if (is_setup_) {

            if (special_moves.count(move) == 0) {
                std::pair<Card, Card> pair_of_cards = GetMoveFromAction(move);
                MoveCards(pair_of_cards);
            }
            else if (special_moves.count(move) == 1) {
                Card source = Card((move - 52) / 100);
                std::deque<Card> * source_container = const_cast<std::deque<Card> *>(GetContainer(source));
                std::deque<Card> split_cards;

                if (source.rank == "A") {
                    for (auto & foundation : foundations) {
                        if (foundation.cards.empty() and foundation.suit == source.suit) {
                            if (*source_container == deck.waste) {
                                foundation.cards.push_back(source_container->front());
                                source_container->pop_front();
                            }
                            else {
                                foundation.cards.push_back(source_container->back());
                                source_container->pop_back();
                                source_container->back().hidden = false;
                            }
                        }
                    }
                }

                else if (source.rank == "K") {
                    for (auto & tableau : tableaus) {
                        if (tableau.cards.empty()) {

                            // Branch if king is in waste
                            if (*source_container == deck.waste) {
                                split_cards.push_back(source_container->front());
                                source_container->pop_front();
                            }

                            // Branch if king is in tableau or foundation
                            else {
                                bool split_flag = false;
                                for (auto it = source_container->begin(); it != source_container->end();) {
                                    if (*it == source) {
                                        split_flag = true;
                                    }
                                    if (split_flag) {
                                        split_cards.push_back(*it);
                                        it = source_container->erase(it);
                                    }
                                    else {
                                        ++it;
                                    }
                                }

                                // Unhide last card in source container if it exists
                                if (!source_container->empty()) {
                                    source_container->back().hidden = false;
                                }

                                // Add split cards to target container
                                for (auto split_card : split_cards) {
                                    tableau.cards.push_back(split_card);
                                }

                                // Break out of the loop
                                break;
                            }
                        }
                    }
                }
            }
            last_move_was_reversible = action_reversible;
        }

        /*
        else if (is_setup_ and special_moves.count(move) == 0) {
            std::pair<Card, Card> pair_of_cards = GetMoveFromAction(move);
            MoveCards(pair_of_cards);
        }

        else if (is_setup_ and special_moves.count(move) == 1) {
            // std::cout << YELLOW << "\nSPECIAL MOVE" << RESET << std::endl;

            Card source = Card((move - 52) / 100);
            std::deque<Card> * source_container = const_cast<std::deque<Card> *>(GetContainer(source));
            std::deque<Card> split_cards;

            if (source.rank == "A") {
                for (auto & foundation : foundations) {
                    if (foundation.cards.empty() and foundation.suit == source.suit) {
                        if (*source_container == deck.waste) {
                            foundation.cards.push_back(source_container->front());
                            source_container->pop_front();
                        }
                        else {
                            foundation.cards.push_back(source_container->back());
                            source_container->pop_back();
                            source_container->back().hidden = false;
                        }
                    }
                }
            }

            else if (source.rank == "K") {
                for (auto & tableau : tableaus) {
                    if (tableau.cards.empty()) {

                        // Branch if king is in waste
                        if (*source_container == deck.waste) {
                            split_cards.push_back(source_container->front());
                            source_container->pop_front();
                        }

                        // Branch if king is in tableau or foundation
                        else {
                            bool split_flag = false;
                            for (auto it = source_container->begin(); it != source_container->end();) {
                                if (*it == source) {
                                    split_flag = true;
                                }
                                if (split_flag) {
                                    split_cards.push_back(*it);
                                    it = source_container->erase(it);
                                }
                                else {
                                    ++it;
                                }
                            }

                            // Unhide last card in source container if it exists
                            if (!source_container->empty()) {
                                source_container->back().hidden = false;
                            }

                            // Add split cards to target container
                            for (auto split_card : split_cards) {
                                tableau.cards.push_back(split_card);
                            }

                            // Break out of the loop
                            break;
                        }
                    }
                }
            }
        }
        */

    }

    void                    KlondikeState::MoveCards(const std::pair<Card, Card> & move) {

        // Target & Source cards =======================================================================================

        Card target = move.first;
        Card source = move.second;

        // Find containers =============================================================================================

        std::deque<Card> * target_container = const_cast<std::deque<Card> *>(GetContainer(target));
        std::deque<Card> * source_container = const_cast<std::deque<Card> *>(GetContainer(source));

        // Split Source Container on Source Card =======================================================================

        std::deque<Card> split_cards;
        std::vector<std::deque<Card>> foundation_containers;

        for (auto & foundation : foundations) {
            foundation_containers.push_back(foundation.cards);
        }

        if (*source_container == deck.waste) {
            // Can only move one card, the top one of the waste
            split_cards.push_back(source_container->front());
            source_container->pop_front();
        }

        else if (std::find(foundation_containers.begin(), foundation_containers.end(), *source_container) != foundation_containers.end()) {
            split_cards.push_back(source_container->back());
            source_container->pop_back();
        }

        else {
            bool split_flag = false;
            for (auto it = source_container->begin(); it != source_container->end();) {
                if (*it == source) {
                    split_flag = true;
                }
                if (split_flag) {
                    split_cards.push_back(*it);
                    it = source_container->erase(it);
                } else {
                    ++it;
                }
            }
        }

        // Unhide Last Card in Source Container ========================================================================

        if (!source_container->empty()) {
            source_container->back().hidden = false;
        }

        // Add Split Cards to Target Container =========================================================================

        for (auto split_card : split_cards) {
            target_container->push_back(split_card);
        }

    }

    double                  KlondikeState::CurrentScore() const {
        std::map<std::string, double> foundation_points = {
                {"A", 100.0},
                {"2", 90.0},
                {"3", 80.0},
                {"4", 70.0},
                {"5", 60.0},
                {"6", 50.0},
                {"7", 40.0},
                {"8", 30.0},
                {"9", 20.0},
                {"T", 10.0},
                {"J", 10.0},
                {"Q", 10.0},
                {"K", 10.0}
        };

        double current_score = 0.0;

        // Calculates score from cards in foundations
        for (auto & foundation : foundations) {
            for (auto & card : foundation.cards) {
                current_score += foundation_points.at(card.rank);
            }
        }

        // Calculates score for revealed cards in tableau (20 pts each)
        int num_hidden_cards = 0;
        for (auto & tableau : tableaus) {
            for (auto & card : tableau.cards) {
                if (card.hidden) {
                    num_hidden_cards += 1;
                }
            }
        }
        current_score += (21 - num_hidden_cards) * 20.0;

        // Calculates score for cards moves from the deck/waste
        int num_cards_in_deck = deck.cards.size() + deck.waste.size();
        current_score += (24 - num_cards_in_deck) * 20.0;

        return current_score;
    }

    std::vector<double>     KlondikeState::Returns() const {
        // TODO: Implementation is no longer correct
        if (!IsTerminal()) {
            return {0.0};
        } else {
            return {1.0};
        }
    }

    std::vector<double>     KlondikeState::Rewards() const {
        if (CurrentPlayer() != kChancePlayerId) {
            return {previous_score, CurrentScore() - previous_score};
        } else {
            return {previous_score, 0.0};
        }
    }

    std::vector<Action>     KlondikeState::CandidateActions() const {
        /* Used by LegalActions() to get potentially legal actions. */
        if (!is_setup_) {
            return {kSetup};
        }
        else {
            std::vector<std::pair<Card, Card>> candidate_moves;
            std::vector<Action> candidate_actions;

            // TARGETS
            std::vector<Card> targets = Targets();                       // All targets
            std::vector<Card> tableau_targets = Targets("tableau");      // Top card of each tableau
            std::vector<Card> foundation_targets = Targets("foundation");   // Top card of each foundation

            // SOURCES
            std::vector<Card> sources = Sources();                       // All sources
            std::vector<Card> tableau_sources = Sources("tableau");      // Every unhidden card in the tableaus
            std::vector<Card> foundation_sources = Sources("foundation");   // Top card of each foundation
            std::vector<Card> waste_sources = Sources("waste");        // Top card of waste

            // ORDINARY MOVES ==========================================================================================

            // Handles "X-to-Tableau" moves
            for (const Card & target : tableau_targets) {
                std::vector<Card> children = GetAllowableChildren(target, "tableau");

                // Source can come from tableau, foundation, or waste
                for (const Card &source : sources) {
                    if (std::find(children.begin(), children.end(), source) != children.end()) {
                        std::pair<Card, Card> move(target, source);
                        candidate_moves.push_back(move);
                    }
                }
            }

            // Handles "X-to-Foundation" moves
            for (const Card & target : foundation_targets) {
                std::vector<Card> children = GetAllowableChildren(target, "foundation");

                // We use "tableau_targets" here because we only want the top card of each tableau.
                // Even though we are technically using them as sources.
                for (const Card &source : tableau_targets) {
                    if (std::find(children.begin(), children.end(), source) != children.end()) {
                        std::pair<Card, Card> move(target, source);
                        candidate_moves.push_back(move);
                    }
                }

                // Usually only one card in waste_sources, but in some variants (like K+ solitaire) there can be many
                for (const Card &source : waste_sources) {
                    if (std::find(children.begin(), children.end(), source) != children.end()) {
                        std::pair<Card, Card> move(target, source);
                        candidate_moves.push_back(move);
                    }
                }
            }

            // Converts moves to actions (ints)
            for (const auto & move : candidate_moves) {
                Action action = GetActionFromMove(move.first, move.second);
                candidate_actions.push_back(action);
            }

            // SPECIAL MOVES ===========================================================================================

            // Handles "Ax-to-EmptyFoundation" moves

            for (const Card & source : waste_sources) {
                // An ace not in a foundation means there's an empty foundation for it automatically
                // Therefore we don't have to check for an empty foundation.
                if (source.rank == "A") {
                    Action action = 100 * (int) source + 52;
                    candidate_actions.push_back(action);
                }
            }

            for (const Card & source : tableau_targets) {
                // Only the top card of a tableau can be moved to the foundation, which is why treat
                // "tableau_targets" as containing source cards.
                if (source.rank == "A") {
                    Action action = 100 * (int) source + 52;
                    candidate_actions.push_back(action);
                }
            }

            // Handles "Kx-to-EmptyTableau" moves

            for (auto & tableau : tableaus) {

                // If a tableau is empty ...
                if (tableau.cards.empty()) {

                    // For source in sources ...
                    for (auto & source : sources) {

                        // If source rank is King ...
                        if (source.rank == "K") {

                            // Then get source container
                            std::deque<Card> *source_container = const_cast<std::deque<Card> *>(GetContainer(source));

                            // If the first card in the container is not a king ...
                            if (source_container->front().rank != "K") {

                                // Then we add it as a legal action
                                Action action = 100 * (int) source + 52;
                                candidate_actions.push_back(action);
                            }
                        }
                    }

                    // We just need to exit after finding this first empty tableau
                    break;
                }
            }

            // OTHER ACTIONS ===========================================================================================

            // Handles draw moves
            if (!deck.cards.empty() or !deck.waste.empty()) {
                candidate_actions.push_back(kDraw);
            }

            for (auto action : candidate_actions) {
                IsReversible(action);
            }

            return candidate_actions;
        }
    }

    std::vector<Action>     KlondikeState::LegalActions() const {

        /*
        "Local Loop Prevention"
        "An action is eliminated from a state, s, in a search tree if, by definition, state s can be
        restored in a single action following that action. In order to maintain a complete search, these
        actions are included only when paired with a second action such that s cannot be trivially recovered"
        - Bjarnason et al. (2007)

        Essentially, at each state, we want to mask the inverse of the action taken at the previous state if
        doing that inverse action would restore us to the previous state.
        Probably easiest just to store previous state, compute result of taking each action, check if any actions
        create a state equivalent to the previous state, if one does, its corresponding action is masked.

        Basically, illegal actions are ones that either:
         - Return the state to the previous state (parent & child have same state)
         - Are reversible after a reversible action was already done in the parent state
        */

        if (!is_setup_) {
            return {kSetup};
        }

        else {

            std::vector<Action> candidate_actions = CandidateActions();
            std::vector<Action> legal_actions;

            for (auto &action : candidate_actions) {

                auto child = Child(action);

                // Filter out all actions that would revert to previous state
                if (previous_string != child->ToString()) {
                    // If last action and current action are both reverisble ...

                    if (last_move_was_reversible and IsReversible(action)) {
                        // Skip action and move on to the next one
                        continue;
                    } else {
                        // Otherwise, add it to legal actions
                        legal_actions.push_back(action);
                    }


                }
            }

            return legal_actions;
        }




    }

    std::vector<Card>       KlondikeState::Targets() const {
        // Targets are cards that another card can be moved to
        // (e.g. if moving Ks to Qs in a foundation, Qs would be the target)

        std::vector<Card> target_cards;

        for (Tableau tableau : tableaus) {
            // Only the top card can be the target of a move
            if (!tableau.cards.empty()) {
                target_cards.push_back(tableau.cards.back());
            }
        }

        for (Foundation foundation : foundations) {
            // Only the top card can be the target of a move
            if (!foundation.cards.empty()) {
                target_cards.push_back(foundation.cards.back());
            }
        }

        return target_cards;
    }

    std::vector<Card>       KlondikeState::Sources() const {
        // Sources are cards that can be moved to a target
        // (e.g. if moving Ks to Qs in a foundation, Ks would be the source)

        std::vector<Card> source_cards;

        for (const Tableau & tableau : tableaus) {
            // Only unhidden cards in a tableau can be the source of a move
            if (!tableau.cards.empty()) {
                for (const Card &card : tableau.cards) {
                    if (!card.hidden) {
                        source_cards.push_back(card);
                    }
                }
            }
        }

        for (const Foundation & foundation : foundations) {
            // Only the top card of a foundation can be the source of a move
            if (!foundation.cards.empty()) {
                source_cards.push_back(foundation.cards.back());
            }
        }

        if (!deck.waste.empty()) {
            // Only the top card of the waste can be the source of a move
            source_cards.push_back(deck.waste.front());
        }

        return source_cards;
    }

    std::vector<Card>       KlondikeState::Targets(const std::string & location) const {
        // Valid values of `location` are "tableau" and "foundation"

        std::vector<Card> target_cards;

        if (location == "tableau") {
            for (Tableau tableau : tableaus) {
                // Only the top card can be the target of a move
                if (!tableau.cards.empty()) {
                    target_cards.push_back(tableau.cards.back());
                }
            }
        }
        else if (location == "foundation") {
            for (Foundation foundation : foundations) {
                // Only the top card can be the target of a move
                if (!foundation.cards.empty()) {
                    target_cards.push_back(foundation.cards.back());
                }
            }
        }

        return target_cards;
    }

    std::vector<Card>       KlondikeState::Sources(const std::string & location) const {
        // Valid values of `location` are "tableau", "foundation" and "waste"

        std::vector<Card> source_cards;

        if (location == "tableau") {
            for (const Tableau & tableau : tableaus) {
                // Only unhidden cards in a tableau can be the source of a move
                if (!tableau.cards.empty()) {
                    for (const Card &card : tableau.cards) {
                        if (!card.hidden) {
                            source_cards.push_back(card);
                        }
                    }
                }
            }
        }

        else if (location == "foundation") {
            for (const Foundation & foundation : foundations) {
                // Only the top card of a foundation can be the source of a move
                if (!foundation.cards.empty()) {
                    source_cards.push_back(foundation.cards.back());
                }
            }
        }

        else if (location == "waste") {
            if (!deck.waste.empty()) {
                // Only the top card of the waste can be the source of a move
                source_cards.push_back(deck.waste.front());
            }
        }

        return source_cards;
    }

    const std::deque<Card> * KlondikeState::GetContainer(Card card_to_find) const {

        for (auto & tableau : tableaus) {
            if (!tableau.cards.empty()) {
                if (std::find(tableau.cards.begin(), tableau.cards.end(), card_to_find) != tableau.cards.end()) {
                    return &tableau.cards;
                }
            }
        }

        for (auto & foundation : foundations) {
            if (!foundation.cards.empty()) {
                if (std::find(foundation.cards.begin(), foundation.cards.end(), card_to_find) != foundation.cards.end()) {
                    return &foundation.cards;
                }
            }
        }

        if (!deck.waste.empty()) {
            return &deck.waste;
        }
    }

    std::unique_ptr<State>  KlondikeState::Clone() const {
        return std::unique_ptr<State>(new KlondikeState(*this));
    }

    std::vector<std::pair<Action, double>> KlondikeState::ChanceOutcomes() const {
        SPIEL_CHECK_TRUE(IsChanceNode());
        std::vector<std::pair<Action, double>> outcomes;

        if (!is_setup_) {
            const double p = 1.0 / deck.cards.size();
            for (const auto & card : deck.cards) {
                outcomes.emplace_back((int) card, p);
            }
        }
        return outcomes;
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

    std::unique_ptr<State> KlondikeGame::NewInitialState() const {
        return std::unique_ptr<State>(new KlondikeState(shared_from_this()));
    }

    std::shared_ptr<const Game> KlondikeGame::Clone() const {
        return std::shared_ptr<const Game>(new KlondikeGame(*this));
    }

}