#include <iostream>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;

int main() {
struct Car {
    string number;
    time_t entry_time;

    Car(string num, time_t entry = time(nullptr)) {
        number = num;
        entry_time = entry;
    }
};

struct ParkingSpot {
    int lane;
    int position;
    bool occupied;
    Car* car;

    ParkingSpot(int l, int p) : lane(l), position(p), occupied(false), car(nullptr) {}
};

class ParkingLot {
    const int lanes = 5;
    const int spots_per_lane = 10;
    vector<vector<ParkingSpot*>> lanes_data;
    unordered_map<string, ParkingSpot*> car_map;
    unordered_map<string, string> token_to_car;
    double total_revenue = 0.0;
    const double rate_per_second = 60.0 / 3600.0;

    string generateToken() {
        string token;
        do {
            token = to_string(rand() % 90000 + 10000);
        } while (token_to_car.count(token));
        return token;
    }

    void loadRevenue() {
        ifstream file("revenue.txt");
        if (file.is_open()) {
            file >> total_revenue;
            file.close();
        }
    }

    void loadParkedCars() {
        ifstream file("parked_cars.txt");
        if (!file.is_open()) return;
        string token, car_number;
        int lane, position;
        time_t entry_time;
        while (file >> token >> car_number >> lane >> position >> entry_time) {
            ParkingSpot* spot = lanes_data[lane][position];
            spot->occupied = true;
            spot->car = new Car(car_number, entry_time);
            car_map[car_number] = spot;
            token_to_car[token] = car_number;
            string ignore_line;
            getline(file, ignore_line);
        }
        file.close();
    }

public:
    ParkingLot() {
        srand(time(0));
        for (int i = 0; i < lanes; ++i) {
            vector<ParkingSpot*> row;
            for (int j = 0; j < spots_per_lane; ++j) {
                row.push_back(new ParkingSpot(i, j));
            }
            lanes_data.push_back(row);
        }
        loadRevenue();
        loadParkedCars();
    }

    ~ParkingLot() {
        for (auto& row : lanes_data) {
            for (auto& spot : row) {
                if (spot->car) delete spot->car;
                delete spot;
            }
        }
    }

    void saveAllData() {
        ofstream file("parked_cars.txt");
        if (file.is_open()) {
            for (auto& entry : token_to_car) {
                string token = entry.first;
                string car_number = entry.second;
                ParkingSpot* spot = car_map[car_number];
                time_t entryTime = spot->car->entry_time;
                file << "Token number is " << token 
                     << "\nCar number is " << car_number
                     << "\nSpot->lane #" << spot->lane 
                     << "\nSpor->position #"<< spot->position 
                     << "\nDate and Time: " << ctime(&entryTime);
            }
            file.close();
        }

        ofstream revenueFile("revenue.txt");
        if (revenueFile.is_open()) {
            revenueFile << "Total Revenue till now: Rs." << total_revenue;
            revenueFile.close();
        }
    }

    void enterParking(string car_number) {
        if (car_map.count(car_number)) {
            cout << "Car '" << car_number << "' is already parked.\n";
            return;
        }

        for (int i = 0; i < lanes; ++i) {
            for (int j = 0; j < spots_per_lane; ++j) {
                if (!lanes_data[i][j]->occupied) {
                    string token = generateToken();
                    lanes_data[i][j]->occupied = true;
                    lanes_data[i][j]->car = new Car(car_number);
                    car_map[car_number] = lanes_data[i][j];
                    token_to_car[token] = car_number;

                    cout << "Car parked successfully!\n";
                    cout << "Location: Lane " << (i + 1) << ", Position " << (j + 1) << "\n";
                    cout << "Your token is: " << token << "\n";
                    return;
                }
            }
        }

        cout << "Parking Full. No space available.\n";
    }

    void formatDuration(double seconds) {
        int total_seconds = static_cast<int>(seconds);
        int hours = total_seconds / 3600;
        int minutes = (total_seconds % 3600) / 60;
        int secs = total_seconds % 60;

        if (hours > 0) cout << hours << " hr ";
        if (minutes > 0) cout << minutes << " min ";
        cout << secs << " sec";
    }

    void exitParking(string token) {
        if (!token_to_car.count(token)) {
            cout << "Invalid token.\n";
            return;
        }

        string car_number = token_to_car[token];
        ParkingSpot* spot = car_map[car_number];
        time_t now = time(nullptr);
        double duration = difftime(now, spot->car->entry_time);
        if (duration < 1) duration = 1;

        double fee = round(duration * rate_per_second);
        total_revenue += fee;

        cout << "Car " << car_number << " is exiting from Lane " << spot->lane + 1 << ", Position " << spot->position + 1 << ".\n";
        cout << "Duration Parked: ";
        formatDuration(duration);
        cout << "\n";
        cout << "Parking Fee: Rs." << fixed << setprecision(2) << fee << "\n";

        delete spot->car;
        spot->car = nullptr;
        spot->occupied = false;
        car_map.erase(car_number);
        token_to_car.erase(token);
    }

    void findCar(string token) {
        if (!token_to_car.count(token)) {
            cout << "Invalid token.\n";
            return;
        }
        string car_number = token_to_car[token];
        ParkingSpot* spot = car_map[car_number];
        cout << "Car Number: " << car_number << " found at Lane " << spot->lane + 1
             << ", Position " << spot->position + 1 << ".\n";
        cout << "Parked At: " << ctime(&spot->car->entry_time);
    }

    void viewAllParkedCars() {
        if (car_map.empty()) {
            cout << "No cars are currently parked.\n";
            return;
        }
        cout << "\nList of All Parked Cars:\n";
        for (auto& entry : car_map) {
            string token = "";
            for (auto& pair : token_to_car) {
                if (pair.second == entry.first) {
                    token = pair.first;
                    break;
                }
            }
            ParkingSpot* spot = entry.second;
            cout << "Car: " << entry.first << ", Token: " << token
                 << ", Lane: " << spot->lane + 1
                 << ", Position: " << spot->position + 1
                 << ", Entry Time: " << ctime(&spot->car->entry_time);
        }
    }

    void showRevenue() {
        cout << "=====================================" << endl;
        cout << "Total Revenue Collected: Rs." << fixed << setprecision(2) << total_revenue << endl;
        cout << "=====================================";
    }
};

    ParkingLot lot;
    int choice;
    string input;

    do {
        cout << "\n============== Parking System ==============\n";
        cout << "1. Park Car\n2. Exit Car\n3. Find Car\n4. View All Parked Cars\n5. Show Revenue\n6. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1:
                cout << "Enter Car Number: ";
                getline(cin, input);
                lot.enterParking(input);
                break;
            case 2:
                cout << "Enter Token: ";
                getline(cin, input);
                lot.exitParking(input);
                break;
            case 3:
                cout << "Enter Token: ";
                getline(cin, input);
                lot.findCar(input);
                break;
            case 4:
                lot.viewAllParkedCars();
                break;
            case 5:
                lot.showRevenue();
                break;
            case 6:
                lot.saveAllData();
                cout << "Exiting... Data saved.\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }

    } while (choice != 6);

    return 0;
}
