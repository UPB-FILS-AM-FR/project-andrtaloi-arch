// ============================================================
// FIREBASE CONFIGURATION
// Replace these values with your own Firebase project settings.
// ============================================================

const firebaseConfig = {
  apiKey: "YOUR_FIREBASE_API_KEY",
  authDomain: "YOUR_PROJECT_ID.firebaseapp.com",
  databaseURL: "https://YOUR_PROJECT_ID-default-rtdb.firebaseio.com",
  projectId: "YOUR_PROJECT_ID",
  storageBucket: "YOUR_PROJECT_ID.appspot.com",
  messagingSenderId: "YOUR_SENDER_ID",
  appId: "YOUR_APP_ID"
};

firebase.initializeApp(firebaseConfig);

const database = firebase.database();

// ============================================================
// CHART.JS
// ============================================================

const chartLabels = [];
const temperatureValues = [];

const ctx = document.getElementById("temperatureChart").getContext("2d");

const temperatureChart = new Chart(ctx, {
  type: "line",
  data: {
    labels: chartLabels,
    datasets: [
      {
        label: "Température °C",
        data: temperatureValues,
        borderWidth: 2,
        tension: 0.3
      }
    ]
  },
  options: {
    responsive: true,
    scales: {
      y: {
        beginAtZero: false
      }
    }
  }
});

// ============================================================
// REALTIME DATABASE LISTENER
// ============================================================

database.ref("/system").on("value", function(snapshot) {
  const data = snapshot.val();

  if (!data) {
    return;
  }

  updateDashboard(data);
  updateChart(data.temperature);
});

function updateDashboard(data) {
  document.getElementById("temperature").innerText = formatValue(data.temperature, " °C");
  document.getElementById("humidity").innerText = formatValue(data.humidity, " %");
  document.getElementById("gas").innerText = data.gas ?? "--";
  document.getElementById("motion").innerText = data.motion == 1 ? "Détecté" : "Aucun";
  document.getElementById("parking").innerText = formatValue(data.parkingPlaces, " places");
  document.getElementById("safety").innerText = data.safetyStatus ?? "NORMAL";
  document.getElementById("lastEvent").innerText = data.lastEvent ?? "Aucun événement";
}

function updateChart(temperature) {
  if (temperature === undefined || temperature === null) {
    return;
  }

  const time = new Date().toLocaleTimeString();

  chartLabels.push(time);
  temperatureValues.push(temperature);

  if (chartLabels.length > 12) {
    chartLabels.shift();
    temperatureValues.shift();
  }

  temperatureChart.update();
}

function formatValue(value, suffix) {
  if (value === undefined || value === null) {
    return "--";
  }

  return value + suffix;
}

// ============================================================
// COMMANDS
// ============================================================

function sendCommand(command) {
  database.ref("/commands/action").set(command);

  alert("Commande envoyée : " + command);
}
