import matplotlib.pyplot as plt
import common

def get_avg(ys):
	return sum(ys) / len(ys)

xs, ys_baseline = common.get_graphic_for_metrics('baseline_metrics.json')
xs, ys_parallel_baseline = common.get_graphic_for_metrics('parallel_baseline_metrics.json')
xs, ys_flow_grouping = common.get_graphic_for_metrics('parallel_flow_grouping.json')
xs, ys_lowerbounds = common.get_graphic_for_lowerbounds('lowerbounds.txt')

avg_baseline = get_avg(ys_baseline)
avg_parallel_baseline = get_avg(ys_parallel_baseline)
avg_flow_grouping = get_avg(ys_flow_grouping)


algo_names = ["baseline", "parallel_baseline", "flow_grouping"]
ys = [avg_baseline, avg_parallel_baseline, avg_flow_grouping]
plt.bar(x=algo_names, height=ys, color=['r', 'g', 'b'])
plt.show()