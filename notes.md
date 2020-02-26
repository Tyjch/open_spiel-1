# `spiel.h`
## `struct GameType`
### `short_name = "klondike"`
A short name that uniquely identifies the game

### `long_name = "Klondike Solitaire"`
A long, human-readable name

### `dynamics = kSequential`
Klondike is sequential because it is played in turns. There are no simultaneous actions.

### `chance_mode = kExplicitStochastic`
`kExplicitStochastic` explicitly returns all possible chance outcomes.
`kSampledStochastic` samples one chance outcome randomly. 

### `information = kImperfectInformation`
Some information is hidden to players in klondike.

### `utility = kGeneralSum`
As klondike is a single-player game, the utility of the player varies in different outcomes.

### `reward_model = kRewards`
At each step, there is a reward in the typical RL-style. Could also use `kTerminal` and just sum the
rewards of each step.

### `max_num_players = 1` & `min_num_players = 1`
Klondike is a single player game

### `provides_information_state_string = true` & `provides_information_state_tensor = true`
An information state is a __perfect-recall__ state from the perspective of one-player.

### `provides_observation_string = true` & `provides_observation_tensor = true`
Some subset of the information state with the property that remembering all the player's observations
and actions is sufficient to reconstruct the information state.

## 
