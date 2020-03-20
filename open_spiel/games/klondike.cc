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

    std::vector<std::string> GetOppositeSuits(const std::string & suit) {
        if (suit == "s" or suit == "c") {
            return {"h", "d"};
        } else if (suit == "h" or suit == "d") {
            return {"s", "c"};
        }
    }

    std::vector<Card> GetAllowableChildren(const Card & card, const std::string & location) {

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

    Action GetActionFromMove(const Card & target, const Card & source) {
        // TODO: Handle special moves
        int target_int = (int) target;
        int source_int = (int) source;
        return target_int * 100 + source_int + 52;
    }

    std::pair<Card, Card> GetMoveFromAction(const Action & action) {
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

    void RenderPair(const std::pair<Card, Card>& pair) {
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
        /* "Local Loop Prevention"
        "An action is eliminated from a state, s, in a search tree if, by definition, state s can be
        restored in a single action following that action. In order to maintain a complete search, these
        actions are included only when paired with a second action such that s cannot be trivially recovered"
        - Bjarnason et al. (2007) */

        // Essentially, at each state, we want to mask the inverse of the action taken at the previous state if
        // doing that inverse action would restore us to the previous state.

        // Probably easiest just to store previous state, compute result of taking each action, check if any actions
        // create a state equivalent to the previous state, if one does, its corresponding action is masked.

        if (!is_setup_) {
            return {kSetup};
        }
        else {
            std::cout << "\nLEGAL ACTIONS:" << std::endl;

            std::vector<std::pair<Card, Card>> legal_moves;
            std::vector<Action> legal_actions;

            std::vector<Card> tableau_targets = Targets("tableau");
            std::vector<Card> foundation_targets = Targets("foundation");
            std::vector<Card> sources = Sources();

            // Gets legal moves for targets in the tableaus
            for (const Card &target : tableau_targets) {
                std::vector<Card> children = GetAllowableChildren(target, "tableau");
                for (const Card &source : sources) {
                    if (std::find(children.begin(), children.end(), source) != children.end()) {
                        std::pair<Card, Card> move(target, source);
                        legal_moves.push_back(move);
                    }
                }
            }

            // Gets legal moves for targets in the foundations
            for (const Card &target : foundation_targets) {
                std::vector<Card> children = GetAllowableChildren(target, "foundation");
                for (const Card &source : sources) {
                    if (std::find(children.begin(), children.end(), source) != children.end()) {
                        std::pair<Card, Card> move(target, source);
                        legal_moves.push_back(move);
                    }
                }
            }

            // This section handles card to card moves
            for (const auto &m : legal_moves) {
                Action action = GetActionFromMove(m.first, m.second);
                legal_actions.push_back(action);

                std::cout << "(" << m.first.rank << m.first.suit << ", " << m.second.rank << m.second.suit << ") ";
                std::cout << action << std::endl;
            }

            // This section handles ace to empty foundation moves
            for (const Card & source : sources) {
                if (source.rank == "A") {
                    for (auto &foundation : foundations) {
                        if (foundation.suit == source.suit) {
                            Action action = 100 * (int) source + 52;
                            legal_actions.push_back(action);

                            std::cout << "(" << foundation.suit << ", " << source.rank << source.suit << ") ";
                            std::cout << action << std::endl;
                        }
                    }
                }
            }

            // This section handles king to empty tableau moves
            // TODO: Implement this

            // If the deck isn't empty ...
            if (!deck.cards.empty()) {
                // ... then we can draw from the deck
                legal_actions.push_back(kDraw);
            }
            // If the deck is empty ...
            else {
                // ... and the waste isn't empty ...
                if (!deck.waste.empty()) {
                    // ... then we can draw from the deck (after rebuilding from the waste)
                    legal_actions.push_back(kDraw);
                }
            }

            std::cout << "\n";
            return legal_actions;
        }
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

        absl::StrAppend(&result, "DECK        : ");
        for (const Card & card : deck.cards) {
            if (card.hidden) {
                absl::StrAppend(&result, "[] ");
            } else {
                absl::StrAppend(&result, card.rank, card.suit, " ");
            }
        }

        absl::StrAppend(&result, "\nWASTE       : ");
        for (const Card & card : deck.waste) {
            absl::StrAppend(&result, card.rank, card.suit, " ");
        }

        absl::StrAppend(&result, "\nFOUNDATIONS : ");
        for (const Foundation & foundation : foundations) {
            for (const Card & card : foundation.cards) {
                absl::StrAppend(&result, card.rank, card.suit, " ");
            }
        }

        absl::StrAppend(&result, "\nTABLEAUS    : ");
        for (const Tableau & tableau : tableaus) {
            absl::StrAppend(&result, "\n");
            for (const Card & card : tableau.cards) {
                if (card.hidden) {
                    absl::StrAppend(&result, "[] ");
                } else {
                    absl::StrAppend(&result, card.rank, card.suit, " ");
                }
            }
        }

        absl::StrAppend(&result, "\n\nTARGETS : ");
        for (const Card & card : Targets()) {
            absl::StrAppend(&result, card.rank, card.suit, " ");
        }

        absl::StrAppend(&result, "\nSOURCES : ");
        for (const Card & card : Sources()) {
            absl::StrAppend(&result, card.rank, card.suit, " ");
        }

        return result;
    }

    bool                    KlondikeState::IsTerminal() const {

        std::cout << "Entering IsTerminal()" << std::endl;

        // =============================================================================================================
        // This section checks if the game has been won by checking if the top
        // card of each foundation has the rank "K" (King).
        // =============================================================================================================

        for (const auto & foundation : foundations) {
            if (!(foundation.cards.back().rank == "K")) {
                break;
            } else {
                return true;
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
        auto rit = history.rbegin();
        for (rit = history.rbegin(); rit != history.rend(); ++rit) {
            if (i > 11 or *rit != kDraw) {
                break;
            } else {
                return false;
            }
        }

    }

    std::vector<double>     KlondikeState::Returns() const {
        if (!IsTerminal()) {
            return {0.0};
        } else {
            return {1.0};
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

    void KlondikeState::DoApplyAction(Action move) {

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

        else if (is_setup_ and special_moves.count(move) == 0) {
            std::cout << YELLOW << "\nORDINARY MOVE" << RESET << std::endl;

            std::pair<Card, Card> pair_of_cards = GetMoveFromAction(move);

            std::cout << "Pair of Cards : "; RenderPair(pair_of_cards);

            MoveCards(pair_of_cards);
        }

        else if (is_setup_ and special_moves.count(move) == 1) {
            std::cout << RED << "\nSPECIAL MOVE" << RESET << std::endl;

            Card source = Card((move - 52) / 100);
            std::deque<Card> * source_container = const_cast<std::deque<Card> *>(GetContainer(source));
            std::deque<Card> split_cards;

            if (source.rank == "A") {
                for (auto & foundation : foundations) {
                    if (foundation.cards.empty() and foundation.suit == source.suit) {
                        // TODO: This is where I left off. Trying to handle moving aces to foundation piles
                        if (*source_container == deck.waste) {
                            // Transfer ace from waste to foundation
                            foundation.cards.push_back(source_container->front());
                            source_container->pop_front();
                        }
                        else {
                            foundation.cards.push_back(source_container->back());
                            source_container->pop_back();
                        }
                    }
                }

            }
            else if (source.rank == "K") {

            }

        }

        std::cout << "History: " << std::endl;
        for (auto & node : History()) {
            std::cout << node << " ";
        }
        std::cout << std::endl;
    }

    bool KlondikeState::IsChanceNode() const {
        return setup_counter_ <= 51;
    }

    std::vector<Card> KlondikeState::Targets() const {
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

    std::vector<Card> KlondikeState::Sources() const {
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

    std::vector<Card> KlondikeState::Targets(const std::string & location) const {
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

    std::vector<Card> KlondikeState::Sources(const std::string & location) const {
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

    void KlondikeState::MoveCards(const std::pair<Card, Card> & move) {

        // Target & Source cards =======================================================================================

        Card target = move.first;
        Card source = move.second;

        std::cout << "TARGET CARD : "; RenderCard(target);
        std::cout << "\nSOURCE CARD : "; RenderCard(source);

        // Find containers =============================================================================================

        std::deque<Card> * target_container = const_cast<std::deque<Card> *>(GetContainer(target));
        std::deque<Card> * source_container = const_cast<std::deque<Card> *>(GetContainer(source));

        // std::cout << "\nTARGET CONTAINER : "; RenderPile(*target_container);
        // std::cout << "SOURCE CONTAINER : ";   RenderPile(*source_container);

        // Split Source Container on Source Card =======================================================================

        std::deque<Card> split_cards;
        std::vector<std::deque<Card>> foundation_containers;

        for (auto & foundation : foundations) {
            foundation_containers.push_back(foundation.cards);
        }

        if (*source_container == deck.waste) {
            // Can only move one card, the top one of the waste
            std::cout << "`source_container` is the waste pile" << std::endl;
            split_cards.push_back(source_container->front());
            source_container->pop_front();
        }

        else if (std::find(foundation_containers.begin(), foundation_containers.end(), *source_container) != foundation_containers.end()) {
            std::cout << "`source_container` is a foundation" << std::endl;
            split_cards.push_back(source_container->back());
            source_container->pop_back();
        }

        else {
            std::cout << "`source_container` is a tableau" << std::endl;
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

        // Render ======================================================================================================
        // std::cout << "\nTARGET CONTAINER : "; RenderPile(*target_container);
        // std::cout << "SOURCE CONTAINER : ";   RenderPile(*source_container);

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