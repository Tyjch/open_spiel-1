#ifndef THIRD_PARTY_OPEN_SPIEL_GAMES_SOLITAIRE_H
#define THIRD_PARTY_OPEN_SPIEL_GAMES_SOLITAIRE_H

#include <array>
#include <memory>
#include <string>
#include <vector>
#include "open_spiel/spiel.h"

namespace open_spiel::solitaire {

    inline constexpr int kDefaultPlayers = 1;

    /*
    To get suit values from suit, just do GetIndex(SUITS, "s");
    To get rank values from rank, just do GetIndex(RANKS, "A");
    To get suit from suit value, just do SUITS[suit_value];
    To get rank from rank value, just do RANKS[rank_value];
    */

    template <typename Container, typename Element>
    int GetIndex (Container container, Element element) {
        return std::distance(std::begin(container), std::find(container.begin(), container.end(), element));
    }

    const std::vector<std::string> SUITS = {"s", "h", "c", "d"};
    const std::vector<std::string> RANKS = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K"};

    // Enumerations ====================================================================================================

    enum ActionType {
        // TODO: Actions need to be contiguous integers starting from 0
        kStartSetup = 0,


    };

    enum Location {
        kDeck       = 0,
        kWaste      = 1,
        kFoundation = 2,
        kTableau    = 3,
        kMissing    = 4,
    };

    // Support Classes =================================================================================================

    class Card {
    public:

        // Attributes ==================================================================================================

        std::string rank;       // Indicates the rank of the card, cannot be changed once set
        std::string suit;       // Indicates the suit of the card, cannot be changed once set
        bool        hidden;     // Indicates whether the card is hidden or not
        Location    location;   // Indicates the type of pile the card is in

        // Constructors ================================================================================================

        Card();                                     // Create an empty card, default constructor
        Card(std::string rank, std::string suit);   // Create a card from rank, suit, and hidden
        explicit Card(int index);                   // Create a card from its index (e.g. 0 -> As)

        // Type Casting ================================================================================================

        explicit operator int() const;  // Represent a card as its integer index

        // Operators ===================================================================================================

        bool operator==(Card & other_card) const;                                   // Compare two cards for equality

        // Other Methods ===============================================================================================

        std::vector<Card> LegalChildren() const;    // Get legal children of the card depending on its location

    };

    class Deck {
    public:

        // Attributes ==================================================================================================

        std::deque<Card> cards;             // Holds the card current in the deck
        std::deque<Card> waste;             // Holds the waste cards, the top of which can be played
        std::deque<Card> initial_order;     // Holds the initial order of the deck, so that it can be rebuilt
        int times_rebuilt;                  // Number of times Rebuild() is called, used for score or terminality

        // Constructors ================================================================================================

        Deck();     // Default constructor

        // Operators ===================================================================================================

        // friend std::ostream & operator<<(std::ostream & os, const Deck & deck);     // Output to stream

        // Other Methods ===============================================================================================

        // TODO: I don't know if shuffle is even needed.
        void draw(int num_cards);                               // Moves cards to the waste
        std::deque<Card> deal(unsigned long int num_cards);     // Removes a certain number of cards and returns them
        // void shuffle(int seed);                                  // Shuffles the cards deterministically
        // void rebuild();                                         // Reconstructs cards from initial order


    };

    class Foundation {
    public:

        // Attributes ==================================================================================================

        const std::string suit;                     // Indicates the suit of cards that can be added
        std::deque<Card>  cards;                    // Contains the cards inside the foundation

        // Constructors ================================================================================================

        explicit Foundation(std::string suit);      // Construct an empty foundation of a given suit

        // Operators ===================================================================================================

        // friend std::ostream & operator<<(std::ostream & os, const Foundation & foundation);     // Output to stream

        // Other Methods ===============================================================================================

        std::vector<Card> Sources() const;                // Cards in the foundation that can be moved
        std::vector<Card> Targets() const;                // A card in the foundation that can have cards moved to it

        std::vector<Card> Split(Card card);       // Splits on given card and returns it and all cards beneath it
        void Extend(std::vector<Card> source_cards);       // Adds cards to the foundation

    };

    class Tableau {
    public:

        // Attributes ==================================================================================================

        std::deque<Card> cards;                     // Contains the cards inside the foundation

        // Constructors ================================================================================================

        explicit Tableau(int num_cards);            // Construct a tableau with the given cards

        // Operators ===================================================================================================

        // friend std::ostream & operator<<(std::ostream & os, const Tableau & tableau);     // Output to stream

        // Other Methods ===============================================================================================

        std::vector<Card> Sources() const;                // Cards in the foundation that can be moved
        std::vector<Card> Targets() const;                // A card in the foundation that can have cards moved to it

        std::vector<Card> Split(Card card);       // Splits on given card and returns it and all cards beneath it
        void Extend(std::vector<Card> source_cards);       // Adds cards to the foundation
    };

    class Move {
    public:

        // Attributes ==================================================================================================

        Card target;
        Card source;

        // Other Methods ===============================================================================================

        std::string ToString() const;
        bool        IsReversible() const;

    };

    // OpenSpiel Classes ===============================================================================================

    class SolitaireGame;

    class SolitaireState : public State {
    public:
        // Attributes ==================================================================================================

        Deck                    deck;
        std::vector<Foundation> foundations;
        std::vector<Tableau>    tableaus;

        // Constructors ================================================================================================

        explicit SolitaireState(std::shared_ptr<const Game> game);

        // Overriden Methods ===========================================================================================

        Player                 CurrentPlayer() const override;
        std::unique_ptr<State> Clone() const override;
        bool                   IsTerminal() const override;
        bool                   IsChanceNode() const override;
        std::string            ToString() const override;
        std::string            ActionToString(Player player, Action action_id) const override;
        std::string            InformationStateString(Player player) const override;
        std::string            ObservationString(Player player) const override;
        void                   InformationStateTensor(Player player, std::vector<double> * values) const override;
        void                   ObservationTensor(Player player, std::vector<double> * values) const override;
        void                   DoApplyAction(Action move) override;
        std::vector<double>    Returns() const override;
        std::vector<double>    Rewards() const override;
        std::vector<Action>    LegalActions() const override;
        std::vector<std::pair<Action, double>> ChanceOutcomes() const override;

        // Other Methods ===============================================================================================

        //std::string         GetContainerType(Card card) const;
        std::vector<Card>   Targets(const std::string & location) const;
        std::vector<Card>   Sources(const std::string & location) const;
        std::vector<Action> CandidateActions() const;

        // TODO: This should take an Move object instead of a pair of cards
        void                MoveCards(Move & move);
        // bool                IsReversible(Action action) const;
        // const std::deque<Card> * GetContainer(Card card) const;
        // double CurrentScore() const;
    private:
        bool is_setup;

    };

    class SolitaireGame : public Game {
    public:

        // Constructor =================================================================================================

        explicit SolitaireGame(const GameParameters & params);

        // Overriden Methods ===========================================================================================

        int     NumDistinctActions() const override;
        int     MaxGameLength() const override;
        int     NumPlayers() const override;
        double  MinUtility() const override;
        double  MaxUtility() const override;

        std::vector<int> InformationStateTensorShape() const override;
        std::vector<int> ObservationTensorShape() const override;

        std::unique_ptr<State>       NewInitialState() const override;
        std::shared_ptr<const Game>  Clone() const override;

    private:
        int num_players_;
    };

} // namespace open_spiel::solitaire

#endif // THIRD_PARTY_OPEN_SPIEL_GAMES_SOLITAIRE_H
