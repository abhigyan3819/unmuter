from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
import numpy as np
import json
import joblib

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

y_pred = model.predict(X_test)
print("Accuracy:", accuracy_score(y_test, y_pred))
joblib.dump(model, 'word_recognition_model.pkl')
