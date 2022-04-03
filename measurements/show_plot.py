import matplotlib.pyplot as plt
import sys
import json

def get_graphic_for_metrics(filename):
	with open(filename) as metric_json:
		metrics = json.load(metric_json)

	xs = list(range(1, 101))
	ys = []

	for measurement in metrics['metrics']:
		metric_id = 3
		for metric in measurement['measurements']:
			metric_id -= 1
			if metric_id == 0:
				ys.append(metric['value'])
				break

	return xs, ys

xs, ys_baseline = get_graphic_for_metrics(sys.argv[1])
xs, ys_parallel_baseline = get_graphic_for_metrics(sys.argv[2])
xs, ys_flow_grouping = get_graphic_for_metrics(sys.argv[3])

plt.plot(xs, ys_baseline, color='r', label='baseline')
plt.plot(xs, ys_parallel_baseline, color='g', label='parallel baseline')
plt.plot(xs, ys_flow_grouping, color='b', label='flow grouping')

plt.ylabel('total memory migration')
plt.xlabel('tests')

plt.legend(loc='upper left')
plt.show()
