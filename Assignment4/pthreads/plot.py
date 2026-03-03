import matplotlib.pyplot as plt
import subprocess


particles = [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000]
threads = [i for i in range(16)]
y = []
initial = float(subprocess.run([
    "./galsim",
    str(5000),
    "input_data/ellipse_N_05000.gal",
    str(100),
    str(0.00001),
    str(0),
    str(1),
], capture_output=True).stdout.decode().strip().split(': ')[1].strip())

y.append(1)

for i in threads[1:]:
    child_output = subprocess.run([
        "./galsim",
        str(5000),
        "input_data/ellipse_N_05000.gal",
        str(100),
        str(0.00001),
        str(0),
        str(i),
    ], capture_output=True).stdout.decode().strip().split(': ')[1].strip()
    y.append(initial / float(child_output))

LW = 3.0

plt.plot(threads, y, linewidth=LW, label="Improvement")
plt.plot(threads, threads, linewidth=LW, label="Linear")

plt.legend(fontsize="xx-large")

plt.show()
