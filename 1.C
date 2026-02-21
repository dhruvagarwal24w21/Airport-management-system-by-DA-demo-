/*
 * ================================================================
 *          AIRPORT MANAGEMENT SYSTEM
 *          Language : C
 *          Author   : [Your Name]
 *          College  : [Your College]
 *          Year     : 1st Year
 * ================================================================
 *
 *  FEATURES:
 *    1. Admin Login (secured)
 *    2. Add / View / Search / Cancel Flights
 *    3. Register Passengers
 *    4. Book Tickets & Generate Boarding Pass
 *    5. Cancel Booking with Refund
 *    6. View all Bookings & Passengers
 *    7. Data saved in files (persistent)
 *    8. Statistics Dashboard
 *
 * ================================================================
 */

#include <stdio.h>      // printf, scanf, FILE operations
#include <stdlib.h>      // exit(), system()
#include <string.h>      // strcmp, strcpy, strlen
#include <time.h>        // time(), for dates

/* ================================================================
 *  SECTION 1: CONSTANTS & CONFIGURATION
 * ================================================================ */

#define MAX_FLIGHTS     100
#define MAX_PASSENGERS  500
#define MAX_BOOKINGS    500

// File names for saving data
#define FLIGHT_FILE     "flights.dat"
#define PASSENGER_FILE  "passengers.dat"
#define BOOKING_FILE    "bookings.dat"

// Admin credentials (change these!)
#define ADMIN_USERNAME  "admin"
#define ADMIN_PASSWORD  "airport123"

/* ================================================================
 *  SECTION 2: DATA STRUCTURES (structs)
 * ================================================================
 *
 *  WHAT IS A STRUCT?
 *  Think of it as a "custom data type" that groups related data.
 *  Just like a form/card that holds multiple fields together.
 *
 * ================================================================ */

// ---------- Flight Information ----------
typedef struct {
    int    id;                    // Unique ID (auto-generated)
    char   flightNumber[15];     // e.g., "AI-101", "6E-205"
    char   airline[40];          // e.g., "Air India"
    char   source[40];           // e.g., "Delhi"
    char   destination[40];      // e.g., "Mumbai"
    char   date[15];             // e.g., "25/01/2025"
    char   departureTime[10];    // e.g., "14:30"
    char   arrivalTime[10];      // e.g., "16:45"
    int    totalSeats;           // e.g., 180
    int    availableSeats;       // decreases when booked
    float  priceEconomy;         // e.g., 4500.00
    float  priceBusiness;        // e.g., 12000.00
    int    isActive;             // 1 = running, 0 = cancelled
} Flight;

// ---------- Passenger Information ----------
typedef struct {
    int    id;                   // Unique ID (auto-generated)
    char   name[50];             // Full name
    int    age;                  // Age
    char   gender;               // 'M', 'F', or 'O'
    char   phone[15];            // Phone number
    char   email[50];            // Email
    char   passport[20];        // Passport number
    char   nationality[30];     // e.g., "Indian"
} Passenger;

// ---------- Booking / Ticket ----------
typedef struct {
    int    id;                   // Booking ID (auto-generated)
    int    flightId;             // Which flight
    int    passengerId;          // Which passenger
    char   seatNumber[6];       // e.g., "12A"
    char   seatClass;           // 'E' = Economy, 'B' = Business
    float  amountPaid;           // Ticket price
    char   bookingDate[15];     // When booked
    int    isActive;             // 1 = confirmed, 0 = cancelled
} Booking;

/* ================================================================
 *  SECTION 3: GLOBAL VARIABLES
 * ================================================================
 *
 *  We store all data in arrays. Think of these as "tables"
 *  in a database. The count variables track how many entries
 *  exist in each array.
 *
 * ================================================================ */

Flight    flights[MAX_FLIGHTS];
Passenger passengers[MAX_PASSENGERS];
Booking   bookings[MAX_BOOKINGS];

int flightCount    = 0;    // How many flights added so far
int passengerCount = 0;    // How many passengers registered
int bookingCount   = 0;    // How many bookings made
int isLoggedIn     = 0;    // 0 = not logged in, 1 = logged in

/* ================================================================
 *  SECTION 4: UTILITY / HELPER FUNCTIONS
 * ================================================================
 *  Small functions used repeatedly throughout the program.
 * ================================================================ */

// Clear the console screen (works on Windows & Linux)
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Pause and wait for user to press Enter
void pauseScreen() {
    printf("\n\t>>> Press Enter to continue...");
    // Clear any leftover characters in input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar();
}

// Get today's date as a string
void getTodayDate(char *buffer) {
    time_t     now  = time(NULL);
    struct tm *info = localtime(&now);
    sprintf(buffer, "%02d/%02d/%04d",
            info->tm_mday,
            info->tm_mon + 1,
            info->tm_year + 1900);
}

// Print a decorative line
void printLine(char ch, int length) {
    printf("\t");
    for (int i = 0; i < length; i++) printf("%c", ch);
    printf("\n");
}

// Print a centered header
void printHeader(const char *title) {
    clearScreen();
    printf("\n");
    printLine('=', 60);
    printf("\t   %s\n", title);
    printLine('=', 60);
}

// Convert string to lowercase (for case-insensitive search)
void toLowerStr(char *dest, const char *src) {
    int i;
    for (i = 0; src[i]; i++) {
        if (src[i] >= 'A' && src[i] <= 'Z')
            dest[i] = src[i] + 32;   // ASCII trick: 'A'+32 = 'a'
        else
            dest[i] = src[i];
    }
    dest[i] = '\0';
}

// Safe string input (reads full line including spaces)
void readString(char *buffer, int maxLen) {
    fgets(buffer, maxLen, stdin);
    // Remove trailing newline
    buffer[strcspn(buffer, "\n")] = '\0';
}

// Flush input buffer
void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ================================================================
 *  SECTION 5: FILE HANDLING (Save & Load Data)
 * ================================================================
 *
 *  WHY FILE HANDLING?
 *  Without files, all data disappears when you close the program.
 *  We save data to .dat files so it persists between runs.
 *
 *  HOW IT WORKS:
 *  fopen()  → Opens a file
 *  fwrite() → Writes struct data to file (binary mode)
 *  fread()  → Reads struct data from file
 *  fclose() → Closes the file
 *
 * ================================================================ */

// ---------- Save all flights to file ----------
void saveFlights() {
    FILE *fp = fopen(FLIGHT_FILE, "wb");  // "wb" = write binary
    if (fp == NULL) {
        printf("\n\t[ERROR] Cannot save flights!\n");
        return;
    }
    // First write the count, then write all flight structs
    fwrite(&flightCount, sizeof(int), 1, fp);
    fwrite(flights, sizeof(Flight), flightCount, fp);
    fclose(fp);
}

// ---------- Load all flights from file ----------
void loadFlights() {
    FILE *fp = fopen(FLIGHT_FILE, "rb");  // "rb" = read binary
    if (fp == NULL) {
        flightCount = 0;  // File doesn't exist yet, start fresh
        return;
    }
    fread(&flightCount, sizeof(int), 1, fp);
    fread(flights, sizeof(Flight), flightCount, fp);
    fclose(fp);
}

// ---------- Save all passengers to file ----------
void savePassengers() {
    FILE *fp = fopen(PASSENGER_FILE, "wb");
    if (fp == NULL) {
        printf("\n\t[ERROR] Cannot save passengers!\n");
        return;
    }
    fwrite(&passengerCount, sizeof(int), 1, fp);
    fwrite(passengers, sizeof(Passenger), passengerCount, fp);
    fclose(fp);
}

// ---------- Load all passengers from file ----------
void loadPassengers() {
    FILE *fp = fopen(PASSENGER_FILE, "rb");
    if (fp == NULL) {
        passengerCount = 0;
        return;
    }
    fread(&passengerCount, sizeof(int), 1, fp);
    fread(passengers, sizeof(Passenger), passengerCount, fp);
    fclose(fp);
}

// ---------- Save all bookings to file ----------
void saveBookings() {
    FILE *fp = fopen(BOOKING_FILE, "wb");
    if (fp == NULL) {
        printf("\n\t[ERROR] Cannot save bookings!\n");
        return;
    }
    fwrite(&bookingCount, sizeof(int), 1, fp);
    fwrite(bookings, sizeof(Booking), bookingCount, fp);
    fclose(fp);
}

// ---------- Load all bookings from file ----------
void loadBookings() {
    FILE *fp = fopen(BOOKING_FILE, "rb");
    if (fp == NULL) {
        bookingCount = 0;
        return;
    }
    fread(&bookingCount, sizeof(int), 1, fp);
    fread(bookings, sizeof(Booking), bookingCount, fp);
    fclose(fp);
}

// ---------- Save everything ----------
void saveAllData() {
    saveFlights();
    savePassengers();
    saveBookings();
}

// ---------- Load everything ----------
void loadAllData() {
    loadFlights();
    loadPassengers();
    loadBookings();
}

/* ================================================================
 *  SECTION 6: ADMIN LOGIN
 * ================================================================ */

int adminLogin() {
    char username[30], password[30];

    printHeader("ADMIN LOGIN");
    printf("\n\tUsername : ");
    scanf("%s", username);
    printf("\tPassword : ");
    scanf("%s", password);

    if (strcmp(username, ADMIN_USERNAME) == 0 &&
        strcmp(password, ADMIN_PASSWORD) == 0) {
        isLoggedIn = 1;
        printf("\n\t[SUCCESS] Login successful! Welcome, Admin.\n");
        pauseScreen();
        return 1;
    } else {
        printf("\n\t[FAILED] Invalid credentials!\n");
        pauseScreen();
        return 0;
    }
}

/* ================================================================
 *  SECTION 7: FLIGHT MANAGEMENT
 * ================================================================ */

// ---------- Add a new flight ----------
void addFlight() {
    printHeader("ADD NEW FLIGHT");

    if (flightCount >= MAX_FLIGHTS) {
        printf("\n\t[ERROR] Maximum flight limit (%d) reached!\n", MAX_FLIGHTS);
        pauseScreen();
        return;
    }

    Flight *f = &flights[flightCount];  // Pointer to next empty slot
    f->id = flightCount + 1001;         // IDs start from 1001
    f->isActive = 1;

    printf("\n\tFlight Number (e.g. AI-101)  : ");
    scanf("%s", f->flightNumber);
    flushInput();

    printf("\tAirline Name                 : ");
    readString(f->airline, 40);

    printf("\tSource City                  : ");
    readString(f->source, 40);

    printf("\tDestination City             : ");
    readString(f->destination, 40);

    printf("\tDate (DD/MM/YYYY)            : ");
    scanf("%s", f->date);

    printf("\tDeparture Time (HH:MM)       : ");
    scanf("%s", f->departureTime);

    printf("\tArrival Time (HH:MM)         : ");
    scanf("%s", f->arrivalTime);

    printf("\tTotal Seats                  : ");
    scanf("%d", &f->totalSeats);
    f->availableSeats = f->totalSeats;

    printf("\tEconomy Class Price (INR)    : ");
    scanf("%f", &f->priceEconomy);

    printf("\tBusiness Class Price (INR)   : ");
    scanf("%f", &f->priceBusiness);

    flightCount++;
    saveFlights();

    printf("\n\t=============================================\n");
    printf("\t  FLIGHT ADDED SUCCESSFULLY!\n");
    printf("\t  Flight ID     : %d\n", f->id);
    printf("\t  Flight Number : %s\n", f->flightNumber);
    printf("\t  Route         : %s -> %s\n", f->source, f->destination);
    printf("\t=============================================\n");

    pauseScreen();
}

// ---------- Display all flights in a table ----------
void viewAllFlights() {
    printHeader("ALL FLIGHTS");

    if (flightCount == 0) {
        printf("\n\tNo flights in the system.\n");
        pauseScreen();
        return;
    }

    printf("\n\t%-5s %-9s %-14s %-11s %-11s %-11s %-7s %-6s %-7s\n",
           "ID", "Flight#", "Airline", "From", "To",
           "Date", "Depart", "Seats", "Status");
    printLine('-', 85);

    for (int i = 0; i < flightCount; i++) {
        Flight f = flights[i];
        printf("\t%-5d %-9s %-14s %-11s %-11s %-11s %-7s %-6d %-7s\n",
               f.id,
               f.flightNumber,
               f.airline,
               f.source,
               f.destination,
               f.date,
               f.departureTime,
               f.availableSeats,
               f.isActive ? "Active" : "Cancel");
    }

    printLine('-', 85);
    printf("\tTotal flights: %d\n", flightCount);

    pauseScreen();
}

// ---------- View detailed info of one flight ----------
void viewFlightDetails(int index) {
    Flight f = flights[index];

    printf("\n\t+------------------------------------------+\n");
    printf("\t|         FLIGHT DETAILS                    |\n");
    printf("\t+------------------------------------------+\n");
    printf("\t  Flight ID       : %d\n", f.id);
    printf("\t  Flight Number   : %s\n", f.flightNumber);
    printf("\t  Airline         : %s\n", f.airline);
    printf("\t  Source          : %s\n", f.source);
    printf("\t  Destination     : %s\n", f.destination);
    printf("\t  Date            : %s\n", f.date);
    printf("\t  Departure       : %s\n", f.departureTime);
    printf("\t  Arrival         : %s\n", f.arrivalTime);
    printf("\t  Total Seats     : %d\n", f.totalSeats);
    printf("\t  Available Seats : %d\n", f.availableSeats);
    printf("\t  Economy Price   : Rs. %.2f\n", f.priceEconomy);
    printf("\t  Business Price  : Rs. %.2f\n", f.priceBusiness);
    printf("\t  Status          : %s\n", f.isActive ? "Active" : "Cancelled");
    printf("\t+------------------------------------------+\n");
}

// ---------- Search for flights ----------
void searchFlight() {
    printHeader("SEARCH FLIGHTS");

    printf("\n\tSearch by:\n");
    printf("\t  1. Flight Number\n");
    printf("\t  2. Route (Source & Destination)\n");
    printf("\t  3. Date\n");
    printf("\t  4. Flight ID\n");
    printf("\n\tYour choice: ");

    int choice, found = 0;
    scanf("%d", &choice);

    switch (choice) {
        case 1: {
            char query[15];
            printf("\n\tEnter Flight Number: ");
            scanf("%s", query);

            char queryLow[15], flightLow[15];
            toLowerStr(queryLow, query);

            for (int i = 0; i < flightCount; i++) {
                toLowerStr(flightLow, flights[i].flightNumber);
                if (strcmp(queryLow, flightLow) == 0) {
                    viewFlightDetails(i);
                    found = 1;
                }
            }
            break;
        }
        case 2: {
            char src[40], dest[40];
            flushInput();
            printf("\n\tEnter Source City      : ");
            readString(src, 40);
            printf("\tEnter Destination City : ");
            readString(dest, 40);

            char srcLow[40], destLow[40], fSrcLow[40], fDestLow[40];
            toLowerStr(srcLow, src);
            toLowerStr(destLow, dest);

            printf("\n\t%-5s %-9s %-14s %-7s %-6s %-10s\n",
                   "ID", "Flight#", "Airline", "Depart", "Seats", "Price(E)");
            printLine('-', 60);

            for (int i = 0; i < flightCount; i++) {
                toLowerStr(fSrcLow, flights[i].source);
                toLowerStr(fDestLow, flights[i].destination);

                if (strcmp(srcLow, fSrcLow) == 0 &&
                    strcmp(destLow, fDestLow) == 0 &&
                    flights[i].isActive) {
                    printf("\t%-5d %-9s %-14s %-7s %-6d Rs.%.0f\n",
                           flights[i].id,
                           flights[i].flightNumber,
                           flights[i].airline,
                           flights[i].departureTime,
                           flights[i].availableSeats,
                           flights[i].priceEconomy);
                    found = 1;
                }
            }
            break;
        }
        case 3: {
            char date[15];
            printf("\n\tEnter Date (DD/MM/YYYY): ");
            scanf("%s", date);

            printf("\n\t%-5s %-9s %-14s %-11s %-11s %-7s\n",
                   "ID", "Flight#", "Airline", "From", "To", "Depart");
            printLine('-', 65);

            for (int i = 0; i < flightCount; i++) {
                if (strcmp(flights[i].date, date) == 0 && flights[i].isActive) {
                    printf("\t%-5d %-9s %-14s %-11s %-11s %-7s\n",
                           flights[i].id,
                           flights[i].flightNumber,
                           flights[i].airline,
                           flights[i].source,
                           flights[i].destination,
                           flights[i].departureTime);
                    found = 1;
                }
            }
            break;
        }
        case 4: {
            int id;
            printf("\n\tEnter Flight ID: ");
            scanf("%d", &id);

            for (int i = 0; i < flightCount; i++) {
                if (flights[i].id == id) {
                    viewFlightDetails(i);
                    found = 1;
                    break;
                }
            }
            break;
        }
        default:
            printf("\n\t[ERROR] Invalid choice!\n");
            pauseScreen();
            return;
    }

    if (!found) {
        printf("\n\tNo matching flights found.\n");
    }

    pauseScreen();
}

// ---------- Cancel a flight (Admin only) ----------
void cancelFlight() {
    printHeader("CANCEL FLIGHT");

    int id;
    printf("\n\tEnter Flight ID to cancel: ");
    scanf("%d", &id);

    for (int i = 0; i < flightCount; i++) {
        if (flights[i].id == id) {
            if (!flights[i].isActive) {
                printf("\n\tThis flight is already cancelled.\n");
                pauseScreen();
                return;
            }

            viewFlightDetails(i);

            char confirm;
            printf("\n\tAre you sure you want to cancel? (Y/N): ");
            scanf(" %c", &confirm);

            if (confirm == 'Y' || confirm == 'y') {
                flights[i].isActive = 0;

                // Also cancel all bookings on this flight
                int cancelledBookings = 0;
                for (int j = 0; j < bookingCount; j++) {
                    if (bookings[j].flightId == id && bookings[j].isActive) {
                        bookings[j].isActive = 0;
                        cancelledBookings++;
                    }
                }

                saveFlights();
                saveBookings();

                printf("\n\t[SUCCESS] Flight %s cancelled.\n", flights[i].flightNumber);
                printf("\t          %d booking(s) also cancelled.\n", cancelledBookings);
            } else {
                printf("\n\tCancellation aborted.\n");
            }

            pauseScreen();
            return;
        }
    }

    printf("\n\tFlight ID %d not found.\n", id);
    pauseScreen();
}

// ---------- Modify flight details (Admin only) ----------
void modifyFlight() {
    printHeader("MODIFY FLIGHT");

    int id;
    printf("\n\tEnter Flight ID to modify: ");
    scanf("%d", &id);

    for (int i = 0; i < flightCount; i++) {
        if (flights[i].id == id) {
            viewFlightDetails(i);

            printf("\n\tWhat to modify?\n");
            printf("\t  1. Date\n");
            printf("\t  2. Departure Time\n");
            printf("\t  3. Arrival Time\n");
            printf("\t  4. Economy Price\n");
            printf("\t  5. Business Price\n");
            printf("\t  6. Total Seats\n");
            printf("\n\tChoice: ");

            int ch;
            scanf("%d", &ch);

            switch (ch) {
                case 1:
                    printf("\tNew Date (DD/MM/YYYY): ");
                    scanf("%s", flights[i].date);
                    break;
                case 2:
                    printf("\tNew Departure Time (HH:MM): ");
                    scanf("%s", flights[i].departureTime);
                    break;
                case 3:
                    printf("\tNew Arrival Time (HH:MM): ");
                    scanf("%s", flights[i].arrivalTime);
                    break;
                case 4:
                    printf("\tNew Economy Price: ");
                    scanf("%f", &flights[i].priceEconomy);
                    break;
                case 5:
                    printf("\tNew Business Price: ");
                    scanf("%f", &flights[i].priceBusiness);
                    break;
                case 6: {
                    int newSeats;
                    printf("\tNew Total Seats: ");
                    scanf("%d", &newSeats);
                    int booked = flights[i].totalSeats - flights[i].availableSeats;
                    if (newSeats < booked) {
                        printf("\n\t[ERROR] Can't set below %d (already booked).\n", booked);
                    } else {
                        flights[i].availableSeats = newSeats - booked;
                        flights[i].totalSeats = newSeats;
                    }
                    break;
                }
                default:
                    printf("\n\tInvalid choice.\n");
                    pauseScreen();
                    return;
            }

            saveFlights();
            printf("\n\t[SUCCESS] Flight updated!\n");
            pauseScreen();
            return;
        }
    }

    printf("\n\tFlight ID %d not found.\n", id);
    pauseScreen();
}

/* ================================================================
 *  SECTION 8: PASSENGER MANAGEMENT
 * ================================================================ */

// ---------- Register a new passenger ----------
int registerPassenger() {
    printHeader("PASSENGER REGISTRATION");

    if (passengerCount >= MAX_PASSENGERS) {
        printf("\n\t[ERROR] Maximum passenger limit reached!\n");
        pauseScreen();
        return -1;
    }

    Passenger *p = &passengers[passengerCount];
    p->id = passengerCount + 5001;   // IDs start from 5001

    flushInput();

    printf("\n\tFull Name           : ");
    readString(p->name, 50);

    printf("\tAge                 : ");
    scanf("%d", &p->age);

    printf("\tGender (M/F/O)      : ");
    scanf(" %c", &p->gender);

    printf("\tPhone Number        : ");
    scanf("%s", p->phone);

    flushInput();
    printf("\tEmail               : ");
    readString(p->email, 50);

    printf("\tPassport Number     : ");
    scanf("%s", p->passport);

    flushInput();
    printf("\tNationality         : ");
    readString(p->nationality, 30);

    passengerCount++;
    savePassengers();

    printf("\n\t=============================================\n");
    printf("\t  PASSENGER REGISTERED!\n");
    printf("\t  Passenger ID : %d\n", p->id);
    printf("\t  Name         : %s\n", p->name);
    printf("\t=============================================\n");

    return p->id;
}

// ---------- View all passengers ----------
void viewAllPassengers() {
    printHeader("ALL PASSENGERS");

    if (passengerCount == 0) {
        printf("\n\tNo passengers registered.\n");
        pauseScreen();
        return;
    }

    printf("\n\t%-6s %-25s %-4s %-3s %-13s %-12s\n",
           "ID", "Name", "Age", "G", "Phone", "Passport");
    printLine('-', 75);

    for (int i = 0; i < passengerCount; i++) {
        Passenger p = passengers[i];
        printf("\t%-6d %-25s %-4d %-3c %-13s %-12s\n",
               p.id, p.name, p.age, p.gender, p.phone, p.passport);
    }

    printLine('-', 75);
    printf("\tTotal passengers: %d\n", passengerCount);

    pauseScreen();
}

// ---------- Find passenger by ID (helper) ----------
int findPassengerIndex(int id) {
    for (int i = 0; i < passengerCount; i++) {
        if (passengers[i].id == id) return i;
    }
    return -1;
}

// ---------- Find flight by ID (helper) ----------
int findFlightIndex(int id) {
    for (int i = 0; i < flightCount; i++) {
        if (flights[i].id == id) return i;
    }
    return -1;
}

/* ================================================================
 *  SECTION 9: BOOKING SYSTEM (Most Important!)
 * ================================================================ */

// ---------- Book a ticket ----------
void bookTicket() {
    printHeader("BOOK A TICKET");

    if (flightCount == 0) {
        printf("\n\tNo flights available! Ask admin to add flights first.\n");
        pauseScreen();
        return;
    }

    // Step 1: Choose or register passenger
    int passengerId;
    printf("\n\tDo you have a Passenger ID? (1=Yes, 2=No, register new): ");
    int hasId;
    scanf("%d", &hasId);

    if (hasId == 1) {
        printf("\tEnter your Passenger ID: ");
        scanf("%d", &passengerId);

        int pIdx = findPassengerIndex(passengerId);
        if (pIdx == -1) {
            printf("\n\t[ERROR] Passenger ID %d not found!\n", passengerId);
            pauseScreen();
            return;
        }
        printf("\n\tWelcome back, %s!\n", passengers[pIdx].name);
    } else {
        passengerId = registerPassenger();
        if (passengerId == -1) return;
    }

    // Step 2: Show available flights
    printf("\n\t--- AVAILABLE FLIGHTS ---\n\n");
    printf("\t%-5s %-9s %-14s %-11s %-11s %-11s %-6s %-10s\n",
           "ID", "Flight#", "Airline", "From", "To", "Date", "Seats", "Price(E)");
    printLine('-', 85);

    int anyAvailable = 0;
    for (int i = 0; i < flightCount; i++) {
        if (flights[i].isActive && flights[i].availableSeats > 0) {
            printf("\t%-5d %-9s %-14s %-11s %-11s %-11s %-6d Rs.%.0f\n",
                   flights[i].id,
                   flights[i].flightNumber,
                   flights[i].airline,
                   flights[i].source,
                   flights[i].destination,
                   flights[i].date,
                   flights[i].availableSeats,
                   flights[i].priceEconomy);
            anyAvailable = 1;
        }
    }

    if (!anyAvailable) {
        printf("\n\tNo flights with available seats!\n");
        pauseScreen();
        return;
    }

    // Step 3: Select flight
    int flightId;
    printf("\n\tEnter Flight ID to book: ");
    scanf("%d", &flightId);

    int fIdx = findFlightIndex(flightId);
    if (fIdx == -1 || !flights[fIdx].isActive) {
        printf("\n\t[ERROR] Invalid or cancelled flight!\n");
        pauseScreen();
        return;
    }

    if (flights[fIdx].availableSeats <= 0) {
        printf("\n\t[ERROR] No seats available on this flight!\n");
        pauseScreen();
        return;
    }

    // Step 4: Choose class
    char seatClass;
    float price;

    printf("\n\tSelect Class:\n");
    printf("\t  E - Economy  (Rs. %.2f)\n", flights[fIdx].priceEconomy);
    printf("\t  B - Business (Rs. %.2f)\n", flights[fIdx].priceBusiness);
    printf("\n\tYour choice (E/B): ");
    scanf(" %c", &seatClass);

    if (seatClass == 'B' || seatClass == 'b') {
        seatClass = 'B';
        price = flights[fIdx].priceBusiness;
    } else {
        seatClass = 'E';
        price = flights[fIdx].priceEconomy;
    }

    // Step 5: Generate seat number
    int seatNum = flights[fIdx].totalSeats - flights[fIdx].availableSeats + 1;
    char seatRow = 'A' + (seatNum % 6);  // A-F columns

    // Step 6: Create booking
    Booking *b = &bookings[bookingCount];
    b->id = bookingCount + 9001;    // Booking IDs start from 9001
    b->flightId = flightId;
    b->passengerId = passengerId;
    sprintf(b->seatNumber, "%d%c", seatNum, seatRow);
    b->seatClass = seatClass;
    b->amountPaid = price;
    b->isActive = 1;
    getTodayDate(b->bookingDate);

    // Update available seats
    flights[fIdx].availableSeats--;

    bookingCount++;
    saveAllData();

    // Step 7: Print boarding pass
    int pIdx = findPassengerIndex(passengerId);

    printf("\n");
    printLine('*', 50);
    printf("\t*                                                *\n");
    printf("\t*          BOARDING PASS                          *\n");
    printf("\t*                                                *\n");
    printLine('*', 50);
    printf("\t  Booking ID    : %d\n", b->id);
    printf("\t  Passenger     : %s\n", passengers[pIdx].name);
    printf("\t  Flight        : %s (%s)\n", flights[fIdx].flightNumber, flights[fIdx].airline);
    printf("\t  Route         : %s --> %s\n", flights[fIdx].source, flights[fIdx].destination);
    printf("\t  Date          : %s\n", flights[fIdx].date);
    printf("\t  Departure     : %s\n", flights[fIdx].departureTime);
    printf("\t  Arrival       : %s\n", flights[fIdx].arrivalTime);
    printf("\t  Seat          : %s (%s)\n", b->seatNumber,
           seatClass == 'B' ? "Business" : "Economy");
    printf("\t  Amount Paid   : Rs. %.2f\n", b->amountPaid);
    printf("\t  Booking Date  : %s\n", b->bookingDate);
    printf("\t  Status        : CONFIRMED\n");
    printLine('*', 50);
    printf("\t  ** Please arrive 2 hours before departure **\n");
    printLine('*', 50);

    pauseScreen();
}

// ---------- Cancel a booking ----------
void cancelBooking() {
    printHeader("CANCEL BOOKING");

    int bookingId;
    printf("\n\tEnter Booking ID: ");
    scanf("%d", &bookingId);

    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].id == bookingId) {
            if (!bookings[i].isActive) {
                printf("\n\tThis booking is already cancelled.\n");
                pauseScreen();
                return;
            }

            // Show booking details
            int fIdx = findFlightIndex(bookings[i].flightId);
            int pIdx = findPassengerIndex(bookings[i].passengerId);

            printf("\n\t--- BOOKING DETAILS ---\n");
            printf("\tBooking ID  : %d\n", bookings[i].id);
            printf("\tPassenger   : %s\n", passengers[pIdx].name);
            printf("\tFlight      : %s\n", flights[fIdx].flightNumber);
            printf("\tRoute       : %s -> %s\n", flights[fIdx].source, flights[fIdx].destination);
            printf("\tSeat        : %s\n", bookings[i].seatNumber);
            printf("\tAmount Paid : Rs. %.2f\n", bookings[i].amountPaid);

            // Calculate refund (80% refund policy)
            float refund = bookings[i].amountPaid * 0.80;

            printf("\n\tRefund Amount (80%%): Rs. %.2f\n", refund);

            char confirm;
            printf("\n\tConfirm cancellation? (Y/N): ");
            scanf(" %c", &confirm);

            if (confirm == 'Y' || confirm == 'y') {
                bookings[i].isActive = 0;

                // Restore seat
                if (fIdx != -1) {
                    flights[fIdx].availableSeats++;
                }

                saveAllData();

                printf("\n\t[SUCCESS] Booking cancelled.\n");
                printf("\tRefund of Rs. %.2f will be processed.\n", refund);
            } else {
                printf("\n\tCancellation aborted.\n");
            }

            pauseScreen();
            return;
        }
    }

    printf("\n\tBooking ID %d not found.\n", bookingId);
    pauseScreen();
}

// ---------- View all bookings ----------
void viewAllBookings() {
    printHeader("ALL BOOKINGS");

    if (bookingCount == 0) {
        printf("\n\tNo bookings yet.\n");
        pauseScreen();
        return;
    }

    printf("\n\t%-7s %-9s %-20s %-6s %-3s %-10s %-8s\n",
           "BookID", "Flight#", "Passenger", "Seat", "Cls", "Amount", "Status");
    printLine('-', 75);

    for (int i = 0; i < bookingCount; i++) {
        int fIdx = findFlightIndex(bookings[i].flightId);
        int pIdx = findPassengerIndex(bookings[i].passengerId);

        char flightNum[15] = "N/A";
        char passName[50] = "N/A";

        if (fIdx != -1) strcpy(flightNum, flights[fIdx].flightNumber);
        if (pIdx != -1) strcpy(passName, passengers[pIdx].name);

        printf("\t%-7d %-9s %-20s %-6s %-3c Rs.%-7.0f %-8s\n",
               bookings[i].id,
               flightNum,
               passName,
               bookings[i].seatNumber,
               bookings[i].seatClass,
               bookings[i].amountPaid,
               bookings[i].isActive ? "Active" : "Cancel");
    }

    printLine('-', 75);
    printf("\tTotal bookings: %d\n", bookingCount);

    pauseScreen();
}

// ---------- View bookings for a specific passenger ----------
void viewMyBookings() {
    printHeader("MY BOOKINGS");

    int passId;
    printf("\n\tEnter your Passenger ID: ");
    scanf("%d", &passId);

    int pIdx = findPassengerIndex(passId);
    if (pIdx == -1) {
        printf("\n\tPassenger ID not found!\n");
        pauseScreen();
        return;
    }

    printf("\n\tBookings for: %s (ID: %d)\n\n", passengers[pIdx].name, passId);

    int found = 0;
    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].passengerId == passId) {
            int fIdx = findFlightIndex(bookings[i].flightId);

            printf("\t--- Booking #%d ---\n", bookings[i].id);
            if (fIdx != -1) {
                printf("\tFlight : %s (%s)\n", flights[fIdx].flightNumber, flights[fIdx].airline);
                printf("\tRoute  : %s -> %s\n", flights[fIdx].source, flights[fIdx].destination);
                printf("\tDate   : %s\n", flights[fIdx].date);
            }
            printf("\tSeat   : %s (%s)\n", bookings[i].seatNumber,
                   bookings[i].seatClass == 'B' ? "Business" : "Economy");
            printf("\tPaid   : Rs. %.2f\n", bookings[i].amountPaid);
            printf("\tStatus : %s\n\n", bookings[i].isActive ? "CONFIRMED" : "CANCELLED");
            found = 1;
        }
    }

    if (!found) {
        printf("\tNo bookings found for this passenger.\n");
    }

    pauseScreen();
}

/* ================================================================
 *  SECTION 10: STATISTICS DASHBOARD
 * ================================================================ */

void showStatistics() {
    printHeader("AIRPORT STATISTICS DASHBOARD");

    int activeFlights = 0, cancelledFlights = 0;
    int totalSeats = 0, bookedSeats = 0;
    float totalRevenue = 0;
    int activeBookings = 0, cancelledBookings = 0;

    for (int i = 0; i < flightCount; i++) {
        if (flights[i].isActive) {
            activeFlights++;
            totalSeats += flights[i].totalSeats;
            bookedSeats += (flights[i].totalSeats - flights[i].availableSeats);
        } else {
            cancelledFlights++;
        }
    }

    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].isActive) {
            activeBookings++;
            totalRevenue += bookings[i].amountPaid;
        } else {
            cancelledBookings++;
        }
    }

    printf("\n\t+--------------------------------------------+\n");
    printf("\t|             STATISTICS                      |\n");
    printf("\t+--------------------------------------------+\n");
    printf("\t|  Total Flights      : %-5d                |\n", flightCount);
    printf("\t|  Active Flights     : %-5d                |\n", activeFlights);
    printf("\t|  Cancelled Flights  : %-5d                |\n", cancelledFlights);
    printf("\t|--------------------------------------------|\n");
    printf("\t|  Total Passengers   : %-5d                |\n", passengerCount);
    printf("\t|--------------------------------------------|\n");
    printf("\t|  Total Bookings     : %-5d                |\n", bookingCount);
    printf("\t|  Active Bookings    : %-5d                |\n", activeBookings);
    printf("\t|  Cancelled Bookings : %-5d                |\n", cancelledBookings);
    printf("\t|--------------------------------------------|\n");
    printf("\t|  Total Seats        : %-5d                |\n", totalSeats);
    printf("\t|  Booked Seats       : %-5d                |\n", bookedSeats);
    float occupancy = totalSeats > 0 ? (bookedSeats * 100.0 / totalSeats) : 0;
    printf("\t|  Occupancy Rate     : %.1f%%               |\n", occupancy);
    printf("\t|--------------------------------------------|\n");
    printf("\t|  Total Revenue      : Rs. %-10.2f       |\n", totalRevenue);
    printf("\t+--------------------------------------------+\n");

    pauseScreen();
}

/* ================================================================
 *  SECTION 11: LOAD SAMPLE DATA (For Testing)
 * ================================================================ */

void loadSampleData() {
    if (flightCount > 0) {
        printf("\n\tData already exists. Sample data not loaded.\n");
        pauseScreen();
        return;
    }

    // Sample Flights
    Flight sampleFlights[] = {
        {1001, "AI-101",  "Air India",    "Delhi",   "Mumbai",    "28/01/2025", "06:00", "08:15",  180, 175, 4500,  12000, 1},
        {1002, "6E-205",  "IndiGo",       "Delhi",   "Bangalore", "28/01/2025", "09:30", "12:30",  200, 198, 5200,  14000, 1},
        {1003, "UK-833",  "Vistara",      "Mumbai",  "Kolkata",   "29/01/2025", "14:00", "16:30",  160, 160, 4800,  13000, 1},
        {1004, "SG-422",  "SpiceJet",     "Chennai", "Delhi",     "29/01/2025", "07:45", "10:30",  180, 180, 3800,  10000, 1},
        {1005, "AI-305",  "Air India",    "Kolkata", "Hyderabad", "30/01/2025", "11:00", "13:15",  150, 150, 4200,  11500, 1},
        {1006, "6E-117",  "IndiGo",       "Bangalore","Delhi",    "30/01/2025", "16:30", "19:30",  200, 200, 5500,  15000, 1},
    };

    int numFlights = sizeof(sampleFlights) / sizeof(sampleFlights[0]);
    for (int i = 0; i < numFlights; i++) {
        flights[i] = sampleFlights[i];
    }
    flightCount = numFlights;

    // Sample Passengers
    Passenger samplePassengers[] = {
        {5001, "Rahul Sharma",    22, 'M', "9876543210", "rahul@email.com",    "A1234567", "Indian"},
        {5002, "Priya Patel",     25, 'F', "9876543211", "priya@email.com",    "B2345678", "Indian"},
        {5003, "Amit Kumar",      30, 'M', "9876543212", "amit@email.com",     "C3456789", "Indian"},
    };

    int numPass = sizeof(samplePassengers) / sizeof(samplePassengers[0]);
    for (int i = 0; i < numPass; i++) {
        passengers[i] = samplePassengers[i];
    }
    passengerCount = numPass;

    saveAllData();

    printf("\n\t[SUCCESS] Sample data loaded!\n");
    printf("\t  %d flights and %d passengers added.\n", numFlights, numPass);
    pauseScreen();
}

/* ================================================================
 *  SECTION 12: MENUS
 * ================================================================ */

// ---------- Admin Menu ----------
void adminMenu() {
    int choice;

    do {
        printHeader("ADMIN PANEL");

        printf("\n\t  1.  Add New Flight\n");
        printf("\t  2.  View All Flights\n");
        printf("\t  3.  Search Flight\n");
        printf("\t  4.  Modify Flight\n");
        printf("\t  5.  Cancel Flight\n");
        printf("\t  6.  View All Passengers\n");
        printf("\t  7.  View All Bookings\n");
        printf("\t  8.  Statistics Dashboard\n");
        printf("\t  9.  Load Sample Data (for testing)\n");
        printf("\t  0.  Logout\n");

        printf("\n\tEnter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addFlight();        break;
            case 2: viewAllFlights();   break;
            case 3: searchFlight();     break;
            case 4: modifyFlight();     break;
            case 5: cancelFlight();     break;
            case 6: viewAllPassengers();break;
            case 7: viewAllBookings();  break;
            case 8: showStatistics();   break;
            case 9: loadSampleData();   break;
            case 0:
                isLoggedIn = 0;
                printf("\n\tLogged out successfully.\n");
                pauseScreen();
                break;
            default:
                printf("\n\tInvalid choice! Try again.\n");
                pauseScreen();
        }

    } while (choice != 0);
}

// ---------- Passenger / User Menu ----------
void userMenu() {
    int choice;

    do {
        printHeader("PASSENGER MENU");

        printf("\n\t  1.  Search Flights\n");
        printf("\t  2.  View All Flights\n");
        printf("\t  3.  Register as Passenger\n");
        printf("\t  4.  Book a Ticket\n");
        printf("\t  5.  Cancel Booking\n");
        printf("\t  6.  View My Bookings\n");
        printf("\t  0.  Back to Main Menu\n");

        printf("\n\tEnter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: searchFlight();      break;
            case 2: viewAllFlights();    break;
            case 3: registerPassenger(); break;
            case 4: bookTicket();        break;
            case 5: cancelBooking();     break;
            case 6: viewMyBookings();    break;
            case 0: break;
            default:
                printf("\n\tInvalid choice! Try again.\n");
                pauseScreen();
        }

    } while (choice != 0);
}

/* ================================================================
 *  SECTION 13: MAIN FUNCTION (Entry Point)
 * ================================================================ */

int main() {
    // Load saved data from files when program starts
    loadAllData();

    int choice;

    do {
        printHeader("AIRPORT MANAGEMENT SYSTEM");

        printf("\n\n");
        printf("\t      [1]  Admin Login\n");
        printf("\t      [2]  Passenger / User\n");
        printf("\t      [3]  About This Project\n");
        printf("\t      [0]  Exit\n");
        printf("\n");
        printLine('-', 60);
        printf("\n\tEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                if (adminLogin()) {
                    adminMenu();
                }
                break;
            case 2:
                userMenu();
                break;
            case 3:
                printHeader("ABOUT THIS PROJECT");
                printf("\n\tProject : Airport Management System\n");
                printf("\tLanguage: C Programming\n");
                printf("\tAuthor  : [Your Name]\n");
                printf("\tCollege : [Your College Name]\n");
                printf("\tYear    : 1st Year, [Your Branch]\n");
                printf("\n\tFeatures:\n");
                printf("\t  - Admin & User roles\n");
                printf("\t  - Flight management (CRUD)\n");
                printf("\t  - Passenger registration\n");
                printf("\t  - Ticket booking with boarding pass\n");
                printf("\t  - Booking cancellation with refund\n");
                printf("\t  - Search (by number/route/date)\n");
                printf("\t  - Statistics dashboard\n");
                printf("\t  - Persistent data storage (files)\n");
                pauseScreen();
                break;
            case 0:
                saveAllData();
                printHeader("THANK YOU!");
                printf("\n\n\tThank you for using Airport Management System!\n");
                printf("\tAll data has been saved.\n\n");
                printLine('=', 60);
                printf("\n");
                break;
            default:
                printf("\n\tInvalid choice! Please try again.\n");
                pauseScreen();
        }

    } while (choice != 0);

    return 0;
}