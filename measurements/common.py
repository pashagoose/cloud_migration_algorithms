import json

def get_graphic_for_metrics(filename):
	with open(filename) as metric_json:
		metrics = json.load(metric_json)

	xs = list(range(1, 101))
	ys = []

	for measurement in metrics['metrics']:
		metric_id = 1
		for metric in measurement['measurements']:
			metric_id -= 1
			if metric_id == 0:
				ys.append(metric['value'])
				break

	return xs, ys


def get_graphic_for_lowerbounds(filename):
	with open(filename) as bounds:
		ys = [float(y) for y in bounds.read().split()]

	return list(range(1, 101)), ys