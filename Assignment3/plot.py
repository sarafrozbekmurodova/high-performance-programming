import matplotlib.pyplot as plt
import subprocess


x = [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000]
y1 = []

for i in x:
    child_output = subprocess.run([
        "./galsim",
        str(i),
        f"input_data/ellipse_N_{i:05}.gal",
        str(100),
        str(0.00001),
        str(0)
    ], capture_output=True).stdout.decode().strip().split(': ')[1].strip()
    y1.append(float(child_output))

LW = 3.0

plt.plot(x, y1, linewidth=LW, label="Our runtimes")

y2 = [y1[0] * (x[i] / x[0])**2 for i in range(len(x))]

plt.plot(x, y2, linewidth=LW, label="Exactly O(N^2)")

plt.legend(fontsize="xx-large")

plt.show()
