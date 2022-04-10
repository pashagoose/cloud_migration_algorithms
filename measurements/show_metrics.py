import matplotlib.pyplot as plt
import common
import sys

xs, ys_baseline = common.get_graphic_for_metrics(sys.argv[1])
xs, ys_parallel_baseline = common.get_graphic_for_metrics(sys.argv[2])
xs, ys_flow_grouping = common.get_graphic_for_metrics(sys.argv[3])
xs, ys_lowerbounds = common.get_graphic_for_lowerbounds(sys.argv[4])

plt.plot(xs, ys_baseline, color='r', label='baseline')
plt.plot(xs, ys_parallel_baseline, color='g', label='parallel baseline')
plt.plot(xs, ys_flow_grouping, color='b', label='flow grouping')
plt.plot(xs, ys_lowerbounds, color='y', label='lowerbound')

plt.ylabel('timespan')
plt.xlabel('tests')

plt.legend(loc='upper left')
plt.show()
