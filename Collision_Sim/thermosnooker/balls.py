"""
Ball class 
"""
import numpy as np
from matplotlib.patches import Circle


class Ball:
    def __init__(self, pos=None, vel=None, radius=1.0, mass=1.0) -> None:
        """Constructor that ensures default initilisation and ensures that there is not an incorrect input for pos and vel."""
        if pos is None:
            pos = [0.0, 0.0]
        if vel is None:
            vel = [1.0, 0.0]

        if not isinstance(pos, (list, tuple, np.ndarray)) or not isinstance(vel, (list, tuple, np.ndarray)):
            raise TypeError("You have entered an invalid type for the creation of this Ball object")

        if len(pos) != 2 or len(vel) != 2:
            raise Exception("Vectors must be exactly 2D")

        self._pos = np.array(pos, dtype=float)
        self._vel = np.array(vel, dtype=float)
        self._radius = radius
        self._mass = float(mass)
        self._patch = Circle((self._pos[0], self._pos[1]), radius=self._radius)  # Expects tuple for centre argument 
    
    def pos(self):
        """Returns Position"""
        return self._pos
    
    def radius(self):
        """Returns Radius"""
        return self._radius
    
    def mass(self):
        """Returns Mass"""
        return self._mass
    
    def vel(self):
        """Returns Velocity"""
        return self._vel
    
    def patch(self):
        """Returns patch object"""
        return self._patch
    
    def set_vel(self, vel):
        """Changes velocity, does not return anything"""
        if len(vel) != 2:
            raise Exception("Velocity must be exactly 2D")
        self._vel = np.array(vel)
    
    def move(self, dt):
        """Changes position based on time ellapsed and current velocity, does not return anything"""
        self._pos += self._vel * dt
        self._patch.set_center((self._pos[0], self._pos[1]))  # Expects tuple for centre argument 
    
    def time_to_collision(self, other):
        """
        Calculate the time until a collision occurs with another object.
        Returns None if no collision occurs.
        """
        deltar = self.pos() - other.pos()
        deltav = self.vel() - other.vel()
        
        is_container = type(self).__name__ == "Container" or type(other).__name__ == "Container"
        
        if type(self).__name__ == "Container":
            dist_for_collision = self.radius() - other.radius()
        elif type(other).__name__ == "Container":
            dist_for_collision = other.radius() - self.radius()
        else:
            dist_for_collision = self.radius() + other.radius()

        a = np.dot(deltav, deltav)
        if a == 0.0:
            return None
            
        b = 2 * np.dot(deltar, deltav)
        c = np.dot(deltar, deltar) - dist_for_collision * dist_for_collision

        discriminant = b * b - 4 * a * c
        if discriminant < 0:
            return None
            
        if is_container:
            t = (-b + np.sqrt(discriminant)) / (2 * a)
            return t if t >= 0 else None
        else:
            
            if b >= 0:
                return None
            t = (-b - np.sqrt(discriminant)) / (2 * a)
            return t if t >= 0 else None
        
    
    def collide(self, other):
        """Changes velocity of objects post collision"""
        if isinstance(other, Ball):
            deltar = self.pos() - other.pos()
            deltav = self.vel() - other.vel()
            mass_sum = other.mass() + self.mass()

            #By PEP8 standard i have split it across multiple lines to avoid the 78 character scroll limit

            v1_new = self.vel() - (2 * other.mass() / mass_sum) * (
                np.dot(deltav, deltar) / np.dot(deltar, deltar)
                ) * deltar
            v2_new = other.vel() - (2 * self.mass() / mass_sum) * (
                np.dot(-deltav, -deltar) / np.dot(deltar, deltar)
                ) * (-deltar)
            
            self.set_vel(v1_new)
            other.set_vel(v2_new)
        elif type(other).__name__ == "Container":
            other.collide(self)
        else:
            raise TypeError("Collision Calculation not supported for this Type")


class Container(Ball):
    def __init__(self, radius=10.0, mass=10000000):
        
        super().__init__(pos=[0.0, 0.0], vel=[0.0, 0.0], radius=radius, mass=mass)
        self._dp_tot = 0.0
        self._n_container_collisions = 0

        self.patch().set_fill(False)
        self.patch().set_edgecolor("r")
    
    def volume(self):
        return np.pi * self.radius() * self.radius()
    
    def surface_area(self):
        return 2 * np.pi * self.radius()
    
    def dp_tot(self):
        return self._dp_tot
    
        
    def collide(self, other):
        modulus = np.linalg.norm(other.pos())
        normal_vec = other.pos() / modulus
        v_old = other.vel()
        v_dot_n = np.dot(v_old, normal_vec)

        dp = 2 * other.mass() * v_dot_n
        self._dp_tot += abs(dp)

        self._n_container_collisions += 1

        v_new = v_old - 2 * v_dot_n * normal_vec
        
        other.set_vel(v_new)

    def n_container_collisions(self):
        return self._n_container_collisions


