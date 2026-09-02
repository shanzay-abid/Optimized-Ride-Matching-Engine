#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <ctime>

using namespace std;

struct Location {
    double x;
    double y;

    double calculateDistance(const Location& other) const {
        return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
    }
};

class User {
protected:
    string userID;
    string name;
    Location currentLoc;

public:
    User(string id, string n, Location loc) : userID(id), name(n), currentLoc(loc) {}
    virtual ~User() {}

    string getUserID() const { return userID; }
    string getName() const { return name; }
    Location getLocation() const { return currentLoc; }

    void updateLocation(double newX, double newY) {
        currentLoc.x = newX;
        currentLoc.y = newY;
    }

    virtual void displayProfile() const = 0;
};

class Rider : public User {
public:
    Rider(string id, string n, Location loc)
        : User(id, n, loc) {
    }

    void displayProfile() const override {
        cout << "   [Rider Status] ID: " << userID << " | Name: " << name
            << " | Location: (" << currentLoc.x << ", " << currentLoc.y << ")\n";
    }
};

// ============================================================
// RIDE RECORD STRUCT - Tracks every completed ride
// ============================================================

struct RideRecord {
    string passengerName;
    string driverName;
    string driverID;
    double pickupX, pickupY;
    double dropX, dropY;
    double distance;
    double fare;
    string pricingMode;
    string algorithmUsed;
    time_t timestamp;

    void display() const {
        cout << "   " << driverName << " -> " << passengerName << "\n";
        cout << "      Distance: " << distance << " km | Fare: " << fare << " PKR\n";
        cout << "      Mode: " << pricingMode << " | Algo: " << algorithmUsed << "\n";

        // Safe time display
        char timeBuffer[26];
#ifdef _WIN32
        ctime_s(timeBuffer, sizeof(timeBuffer), &timestamp);
#else
        ctime_r(&timestamp, timeBuffer);
#endif
        cout << "      Time: " << timeBuffer;
    }
};

// ============================================================
// DRIVER CLASS WITH RIDE COUNT, FATIGUE, TIME TRACKING, AND LOCATION UPDATE
// ============================================================

class Driver : public User {
private:
    string vehicleNumber;
    bool isAvailable;
    double driverRating;
    int rideCount;          // Total rides completed today
    int fatigueLevel;       // 0-100, increases with each ride
    time_t lastRideTime;    // When the driver started the current ride

public:
    Driver(string id, string n, Location loc, string vehicle, double rating)
        : User(id, n, loc), vehicleNumber(vehicle), isAvailable(true),
        driverRating(rating), rideCount(0), fatigueLevel(0), lastRideTime(0) {
        if (rating < 1.0 || rating > 5.0) {
            cout << "Driver rating must be between 1.0 and 5.0." << endl;
        }
    }

    bool checkAvailability() const { return isAvailable; }
    
    // Set availability and track time when becoming unavailable
    void setAvailability(bool status) { 
        isAvailable = status;
        if (!status) {
            // Record when driver becomes unavailable (starts a ride)
            lastRideTime = time(0);
        }
    }
    
    // Update driver location after completing a ride
    void updateLocationAfterRide(const Location& newLocation) {
        currentLoc.x = newLocation.x;
        currentLoc.y = newLocation.y;

    }
    
    double getRating() const { return driverRating; }
    int getRideCount() const { return rideCount; }
    int getFatigueLevel() const { return fatigueLevel; }
    time_t getLastRideTime() const { return lastRideTime; }

    // Check if driver should become available again
    bool shouldBecomeAvailable() const {
        if (isAvailable) return false;  // Already available
        
        time_t now = time(0);
        double secondsSinceRide = difftime(now, lastRideTime);
        return secondsSinceRide >= 45;  // Available after 45 seconds
    }

    void incrementRideCount() {
        rideCount++;
        fatigueLevel += 10;  // Each ride adds 10% fatigue
        if (fatigueLevel > 100) fatigueLevel = 100;
    }

    void resetFatigue() { fatigueLevel = 0; }  // After rest period

    void displayProfile() const override {
        cout << "\n   Driver ID: " << userID << " | Name: " << name
            << " | Rating: " << driverRating << " | Available: " << (isAvailable ? "YES" : "NO")
            << " | Rides Today: " << rideCount
            << " | Fatigue: " << fatigueLevel << "%"
            << " | Position: (" << currentLoc.x << ", " << currentLoc.y << ")";
        
        // Show remaining time if unavailable
        if (!isAvailable) {
            time_t now = time(0);
            double secondsPassed = difftime(now, lastRideTime);
            int remaining = max(0, 45 - (int)secondsPassed);
            cout << " | ? Re-activates in: " << remaining << "s";
        }
        cout << "\n";
    }
};

// ============================================================
// SMART DRIVER SELECTION ALGORITHM
// Weighted Scoring: 60% Distance, 20% Workload, 20% Fatigue
// ============================================================

class SmartDriverSelector {
public:
    struct ScoredDriver {
        Driver* driver;
        double distance;
        int rideCount;
        int fatigue;
        double finalScore;
    };

    static Driver* selectBestDriver(Driver** drivers, int count, Location riderLoc) {
        if (count == 0) return NULL;

        ScoredDriver candidates[50];
        int validCount = 0;

        // Step 1: Collect all available drivers
        for (int i = 0; i < count; i++) {
            if (drivers[i]->checkAvailability()) {
                double dist = drivers[i]->getLocation().calculateDistance(riderLoc);
                candidates[validCount] = {
                    drivers[i],
                    dist,
                    drivers[i]->getRideCount(),
                    drivers[i]->getFatigueLevel(),
                    0.0
                };
                validCount++;
            }
        }

        if (validCount == 0) return NULL;

        // Step 2: Find max values for normalization
        double maxDist = 0;
        int maxRides = 0;
        int maxFatigue = 0;

        for (int i = 0; i < validCount; i++) {
            if (candidates[i].distance > maxDist) maxDist = candidates[i].distance;
            if (candidates[i].rideCount > maxRides) maxRides = candidates[i].rideCount;
            if (candidates[i].fatigue > maxFatigue) maxFatigue = candidates[i].fatigue;
        }

        // Step 3: Calculate weighted score for each driver
        for (int i = 0; i < validCount; i++) {
            double distScore = (maxDist > 0) ? 1 - (candidates[i].distance / maxDist) : 1.0;
            double workloadScore = (maxRides > 0) ? 1 - ((double)candidates[i].rideCount / maxRides) : 1.0;
            double fatigueScore = (maxFatigue > 0) ? 1 - ((double)candidates[i].fatigue / maxFatigue) : 1.0;

            candidates[i].finalScore = (distScore * 0.6) + (workloadScore * 0.2) + (fatigueScore * 0.2);

            if (candidates[i].rideCount < 3) {
                candidates[i].finalScore += 0.05;
            }
        }

        // Step 4: Find driver with highest score
        int bestIndex = 0;
        for (int i = 1; i < validCount; i++) {
            if (candidates[i].finalScore > candidates[bestIndex].finalScore) {
                bestIndex = i;
            }
        }

        // Step 5: Display selection analysis
        cout << "\n   [SMART SELECTION] Algorithm Analysis:\n";
        cout << "   ------------------------------------------------\n";
        cout << "   * Available drivers considered: " << validCount << "\n";
        cout << "   * Weighting: 60% Distance | 20% Workload | 20% Fatigue\n";
        cout << "   ------------------------------------------------\n";
        cout << "   SELECTED DRIVER:\n";
        cout << "      * Name: " << candidates[bestIndex].driver->getName() << "\n";
        cout << "      * Distance: " << candidates[bestIndex].distance << " km\n";
        cout << "      * Rides Today: " << candidates[bestIndex].rideCount << "\n";
        cout << "      * Fatigue Level: " << candidates[bestIndex].fatigue << "%\n";
        cout << "      * Final Score: " << candidates[bestIndex].finalScore << "\n";
        cout << "   ------------------------------------------------\n";

        return candidates[bestIndex].driver;
    }
};

// ============================================================
// FARE STRATEGY CLASSES
// ============================================================

class FareStrategy {
public:
    virtual ~FareStrategy() {}
    virtual double calculateFare(double distance) const = 0;
    virtual string getStrategyName() const = 0;
};

class StandardPricing : public FareStrategy {
public:
    double calculateFare(double distance) const override {
        return 50.0 + (distance * 12.0);
    }
    string getStrategyName() const override { return "Standard Baseline Rates"; }
};

class SurgePricing : public FareStrategy {
private:
    double multiplier;
public:
    SurgePricing(double mult) : multiplier(mult) {}
    double calculateFare(double distance) const override {
        return (50.0 + (distance * 12.0)) * multiplier;
    }
    string getStrategyName() const override { return "Surge Pricing Enabled"; }
};

// ============================================================
// RIDE MATCHING ENGINE WITH SMART ALGORITHM + ANALYTICS
// ============================================================

class RideMatchingEngine {
private:
    Driver** driverRegistry;
    int maxDrivers;
    int currentDriverCount;
    FareStrategy* activePricing;
    bool useSmartAlgorithm;

    RideRecord rideHistory[100];
    int rideCount;

public:
    RideMatchingEngine(int allocationSlots) : maxDrivers(allocationSlots), currentDriverCount(0),
        useSmartAlgorithm(true), rideCount(0) {
        driverRegistry = new Driver * [maxDrivers];
        for (int i = 0; i < maxDrivers; i++) driverRegistry[i] = NULL;
        activePricing = new StandardPricing();
    }

    ~RideMatchingEngine() {
        for (int i = 0; i < currentDriverCount; i++) delete driverRegistry[i];
        delete[] driverRegistry;
        delete activePricing;
    }

    bool registerDriver(Driver* d) {
        if (currentDriverCount >= maxDrivers) {
            cout << "\n[REGISTRY ERROR] Maximum driver capacity reached!\n";
            delete d;
            return false;
        }
        driverRegistry[currentDriverCount++] = d;
        return true;
    }

    void setPricingStrategy(FareStrategy* newStrategy) {
        if (activePricing != NULL) delete activePricing;
        activePricing = newStrategy;
    }

    void toggleAlgorithm() {
        useSmartAlgorithm = !useSmartAlgorithm;
        cout << "\n[ALGORITHM SWITCH] Now using: "
            << (useSmartAlgorithm ? "Smart 60/20/20 Fairness Algorithm" : "Basic Nearest-Driver Algorithm") << "\n";
    }

    string getCurrentAlgorithmName() const {
        return useSmartAlgorithm ? "Smart 60/20/20 Fairness Algorithm" : "Basic Nearest-Driver Algorithm";
    }

    void recordRide(Rider& rider, Driver* driver, double distance, double fare, string pricingMode) {
        if (rideCount < 100) {
            RideRecord record;
            record.passengerName = rider.getName();
            record.driverName = driver->getName();
            record.driverID = driver->getUserID();

            Location riderLoc = rider.getLocation();
            record.pickupX = riderLoc.x;
            record.pickupY = riderLoc.y;
            record.dropX = riderLoc.x;
            record.dropY = riderLoc.y;
            record.distance = distance;
            record.fare = fare;
            record.pricingMode = pricingMode;
            record.algorithmUsed = getCurrentAlgorithmName();
            record.timestamp = time(0);

            rideHistory[rideCount] = record;
            rideCount++;
        }
    }

    // ============================================================
    // AUTO REACTIVATE DRIVERS AFTER 45 SECONDS
    // ============================================================

    void autoReactivateDrivers() {
        bool anyReactivated = false;
        for (int i = 0; i < currentDriverCount; i++) {
            if (driverRegistry[i]->shouldBecomeAvailable()) {
                driverRegistry[i]->setAvailability(true);
                cout << "\n   [AUTO-REACTIVATE] " << driverRegistry[i]->getName() 
                     << " is now available again!\n";
                anyReactivated = true;
            }
        }
        if (anyReactivated) {
            cout << "   [SYSTEM] Driver availability refreshed\n";
        }
    }

    // ============================================================
    // RIDE HISTORY DISPLAY
    // ============================================================

    void showRideHistory() const {
        cout << "\n=========================================================\n";
        cout << "                   RIDE HISTORY LOG                       \n";
        cout << "=========================================================\n";

        if (rideCount == 0) {
            cout << "   No rides completed yet.\n";
            return;
        }

        cout << "   Total Rides: " << rideCount << "\n\n";

        for (int i = 0; i < rideCount; i++) {
            cout << "   [" << (i + 1) << "] ";
            rideHistory[i].display();
            cout << "\n";
        }
        cout << "=========================================================\n";
    }

    // ============================================================
    // ANALYTICS DASHBOARD
    // ============================================================

    void showAnalytics() const {
        cout << "\n=========================================================\n";
        cout << "                   ANALYTICS DASHBOARD                    \n";
        cout << "=========================================================\n";

        if (rideCount == 0) {
            cout << "   No data available. Complete some rides first!\n";
            return;
        }

        double totalDistance = 0;
        double totalFare = 0;
        int driverRides[50] = { 0 };
        double driverRevenue[50] = { 0 };

        for (int i = 0; i < rideCount; i++) {
            totalDistance += rideHistory[i].distance;
            totalFare += rideHistory[i].fare;

            for (int j = 0; j < currentDriverCount; j++) {
                if (driverRegistry[j]->getUserID() == rideHistory[i].driverID) {
                    driverRides[j]++;
                    driverRevenue[j] += rideHistory[i].fare;
                    break;
                }
            }
        }

        double avgDistance = totalDistance / rideCount;
        double avgFare = totalFare / rideCount;

        cout << "\n   OVERALL STATISTICS:\n";
        cout << "   ------------------------------------------------\n";
        cout << "   * Total Rides:      " << rideCount << "\n";
        cout << "   * Total Distance:   " << totalDistance << " km\n";
        cout << "   * Total Revenue:    " << totalFare << " PKR\n";
        cout << "   * Avg Distance:     " << avgDistance << " km\n";
        cout << "   * Avg Fare:         " << avgFare << " PKR\n";
        cout << "   ------------------------------------------------\n\n";

        cout << "   DRIVER PERFORMANCE:\n";
        cout << "   ------------------------------------------------\n";

        int maxRides = 0;
        int maxRidesIndex = 0;
        int driversWithRides = 0;

        for (int i = 0; i < currentDriverCount; i++) {
            if (driverRides[i] > 0) {
                cout << "   * " << driverRegistry[i]->getName() << ": "
                    << driverRides[i] << " rides | "
                    << driverRevenue[i] << " PKR revenue | "
                    << "Fatigue: " << driverRegistry[i]->getFatigueLevel() << "%\n";

                if (driverRides[i] > maxRides) {
                    maxRides = driverRides[i];
                    maxRidesIndex = i;
                }
                driversWithRides++;
            }
        }

        if (driversWithRides > 0) {
            cout << "   ------------------------------------------------\n";
            cout << "   TOP PERFORMER: " << driverRegistry[maxRidesIndex]->getName()
                << " (" << maxRides << " rides)\n";
        }

        cout << "\n   FAIRNESS METRIC:\n";
        cout << "   ------------------------------------------------\n";

        if (currentDriverCount > 1 && driversWithRides > 0) {
            int minRides = 9999;
            int maxRidesFound = 0;
            for (int i = 0; i < currentDriverCount; i++) {
                if (driverRides[i] > 0) {
                    if (driverRides[i] < minRides) minRides = driverRides[i];
                    if (driverRides[i] > maxRidesFound) maxRidesFound = driverRides[i];
                }
            }

            double fairnessScore = 0;
            if (maxRidesFound > 0) {
                fairnessScore = ((double)minRides / maxRidesFound) * 100;
            }

            cout << "   * Rides Distribution Range: " << minRides << " - " << maxRidesFound << "\n";
            cout << "   * Fairness Score: " << fairnessScore << "% ";

            if (fairnessScore >= 80) {
                cout << "[EXCELLENT - Very balanced!]\n";
            }
            else if (fairnessScore >= 50) {
                cout << "[MODERATE - Some imbalance]\n";
            }
            else {
                cout << "[POOR - Very unfair distribution]\n";
            }
        }
        cout << "   ------------------------------------------------\n";
        cout << "=========================================================\n";
    }

    // ============================================================
    // REAL ALGORITHM COMPARISON
    // ============================================================

    void showAlgorithmComparison() const {
        cout << "\n=========================================================\n";
        cout << "           REAL ALGORITHM PERFORMANCE COMPARISON          \n";
        cout << "=========================================================\n";

        if (rideCount == 0) {
            cout << "   No ride data available. Complete some rides first!\n";
            cout << "=========================================================\n";
            return;
        }

        double totalDistance = 0;
        double totalFare = 0;
        int driverRides[50] = { 0 };
        double driverRevenue[50] = { 0 };

        for (int i = 0; i < rideCount; i++) {
            totalDistance += rideHistory[i].distance;
            totalFare += rideHistory[i].fare;

            for (int j = 0; j < currentDriverCount; j++) {
                if (driverRegistry[j]->getUserID() == rideHistory[i].driverID) {
                    driverRides[j]++;
                    driverRevenue[j] += rideHistory[i].fare;
                    break;
                }
            }
        }

        int minRides = 9999;
        int maxRidesFound = 0;
        int totalRidesForFairness = 0;
        int driversWithRides = 0;

        for (int i = 0; i < currentDriverCount; i++) {
            if (driverRides[i] > 0) {
                if (driverRides[i] < minRides) minRides = driverRides[i];
                if (driverRides[i] > maxRidesFound) maxRidesFound = driverRides[i];
                totalRidesForFairness += driverRides[i];
                driversWithRides++;
            }
        }

        double fairnessScore = (maxRidesFound > 0) ? ((double)minRides / maxRidesFound) * 100 : 0;
        double avgRidesPerDriver = (driversWithRides > 0) ? (double)totalRidesForFairness / driversWithRides : 0;

        cout << "\n   COMPARISON: UBER (Nearest) vs SMART (60/20/20)\n";
        cout << "   ------------------------------------------------\n";
        cout << "   METRIC                 | UBER      | SMART\n";
        cout << "   ------------------------------------------------\n";

        if (useSmartAlgorithm) {
            cout << "   CURRENT ALGORITHM      |           | ACTIVE\n";
        }
        else {
            cout << "   CURRENT ALGORITHM      | ACTIVE    |\n";
        }
        cout << "   ------------------------------------------------\n";

        cout << "   Total Rides            | " << rideCount << "\n";
        cout << "   Active Drivers         | " << driversWithRides << "\n";
        cout << "   Avg Rides/Driver       | " << avgRidesPerDriver << "\n";
        cout << "   ------------------------------------------------\n";
        cout << "   FAIRNESS METRICS:\n";
        cout << "   Min Rides (Driver)     | " << minRides << "\n";
        cout << "   Max Rides (Driver)     | " << maxRidesFound << "\n";
        cout << "   Fairness Score         | " << fairnessScore << "%\n";
        cout << "   ------------------------------------------------\n";

        cout << "\n   INTERPRETATION:\n";
        if (fairnessScore >= 80) {
            cout << "   EXCELLENT: Rides are very fairly distributed!\n";
        }
        else if (fairnessScore >= 50) {
            cout << "   MODERATE: Some imbalance in ride distribution.\n";
        }
        else {
            cout << "   POOR: Rides are concentrated on few drivers.\n";
        }

        if (!useSmartAlgorithm && fairnessScore < 60) {
            cout << "\n   RECOMMENDATION: Switch to SMART algorithm (Option 5)\n";
        }
        else if (useSmartAlgorithm && fairnessScore >= 70) {
            cout << "   SMART algorithm is effectively balancing rides!\n";
        }

        cout << "=========================================================\n";
    }

    // ============================================================
    // PROCESS RIDE REQUEST - UPDATED WITH DRIVER LOCATION CHANGE
    // ============================================================

    void processRideRequest(Rider& rider, Location destination) {
        Location riderLoc = rider.getLocation();
        Driver* optimalDriver = NULL;

        if (useSmartAlgorithm) {
            optimalDriver = SmartDriverSelector::selectBestDriver(driverRegistry, currentDriverCount, riderLoc);
        }
        else {
            double shortestDistance = 999999.0;
            for (int i = 0; i < currentDriverCount; i++) {
                if (driverRegistry[i]->checkAvailability()) {
                    double distanceToRider = driverRegistry[i]->getLocation().calculateDistance(riderLoc);
                    if (distanceToRider < shortestDistance) {
                        shortestDistance = distanceToRider;
                        optimalDriver = driverRegistry[i];
                    }
                }
            }
        }

        if (optimalDriver == NULL) {
            cout << "\nNO AVAILABLE DRIVERS: All fleet vehicles are currently busy.\n";
            return;
        }

        double tripDistance = riderLoc.calculateDistance(destination);
        double finalFare = activePricing->calculateFare(tripDistance);

        cout << "\n=========================================================\n";
        cout << "                    RIDE MATCH FOUND                     \n";
        cout << "=========================================================\n";
        cout << " * Passenger:       " << rider.getName() << "\n";
        cout << " * Assigned Driver: " << optimalDriver->getName() << "\n";
        cout << " * Driver Rating:   " << optimalDriver->getRating() << " \n";
        cout << " * Rides Today:     " << optimalDriver->getRideCount() << "\n";
        cout << " * Fatigue Level:   " << optimalDriver->getFatigueLevel() << "%\n";
        cout << " * Algorithm:       " << getCurrentAlgorithmName() << "\n";
        cout << " * Ride Distance:   " << tripDistance << " km\n";
        cout << " * Pricing Mode:    " << activePricing->getStrategyName() << "\n";
        cout << " * Total Cost:      " << finalFare << " PKR\n";
        cout << "=========================================================\n";

        // Mark driver as unavailable
        optimalDriver->setAvailability(false);
        optimalDriver->incrementRideCount();

        // CRITICAL: Update driver location to passenger's destination
        optimalDriver->updateLocationAfterRide(destination);

        // Record the ride in history
        recordRide(rider, optimalDriver, tripDistance, finalFare, activePricing->getStrategyName());

        // Update drop location in record
        if (rideCount > 0) {
            rideHistory[rideCount - 1].dropX = destination.x;
            rideHistory[rideCount - 1].dropY = destination.y;
        }

        // Update rider location
        rider.updateLocation(destination.x, destination.y);

        // Log to file
        ofstream logStream("dispatch_manifest_logs.txt", ios::app);
        if (logStream) {
            logStream << "Rider: " << rider.getName()
                << " | Driver: " << optimalDriver->getName()
                << " | Rides Today: " << optimalDriver->getRideCount()
                << " | Fatigue: " << optimalDriver->getFatigueLevel() << "%"
                << " | Fare: " << finalFare << " PKR\n";
            logStream.close();
        }
    }

    void displayDrivers() const {
        cout << "\n--- Current Active Fleet Status ---\n";
        cout << "   Algorithm Mode: " << getCurrentAlgorithmName() << "\n";
        if (currentDriverCount == 0) cout << "   No drivers currently on duty.\n";
        for (int i = 0; i < currentDriverCount; i++) {
            driverRegistry[i]->displayProfile();
        }
    }
};

// ============================================================
// MAIN FUNCTION
// ============================================================

int main() {
    RideMatchingEngine uberCore(10);

    // Pre-populate with 4 default drivers
    uberCore.registerDriver(new Driver("DRV01", "Ahmed Ali", { 2.0, 3.0 }, "ICT-5921", 4.8));
    uberCore.registerDriver(new Driver("DRV02", "Zainab Khan", { 8.0, 7.0 }, "LHR-1102", 4.9));
    uberCore.registerDriver(new Driver("DRV03", "Usman Malik", { 5.0, 2.0 }, "ISB-3344", 3.8));
    uberCore.registerDriver(new Driver("DRV04", "Fatima Noor", { 1.0, 9.0 }, "KHI-7788", 4.3));

    int choice = 0;
    cout << "=========================================================\n";
    cout << "          WELCOME TO THE MINI-UBER SYSTEM ENGINE         \n";
    cout << "=========================================================\n";

    while (choice != 10) {
        // Auto-reactivate drivers at the start of each loop
        uberCore.autoReactivateDrivers();

        cout << "\n--- MAIN CONTROL PANEL ---\n";
        cout << "1. View Active Driver Fleet\n";
        cout << "2. Register a New Driver on Duty\n";
        cout << "3. Simulate a Passenger Ride Request\n";
        cout << "4. Modify Global Pricing Strategy (Surge/Standard)\n";
        cout << "5. Toggle Driver Selection Algorithm\n";
        cout << "6. Show Algorithm Details & Statistics\n";
        cout << "7. Show Ride History\n";
        cout << "8. Show Analytics Dashboard\n";
        cout << "9. Show Algorithm Comparison\n";
        cout << "10. Shutdown System Application\n";
        cout << "Enter selection (1-10): ";

        if (!(cin >> choice)) {
            cout << "[INPUT ERROR] Invalid menu option selected.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        try {
            switch (choice) {
            case 1:
                uberCore.displayDrivers();
                break;

            case 2: {
                string dID, dName, plate;
                double x, y, rating;
                cout << "\n--- Driver Setup ---\n";
                cout << "Enter unique Driver ID (e.g., DRV04): "; cin >> dID;
                cout << "Enter Driver Full Name: "; cin.ignore(); getline(cin, dName);
                cout << "Enter Vehicle License Plate: "; cin >> plate;
                cout << "Enter Initial Map Position coordinates (X Y): "; cin >> x >> y;
                cout << "Enter Performance Rating (1.0 - 5.0): "; cin >> rating;

                Driver* newDriver = new Driver(dID, dName, { x, y }, plate, rating);
                if (uberCore.registerDriver(newDriver)) {
                    cout << "\n[SUCCESS] Driver " << dName << " is now active on the map.\n";
                }
                break;
            }

            case 3: {
                string rName;
                double rx, ry, dx, dy;
                cout << "\n--- Passenger Ride Request Wizard ---\n";
                cout << "Enter Passenger Name: "; cin.ignore(); getline(cin, rName);
                cout << "Enter your current Pickup coordinates (X Y): "; cin >> rx >> ry;
                cout << "Enter your Target Destination coordinates (X Y): "; cin >> dx >> dy;

                Rider activeCustomer("TEMP_RIDER", rName, { rx, ry });
                uberCore.processRideRequest(activeCustomer, { dx, dy });
                break;
            }

            case 4: {
                int priceChoice;
                cout << "\n--- Pricing Adjustment Dashboard ---\n";
                cout << "1. Set Standard Pricing (Baseline rate)\n";
                cout << "2. Trigger High-Demand Surge Multiplier\n";
                cout << "Select pricing framework: "; cin >> priceChoice;

                if (priceChoice == 1) {
                    uberCore.setPricingStrategy(new StandardPricing());
                    cout << "\n[SYSTEM CONFIG] Rates returned to Standard Base Settings.\n";
                }
                else if (priceChoice == 2) {
                    double multiplier;
                    cout << "Enter Surge Multiplier Rate (e.g., 1.5, 2.0): "; cin >> multiplier;
                    uberCore.setPricingStrategy(new SurgePricing(multiplier));
                    cout << "\n[SYSTEM CONFIG] Global Surge Multiplier applied successfully.\n";
                }
                else {
                    cout << "[CONFIG SELECTION ERROR] Invalid strategy parameter.\n";
                }
                break;
            }

            case 5:
                uberCore.toggleAlgorithm();
                break;

            case 6: {
                cout << "\n=========================================================\n";
                cout << "              ALGORITHM DETAILS & STATISTICS              \n";
                cout << "=========================================================\n";
                cout << "\nCURRENT ALGORITHM: " << uberCore.getCurrentAlgorithmName() << "\n\n";
                cout << "WEIGHTING FORMULA:\n";
                cout << "   Score = (Distance x 0.6) + (Workload x 0.2) + (Fatigue x 0.2)\n\n";
                cout << "   Where:\n";
                cout << "   * Distance: Closer driver = Higher score (0 to 1)\n";
                cout << "   * Workload: Fewer rides today = Higher score (0 to 1)\n";
                cout << "   * Fatigue: Less tired = Higher score (0 to 1)\n\n";
                cout << "WHY THIS MATTERS:\n";
                cout << "   * Fairer distribution of rides among drivers\n";
                cout << "   * Reduces driver burnout\n";
                cout << "   * Improves overall service quality\n";
                cout << "   * More ethical than basic nearest-driver approach\n";
                cout << "=========================================================\n";
                break;
            }

            case 7:
                uberCore.showRideHistory();
                break;

            case 8:
                uberCore.showAnalytics();
                break;

            case 9:
                uberCore.showAlgorithmComparison();
                break;

            case 10:
                cout << "\nShutting down central grid matching modules safely...\n";
                cout << "System terminated successfully.\n";
                break;

            default:
                cout << "[SELECTION ERROR] Out of menu options boundary bounds.\n";
            }
        }
        catch (const exception& e) {
            cout << "\n>>> Trapped Runtime Warning: " << e.what() << " <<<\n";
        }
    }

    return 0;
}
