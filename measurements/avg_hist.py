import matplotlib.pyplot as plt
import common
import sys

def get_avg(ys):
	return sum(ys) / len(ys)

xs, ys_baseline = common.get_graphic_for_metrics(sys.argv[1])
xs, ys_parallel_baseline = common.get_graphic_for_metrics(sys.argv[2])
xs, ys_flow_grouping = common.get_graphic_for_metrics(sys.argv[3])

avg_baseline = get_avg(ys_baseline)
avg_parallel_baseline = get_avg(ys_parallel_baseline)
avg_flow_grouping = get_avg(ys_flow_grouping)


algo_names = ["baseline", "parallel baseline", "flow grouping"]
ys = [avg_baseline, avg_parallel_baseline, avg_flow_grouping]
plt.bar(x=algo_names, height=ys, color=['r', 'g', 'b'])
plt.show()