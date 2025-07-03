import numpy as np
import matplotlib.pyplot as plt
from scipy import stats

def func(x):
    return m*x + c

file = open("temppt.txt", "r")
temp = [float(l.strip()) for l in file]
#temp = sorted(temp)
temp = np.array(temp)
smpl = np.arange(0, len(temp), 1, dtype=int)
print(temp)
'''
file2 = open("tempntc1.txt", "r")
temp2 = [float(l.strip()) for l in file2]
temp2 = np.array(temp2)

file3 = open("tempntc2.txt", "r")
temp3 = [float(l.strip()) for l in file3]
temp3 = np.array(temp3)

m, c, r, p, err = stats.linregress(smpl, temp)
lin_temp = list(func(smpl))
'''
plt.plot(smpl, temp, color="r", label='PT100')
plt.scatter(smpl, temp, color="r")
'''
plt.plot(smpl, temp2, color="g", label='NTC1')
plt.scatter(smpl, temp2, color="g")
plt.plot(smpl, temp3, color="orange", label='NTC2')
plt.scatter(smpl, temp3, color="orange")
'''
plt.grid(True)
plt.legend()
plt.xlabel("Samples (1/6sec)")
plt.ylabel("Temperature (K)")
plt.title("PT100 test (Kalman filter)")
plt.show()
