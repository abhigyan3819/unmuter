import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
import json

with open("data.json", "r") as f:
    data = json.load(f)

X, y = [], []
for label, samples in data.items():
    for s in samples:
        X.append(s)
        y.append(label)

X = np.array(X)
y = np.array(y)

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

print("Accuracy:", accuracy_score(y_test, model.predict(X_test)))

class_samples = {label: [] for label in set(y)}
for xi, yi in zip(X, y):
    class_samples[yi].append(xi)

def safe_predict(model, new_data, class_samples, prob_threshold=0.6, distance_threshold=75):
    new_data = np.array(new_data)
    proba = model.predict_proba([new_data])[0]
    max_prob = np.max(proba)
    pred_label = model.classes_[np.argmax(proba)]

    if max_prob < prob_threshold:
        return None

    distances = [np.linalg.norm(new_data - np.array(s)) for s in class_samples[pred_label]]
    min_dist = min(distances)
    if min_dist > distance_threshold:
        return None

    return pred_label

normal_input = [60, 42, 57, 41, 57, 49]
random_input = [36,49,132,-4,55,-6]

print("Predicted (normal):", safe_predict(model, normal_input, class_samples))
print("Predicted (random):", safe_predict(model, random_input, class_samples))
