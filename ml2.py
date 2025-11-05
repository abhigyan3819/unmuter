import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
from sklearn.preprocessing import StandardScaler
import json
import joblib

# === Load dataset ===
with open("data.json", "r") as f:
    data = json.load(f)

# === Prepare raw data ===
X_raw, y = [], []
for label, samples in data.items():
    for s in samples:
        X_raw.append(s)
        y.append(label)

X_raw = np.array(X_raw)
y = np.array(y)

# === Normalize features for model training ===
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X_raw)

# === Split data for evaluation ===
X_train, X_test, y_train, y_test = train_test_split(X_scaled, y, test_size=0.2, random_state=42)

# === Train RandomForestClassifier ===
model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# === Evaluate accuracy ===
y_pred = model.predict(X_test)
print("Accuracy:", accuracy_score(y_test, y_pred))

# === Save model and scaler ===
joblib.dump(model, "word_recognition_model.pkl")
joblib.dump(scaler, "scaler.pkl")

# === Compute class means and thresholds using raw data ===
class_samples_raw = {label: [] for label in set(y)}
for xi, yi in zip(X_raw, y):
    class_samples_raw[yi].append(np.array(xi))

class_means = {label: np.mean(samples, axis=0) for label, samples in class_samples_raw.items()}
class_thresholds = {}
for label, samples in class_samples_raw.items():
    distances = [np.linalg.norm(s - class_means[label]) for s in samples]
    class_thresholds[label] = max(distances) * 2  # 2× margin

print("Class thresholds:", class_thresholds)

# === Safe predict function using distance to class mean ===
def safe_predict(model, scaler, new_data, class_means, class_thresholds, prob_threshold=0.6):
    new_data = np.array(new_data)
    
    # Scale input for model
    new_data_scaled = scaler.transform([new_data])[0]
    
    # Predict probabilities
    proba = model.predict_proba([new_data_scaled])[0]
    max_prob = np.max(proba)
    pred_label = model.classes_[np.argmax(proba)]
    
    # 1️⃣ Probability check
    if max_prob < prob_threshold:
        return None
    
    # 2️⃣ Distance check (to class mean in raw space)
    threshold = class_thresholds.get(pred_label, 1.0)
    dist_to_mean = np.linalg.norm(new_data - class_means[pred_label])
    if dist_to_mean > threshold:
        return None
    
    return pred_label

# === Example usage ===
normal_input = [498,-70,-112,-6,-18,-2]  # should match known class
random_input = [999, 999, 999, 999, 999, 999]  # unknown input

print("Predicted (normal):", safe_predict(model, scaler, normal_input, class_means, class_thresholds))
print("Predicted (random):", safe_predict(model, scaler, random_input, class_means, class_thresholds))


