import numpy as np

def maxwell(speed, kbt, mass = 1.0):
    return (mass * speed / kbt) * np.exp(-mass * speed**2 / (2 * kbt))