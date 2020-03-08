# Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Python spiel example."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import random
from absl import app
from absl import flags
import numpy as np
from pprint import pprint

import sys
sys.path.append("/Users/tylerchurchill/CLionProjects/open_spiel")
sys.path.append("/Users/tylerchurchill/CLionProjects/open_spiel/build/python")
import pyspiel

FLAGS = flags.FLAGS

flags.DEFINE_string("game", "tic_tac_toe", "Name of the game")
flags.DEFINE_integer("players", None, "Number of players")
flags.DEFINE_string("load_state", None,
                    "A file containing a string to load a specific state")


def main(_):
  games_list = pyspiel.registered_games()
  print("Registered games:")
  print(games_list)

  action_string = None

  print("Creating game: " + FLAGS.game)
  if FLAGS.players is not None:
    game = pyspiel.load_game(FLAGS.game,
                             {"players": pyspiel.GameParameter(FLAGS.players)})
  else:
    game = pyspiel.load_game(FLAGS.game)

  # Get a new state
  if FLAGS.load_state is not None:
    # Load a specific state
    state_string = ""
    with open(FLAGS.load_state, encoding="utf-8") as input_file:
      for line in input_file:
        state_string += line
    state_string = state_string.rstrip()
    print("Loading state:")
    print(state_string)
    print("")
    state = game.deserialize_state(state_string)
  else:
    pprint(dir(game))


    print('get_parameters():               ', game.get_parameters())
    print('get_type():                     ', game.get_type())
    print('num_players():                  ', game.num_players())
    print('information_state_tensor_layout:', game.information_state_tensor_layout())
    print('information_state_tensor_shape: ', game.information_state_tensor_shape())
    print('information_state_tensor_size:  ', game.information_state_tensor_size())
    print('max_chance_outcomes:            ', game.max_chance_outcomes())
    print('max_game_length:                ', game.max_game_length())
    print('max_utility:                    ', game.max_utility())
    print('min_utility:                    ', game.min_utility())
    print('new_initial_state:              ', game.new_initial_state())
    print('num_distinct_actions:           ', game.num_distinct_actions())
    print('num_players:                    ', game.num_players())
    print('observation_tensor_layout:      ', game.observation_tensor_layout())
    print('observation_tensor_shape:       ', game.observation_tensor_shape())
    print('observation_tensor_size:        ', game.observation_tensor_size())
    print('policy_tensor_shape:            ', game.policy_tensor_shape())
    print('utility_sum:                    ', game.utility_sum())


    state = game.new_initial_state()

  # Print the initial state
  print(str(state))

  while not state.is_terminal():
    # The state can be three different types: chance node,
    # simultaneous node, or decision node
    if state.is_chance_node():
      # Chance node: sample an outcome
      outcomes = state.chance_outcomes()
      num_actions = len(outcomes)
      print("Chance node, got " + str(num_actions) + " outcomes")
      action_list, prob_list = zip(*outcomes)
      action = np.random.choice(action_list, p=prob_list)
      print("Sampled outcome: ",
            state.action_to_string(state.current_player(), action))
      state.apply_action(action)

    elif state.is_simultaneous_node():
      # Simultaneous node: sample actions for all players.
      chosen_actions = [
          random.choice(state.legal_actions(pid))
          for pid in range(game.num_players())
      ]
      print("Chosen actions: ", [
          state.action_to_string(pid, action)
          for pid, action in enumerate(chosen_actions)
      ])
      state.apply_actions(chosen_actions)

    else:
      # Decision node: sample action for the single current player
      action = random.choice(state.legal_actions(state.current_player()))
      action_string = state.action_to_string(state.current_player(), action)
      print("Player ", state.current_player(), ", randomly sampled action: ",
            action_string)
      state.apply_action(action)

    print(str(state))

  # Game is now done. Print utilities for each player
  returns = state.returns()
  for pid in range(game.num_players()):
    print("Utility for player {} is {}".format(pid, returns[pid]))


if __name__ == "__main__":
  app.run(main)
