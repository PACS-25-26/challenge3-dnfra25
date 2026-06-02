import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("results.csv")

# SPEEDUP
t1 = data[data["np"] == 1]["time"].values[0]
data["speedup"] = t1 / data["time"]

# ===== PLOT TIME =====
plt.figure()
plt.plot(data["np"], data["time"], marker="o")
plt.xlabel("MPI ranks")
plt.ylabel("Time (s)")
plt.title("Strong Scaling - Time")
plt.grid()
plt.savefig("time.png")

# ===== SPEEDUP =====
plt.figure()
plt.plot(data["np"], data["speedup"], marker="o")
plt.xlabel("MPI ranks")
plt.ylabel("Speedup")
plt.title("Speedup Curve")
plt.grid()
plt.savefig("speedup.png")

print("Plots saved: time.png, speedup.png")
