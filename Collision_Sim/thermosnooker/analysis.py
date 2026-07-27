"""Analysis Module."""
import itertools
import matplotlib.pyplot as plt
import numpy as np
from thermosnooker.balls import Container, Ball
from thermosnooker.simulations import SingleBallSimulation, MultiBallSimulation, BrownianSimulation
from thermosnooker._utils.decorators import SaveOutput
from thermosnooker.physics import maxwell
from scipy.optimize import curve_fit
import scipy.constants as const
import matplotlib.animation as animation

def task9():
    """
    Task 9.

    In this function, you should test your animation. To do this, create a container
    and ball as directed in the project brief. Create a SingleBallSimulation object from these
    and try running your animation. Ensure that this function returns the balls final position and
    velocity.

    Returns:
        tuple[NDArray[np.float64], NDArray[np.float64]]: The balls final position and velocity
    """
    container = Container(radius=10.0)
    ball = Ball(pos=[-5.0,0.0], vel=[1.0, 0.0], radius=1.0, mass=1.0)
    sim = SingleBallSimulation(container=container, ball=ball)

    sim.run(num_collisions=20, animate=True, pause_time=0.5)

    return ball.pos(), ball.vel()


@SaveOutput("task10")
def task10():
    """
    Task 10.

    In this function we shall test your MultiBallSimulation. Create an instance of this class using
    the default values described in the project brief and run the animation for 500 collisions.

    Watch the resulting animation carefully and make sure you aren't seeing errors like balls sticking
    together or escaping the container.

    Returns:
        Figure: The MultiBallSimulation simulation plot
    """
    sim = MultiBallSimulation()

    sim.run(num_collisions=500, animate=True, pause_time=0.001)

    return sim._fig


@SaveOutput(["task11a", "task11b"])
def task11():
    """
    Task 11.

    In this function we shall be quantitatively checking that the balls aren't escaping or sticking.
    To do this, create the two histograms as directed in the project script. Ensure that these two
    histogram figures are returned.

    Returns:
        tuple[Figure, Firgure]: The histograms (distance from centre, inter-ball spacing).
    """

    sim = MultiBallSimulation()
    sim.run(num_collisions=2000, animate=False)
    balls = sim.balls()

    centre_distances = []
    for ball in balls:
        dist = np.linalg.norm(ball.pos())
        centre_distances.append(dist)
    
    interball_distances = []
    num_balls = len(balls)
    for i in range(num_balls):
        for j in range(i + 1, num_balls):
            delta_p = balls[i].pos() - balls[j].pos()
            dist = np.linalg.norm(delta_p)
            interball_distances.append(dist)
    
    fig_a, ax_a = plt.subplots()
    ax_a.hist(centre_distances, bins=30, edgecolor="black", color="skyblue")
    ax_a.set_title("Distribution of Ball Distances from Container Centre")
    ax_a.set_xlabel("Distance from Centre")
    ax_a.set_ylabel("Frequency Count")
    ax_a.axvline(x=9.0, color="red", linestyle="--", label="Max Allowed (R - r)")
    ax_a.legend()

    fig_b, ax_b = plt.subplots()
    ax_b.hist(interball_distances, bins=40, edgecolor="black", color="salmon")
    ax_b.set_title("Distribution of Pairwise Inter-Ball Distances")
    ax_b.set_xlabel("Inter-Ball Distance")
    ax_b.set_ylabel("Frequency Count")
    ax_b.axvline(x=2.0, color="red", linestyle="--", label="Min Allowed (r1 + r2)")
    ax_b.legend()

    return fig_a, fig_b


@SaveOutput(["task12a", "task12b", "task12c", "task12d"])
def task12():
    """
    Task 12.

    In this function we shall check that the fundamental quantities of energy and momentum are conserved.
    Additionally we shall investigate the pressure evolution of the system. Ensure that the 4 figures
    outlined in the project script are returned.

    Returns:
        tuple[Figure, Figure, Figure, Figure]: matplotlib Figures of the KE, momentum_x, momentum_y ratios
        as well as pressure evolution.
    """
    sim = MultiBallSimulation()

    initial_ke = sim.kinetic_energy()
    initial_mom = sim.momentum()

    init_px = initial_mom[0] if abs(initial_mom[0]) > 1e-9 else 1.0
    init_py = initial_mom[1] if abs(initial_mom[1]) > 1e-9 else 1.0

    times = []
    ke_ratios = []
    px_ratios = []
    py_ratios = []
    pressures = []

    for i in range(1000):
        sim.next_collision()
        current_time = sim.time()
        current_mom = sim.momentum()

        times.append(current_time)
        ke_ratios.append(sim.kinetic_energy() / initial_ke)
        px_ratios.append(current_mom[0] / init_px)
        py_ratios.append(current_mom[1] / init_py)
        pressures.append(sim.pressure())
    
    fig_ke, ax_ke = plt.subplots()
    ax_ke.plot(times, ke_ratios, color="blue")
    ax_ke.set_title("Kinetic Energy Conservation")
    ax_ke.set_xlabel("Time (s)")
    ax_ke.set_ylabel("E_k(t) / E_k(0)")
    ax_ke.set_ylim(0.99, 1.01)

    fig_px, ax_px = plt.subplots()
    ax_px.plot(times, px_ratios, color="green")
    ax_px.set_title("X-Momentum Ratio")
    ax_px.set_xlabel("Time (s)")
    ax_px.set_ylabel("P_x(t) / P_x(0)")

    fig_py, ax_py = plt.subplots()
    ax_py.plot(times, py_ratios, color="purple")
    ax_py.set_title("Y-Momentum Ratio")
    ax_py.set_xlabel("Time (s)")
    ax_py.set_ylabel("P_y(t) / P_y(0)")

    fig_p, ax_p = plt.subplots()
    ax_p.plot(times, pressures, color="red")
    ax_p.set_title("System Pressure over Time")
    ax_p.set_xlabel("Time (s)")
    ax_p.set_ylabel("Pressure")
    return fig_ke, fig_px, fig_py, fig_p


@SaveOutput(["task13a", "task13b", "task13c"])
def task13():
    """
    Task 13.

    In this function we investigate how well our simulation reproduces the distributions of the IGL.
    Create the 3 figures directed by the project script, namely:
    1) PT plot
    2) PV plot
    3) PN plot
    Ensure that this function returns the three matplotlib figures.

    Returns:
        tuple[Figure, Figure, Figure]: The 3 requested figures: (PT, PV, PN)
    """

    speeds = np.linspace(2, 20, 7)
    container_radii = np.linspace(6, 15, 7)
    ring_counts = [1, 2, 3 ,4]

    ball_radii = [0.01, 0.1, 0.5]
    num_collisions_per_run = 800

    fig_pt, ax_pt = plt.subplots()
    fig_pv, ax_pv = plt.subplots()
    fig_pn, ax_pn = plt.subplots()

    for b_r in ball_radii:
        sim_pressures = []
        sim_temps = []

        for speed in speeds:
            sim = MultiBallSimulation(c_radius=10.0, b_radius=b_r, b_speed=speed)
            sim.run(num_collisions=num_collisions_per_run, animate=False)
            sim_pressures.append(sim.pressure())
            sim_temps.append(sim.t_equipartition())
        
        ax_pt.scatter(sim_temps, sim_pressures, label=f"Sim r ={b_r}")

        if b_r == 0.01:
            temps_fine = np.linspace(min(sim_temps), max(sim_temps), 100)

            igl_p = (36 * temps_fine) / (np.pi * 10.0**2)
            ax_pt.plot(temps_fine, igl_p, linestyle="--", label="Ideal Gas Law")
    
    ax_pt.set_title("Pressure vs Temperature (PT)")
    ax_pt.set_xlabel("Temperature")
    ax_pt.set_ylabel("Pressure")
    ax_pt.legend()
    ax_pt.grid(True)

    for b_r in ball_radii:
        sim_pressures = []
        container_areas = []

        for c_r in container_radii:
            sim = MultiBallSimulation(c_radius=c_r, b_radius=b_r, b_speed=10.0)
            sim.run(num_collisions=num_collisions_per_run, animate=False)
            sim_pressures.append(sim.pressure())
            container_areas.append(np.pi * c_r**2)

        ax_pv.scatter(container_areas, sim_pressures, label=f"Sim r={b_r}")

        if b_r == 0.01:
            areas_fine = np.linspace(min(container_areas), max(container_areas), 100)
            igl_p = (36 * 50.0) / areas_fine
            ax_pv.plot(areas_fine, igl_p, linestyle="--", label="Ideal Gas Law")
    
    ax_pv.set_title("Pressure vs Area (PV)")
    ax_pv.set_xlabel("Container Area")
    ax_pv.set_ylabel("Pressure")
    ax_pv.legend()
    ax_pv.grid(True)

    for b_r in ball_radii:
        sim_pressures = []
        particle_counts = []

        for rings in ring_counts:
            sim  = MultiBallSimulation(c_radius=10.0, b_radius=b_r, b_speed=10.0, nrings=rings)
            sim.run(num_collisions=num_collisions_per_run, animate=False)
            sim_pressures.append(sim.pressure())
            particle_counts.append(len(sim.balls()))
        
        ax_pn.scatter(particle_counts, sim_pressures, label=f"Sim (r={b_r})")
        
        if b_r == 0.01:
            n_fine = np.linspace(min(particle_counts), max(particle_counts), 100)
            igl_p = (n_fine * 50.0) / (np.pi * 10.0**2)
            ax_pn.plot(n_fine, igl_p, linestyle="--", label="Ideal Gas Law")

    ax_pn.set_title("Pressure vs Number of Particles (PN)")
    ax_pn.set_xlabel("Particle Count (N)")
    ax_pn.set_ylabel("Pressure")
    ax_pn.legend()
    ax_pn.grid(True)

    return fig_pt, fig_pv, fig_pn


@SaveOutput("task14")
def task14():
    """
    Task 14.

    In this function we shall be looking at the divergence of our simulation from the IGL. We shall
    quantify the ball radii dependence of this divergence by plotting the temperature ratio defined in
    the project brief.

    Returns:
        Figure: The temperature ratio figure.
    """

    ball_radii = np.linspace(0.01, 0.5, 15)
    temperature_ratios = []

    for r in ball_radii:
        sim = MultiBallSimulation(b_radius=r)

        sim.run(num_collisions=1000, animate=False)

        t_eq = sim.t_equipartition()
        t_id = sim.t_ideal()

        if t_id > 1e-9:
            ratio = t_eq / t_id
        else:
            ratio = 1.0

        temperature_ratios.append(ratio)
    
    fig, ax = plt.subplots()
    ax.plot(ball_radii, temperature_ratios, marker="o", color="darkmagenta", linestyle="-")
    ax.set_title("Deviation from Ideal Gas Law")
    ax.set_xlabel("Ball Radius (m)")
    ax.set_ylabel(r"Temperature Ratio ($T_{eq} / T_{ideal}$)")
    
    # Add a baseline for a perfect Ideal Gas
    ax.axhline(y=1.0, color="gray", linestyle="--", label="Perfect Ideal Gas (Point Masses)")
    ax.legend()
    ax.grid(True)
    return fig


@SaveOutput("task15")
def task15():
    """
    Task 15.

    In this function we shall plot a histogram to investigate how the speeds of the balls evolve from the initial
    value. We shall then compare this to the Maxwell-Boltzmann distribution. Ensure that this function returns
    the created histogram.

    Returns:
        Figure: The speed histogram.
    """
    fig, ax = plt.subplots(figsize=(10, 6))

    initial_speed = [5.0, 10.0, 15.0]
    colours = ["blue", "green", "red"]

    for v_init, colour in zip(initial_speed, colours):
        sim = MultiBallSimulation(b_speed=v_init)

        sim.run(num_collisions=1000, animate=False)

        collected_speeds = []
        for i in range(1000):
            sim.next_collision()
            collected_speeds.extend(sim.speeds())
        
        kbt = sim.t_equipartition()

        ax.hist(collected_speeds, bins=40, density=True, alpha=0.4,
                color=colour, edgecolor="black", label=f"Sim Histogram v_0 = {v_init}")
        
        v_max = max(collected_speeds)
        v_arr = np.linspace(0, v_max * 1.1, 200)
        pdf = maxwell(v_arr, kbt, mass=1.0)

        ax.plot(v_arr, pdf, color=colour, linewidth=2.5, linestyle="-",
                label = f"MB v_0={v_init}")
    
    ax.set_title("2D Maxwell-Boltzmann Speed Distribution")
    ax.set_xlabel("Speed (m/s)")
    ax.set_ylabel("Probability Density")
    ax.legend()
    ax.grid(True, alpha=0.3)

    return fig


@SaveOutput(["task16a", "task16b"])
def task16():
    """
    Task 16.

    In this function we shall also be looking at the divergence of our simulation from the IGL. We shall
    quantify the ball radii dependence of this divergence by plotting the temperature ratio
    and volume fraction defined in the project brief. We shall fit this temperature ratio before
    plotting the VDW b parameters radii dependence.

    Returns:
        tuple[Figure, Figure]: The ratio figure and b parameter figure.
    """
    ball_radii = np.linspace(0.01, 0.5, 15)
    sim_ratios = []

    N = 36
    V_container = np.pi * 10.0**2
    N_A = const.N_A

    for r in ball_radii:
        sim = MultiBallSimulation(b_radius=r)
        sim.run(num_collisions=1000, animate=False)
        
        t_eq = sim.t_equipartition()
        t_id = sim.t_ideal()

        ratio = t_eq / t_id if t_id > 1e-9 else 1.0
        sim_ratios.append(ratio)
    
    sim_ratios = np.array(sim_ratios)

    V_ball = np.pi * ball_radii**2
    B = 2 * N * V_ball
    f = (V_container - B) / V_container

    def fit_func(r, c):
        return 1.0 - c * r**2
    
    popt, _ = curve_fit(fit_func, ball_radii, sim_ratios)
    c_fit = popt[0]
    fit_ratios = fit_func(ball_radii, c_fit)

    B_fit = c_fit * (ball_radii**2) * V_container
    b_approx = (B * N_A) / N
    b_fit = (B_fit * N_A) / N

    fig_ratio, ax_ratio = plt.subplots()
    ax_ratio.plot(ball_radii, sim_ratios, 'ko', label="Simulation Data")
    ax_ratio.plot(ball_radii, f, 'b--', label="Trivial Approx (f)")
    ax_ratio.plot(ball_radii, fit_ratios, 'r-', label="SciPy Dynamic Fit")
    
    ax_ratio.set_title("Temperature Ratio vs Volume Fraction")
    ax_ratio.set_xlabel("Ball Radius (m)")
    ax_ratio.set_ylabel(r"Ratio ($T_{eq}/T_{ideal}$)")
    ax_ratio.legend()
    ax_ratio.grid(True)

    fig_b, ax_b = plt.subplots()
    ax_b.plot(ball_radii, b_fit, 'r-', linewidth=2, label="Dynamic Fit 'b'")
    ax_b.plot(ball_radii, b_approx, 'b--', linewidth=2, label="Trivial Approx 'b'")
    
    ax_b.set_title("Van der Waals 'b' Parameter vs Radius")
    ax_b.set_xlabel("Ball Radius (m)")
    ax_b.set_ylabel(r"VDW $b$ ($m^2 / mol$)")
    ax_b.legend()
    ax_b.grid(True)

    return fig_ratio, fig_b


@SaveOutput("task17")
def task17():
    """
    Task 17.

    In this function we shall run a Brownian motion simulation and plot the resulting trajectory of the 'big' ball.

    Returns:
        Figure: The Brownian motion simulation plot.
    """
    sim = BrownianSimulation()

    sim.run(num_collisions=1500, animate=False)
    fig, ax = sim.setup_figure()
    path = np.array(sim.bb_positions())
    ax.plot(path[:, 0], path[:, 1], color="blue", linewidth=2.0, alpha=0.8, label="Brownian Trajectory")
    
    ax.set_title("Brownian Motion Trajectory")
    ax.legend()

    return fig


@SaveOutput("task18")
def task18():
    """
    Task 18.

    In this function we shall calculate and plot the radial dependence of the mean free path and compare to the
    dilute-gas Boltzmann mean free path. We shall then investigate the Enskog correction as the larger radii put us in
    dense-gas region.

    Returns:
        Figure: The plot of your mean free path investigation.
    """

    ball_radii = np.linspace(0.1, 1.0, 10)
    total_collisions = 2500
    c_radius = 10.0

    sim_mfp = []
    boltz_mfp = []
    enskog_mfp = []

    for r in ball_radii:
        sim = MultiBallSimulation(c_radius=c_radius, b_radius=r)
        sim.run(num_collisions=total_collisions, animate=False)

        n_container = sim.container().n_container_collisions()
        n_ball_ball = total_collisions - n_container
        n_balls = len(sim.balls())
        sim_time = sim.time()
        if n_ball_ball > 0 and sim_time > 0:
            nu = (2 * n_ball_ball) / (n_balls * sim_time)
            
            mean_speed = np.mean(sim.speeds())
            sim_lambda = mean_speed / nu
        else:
            sim_lambda = 0.0
        sim_mfp.append(sim_lambda)
        container_area = np.pi * (c_radius ** 2)
        number_density = n_balls / container_area
        lambda_boltz = 1.0 / (4 * np.sqrt(2) * number_density * r)
        boltz_mfp.append(lambda_boltz)

        packing_fraction = number_density * np.pi * (r ** 2)

        g_contact = (1.0 - (7.0 / 16.0) * packing_fraction) / ((1.0 - packing_fraction) ** 2)
        lambda_enskog = lambda_boltz / g_contact
        enskog_mfp.append(lambda_enskog)
    fig, ax = plt.subplots(figsize=(8, 6))
    
    ax.plot(ball_radii, sim_mfp, 'ko-', label="Simulation Data", linewidth=2)
    ax.plot(ball_radii, boltz_mfp, 'b--', label="Boltzmann (Dilute Gas)", linewidth=2)
    ax.plot(ball_radii, enskog_mfp, 'r-.', label="Enskog (Dense Gas)", linewidth=2)

    ax.set_title("Mean Free Path vs Ball Radius")
    ax.set_xlabel("Ball Radius (m)")
    ax.set_ylabel(r"Mean Free Path $\lambda$ (m)")
    ax.legend()
    ax.grid(True)

    return fig


@SaveOutput(["task19a", "task19b", "task19c"])
def task19():
    """
    Task 19.

    In this function, we shall be computing the radial distribution function. We will see what the function looks like
    at time t = 0 as well as a later time t for a MultiBallSimulation where only a single ball has some velocity.
    We will also create an animation to so the evolution of this function as our simulation progresses.

    Returns:
        tuple[Figure, Figure, ArtistAnimation]: The g(r) histograms for t = 0, t = some time later, g(r) animation
    """

    sim = MultiBallSimulation(c_radius=10.0, b_radius=0.1, nrings=8, b_speed=0.0)
    sim.balls()[0].set_vel([10.0, 0.0]) #  if no major change check this

    nbins = 100
    bin_range = (0.0, 20.0)

    g_r_0, bin_edges = sim.rdf(nbins=nbins, bin_range=bin_range)
    bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
    dr = bin_centers[1] - bin_centers[0]

    fig_0, ax_0 = plt.subplots(figsize=(8, 5))
    ax_0.bar(bin_centers, g_r_0, width=dr, edgecolor="black", color="skyblue")
    ax_0.set_title("Radial Distribution Function $g(r)$ at $t=0$")
    ax_0.set_xlabel("Distance $r$")
    ax_0.set_ylabel("$g(r)$")
    ax_0.axhline(y=1.0, color="red", linestyle="--", alpha=0.5)

    fig_anim, ax_anim = plt.subplots(figsize=(8, 5))
    ax_anim.set_title("Time Evolution of $g(r)$")
    ax_anim.set_xlabel("Distance $r$")
    ax_anim.set_ylabel("$g(r)$")
    ax_anim.axhline(y=1.0, color="red", linestyle="--", alpha=0.5)

    artists = []

    (line,) = ax_anim.plot(bin_centers, g_r_0, color="blue", linewidth=2)
    artists.append([line])

    num_frames = 15
    for i in range(num_frames):
        sim.run(num_collisions=500, animate=False)
        g_r_t, _ = sim.rdf(nbins, bin_range=bin_range)
        (line,) = ax_anim.plot(bin_centers, g_r_t, color = "blue", linewidth=2)
        artists.append([line])
    
    anim = animation.ArtistAnimation(fig = fig_anim, artists=artists, interval=400, blit=True)
    
    fig_t, ax_t = plt.subplots(figsize=(8, 5))
    ax_t.bar(bin_centers, g_r_t, width=dr, edgecolor="black", color="salmon")
    ax_t.set_title("Radial Distribution Function $g(r)$ after Thermalization")
    ax_t.set_xlabel("Distance $r$")
    ax_t.set_ylabel("$g(r)$")
    ax_t.axhline(y=1.0, color="red", linestyle="--", alpha=0.5)

    return fig_0, fig_t, anim


if __name__ == "__main__":

    # Run task 9 function
    #BALL_POS, BALL_VEL = task9()

    # Run task 10 function
    #FIG10 = task10()

    # Run task 11 function
    #FIG11_BALLCENTRE, FIG11_INTERBALL = task11()

    # Run task 12 function
    #FIG12_KE, FIG12_MOMX, FIG12_MOMY, FIG12_PT = task12()

    # Run task 13 function
    #FIG13_PT, FIG13_PV, FIG13_PN = task13()

    # Run task 14 function
    #FIG14 = task14()

    # Run task 15 function
    #FIG15 = task15()

    # Run task 16 function
    #FIG16_RATIO, FIG16_BPARAM = task16()

    # Run task 17 function
    #FIG17 = task17()

    # Run task 18 function
    #FIG18 = task18()

    # Run task 19 function
    #FIG19_HIST1, FIG19_HIST2, FIG19_ANIM = task19()

    plt.show()
