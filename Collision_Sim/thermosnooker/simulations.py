"""Simulation Module"""
import matplotlib.pylab as plt
from thermosnooker.balls import Container, Ball
import numpy as np


class Simulation:
    def next_collision(self):
        raise NotImplementedError('next_collision() needs to be implemented in derived classes')
    
    def setup_figure(self):
        raise NotImplementedError('setup_figure() needs to be implemented in derived classes')
    
    def run(self, num_collisions, animate=False, pause_time=0.001):
        if animate:
            fig, axes = self.setup_figure()
        for _ in range(num_collisions):
            self.next_collision()
            if animate:
                plt.pause(pause_time)
        if animate:
            plt.show()

class SingleBallSimulation(Simulation):
    def __init__(self, container, ball ):
        self._container = container
        self._ball = ball

    def container(self):
        return self._container
    
    def ball(self):
        return self._ball
    
    def setup_figure(self):
        radius = self.container().radius()
        fig = plt.figure()
        ax = plt.axes(xlim = (-radius, radius), ylim = (-radius,radius))
        ax.add_artist(self.container().patch())
        ax.add_patch(self.ball().patch())

        return fig, ax
    
    def next_collision(self):
        dt = self._container.time_to_collision(self.ball())
        self._ball.move(dt)
        self._container.collide(self._ball)

class MultiBallSimulation(Simulation):
    def __init__(self, 
            c_radius = 10.0, 
            b_radius = 1.0, 
            b_speed = 10.0, 
            b_mass=1.0, 
            rmax=8.0,
            nrings=3,
            multi=6
        ):
        self._container = Container(radius=c_radius)
        self._balls = []
        self._fig = None

        for k in range(1,nrings + 1):
            r = k * (rmax/nrings)
            n_balls = k * multi
            for i in range(n_balls):
                theta = i * (2 * np.pi / n_balls)
                pos = [r * np.sin(theta), r * np.cos(theta)]
                phi = np.random.uniform(0, 2 * np.pi)
                vel = [b_speed * np.cos(phi), b_speed * np.sin(phi)]

                self._balls.append(Ball(pos=pos, vel=vel, radius=b_radius, mass = b_mass))
        self._balls.append(Ball(pos=[0.0, 0.0], vel=[0.0, 0.0], radius=b_radius, mass=b_mass))
        self._time = 0.0
    
    def container(self):
        return self._container
    
    def balls(self):
        return self._balls
    
    def setup_figure(self):
        rad = self.container().radius()
        self._fig = plt.figure()
        ax = plt.axes(xlim=(-rad,rad), ylim=(-rad,rad))
        ax.set_aspect("equal")
        ax.add_artist(self.container().patch())

        for ball in self.balls():
            ax.add_patch(ball.patch())
        return self._fig, ax
    
    def next_collision(self):
        min_time = float("inf")
        collision_pair = (None, None)

        for ball in self._balls:
            t = self._container.time_to_collision(ball)
            if t is not None and 1e-9 < t < min_time:
                min_time = t
                collision_pair = (self._container, ball)
        
        num_balls = len(self._balls)
        for i in range(num_balls):
            for j in range(i + 1, num_balls):
                t = self._balls[i].time_to_collision(self._balls[j])
                if t is not None and 1e-9 < t < min_time:
                    min_time = t
                    collision_pair = (self._balls[i], self._balls[j])
        self._time += min_time
        
        for ball in self._balls:
            ball.move(min_time)

        obj1, obj2 = collision_pair
        if obj1 is not None and obj2 is not None:
            obj1.collide(obj2)
    
    def time(self):
        return self._time
    
    def kinetic_energy(self):
        total_ke = 0.0
        for ball in self._balls:
            v = ball.vel()
            total_ke += 0.5 * ball.mass() * np.dot(v,v)
        
        return total_ke
    
    def t_equipartition(self):
        num_balls = len(self._balls)
        if num_balls == 0:
            return 0.0
        return self.kinetic_energy() / num_balls

    def momentum(self):
        total_mom = np.array([0.0, 0.0])
        for ball in self._balls:
            total_mom += ball.mass() * ball.vel()
        
        return total_mom
    
    def pressure(self):
        if self._time == 0.0:
            return 0.0
        circumference = 2 * np.pi * self._container.radius()
        
        return self._container.dp_tot() / (self._time * circumference)
    
    def t_ideal(self):
        num_balls = len(self._balls)
        if num_balls == 0:
            return 0.0
        
        container_area = np.pi * (self._container.radius() ** 2)
        return (self.pressure() * container_area) / num_balls
    
    def speeds(self):
        return [np.linalg.norm(ball.vel()) for ball in self._balls]
    
    def rdf(self, nbins, bin_range):
        pos = np.array([ball.pos() for ball in self._balls])
        num_balls = len(pos)
        r_cont = self._container.radius()
        
        distances = []

        for i in range(num_balls):
            for j in range(i + 1, num_balls):
                dist = np.linalg.norm(pos[i] - pos[j])
                distances.append(dist)
        
        counts, bin_edges = np.histogram(distances, bins=nbins, range = bin_range)
        counts = counts * 2

        bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
        dr = bin_edges[1] - bin_edges[0]

        number_density = num_balls / (np.pi * r_cont ** 2)
        g_r = np.zeros(nbins)

        for b in range(nbins):
            r = bin_centers[b]
            a_shell = 2 * np.pi * r * dr
            a_eff_r = 0.0

            for i in range(num_balls):
                di = np.linalg.norm(pos[i])
                if di == 0.0:
                    f_arc = 1.0 if r <= r_cont else 0.0
                
                elif di + r <= r_cont:
                    f_arc = 1.0
                elif di >= r_cont + r:
                    f_arc = 0.0
                else:
                    val = (di**2 + r**2 - r_cont**2) / (2 * di * r)
                    val = np.clip(val, -1.0, 1.0)
                    theta = 2 * np.arccos(val)
                    f_arc = theta / (2 * np.pi)
                a_eff_r += a_shell * f_arc
            
            if a_eff_r > 0 :
                g_r[b] = counts[b] / (number_density * a_eff_r)
        
        return g_r, bin_edges
    
    

class BrownianSimulation(MultiBallSimulation):
    def __init__(self, c_radius=10.0, b_radius=1.0, b_speed=1.0,
            b_mass=1.0, rmax=8.0, nrings=3, multi=6,
            bb_radius=2.0, bb_mass=10.0):
        super().__init__(c_radius=c_radius,b_radius=b_radius,
                        b_speed=b_speed, b_mass=b_mass, rmax=rmax, 
                        nrings=nrings, multi=multi)
        self._bb = Ball(pos=[0.0, 0.0], vel=[0.0, 0.0], radius=bb_radius, mass=bb_mass)
        
        self._balls.append(self._bb)
        self._bb_positions = [self._bb.pos().copy()]
    
    def bb_positions(self):
        return self._bb_positions
    
    def next_collision(self):
        super().next_collision()
        self._bb_positions.append(self._bb.pos().copy())