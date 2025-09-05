//v0.0.1-betaX
//Credit: KhoabuHtiG@github.com
//This is just a beta project! Used for practicing so there is a lot of things that's not good. Thank you!
#pragma once
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <thread>
#include <chrono>
#include <conio.h>
#include "UserData.h"
namespace fs = std::filesystem;

//Clear screen function
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
//Get current timestamp
std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&time));

    return std::string(buf);
}

//Create main directory for storing user data
void createDirectory() {
    try {
        if (!fs::exists(MainDirec)) {
           fs::create_directory(MainDirec);
        }
    } catch (fs::filesystem_error &e) { //Case if there is an error
        std::cout << "Error found: " << e.what();
    }
}

//Create a folder for the user
void createUserFolder(std::string username){
    fs::path userFolder = MainDirec / ("UserData_" + username);

    try {
        fs::create_directory(userFolder);
    } catch(fs::filesystem_error &e) { //Case if there is an error
        std::cout << "Error found: " << e.what();
    }
}

//Save user data into a file
bool savingData(std::string username, std::string password, int balance, int pin) {
    Data Data(username, password, balance, pin);

    if (!Data.saveData()) return false;

    return true;
}

//Get user data from a file
bool gettingData(std::string &username, std::string &password, int &balance, int &pin) {
    Data Data(username, password, balance, pin);

    if (!Data.getData()) return false;

    username = Data.getUsername();
    password = Data.getPassword();
    balance = Data.getBalance();
    pin = Data.getPin();

    return true;
}

//View user transaction log
bool viewingTransactionlog(std::string username, std::string password, int balance, int pin) {
    Data Data(username, password, balance, pin);
    return Data.viewTransactionLog();
}

//View log class for saving transaction logs
class ViewLog {
public:
    //Save transaction log
    static void saveTransactionLog(std::string username, int const amount, std::string const transactionType, std::string target, std::string targetName, int balance) {
        try {
            fs::path transactionLogFile = getUserTranslogFile(username);
            std::ofstream transactionLog(transactionLogFile, std::ios::app);

            if (transactionLog.is_open()) {
                if (transactionType == "Transfer") {
                    transactionLog << "[" << getTimestamp() << "] " << "Transfer " << target << " " << targetName 
                        << ": -" << amount << "$ | Balance: " << balance << "$\n";
                }
                transactionLog.close();
            }
        } catch (fs::filesystem_error &e) { //Case if there is an error
            std::cout << "Error found: " << e.what();
        }
    }

    //Save withdraw log
    static void saveWithdrawLog(std::string username, int const amount, std::string const transactionType, int balance) {
        try {
            fs::path transactionLogFile = getUserTranslogFile(username);
            std::ofstream transactionLog(transactionLogFile, std::ios::app);

            if (transactionLog.is_open()) {
                if (transactionType == "Withdraw") {
                    transactionLog << "[" << getTimestamp() << "] " 
                        << "Withdrew: " << ": -" << amount << "$ | Balance: " << balance << "$\n";
                }
                transactionLog.close();
            }
        } catch (fs::filesystem_error &e) { //Case if there is an error
            std::cout << "Error found: " << e.what();
        }
    }

    //Save deposit log
    static void saveDepositLog(std::string username, int const amount, std::string const transactionType, int balance) {
        try {
            fs::path transactionLogFile = getUserTranslogFile(username);
            std::ofstream transactionLog(transactionLogFile, std::ios::app);

            if (transactionLog.is_open()) {
                if (transactionType == "Deposit") {
                    transactionLog << "[" << getTimestamp() << "] " 
                        << "Deposited: " << ": +" << amount << "$ | Balance: " << balance << "$\n";
                }
                transactionLog.close();
            }
        } catch (fs::filesystem_error &e) { //Case if there is an error
            std::cout << "Error found: " << e.what();
        }
    };
};


//Transaction function
void transaction(std::string username, int pin, int &current_balance, std::string password) {
    int comfirmPin, transactionBalance, targetBalance, targetPin;
    std::string targetName, targetPassword, type = "Transfer", to = "to";
    targetName.clear();

    std::cout << "Enter the recipient's username: ";
    std::cin >> targetName;

    fs::path targetDataFile = getUserDataFile(targetName);
    std::ifstream targetuserdata(targetDataFile, std::ios::in);

    //Check if the target user exists
    if (!targetuserdata.is_open()) {
        std::cout << "No account found with this name.\n";
        return;
    }

    //Get target user data for the transaction
    targetuserdata >> targetName >> targetPassword >> targetBalance >> targetPin;
    std::cout << "Enter your amount for transaction: ";
    std::cin >> transactionBalance;

    if (!(transactionBalance <= current_balance)) {
        std::cout << "Insufficient funds\n";
        return;
    }

    //Prevent negative or zero transaction
    if (transactionBalance < 1) {
        std::cout << "Can't transfer negative number or zero";
        return;
    }

    std::cout << "Enter your PIN for continue: ";
    std::cin >> comfirmPin;

    if (std::cin.fail()) {
        std::cin.clear(); // Clear the fail state
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
        std::cout << "Invalid input. Please enter a valid PIN.\n";
        return;
    }

    //Check if the PIN is correct
    if (comfirmPin != pin) {
        std::cout << "Incorrect PIN. \n";
        return;
    }

    //Process the transaction
    current_balance -= transactionBalance;
    targetBalance += transactionBalance;

    std::cout << "Transacted successfully\n";
    std::cout << "You have transacted " << transactionBalance << "$ to " << targetName << '\n';
    std::cout << "Remaining balance: " << current_balance << "$\n";

    //Save transaction log for both users
    if (!(savingData(username, password, current_balance, pin)) || !(savingData(targetName, targetPassword, targetBalance, targetPin))) {
        std::cout << "Error while transacting. Please try again later.\n";
        return;
    }
    ViewLog::saveTransactionLog(username, transactionBalance, type, to, targetName, current_balance);

    targetuserdata.close();
    return;
};

//Login function
bool login(std::string password) {
    std::string comfirmPassword;
    char characters;
    int tries = 0, waitTime = 30;

    while (true) {
        //Clear the previous input
        comfirmPassword.clear();

        std::cout << "Enter your password: ";
        characters = getch();

        //Input password with masking
        while (characters != 13) { // 13 is Enter
            if (characters == '\b') { // Backspace
                if (!comfirmPassword.empty()) {
                    comfirmPassword.pop_back();
                    std::cout << "\b \b"; // Erase last '*'
                }
            } else {
                comfirmPassword.push_back(characters);
                std::cout << "*";
            }
            characters = getch();
        }

        //Check if the password is correct
        if (comfirmPassword != password) {
            std::cout << "\nIncorrect password.\n";
            comfirmPassword.clear();
            tries++;

            //Lock the user out after 3 failed attempts
            if (tries >= 3) {
                std::cout << "Too many failed attempts. Please wait before retrying.\n";    
                for (int i = waitTime; i > 0; i--) {
                    std::cout << "\rRetry available in: " << i << " seconds..";
                    std::cout.flush();
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }

                std::cout << "\nYou may try again now. ";
                tries = 2, waitTime *= 2;
            }
        } else return true;
    }
};


//Create a new account
void newAccount() {
    std::string username, checkUsername, password;
    char characters;
    int balance = 10000, pin;

    while (true) {
        //Clear the previous input
        username.clear();
        password.clear();

        std::cout << "Set a username: ";
        std::cin >> username;

        //Check if the username is already taken
        fs::path checkUsernameFile = getUserDataFile(username);
        std::ifstream targetuserdata(checkUsernameFile, std::ios::in);

        //Case if the username is already taken
        if (targetuserdata.is_open()) {
            std::cout << "Someone already had this name! Please try another one.\n";
            username.clear();
            continue;
        }

        //Case if the username is not taken
        while (true) {
            std::cout << "Set a password (min 8 characters): ";
            characters = getch();

            //Input password with masking
            while (characters != 13) { // 13 is Enter
                if (characters == '\b') { // Backspace
                    if (!password.empty()) {
                        password.pop_back();
                        std::cout << "\b \b"; // Erase last '*'
                    }
                } else {
                    password.push_back(characters);
                    std::cout << "*";
                }
                characters = getch();
            }

            //Check password length
            if (password.size() < 8) {
                std::cout << "\nPassword must be 8 characters at least.\n";
                password.clear();
                continue;
            }

            std::cout << "\n";
            break;
        }

        while (true) {
            std::cout << "Set a 4-digit PIN (for withdrawals, deposits, and security): ";
            std::cin >> pin;

            if (std::cin.fail()) {
                std::cin.clear(); // Clear the fail state
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
                std::cout << "Invalid input. Please enter a valid 4-digit PIN.\n";
                pin = 0;
                continue;
            }

            //Check if the PIN is 4 digits
            if (pin < 1000 || pin > 9999) {
                std::cout << "PIN must be 4 numbers at least.\n";
                pin = 0;
                continue;
            }

            break;
        }

        //Create user folder and save data
        createDirectory();
        createUserFolder(username);
        if (!(savingData(username, password, balance, pin))) {
            std::cout << "Failed to create a new account.\n";
            continue;
        }

        std::cout << "Successfully created an account!\n";
        return;
    }
};

//Withdraw function
void withdraw(int &balance, std::string &username, std::string &password, int &pin) {
    std::string withdrawType = "Withdraw";
    int withdrawMoney, comfirmPin;

    std::cout << "Enter your PIN: ";
    std::cin >> comfirmPin;

    if (std::cin.fail()) {
        std::cin.clear(); // Clear the fail state
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
        std::cout << "Invalid input. Please enter a valid PIN.\n";
        return;
    }

    //Check if the PIN is correct
    if (comfirmPin != pin) {
        std::cout << "Incorrect PIN. \n";
        return;
    }

    std::cout << "Current balance: " << balance << "$\n";
    std::cout << "Enter the amount you want to withdraw: ";
    std::cin >> withdrawMoney;


    //Case if the withdraw amount is more than the balance
    if (withdrawMoney > balance) {
        std::cout << "Insufficient funds\n";
        return;
    }

    //Prevent negative or zero withdraw
    if (withdrawMoney < 1) {
        std::cout << "Can't be a negative or zero\n";
        return;
    }

    //Process the withdraw
    balance -= withdrawMoney;

    std::cout << "Withdrew successfully\n";
    std::cout << "You withdrew: " << withdrawMoney << "$\n";
    std::cout << "Current balance: " << balance << "$\n";

    //Save the new balance
    if (!(savingData(username, password, balance, pin))) {
        std::cout << "Error while withdrawing. Please try again later.\n";
        return;
    }

    ViewLog::saveWithdrawLog(username, withdrawMoney, withdrawType, balance);
    return;
}


//Deposit function
void deposit(int &balance, std::string &username, std::string &password, int &pin) {
    std::string depositType = "Deposit";
    int depositMoney, comfirmPin;

    std::cout << "Enter your PIN: ";
    std::cin >> comfirmPin;

    if (std::cin.fail()) {
        std::cin.clear(); // Clear the fail state
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
        std::cout << "Invalid input. Please enter a valid PIN.\n";
        return;
    }

    //Check if the PIN is correct
    if (comfirmPin != pin) {
        std::cout << "Incorrect PIN. \n";
        return;
    }

    std::cout << "Current balance: " << balance << "$\n";
    std::cout << "Enter the amount you want to deposit: ";
    std::cin >> depositMoney;

    //Prevent negative or zero deposit
    if (depositMoney < 1) {
        std::cout << "Can't be a negative numbers or zero\n";
        return;
    }

    //Process the deposit
    balance += depositMoney;

    std::cout << "Deposited successfully\n";
    std::cout << "You deposited: " << depositMoney << "$\n";
    std::cout << "Current balance: " << balance << "$\n";

    //Save the new balance
    if (!(savingData(username, password, balance, pin))) {
        std::cout << "Error while depositing. Please try again later.\n";
        return;
    }

    ViewLog::saveDepositLog(username, depositMoney, depositType, balance);
    return;
}

//Change password function
void changePassword(std::string &username, std::string &password, int &balance, int &pin) {
    int comfirmPin;
    char characters;
    std::string newPassword;

    std::cout << "Please type in your PIN to do this action: ";
    std::cin >> comfirmPin;

    if (std::cin.fail()) {
        std::cin.clear(); // Clear the fail state
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
        std::cout << "Invalid input. Please enter a valid PIN.\n";
        return;
    }

    //Check if the PIN is correct
    if (comfirmPin != pin) {
        std::cout << "Inccorect PIN!\n";
        return;
    }

    std::cout << "Please type in your new password (min 8 characters): ";
    characters = getch();

    //Input password with masking
    while (characters != 13) { // 13 is Enter
        if (characters == '\b') { // Backspace
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b"; // Erase last '*'
            }
        } else {
            password.push_back(characters);
            std::cout << "*";
        }
        characters = getch();
    }

    //Handle backspace
    if (characters == '\b') {
        if (!newPassword.empty()) {
            newPassword.pop_back();
            std::cout << "\b \b";
        }
    }

    if (newPassword.empty()) {
        std::cout << "\nPassword can't be empty.\n";
        newPassword.clear();
        return;
    }

    if (newPassword == password) {
        std::cout << "\nNew password can't be the same as the old one.\n";
        newPassword.clear();
        return;
    }

    //Check new password length
    if (newPassword.size() >= 8) {
        password = newPassword;
        if (!(savingData(username, newPassword, balance, pin))) {
            std::cout << "\nFailed to changed password due to an error.\n";
            return;
        }
        std::cout << "\nChanged password successfully!\n";
        return;
    }

    std::cout << "\nPassword length must be 8 characters at least.\n";
    return;
}

//Remove user account function
bool removeAccount(std::string &username) {
    fs::path userFolder = MainDirec / ("UserData_" + username);
    
    try {
        //Check if the folder is removed or not (Permission denied, etc..)
        if (!fs::remove_all(userFolder)) {
            std::cout << "Failed to remove account. Please try again later.\n";
            return false;
        }

        std::cout << "Removed successfully\n";
        return true;
    } catch(fs::filesystem_error &e) { //Case if there is an error
        std::cout << "Error found: " << e.what() << '\n';
        return false;
    }
};


//ATM menu
class YourAtm {
    User_Data userData;
public:
    //Syncing the value to the userData struct
    YourAtm(int balance, std::string username, int pin, std::string password) {
        userData.balance = balance, userData.username = username;
        userData.pin = pin, userData.password = password;
    }

    //Main ATM function
    int Atm() {
        clearScreen();
        char option;
        std::string input;

        //ATM menu
        std::vector<std::string> menu = {
            "=====================================",
            "               ATM MENU              ",
            "=====================================",
            " [1] Check Balance", " [2] Transfer Funds", " [3] Withdraw Money", " [4] Deposit Money",
            " [5] Change Password", " [6] View transaction History, ", " [7] Delete/remove account"
        };

        //Print the menu
        std::cout << userData.username << " account\n";
        for (std::string i : menu) {std::cout << i << '\n';}

        while (true) {
            input.clear();
            std::cout << "Options: 1-7 || Menu: 'm' || Log out: 'q': ";
            std::cout << "Choose: ";
            std::getline(std::cin, input);
            
            //Case if the input is empty
            if (input.empty()) {
                std::cout << "Invalid option\n";
                continue;
            }

            option = input[0];

            //Exit the program
            if (option == 'q' || option == 'Q') {std::cout << "Data saved\n"; break;}
            
            //Print the menu again
            if (option == 'm' || option == 'M') {
                clearScreen();
                for (std::string i : menu) {
                    std::cout << i << '\n';
                }
            }

            //Options
            switch(option) {
                case('1'):
                    std::cout << "Current balance: " << userData.balance << "$\n";
                    break;

                case('2'):
                    transaction(userData.username, userData.pin, userData.balance, userData.password);
                    break;

                case('3'):
                    withdraw(userData.balance, userData.username, userData.password, userData.pin);
                    break;

                case('4'):
                    deposit(userData.balance, userData.username, userData.password, userData.pin);
                    break;

                case('5'):
                    changePassword(userData.username, userData.password, userData.balance, userData.pin);
                    break;

                case('6'):
                    viewingTransactionlog(userData.username, userData.password, userData.balance, userData.pin);
                    break;

                case('7'):
                    if (removeAccount(userData.username)) return 0;
                    break;

                default:
                    std::cout << "Invalid option\n";
                    break;
            }
        }
        return 0;
    }
};

//Login menu
void loginMenu() {
    User_Data userData;
    char option;
    std::string input;

    clearScreen();
    std::cout << "=== Welcome to ATM System ===" << std::endl;
 
    //Main loop for login menu
    while (true) {
        std::cout << "|| Log in: l || Sign up: y || Quit: q" << std::endl;
        std::cout << "Choose: ";
        std::getline(std::cin, input);

        //Check if the input is empty
        if (input.empty()) {
            std::cout << "Invalid option\n";
            continue;
        }

        option = input[0];

        //Quit the program
        if (option == 'q' || option == 'Q') return;

        //Options
        switch (option) {
            case 'l':
            case 'L': {
                std::cout << "Enter your username: ";
                std::cin >> userData.username;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer after reading username

                if (userData.username.empty()) {
                    std::cout << "Username can't be empty.\n";
                    break;
                }

                //Check if the user exists and get their data
                if (!(gettingData(userData.username, userData.password, userData.balance, userData.pin))) {
                    std::cout << "No account found with this name\n";
                } else {
                    bool logged = login(userData.password);
                    if (logged == true) {
                        YourAtm myClass(userData.balance, userData.username, userData.pin, userData.password);
                        myClass.Atm();
                    }
                }

                userData.username.clear();
                break;
            }

            //Create a new account
            case 'y':
            case 'Y': 
                std::cout << "Creating a new account for you . . .\n";
                newAccount();
                break;

            //Invalid option
            default:
                std::cout << "Invalid option\n";
                break;
        }
    }
}