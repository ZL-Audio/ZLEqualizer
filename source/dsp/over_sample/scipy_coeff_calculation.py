#  Copyright (C) 2025 - zsliu98
#  This file is part of ZLCompressor
#
#  ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
#
#  ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.
#
#  ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
#
#  ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

import numpy as np
from scipy import signal
import matplotlib.pyplot as plt

def print_and_plot_coeff(ax, order=64, width=0.10, stop=80, method="remez"):
    label = "{}_{}_{}_{}".format(method, order, width, stop)
    coeff = None
    if method == "remez":
        bands = np.array([0., .25 - width * 0.5, .25 + width * 0.5, .5])
        coeff = signal.remez(order + 1, bands, [1.0, np.power(10.0, -stop * 0.05)])
    elif method == "firls":
        bands = np.array([0., .25 - width * 0.5, .25 + width * 0.5, .5]) * 2.0
        coeff = signal.firls(order + 1, bands, [1.0, 1.0, np.power(10.0, -stop * 0.05), np.power(10.0, -stop * 0.05)])
    else:
        coeff = signal.firwin(order + 1, 0.5)
    for i in range(order + 1):
        if i % 2 == 0 and i != order / 2:
            coeff[i] = 0.0

    (w, H) = signal.freqz(coeff, worN=2048)
    ax.plot(w, 20 * np.log10(np.abs(H)), label=label)
    print(",".join(x.astype(str) for x in coeff))

fig, ax = plt.subplots()
o, w, s = 32, 0.20, 80
print_and_plot_coeff(ax, order=o, width=w, stop=s, method="remez")
print_and_plot_coeff(ax, order=o, width=w, stop=s, method="firwin")
print_and_plot_coeff(ax, order=o, width=w, stop=s, method="firls")
ax.axis([0, np.pi, -100, 3])
ax.grid('on')
ax.set_ylabel('Magnitude (dB)')
ax.set_xlabel('Normalized Frequency (radians)')
ax.set_title('Half Band Filter Frequency Response')
ax.axvline(np.pi * 0.5, color='g', linestyle='--')
ax.legend()
plt.show()
