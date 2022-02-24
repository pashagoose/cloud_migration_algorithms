import matplotlib.pyplot as plt
import json

with open('baseline_metrics.json') as baseline_json:
    baseline_metrics = json.load(baseline_json)

with open('parallel_baseline_metrics.json') as parallel_baseline_json:
    parallel_baseline_metrics = json.load(parallel_baseline_json)

xs = list(range(1, 101))

ys_baseline = []
ys_parallel_baseline = []

for measurement in baseline_metrics['metrics']:
	for metric in measurement['measurements']:
		ys_baseline.append(metric['value'])
		break

for measurement in parallel_baseline_metrics['metrics']:
	for metric in measurement['measurements']:
		ys_parallel_baseline.append(metric['value'])
		break

plt.plot(xs, ys_baseline, color='r', label='baseline')
plt.plot(xs, ys_parallel_baseline, color='g', label='parallel baseline')

plt.ylabel('timespan')
plt.xlabel('tests')

plt.legend(loc='upper left')
plt.show()
