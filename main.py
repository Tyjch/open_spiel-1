import sys

sys.path.append("/Users/tylerchurchill/CLionProjects/open_spiel")
sys.path.append("/Users/tylerchurchill/CLionProjects/open_spiel/build/python")

import pyspiel
import numpy as np
from pprint import pprint

import tensorflow.compat.v1 as tf
from open_spiel.python import policy
from open_spiel.python.algorithms import deep_cfr, exploitability

games = [
    'klondike',
    #'blotto',
    #'bridge_uncontested_bidding',
    #'catch',
    #'cliff_walking',
    #'deep_sea',
    #'first_sealed_auction',
    #'kuhn_poker',
    #'leduc_poker',
    #'liars_dice',
    #'matching_pennies_3p',
    #'phantom_ttt',
    #'tiny_bridge_2p',
    #'tiny_bridge_4p',
    #'tiny_hanabi',
]


game = pyspiel.load_game('klondike')


with tf.Session() as session:
    algo = deep_cfr.DeepCFRSolver(
        session,
        game,
        policy_network_layers=(8, 4),
        advantage_network_layers=(4, 2),
        num_iterations=10,
        num_traversals=2,
        learning_rate=1e-3,
        batch_size_advantage=None,
        batch_size_strategy=None,
        memory_capacity=1e7
    )
    print(game)
    print(type(game))
    print(dir(game))

    session.run(tf.global_variables_initializer())
    algo.solve()
    conv = exploitability.nash_conv(
        game,
        policy.PolicyFromCallable(game, algo.action_probabilities)
    )

    print(f'Exploitability: {conv}')


