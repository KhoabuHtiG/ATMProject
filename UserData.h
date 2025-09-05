#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
namespace fs = std::filesystem;
static const fs::path MainDirec = "./ATM_Data";

typedef struct {
    int pin;
    int balance;
    std::string username;
    std::string password;
} User_Data;

//Find the user folder
fs::path getUserFolder(std::string username) {
    fs::path userFolder = MainDirec / ("UserData_" + username);
    return userFolder;
}

//Find the user data file
fs::path getUserDataFile(std::string username) {
    fs::path userFolder = getUserFolder(username);
    fs::path userdfile = userFolder / ("UserData_" + username + ".txt");
    return userdfile;
}


//Find the user transaction log file
fs::path getUserTranslogFile(std::string username) {
    fs::path userFolder = getUserFolder(username);
    fs::path userTranslogFile = userFolder / ("UserTransactionLog_" + username + ".txt");
    return userTranslogFile;
}

class Data {
    User_Data userData;
public:
    //Syncing the value to the userData struct
    Data (std::string username, std::string password, int balance, int pin) {
        userData.username = username, userData.password = password;
        userData.balance = balance, userData.pin = pin;
    }

    //Saving data for the user
    bool saveData() {
        fs::path userdfile = getUserDataFile(userData.username);

        try {
            std::ofstream outFile(userdfile, std::ios::trunc);

            //Check if the file is opened or not
            if (!(outFile.is_open())) return false;

            //Check if the file is corrupted or not
            if (!(outFile << userData.username << '\n' << userData.password << '\n' << userData.balance << '\n' << userData.pin << '\n')) {
                return false;
            }

            outFile.close();
            return true;
        } catch(fs::filesystem_error &e) { //Case if there is an error
            std::cout << "Error found: " << e.what();
            return false;
        }
    };

    //Getting data for the user
    bool getData() {
        fs::path userdfile = getUserDataFile(userData.username);

        try {
            std::ifstream inFile(userdfile, std::ios::in);

            //Check if the file is opened or not
            if (!(inFile.is_open())) return false;
            
            //Check if the file is corrupted or not
            if (!(inFile >> userData.username && inFile >> userData.password && inFile >> userData.balance && inFile >> userData.pin)) {
                return false;
            }

            inFile.close();
            return true;
        } catch (fs::filesystem_error &e) { //Case if there is an error
            std::cout << "Error found: " << e.what();
            return false;
        }
    };

    //View transaction log
    bool viewTransactionLog() {
        fs::path transactionLogFile = getUserTranslogFile(userData.username);
        bool hasTransactions = false;

        try {
            std::ifstream transactionLog(transactionLogFile, std::ios::in);

            if (!(transactionLog.is_open())) {
                std::cout << "No transaction history found. ";
                return false;
            }

            std::cout << "===TRANSACTION HISTORY===\n";
            std::string line;
            int count = 0;

            //Display only the last 10 transactions
            while (std::getline(transactionLog, line) && count < 10) {
                std::cout << line << '\n';
                count++;
            }
            transactionLog.close();
        } catch (fs::filesystem_error &e) { //Case if there is an error
            std::cout << "Error found: " << e.what();
            return false;
        }
        return true;
    };

    //Getters
    std::string getUsername() const {return userData.username;}

    std::string getPassword() const {return userData.password;}

    int getBalance() const {return userData.balance;}

    int getPin() const {return userData.pin;}
};