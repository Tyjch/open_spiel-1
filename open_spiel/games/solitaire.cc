#include "solitaire.h"

#include <deque>
#include <numeric>
#include <random>
#include <algorithm>
#include <optional>
#include <variant>
#include <math.h>

#include "open_spiel/abseil-cpp/absl/strings/str_format.h"
#include "open_spiel/abseil-cpp/absl/strings/str_join.h"
#include "open_spiel/game_parameters.h"
#include "open_spiel/spiel_utils.h"

#define RED    "\033[31m"
#define WHITE  "\033[37m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"

/* TODO LIST
 * - Change std::find to count to check if something exists in a container.
 * - Hidden cards shouldn't show up as sources or target.
 * - ToString() methods shouldn't include padding. Their callers should handle how the strings are formatted.
 * - Maybe use std::variant or std::any to consolidate finding a container instead of FindTableau
 *   and FindFoundation.
 */


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
            std::cout << YELLOW << "WARNING: `suit` is not in (s, h, c, d)" << RESET << std::endl;
        }
    }

    // Card Methods ====================================================================================================

    /* Special cards:
     - Card("", "", kTableau) is an empty tableau,
     - Card("", "s", kFoundation) is an empty foundation */

    Card::Card() : rank(""), suit(""), hidden(true), location(kMissing) {};

    Card::Card(std::string rank, std::string suit) : rank(rank), suit(suit), hidden(true), location(kMissing) {};

    Card::Card(int index) : hidden(false), location(kMissing) {
        // Handles special cards
        if (index < 0) {
            rank = "";
            switch (index) {
                case -1 : { suit = "s"; break; }
                case -2 : { suit = "h"; break; }
                case -3 : { suit = "c"; break; }
                case -4 : { suit = "d"; break; }
                case -5 : { suit =  ""; break; }
            }
        }
        // Handles ordinary cards
        else {
            int rank_value = index % 13;
            int suit_value = floor(index / 13);
            rank = RANKS.at(rank_value);
            suit = SUITS.at(suit_value);
        }

    }

    Card::operator int() const {
        // Handles special cards
        if (rank.empty()) {
            if      (suit == "s")  { return -1; }
            else if (suit == "h")  { return -2; }
            else if (suit == "c")  { return -3; }
            else if (suit == "d")  { return -4; }
            else if (suit.empty()) { return -5; }
        }
        // Handles ordinary cards
        else {
            int rank_value = GetIndex(RANKS, rank);
            int suit_value = GetIndex(SUITS, suit);
            return 13 * suit_value + rank_value;
        }
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

        // A hidden card has no legal children
        if (hidden) {
            return legal_children;
        }

        switch (location) {
            case kTableau:

                // Handles empty tableau cards (children are kings of all suits)
                if (rank == "") {
                    child_rank  = "K";
                    child_suits = SUITS;
                }
                // Handles regular cards (except aces)
                else if (rank != "A") {
                    child_rank  = RANKS.at(GetIndex(RANKS, rank) - 1);
                    child_suits = GetOppositeSuits(suit);
                }
                break;

            case kFoundation:

                // Handles empty foundation cards (children are aces of same suit)
                if (rank == "") {
                    child_rank  = "A";
                    child_suits = {suit};
                }
                // Handles regular cards (except kings)
                else if (rank != "K") {
                    child_rank  = RANKS.at(GetIndex(RANKS, rank) + 1);
                    child_suits = {suit};
                }
                break;

            default:
                return legal_children;
        }

        for (auto child_suit : child_suits) {
            auto child   = Card(child_rank, child_suit);
            child.hidden = false;
            legal_children.push_back(child);
        }

        return legal_children;
    }

    std::string Card::ToString() const {
        /* // Old Implementation
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
        */

        std::string result;

        if (hidden) {
            // Representation of a hidden card
            absl::StrAppend(&result, "\U0001F0A0", " ");
        }
        else {
            // Suit Color
            if (suit == "s" or suit == "c") {
                absl::StrAppend(&result, WHITE);
            } else if (suit == "h" or suit == "d") {
                absl::StrAppend(&result, RED);
            }

            // Special Cards
            if (rank == "") {
                // Handles special tableau cards which have no rank or suit
                if (suit == "") {
                    absl::StrAppend(&result, "\U0001F0BF");
                }
                // Handles special foundation cards which have a suit but not a rank
                else {
                    if (suit == "s") {
                        absl::StrAppend(&result, "\U00002660");
                    } else if (suit == "h") {
                        absl::StrAppend(&result, "\U00002665");
                    } else if (suit == "c") {
                        absl::StrAppend(&result, "\U00002663");
                    } else if (suit == "d") {
                        absl::StrAppend(&result, "\U00002666");
                    }
                }
            }

            // Ordinary Cards
            else {
                absl::StrAppend(&result, rank, suit);
            }



        }

        absl::StrAppend(&result, RESET, " ");
        return result;
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

    std::vector<Card> Deck::Sources() const {
        // If the waste is not empty, sources is just a vector of the top card of the waste
        if (not waste.empty()) {
            return {waste.front()};
        }
        // If it is empty, sources is just an empty vector
        else {
            return {};
        }

    }

    std::vector<Card> Deck::Split(Card card) {
        std::vector<Card> split_cards;
        if (waste.front() == card) {
            split_cards = {waste.front()};
            waste.pop_front();
            return split_cards;
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
            for (Card & card : initial_order) {
                if (std::find(waste.begin(), waste.end(), card) != waste.end()) {
                    card.location = kDeck;
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

    Foundation::Foundation() {
        cards = {};
    }

    Foundation::Foundation(std::string suit) : suit(suit) {
        cards = {};
    };

    std::vector<Card> Foundation::Sources() const {
        // If the foundation is not empty, sources is just a vector of the top card of the foundation
        if (not cards.empty()) {
            return {cards.back()};
        }
        // If it is empty, then sources is just an empty vector
        else {
            return {};
        }
    }

    std::vector<Card> Foundation::Targets() const {

        // If the foundation is not empty, targets is just the top card of the foundation
        if (not cards.empty()) {
            return {cards.back()};
        }
        // If it is empty, then targets is just a special card with no rank and a suit matching this foundation
        else {
            auto card     = Card("", suit);
            card.hidden   = false;
            card.location = kFoundation;
            return {card};
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
        // TODO: Cards location should be changed to kFoundation when added
        for (auto card : source_cards) {
            card.location = kFoundation;
            cards.push_back(card);
        }
    }

    // Tableau Methods =================================================================================================

    Tableau::Tableau() {}

    Tableau::Tableau(int num_cards) {
        for (int i = 1; i <= num_cards; i++) {
            cards.emplace_back();
        }
        for (auto & card : cards) {
            card.location = kTableau;
        }
    };

    std::vector<Card> Tableau::Sources() const {
        // If the tableau is not empty, sources is just a vector of all cards that are not hidden
        if (not cards.empty()) {
            std::vector<Card> sources;
            for (auto & card : cards) {
                if (not card.hidden) {
                    sources.push_back(card);
                }
            }
            return sources;
        }
        // If it is empty, then sources is just an empty vector
        else {
            return {};
        }
    }

    std::vector<Card> Tableau::Targets() const {
        /*
        DECISION: Should targets return a vector, even though it will only return one card?
        */

        // If the tableau is not empty, targets is just a vector of the top card of the tableau
        if (not cards.empty()) {
            return {cards.back()};
        }
        // If it is empty, then targets is just a special card with no rank or suit
        else {
            auto card     = Card();
            card.hidden   = false;
            card.location = kTableau;
            return {card};
        }
    }

    std::vector<Card> Tableau::Split(Card card) {

        std::vector<Card> split_cards;
        if (not cards.empty()) {
            bool split_flag = false;
            for (auto it = cards.begin(); it != cards.end();) {
                if (*it == card) {
                    split_flag = true;
                }
                if (split_flag) {
                    split_cards.push_back(*it);
                    it = cards.erase(it);
                } else {
                    ++it;
                }
            }
        } else {
            std::cout << YELLOW << "WARNING: Cannot split an empty tableau" << RESET << std::endl;
        }

        return split_cards;
    }

    void Tableau::Extend(std::vector<Card> source_cards) {
        for (auto card : source_cards) {
            card.location = kTableau;
            cards.push_back(card);
        }
    }

    // Move Methods ====================================================================================================

    Move::Move(Card target_card, Card source_card) {
        target = target_card;
        source = source_card;
    }

    Move::Move(Action action_id) {
        auto card_pair = ACTION_TO_MOVE.at(action_id);
        target = Card(card_pair.first);
        source = Card(card_pair.second);
    }

    std::string Move::ToString() const {
        std::string result;
        absl::StrAppend(&result, target.ToString(), "\U00002190", " ",source.ToString());
        return result;
    }

    Action Move::ActionId() const {
        return MOVE_TO_ACTION.at(std::make_pair((int) target, (int) source));
    }

    // SolitaireState Methods ==========================================================================================

    SolitaireState::SolitaireState(std::shared_ptr<const Game> game) :
        State(game),
        deck(),
        foundations(),
        tableaus() {
            is_setup = false;
            previous_score = 0.0;
        };

    // Overriden Methods -----------------------------------------------------------------------------------------------

    Player                  SolitaireState::CurrentPlayer() const {

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
        // TODO: Create methods that allow casting to std::string for card, deck, waste, foundation, and tableau

        std::string result;

        absl::StrAppend(&result, "CURRENT PLAYER : ", CurrentPlayer());

        absl::StrAppend(&result, "\n\nDECK        : ");
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
            if (foundation.cards.empty()) {
                Card foundation_base = Card("", foundation.suit);
                foundation_base.hidden = false;
                absl::StrAppend(&result, foundation_base.ToString());
            } else {
                absl::StrAppend(&result, foundation.cards.back().ToString());
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

        absl::StrAppend(&result, "\n\nTARGETS : ");
        for (const Card & card : Targets()) {
            absl::StrAppend(&result, card.ToString());
        }

        absl::StrAppend(&result, "\nSOURCES : ");
        for (const Card & card : Sources()) {
            absl::StrAppend(&result, card.ToString());
        }

        absl::StrAppend(&result, "\n\nCANDIDATE MOVES : ");
        for (const Move & move : CandidateMoves()) {
            absl::StrAppend(&result, "\n", move.ToString(), ": ", move.ActionId());
        }

        return result;
    }

    std::string             SolitaireState::ActionToString(Player player, Action action_id) const {
        switch (action_id) {
            case 0 : {
                return "kSetup";
            }
            case 1 ... 52 : {
                // Reveal starts at 1 while card indices start at 0, so we subtract one here
                Card revealed_card = Card(action_id - 1);

                std::string result;
                absl::StrAppend(&result, "kReveal", revealed_card.rank, revealed_card.suit);

                return result;
            }
            case 53 : {
                return "kDraw";
            }
            case 54 ... 205 : {
                Move move = Move(action_id);
                std::string result;

                absl::StrAppend(&result, "kMove");
                if (move.target.rank.empty()) { absl::StrAppend(&result, "__"); }
                else { absl::StrAppend(&result, move.target.rank, move.target.suit); }
                absl::StrAppend(&result, move.source.rank, move.source.suit);

                return result;
            }
            default : {
                return "kMissingAction";
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

        // Change previous score to be the current score of the state before the action is applied
        /*
        if (not IsChanceNode()) {
            previous_score = Returns().front();
        }
        */

        // Chance Node Actions =========================================================================================

        // Handles kSetup
        if (move == kSetup) {

            // Creates tableaus
            for (int i = 1; i <= 7; i++) {
                tableaus.emplace_back(i);
            }

            // Creates foundations
            for (const auto & suit : SUITS) {
                foundations.emplace_back(suit);
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

        // Decision Node Actions =======================================================================================

        else {
            previous_score = Returns().front();

            // Handles kDraw
            if (move == kDraw) {
                if (deck.cards.empty()) {
                    deck.rebuild();
                }
                deck.draw(3);
            }
            // Handles kMove
            else {
                Move selected_move = Move(move);
                MoveCards(selected_move);
            }
        }

        /*
        else if (move == kDraw) {
            if (deck.cards.empty()) {
                deck.rebuild();
            }
            deck.draw(3);
        }
        else {
            Move selected_move = Move(move);
            MoveCards(selected_move);
        }
        */

    }

    std::vector<double>     SolitaireState::Returns() const {
        // Equal to the sum of all rewards up to the current state
        // TODO: Needs to handle moves that have a source on top of a hidden card that will be revealed next round

        if (is_setup) {
            double returns = 0.0;

            // Calculates score from cards in foundations
            for (auto & foundation : foundations) {
                for (auto & card : foundation.cards) {
                    returns += FOUNDATION_POINTS.at(card.rank);
                }
            }

            // Calculates score for revealed cards in tableau
            int num_hidden_cards = 0;
            for (auto & tableau : tableaus) {
                for (auto & card : tableau.cards) {
                    // Cards that will be revealed by a chance node next turn are not counted
                    if (card.hidden and not (tableau.cards.back() == card)) {
                        num_hidden_cards += 1;
                    }
                }
            }
            // Difference of maximum number of hidden cards and the actual number, multiplied by 20 for each one
            returns += (21 - num_hidden_cards) * 20;

            // Calculates score for move from the waste
            int waste_cards_remaning = deck.cards.size() + deck.waste.size();
            returns += (24 - waste_cards_remaning) * 20;

            return {returns};
        } else {
            return {0.0};
        }
    }

    std::vector<double>     SolitaireState::Rewards() const {
        /* DECISION: Two ways of implementing this:
        - Find difference between current_score and previous_score
        - Calculate reward based on locations and ranks of cards in the move

       // Highest possible reward per action is 120.0 (e.g. ♠ ← As where As is on a hidden card)
       // Lowest possible reward per action is -100.0 (e.g. 2h ← As where As is in foundation initially) */

        // TODO: Should not be called on chance nodes (undefined and crashes)
        // SPIEL_CHECK_FALSE(IsChanceNode());
        if (is_setup) {
            std::vector<double> current_returns = Returns();
            double current_score = current_returns.front();
            return {current_score - previous_score};
        } else {
            return {0.0};
        }

    }

    std::vector<Action>     SolitaireState::LegalActions() const {

        std::vector<Action> legal_actions;
        for (auto move : CandidateMoves()) {
            legal_actions.push_back(move.ActionId());
        }
        legal_actions.push_back(kDraw);
        return legal_actions;

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

    std::vector<Card>       SolitaireState::Targets(std::optional<std::string> location) const {

        std::string loc = location.value_or("all");
        std::vector<Card> targets;

        // Gets targets from tableaus
        if (loc == "tableau" or loc == "all") {
            for (Tableau tableau : tableaus) {
                std::vector<Card> current_targets = tableau.Targets();
                targets.insert(targets.end(), current_targets.begin(), current_targets.end());
            }
        }

        // Gets targets from foundations
        if (loc == "foundation" or loc == "all") {
            for (Foundation foundation : foundations) {
                std::vector<Card> current_targets = foundation.Targets();
                targets.insert(targets.end(), current_targets.begin(), current_targets.end());
            }
        }

        // Returns targets as a vector of cards in all piles specified by "location"
        return targets;

    }

    std::vector<Card>       SolitaireState::Sources(std::optional<std::string> location) const {

        std::string loc = location.value_or("all");
        std::vector<Card> sources;

        // Gets sources from tableaus
        if (loc == "tableau" or loc == "all") {
            for (Tableau tableau : tableaus) {
                std::vector<Card> current_sources = tableau.Sources();
                sources.insert(sources.end(), current_sources.begin(), current_sources.end());
            }
        }

        // Gets sources from foundations
        if (loc == "foundation" or loc == "all") {
            for (Foundation foundation : foundations) {
                std::vector<Card> current_sources = foundation.Sources();
                sources.insert(sources.end(), current_sources.begin(), current_sources.end());
            }
        }

        // Gets sources from waste
        if (loc == "waste" or loc == "all") {
            std::vector<Card> current_sources = deck.Sources();
            sources.insert(sources.end(), current_sources.begin(), current_sources.end());
        }

        // Returns sources as a vector of cards in all piles specified by "location"
        return sources;
    }

    std::vector<Move>       SolitaireState::CandidateMoves() const {

        std::vector<Move> candidate_moves;
        std::vector<Card> targets = Targets();
        std::vector<Card> sources = Sources();

        // For target in targets ...
        for (auto target : targets) {

            // Get the targets legal children
            std::vector<Card> legal_children = {};

            try { legal_children = target.LegalChildren(); }
            catch (std::out_of_range()) {
                legal_children = {};
                continue;
            }

            // For child in targets legal children ...
            for (auto child : legal_children) {

                // If a legal child is found in sources ...
                if (std::find(sources.begin(), sources.end(), child) != sources.end()) {

                    // Add target, source pair to candidate moves
                    candidate_moves.emplace_back(target, child);
                }
            }
        }

        return candidate_moves;

    }

    Tableau *               SolitaireState::FindTableau(Card card) const {

        if (card.rank.empty() and card.suit.empty()) {
            for (auto & tableau : tableaus) {
                if (tableau.cards.empty()) {
                    return const_cast<Tableau *>(& tableau);
                }
            }
        }

        else {
            for (auto & tableau : tableaus) {
                if (not tableau.cards.empty()) {
                    if (std::find(tableau.cards.begin(), tableau.cards.end(), card) != tableau.cards.end()) {
                        return const_cast<Tableau *>(& tableau);
                    }
                }
            }
        }

        /*
        } else {
            std::cout << YELLOW << "WARNING: 'card.location' is not kTableau" << RESET << std::endl;
        }
        */
    }

    Foundation *            SolitaireState::FindFoundation(Card card) const {

        if (card.rank == "") {
            for (auto & foundation : foundations) {
                if (foundation.cards.empty() and foundation.suit == card.suit) {
                    return const_cast<Foundation *>(& foundation);
                }
            }
        }
        else {
            for (auto & foundation : foundations) {
                if (not foundation.cards.empty() and foundation.suit == card.suit) {
                    if (std::find(foundation.cards.begin(), foundation.cards.end(), card) != foundation.cards.end()) {
                        return const_cast<Foundation *>(& foundation);
                    }
                }
            }
        }

        /*
        } else {
            std::cout << YELLOW << "WARNING: 'card.location' is not kFoundation" << RESET << std::endl;
        }
        */
    }

    Location                SolitaireState::FindLocation(Card card) const {

        // Handles special cards
        if (card.rank.empty()) {
            if (card.suit.empty()) {
                return kTableau;
            } else {
                return kFoundation;
            }
        }

        // Attempts to find the card in a tableau
        for (auto & tableau : tableaus) {
            if (std::find(tableau.cards.begin(), tableau.cards.end(), card) != tableau.cards.end()) {
                return kTableau;
            }
        }

        // Attempts to find the card in a foundation
        for (auto & foundation : foundations) {
            if (std::find(foundation.cards.begin(), foundation.cards.end(), card) != foundation.cards.end()) {
                return kFoundation;
            }
        }

        // Attempts to find the card in the waste
        if (std::find(deck.waste.begin(), deck.waste.end(), card) != deck.waste.end()) {
            return kWaste;
        }

        // Attempts to find the card in the deck
        if (std::find(deck.cards.begin(), deck.cards.end(), card) != deck.cards.end()) {
            return kDeck;
        }

        // Default value is returned if the card isn't found
        return kMissing;
        
    }
    
    void                    SolitaireState::MoveCards(Move move) {
        // Unpack target and source from move
        Card target = move.target;
        Card source = move.source;

        // Find their locations in this state
        target.location = FindLocation(target);
        source.location = FindLocation(source);

        std::vector<Card> split_cards;

        switch (source.location) {
            case kTableau : {
                split_cards = FindTableau(source)->Split(source);
                break;
            }
            case kFoundation : {
                split_cards = FindFoundation(source)->Split(source);
                break;
            }
            case kWaste : {
                split_cards = deck.Split(source);
                break;
            }
            default : {
                std::cout << YELLOW << "WARNING: 'source' is not in a tableau, foundation, or waste" << RESET << std::endl;
                std::cout << YELLOW << "WARNING: 'source' = " << source.ToString() << RESET << std::endl;
            }
        }

        switch (target.location) {
            case kTableau : {
                Tableau * target_container = FindTableau(target);
                target_container->Extend(split_cards);
                break;
            }
            case kFoundation : {
                Foundation * target_container = FindFoundation(target);
                target_container->Extend(split_cards);
                break;
            }
            default : {
                std::cout << YELLOW << "WARNING: 'target' is not in a tableau or foundation" << RESET << std::endl;
                std::cout << YELLOW << "WARNING: 'target' = " << target.ToString() << RESET << std::endl;
            }
        }

    }

    bool                    SolitaireState::OverHidden(Card card) const {
        if (card.location == kTableau) {
            Tableau * container = FindTableau(card);
            bool previous_card_hidden = false;
            for (auto & current_card : container->cards) {
                if (card == current_card) {
                    return previous_card_hidden;
                }
                if (card.hidden) {
                    previous_card_hidden = true;
                }
            }
        }
        return false;
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


/*
 *
 */
