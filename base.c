// ==================== HEADER FILES ====================
// Required header files for input/output, memory utilities,string handling, character validation and date/time operations.
#include<stdio.h>
#include<stdlib.h> 
#include<string.h>
#include<ctype.h> 
#include<time.h>

// ==================== GLOBAL CONSTANTS ====================
// Global constants used for ticket pricing, bus speed, maximum ticket capacity, seat capacity and booking status.
#define PRICE_PER_KM 20
#define AVG_SPEED 75          // Average bus speed in km/h to estimate travel duration
#define MAX_TICKETS 100      // Global array used to store all booked and cancelled tickets. //
#define MAX_SEATS 40
#define BOOKED 1
#define CANCELLED 0
/*==============================================================================================================

                                             STRUCTURE 

================================================================================================================*/ 
// Structure representing a city stop and its distance from the starting city to the destination.
typedef struct{
    int ticket_id;
    char name[50];
    char contact[15];
    int age;
    char gender[10];
    char date[20];
    int timeSlot;
    int seatNumber;
    char source[50];
    char destination[50];
    int duration;
    float price;
    int bookingStatus;
    int discount;
    float total_price;
    char routeDisplay[100];
    int routeDistance;
    int direction;
    int fromKm;
    int toKm;
} BusTicket;
BusTicket tickets[MAX_TICKETS];
int count = 0;
int lastTicketID = 1000;
typedef struct{
    const char *name;      // Pointer to constant city name string
    int km;
} Stop;

// Stores all six cities in their fixed route order along with their respective distances from Karachi.
#define NUM_STOPS 6
static const Stop stops[NUM_STOPS] = {
    {"Karachi",0},{"Hyderabad",165},{"Sukkur",470},
    {"Multan",880},{"Lahore",1218},{"Islamabad",1593}
};
/*==============================================================================================================

                                        FUNCTIONS DECLARATION

================================================================================================================*/
// Function prototypes for booking, cancellation, searching, displaying tickets, calculations, validation and file handling.

// Core Ticket Management & Menu Functions
void inputTicket(BusTicket tickets[], int index);
void displayTicket(BusTicket tickets[], int index);
void userGuide(void);
void displayAllTickets(void);
void bookTicket(void);
void cancelTicket(void);
int findTicketByID(int ticket_id);
void searchTicket(void);
int calculateDiscount(int age);
float calculateTotalPrice(float price, int discount);
void setRouteDetails(BusTicket tickets[], int index);
int generateTicketID(void);
void saveTickets(void);
void loadTickets(void);
void menu(void);

// Internal Helper & Input Validation Functions (Scoped to this file using 'static')
static int isValidName(const char *s);
static int isValidContact(const char *s);
static int isValidAge(int age);
static int isValidGender(const char *s);
static int isValidDate(char *s);
static int isSeatTaken(int seat, int timeSlot, const char *date, int direction, int fromKm, int toKm);
static int showAvailableSeats(const char *date, int timeSlot, int direction, int fromKm, int toKm);
static int findStop(const char *name);
static int strcasecmp_custom(const char *s1, const char *s2);     // Compares two strings ignoring uppercase/lowercase differences
static void clearInput(void);                   // Flushes leftover characters from the standard input buffer
/*==============================================================================================================

                                      CONDITION CHECK FUNCTIONS

================================================================================================================*/
// Validates the passenger name by allowing : letters, spaces, dots, hyphens, and apostrophes.
static int isValidName(const char *s){
    int letters = 0;
    for(int i = 0; s[i]; i++){
        unsigned char c = (unsigned char)s[i];
        if(isalpha(c))
            letters++;
        else if(c != ' ' && c != '.' && c != '-' && c != '\'')
            return 0;                 // Reject digits, commas, or invalid special characters
    }
    return letters > 2;              // Ensure name has at least 3 actual letters
}

// Validates Pakistani mobile contact format: must be exactly 11 digits starting with "03"
static int isValidContact(const char *s){
    if(strlen(s)!=11)
    return 0;                        // Reject if length is not 11
    if(s[0]!='0' || s[1]!='3')
    return 0;                        // Reject if prefix is not "03"
    for(int i=0;i<11;i++){
        if(!isdigit((unsigned char)s[i]))
        return 0;                    // Reject if any character is non-numeric
    }
    return 1;
}

// Checks whether the passenger's age is within the allowed range of 1 to 120 years.
static int isValidAge(int age){
    return age>0 && age<=120;
}

// Validates the entered gender against the accepted options: Male, Female or Other.
static int isValidGender(const char *s){
    if(strcasecmp_custom(s, "m") == 0 || strcasecmp_custom(s, "male") == 0)
        return 1;
    if(strcasecmp_custom(s, "f") == 0 || strcasecmp_custom(s, "female") == 0)
        return 1;
    if(strcasecmp_custom(s, "other") == 0)
        return 1;
    return 0;
}

// Validates the travel date format, checks date limits and prevents booking for past or same-day dates.
static int isValidDate(char *s){
    int d, m, y;
    char extra;
  
  // Extract day, month, year; 'extra' catches invalid text at the end (e.g., "12/05/2026abc").
    if(sscanf(s, "%d/%d/%d%c", &d, &m, &y, &extra) != 3){
        printf("Use the format dd/mm/yyyy.\n");
        return 0;
    }
    if(m < 1 || m > 12 || d < 1 || d > 31){
        printf("Invalid day or month!\n");
        return 0;
    }
    if((m == 4 || m == 6 || m == 9 || m == 11) && d > 30){
        printf("This month only has 30 days!\n");
        return 0;
    }
    if(m == 2 && d > 28){
        printf("February has at most 28 days here!\n");
        return 0;
    }

    // Fetch current system date using <time.h>.
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int today = (t->tm_year + 1900) * 365 + (t->tm_mon + 1) * 30 + t->tm_mday;
    int user  = y * 365 + m * 30 + d;

    if(user < today){
        printf("Past dates are not allowed.\n");
        return 0;
    }
    if(user == today){
        printf("No booking possible for today.\n");
        return 0;
    }
    if(user > today + 60){
        printf("Bookings are only open up to 2 months ahead.\n");
        return 0;
    }

    sprintf(s, "%02d/%02d/%04d", d, m, y);   // Stores as dd/mm/yyyy
    return 1;
}
// Converts the numeric time slot into its corresponding readable departure time.
static const char *timeSlotC(int timeSlot) {
    switch (timeSlot) {
        case 1: return "09:00 AM";
        case 2: return "02:00 PM";
        case 3: return "08:00 PM";
        default: return "Invalid Slot";
    }
}
// Checks whether a particular seat is already booked for the selected date, time slot and overlapping route.
static int isSeatTaken(int seat, int timeSlot, const char *date, int direction, int fromKm, int toKm){
    for(int i = 0; i < count; i++){
        if(tickets[i].bookingStatus == BOOKED && 
           tickets[i].seatNumber == seat && 
           tickets[i].timeSlot == timeSlot && 
           strcmp(tickets[i].date, date) == 0 &&
           tickets[i].direction == direction && 
           fromKm < tickets[i].toKm && tickets[i].fromKm < toKm)     // Overlap condition
            return 1;
    }
    return 0;
}

// Displays all seats and identifies which seats are available or already booked for the selected trip.
// Prints a 5-column visual seat map ([XX] = booked, [01] = free).
static int showAvailableSeats(const char *date,int timeSlot, int direction, int fromKm, int toKm){
    int freeCount = 0;
    printf("\n--- SEATS for %s (Slot: %s) ---\nNote: [XX] = booked seat for part of your trip\n\n", date, timeSlotC(timeSlot));
    printf("\n+----------------------------+\n");
    for(int s = 1; s <= MAX_SEATS; s++){
        if(isSeatTaken(s, timeSlot, date, direction, fromKm, toKm))
            printf("[XX] ");
        else{
            printf("[%2d] ", s);
            freeCount++;
        }
        if(s % 5 == 0) 
        printf("\n");          // New row after every 5 seats
    }
    printf("+----------------------------+\n");
    printf("\nAvailable seats: %d of %d\n", freeCount, MAX_SEATS);
    return freeCount;
}

// Searches for a city name in the 'stops' array (case-insensitive) and returns its index if found or -1
static int findStop(const char *name){
    for(int i = 0; i < NUM_STOPS; i++){
        if(strcasecmp_custom(stops[i].name, name) == 0)
            return i;
    }
    return -1;
}

// Custom case-insensitive string comparison function (returns 0 if strings match)
// Compares two strings without considering uppercase and lowercase differences.
static int strcasecmp_custom(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2))
            return 1;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

// Clears the remaining characters from the input buffer until a newline or end-of-file is encountered.
static void clearInput(void){
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}
 
/*==============================================================================================================

                                            MAIN FUNCTION 

================================================================================================================*/
// Main entry point of the program.
// Loads previously saved tickets, displays the main menu, handles user selections and saves ticket records on exit.
int main(){
  // Load previously saved ticket records from 'tickets.csv' into memory on startup
    loadTickets();
    printf("\n+============================================================================+\n");
    printf("|                                                                            |\n");
    printf("|                 WELCOME TO BUS TICKET RESERVATION SYSTEM                   |\n");
    printf("|                                                                            |\n");
    printf("|                     YOUR JOURNEY, OUR PRIORITY                             |\n");
    printf("|                                                                            |\n");
    printf("|                  SAFE  |  RELIABLE  |  CONVENIENT                          |\n");
    printf("|                                                                            |\n");
    printf("+============================================================================+\n");
    int opt;
    do{
        menu();    // Display main menu options
      // Validate that user entered a numeric choice; flush buffer if non-numeric input is given
        if(scanf("%d", &opt) != 1){
            clearInput();
            opt = 0;
        }
      // Route execution based on the user's menu selection
        switch(opt){
            case 1:
                bookTicket();
                saveTickets();   // Saving booking info into the file.
                break;
            case 2:
                cancelTicket();
                saveTickets();   //saving cancellation into the file.
                break;
            case 3:
                searchTicket();
                break;
            case 4:
                displayAllTickets();
                break;
            case 5:
                userGuide();
                break;
            case 6:
                printf("\n+============================================================================+\n");
                printf("|                                                                            |\n");
                printf("|                       THANK YOU FOR CHOOSING US!                           |\n");
                printf("|                                                                            |\n");
                printf("|                 All records have been saved successfully.                  |\n");
                printf("|                                                                            |\n");
                printf("|              WISHING YOU A SAFE AND COMFORTABLE JOURNEY!                   |\n");
                printf("|                                                                            |\n");
                printf("+============================================================================+\n");
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    }while(opt != 6);
  
    return 0;
}
/*==============================================================================================================

                                          MENU FUNCTIONS

================================================================================================================*/
// Displays the main menu and all available operations of Our Bus Ticket Reservation System.
void menu(void){
    printf("\n+----------------------------------------------------------------------------+\n");
    printf("|                        BUS TICKET RESERVATION SYSTEM                       |\n");
    printf("+-------------------------------------+--------------------------------------+\n");
    printf("| 1. Book Ticket                      | 4. Display All Tickets               |\n");
    printf("| 2. Cancel Ticket                    | 5. User Guide                        |\n");
    printf("| 3. Search Ticket                    | 6. Exit                              |\n");
    printf("+-------------------------------------+--------------------------------------+\n");
    printf("Enter your choice (1-6): ");
    
}
/*==============================================================================================================

                                          INPUT FUNCTIONS

================================================================================================================*/
// Collects passenger details, validates user inputs, selects the travel route 
// and time slot, checks seat availability and completes ticket information.
void inputTicket(BusTicket tickets[], int index){
  
// Assign a new unique ticket ID (e.g., 1001, 1002...)
tickets[index].ticket_id = generateTicketID();

// Taking passenger name as input
    do{
    printf("Enter passenger name: ");
    if(scanf(" %49[^\n]", tickets[index].name) != 1)
        tickets[index].name[0] = '\0';
    clearInput();
    if(!isValidName(tickets[index].name))
        printf("Name must contain letters only (no numbers).\n");
    } while(!isValidName(tickets[index].name));

// Taking contact number as input and validating it to be in the format 03XXXXXXXXX
    do{
        printf("Enter contact number(03XXXXXXXXX): ");
        if(scanf(" %14[^\n]", tickets[index].contact) !=1){    
            tickets[index].contact[0] = '\0';
        }
        clearInput();
        if(!isValidContact(tickets[index].contact))
            printf("Please Enter a valid contact number\n");
    } while(!isValidContact(tickets[index].contact));

// Taking age as input and validating it to be between 1 and 120 
    do{
        printf("Enter age(1-120): ");
        if(scanf("%d", &tickets[index].age) != 1){
            tickets[index].age = -1;
            clearInput();
        }
        if(!isValidAge(tickets[index].age))
            printf("Enter correct age.\n");
    } while(!isValidAge(tickets[index].age));

// Taking Gender as input and validating it to be (MALE/FEMALE/OTHER)
   do{
    printf("Enter gender (Male/Female/Other): ");
    if(scanf(" %9[^\n]", tickets[index].gender) != 1)
        tickets[index].gender[0] = '\0';
    clearInput();
    if(!isValidGender(tickets[index].gender))
        printf("Enter correct gender.\n");
    } while(!isValidGender(tickets[index].gender));

// Taking travel date as input and validating it to be in the format dd/mm/yyyy and it must be within 2 months ahead 
    do{
    printf("Enter travel date (dd/mm/yyyy): ");
    if(scanf(" %19[^\n]", tickets[index].date) != 1)
        tickets[index].date[0] = '\0';
    clearInput();
    } while(!isValidDate(tickets[index].date));

    // Selecting time slot for the trip
    printf("\n+----------------------------------+\n");
    printf("| Select a time slot:              |\n");
    printf("| 1. 09:00 AM                      |\n");
    printf("| 2. 02:00 PM                      |\n");
    printf("| 3. 08:00 PM                      |\n");
    printf("+----------------------------------+\n");
    do {
        printf("Enter your Slot choice (1-3): ");
        if(scanf("%d", &tickets[index].timeSlot) != 1) {
            tickets[index].timeSlot = 0;
            clearInput();
        }
        if(tickets[index].timeSlot < 1 || tickets[index].timeSlot > 3)
            printf("Invalid selection. Please choose a valid time slot (1-3).\n");
    } while(tickets[index].timeSlot < 1 || tickets[index].timeSlot > 3);
 
    // Work out the route first so we can reject invalid routes early 
    setRouteDetails(tickets, index);
    BusTicket *t = &tickets[index];      // Creating a pointer shortcut to access current ticket fields easily
    if(showAvailableSeats(t->date,t->timeSlot, t->direction, t->fromKm, t->toKm) == 0){    // Show available seats; if 0 seats are free, mark booking as failed and exit early
        printf("Sorry, this trip is fully booked.\n");
        t->routeDistance = 0;
        return;
    }
  
    // Validating seat selection (must be 1-40 and not already taken)
    int seat;
    do{
        printf("Enter seat number (1-%d): ", MAX_SEATS);
        if(scanf("%d", &seat) != 1) 
            seat = 0;
        clearInput();
        if(seat < 1 || seat > MAX_SEATS)
            printf("Invalid seat number.\n");
        else if(isSeatTaken(seat, t->timeSlot, t->date, t->direction, t->fromKm, t->toKm)){
            printf("Seat %d is already booked for part of this trip.\n", seat);
            seat = 0;        // Reset seat to 0 so the loop repeats
        }
    } while(seat < 1 || seat > MAX_SEATS);     
    tickets[index].discount = calculateDiscount(tickets[index].age);        
    tickets[index].total_price = calculateTotalPrice(tickets[index].price, tickets[index].discount);    // Calculate final price with discount 
    tickets[index].seatNumber = seat;
    tickets[index].bookingStatus = BOOKED;             // Confirm booking status
}
/*==============================================================================================================

                                          DISPLAY FUNCTIONS

================================================================================================================*/
// Displays all details of a particular ticket, including passenger information, route, fare and status.
void displayTicket(BusTicket tickets[], int index){
    printf("\n+----------------------------------------------------+\n");
    printf("|               BUS TICKET / BOARDING PASS           |\n");
    printf("+----------------------------------------------------+\n");
    printf("| Ticket ID: %d\n", tickets[index].ticket_id);
    printf("| Passenger Name: %s\n", tickets[index].name);
    printf("| Contact Number: %s\n", tickets[index].contact);
    printf("| Age: %d\n", tickets[index].age);
    printf("| Gender: %s\n", tickets[index].gender);
    printf("| Travel Date: %s\n", tickets[index].date);
    printf("| Time Slot: %s\n", timeSlotC(tickets[index].timeSlot));
    printf("| Seat Number: %d\n", tickets[index].seatNumber);
    printf("| Source: %s\n", tickets[index].source);
    printf("| Destination: %s\n", tickets[index].destination);
    printf("| Duration: %d hours\n", tickets[index].duration);
    printf("| Price: %.2f\n", tickets[index].price);
    printf("| Discount: %d%%\n", tickets[index].discount);
    printf("| Total Price: %.2f\n", tickets[index].total_price);
    printf("| Route Display: %s\n", tickets[index].routeDisplay);
    printf("| Status: %s\n", tickets[index].bookingStatus == BOOKED ? "Booked" : "Cancelled");
    printf("+---------------------------------------------------+\n");
}


/*==============================================================================================================

                                          ROUTE DETAILS FUNCTIONS

================================================================================================================*/
// Takes source and destination cities from the user, determines the travel direction, 
// builds the route display and calculates total distance, duration and ticket price.


void setRouteDetails(BusTicket tickets[], int index){
    BusTicket *t = &tickets[index];      // Pointer shortcut for the current ticket
    char srcIn[50], dstIn[50];
    int from, to;

  // Display all available cities from the stops array
    printf("\nAvailable cities: ");
    for(int i = 0; i < NUM_STOPS; i++){
    printf("%s", stops[i].name);
    if(i < NUM_STOPS - 1)
        printf(", ");
    else
        printf("\n");
    }
  
  // Get and validate starting city (source)
    do{
        printf("Enter source: ");
        if(scanf(" %49[^\n]", srcIn) != 1) 
            srcIn[0] = '\0';
        clearInput();
        from = findStop(srcIn);    // Find index of entered city
        if(from == -1) 
        printf("City not found. Choose from the list.\n");
    } while(from == -1);

    // Get and validate ending city (destination)
    do{
        printf("Enter destination: ");
        if(scanf(" %49[^\n]", dstIn) != 1) 
            dstIn[0] = '\0';
        clearInput();
        to = findStop(dstIn);
        if(to == -1)
            printf("City not found. Choose from the list.\n");
        else if(to == from){
            printf("Source and destination must be different.\n");
            to = -1;    // Reset to -1 so the loop repeats
        }
    } while(to == -1);
  
    // Store the selected source and destination city names in the ticket record.
    strcpy(t->source, stops[from].name);
    strcpy(t->destination, stops[to].name);

    // Determine travel direction and kilometer range (keep fromKm < toKm always)
    
    int step;
  // Determine the direction of travel.
     if(from < to){
        step = 1;                       // Forward route (Karachi -> Islamabad)
        t->direction = 1;
        t->fromKm = stops[from].km;
        t->toKm   = stops[to].km;
     }
     else {
        step = -1;                     // Return route (Islamabad -> Karachi)
        t->direction = -1;
        t->fromKm = stops[to].km;
        t->toKm   = stops[from].km;
     }
  
    // Build the complete route string by joining all intermediate cities between source and destination.
     t->routeDisplay[0] = '\0';
     for(int i = from; ; i += step){
        if(i != from) 
            strcat(t->routeDisplay, " -> ");
        strcat(t->routeDisplay, stops[i].name);
        if(i == to) 
            break;              // Stop loop once destination city is added
    }
    // Calculate route distance, estimated travel duration  and base fare using the predefined speed and price per kilometer.
    t->routeDistance = t->toKm - t->fromKm;   /* always positive now, no abs() needed */
    t->duration = (t->routeDistance + AVG_SPEED - 1) / AVG_SPEED;     // rounded-up travel hours,
    t->price = (float)t->routeDistance * PRICE_PER_KM;

    // Show calculated route summary to the user
    printf("\n+--------------------------------------------------+\n");
    printf("\n--- ROUTE DETAILS ---\n");
    printf("Route: %s\n", t->routeDisplay);
    printf("Total Distance: %d km\n", t->routeDistance);
    printf("Total Duration: %d hours\n", t->duration);
    printf("\n+--------------------------------------------------+\n");
}

/*==============================================================================================================

                                          DISCOUNT FUNCTIONS

================================================================================================================*/
// Returns discount percentage based on passenger age (50% for kids, 30% for seniors)
int calculateDiscount(int age){
    if(age<=12)
        return 50;
    else if(age>=60)
        return 30;
    else
        return 0;
}
/*==============================================================================================================

                                          TOTAL PRICE FUNCTION

================================================================================================================*/
// Returns the final ticket price after subtracting the applicable discount from the base fare.
float calculateTotalPrice(float price, int discount){
    return price - (price * discount / 100.0f);     // Used 100.0f to keep decimal precision
}
 
/*==============================================================================================================

                                          BOOKING FUNCTIONS

================================================================================================================*/
// Handles the complete ticket booking process.
// Checks ticket capacity, collects passenger information, confirms successful booking and updates the ticket count.
void bookTicket(void){
    if(count >= MAX_TICKETS){                        // Check if the tickets array has reached its maximum limit
        printf("\nNo more tickets can be booked.\n");
        return;
    }
    inputTicket(tickets, count);
 
    /* InputTicket returns early if the route is invalid */
    if(tickets[count].routeDistance == 0){
        printf("Booking failed. Please try again with a valid route.\n");
        return;
    }

    // Show confirmation slip and increase total ticket counter
    printf("\nTicket booked successfully!");
    displayTicket(tickets, count);
    count++;
}
/*==============================================================================================================

                                          CANCEL FUNCTIONS

================================================================================================================*/
// Cancels an existing ticket using its ticket ID.
// Checks whether the ticket exists and is not already cancelled, then updates its booking status and releases its seat.
void cancelTicket(void){
    int id,done=0,idx;
    do{
        printf("Enter ticket ID to cancel(0 to exit): ");
        if(scanf("%d", &id) != 1){
            clearInput();
            printf("Please enter a valid Ticket ID.\n");
            continue;
        }
        if(id == 0){
            return;
        }
        idx = findTicketByID(id);
        if(idx == -1){
            printf("Ticket not found.\n");
        } 
        else if(tickets[idx].bookingStatus == CANCELLED){
            printf("This ticket is already cancelled.\n");
            done = 1;
        }
        else {
            tickets[idx].bookingStatus = CANCELLED;
            printf("Ticket %d cancelled.\nSeat %d is now free.\n", id, tickets[idx].seatNumber);
            done=1;
        }
    }while (!done);
}
/*==============================================================================================================

                                          SEARCH FUNCTIONS

================================================================================================================*/
// Allows users to search for tickets using a ticket ID.
void searchTicket(void){
    int choice,done=0;
    do{
        printf("\n+----------------------------------+\n");
        printf("| Search by:                       |\n");
        printf("| 1. Ticket ID                     |\n");
        printf("| 2. Passenger Name                |\n");
        printf("+----------------------------------+\n");
        printf("Enter Your Choice(0 to exit): ");
        if(scanf("%d", &choice) != 1){
            clearInput();
            printf("Invalid input.\n");
            continue;
        }
        if(choice == 0){
            return;
        }
        if(choice == 1){
            int id,idx,found=0;
            do{
                printf("Enter ticket ID: ");
                if(scanf("%d", &id) != 1){
                    clearInput();
                    printf("Invalid input.\n");
                    continue;
                }
                idx = findTicketByID(id);
                if(idx == -1) 
                    printf("Ticket not found.\n");
                else{ 
                    displayTicket(tickets, idx);
                    found=1;
                }
            }while(!found);
            if(found)
                done = 1;
        } 
        else if(choice == 2){
            char name[50];
            int found = 0;
            do{
            printf("Enter passenger name: ");
            scanf(" %49[^\n]", name);
            for(int i = 0; i < count; i++){
                if(strcasecmp_custom(tickets[i].name, name) == 0){
                    displayTicket(tickets, i);
                    found = 1;
                }
            }
            if(!found) 
                printf("No ticket found for \"%s\".\n", name);
            }while(!found);
            if (found){
                done=1;
            }
            
        } 
        else {
            printf("Please select from the given options.\n");
        }
    }while(!done);
}
/*==============================================================================================================

                                     DISPLAY ALL TICKETS FUNCTION

================================================================================================================*/
// Loops through all stored records and prints every ticket
void displayAllTickets(void){
    if(count == 0){
        printf("\n+----------------------------------------------------+\n");
        printf("| No tickets booked yet.                             |\n");
        printf("+----------------------------------------------------+\n");
        return;
    }
    printf("\n+====================================================+\n");
    printf("| ALL TICKETS (%d)                                   |\n", count);
    printf("+====================================================+\n");
    for(int i = 0; i < count; i++)
        displayTicket(tickets, i);
}
/*==============================================================================================================

                                          FILE FUNCTIONS

================================================================================================================*/
// Saves all ticket records into tickets.csv so that booking information can be restored later.
void saveTickets(void){
    FILE *fp = fopen("tickets.csv", "w");
    if(fp == NULL){
        printf("Error opening file for writing.\n");
        return;
    }
    // Write the column header row at the top of the CSV file
     fprintf(fp, "Ticket ID, Name, Contact Number, Age, Gender, Travel Date, Time Slot, Seat Number, Source, Destination, Duration, Route Display, Route Distance, Price, Discount, Total Price, Direction, From Km, To Km, Booking Status\n");
    for(int i = 0; i < count; i++){
        fprintf(fp, "%d,%s,%s,%d,%s,%s,%s,%d,%s,%s,%d,%s,%d,%.2f,%d,%.2f,%d,%d,%d,%s\n",   // Write each ticket's 20 fields separated by commas
                                                                    tickets[i].ticket_id, 
                                                                    tickets[i].name,
                                                                    tickets[i].contact, 
                                                                    tickets[i].age, 
                                                                    tickets[i].gender, 
                                                                    tickets[i].date,
                                                                    timeSlotC(tickets[i].timeSlot),
                                                                    tickets[i].seatNumber, 
                                                                    tickets[i].source, 
                                                                    tickets[i].destination, 
                                                                    tickets[i].duration, 
                                                                    tickets[i].routeDisplay, 
                                                                    tickets[i].routeDistance, 
                                                                    tickets[i].price, 
                                                                    tickets[i].discount, 
                                                                    tickets[i].total_price,
                                                                    tickets[i].direction,
                                                                    tickets[i].fromKm,
                                                                    tickets[i].toKm,
                                                                    tickets[i].bookingStatus==1?"Booked":"Cancelled");
    }
    fclose(fp);
}

// Loads previously saved ticket records from tickets.csv,
// restores their booking details and updates the ticket count and last generated ticket ID.
void loadTickets(void){
    FILE *fp = fopen("tickets.csv", "r");
    if(fp == NULL){
        printf("No previous ticket data found.\n");
        return;
    }
    char line[512];
    char status[20];
    char timeSlotStr[20];
    fgets(line, sizeof(line), fp); // Read and skip the first header line so fscanf starts from actual ticket data
    while(fscanf(fp, "%d,%49[^,],%14[^,],%d,%9[^,],%19[^,],%19[^,],%d,%49[^,],%49[^,],%d,%99[^,],%d,%f,%d,%f,%d,%d,%d,%19[^\r\n]",
                                                                                          &tickets[count].ticket_id, 
                                                                                          tickets[count].name, 
                                                                                          tickets[count].contact, 
                                                                                          &tickets[count].age,  
                                                                                          tickets[count].gender, 
                                                                                          tickets[count].date,
                                                                                          timeSlotStr,
                                                                                          &tickets[count].seatNumber, 
                                                                                          tickets[count].source, 
                                                                                          tickets[count].destination, 
                                                                                          &tickets[count].duration, 
                                                                                          tickets[count].routeDisplay, 
                                                                                          &tickets[count].routeDistance, 
                                                                                          &tickets[count].price, 
                                                                                          &tickets[count].discount, 
                                                                                          &tickets[count].total_price,
                                                                                          &tickets[count].direction,
                                                                                          &tickets[count].fromKm, 
                                                                                          &tickets[count].toKm, 
                                                                                          status) == 20){
      // Convert time slot text back into integer code (1, 2, or 3)
        if(strcmp(timeSlotStr, "09:00 AM") == 0)
            tickets[count].timeSlot = 1;
        else if(strcmp(timeSlotStr, "02:00 PM") == 0)
            tickets[count].timeSlot = 2;
        else if(strcmp(timeSlotStr, "08:00 PM") == 0)
            tickets[count].timeSlot = 3;
        else
            tickets[count].timeSlot = 1;
        // Convert status text back into integer constant (BOOKED or CANCELLED)
        tickets[count].bookingStatus= (strcmp(status, "Booked")== 0) ? BOOKED : CANCELLED;
        count++;
        if(tickets[count-1].ticket_id > lastTicketID)
        lastTicketID = tickets[count-1].ticket_id;     // Update lastTicketID to the last loaded ticket ID
        if(count >= MAX_TICKETS) 
        break;
    }
    fclose(fp);
}
/*==============================================================================================================

                                          TICKET ID FUNCTIONS

================================================================================================================*/
 
// 1. Generate Unique Ticket ID by incrementing the last used ticket ID.
int generateTicketID(void) {
    return ++lastTicketID;
}
 
// 2. Find Ticket Index By ID and returns its array index if found, otherwise -1.
int findTicketByID(int ticket_id) {
    for (int i = 0; i < count; i++) {
        if (tickets[i].ticket_id == ticket_id) {
            return i;
        }
    }
    return -1;
}
/*==============================================================================================================

                                      USER GUIDE / INFO FUNCTION

================================================================================================================*/
// Displays instructions about ticket booking, passenger requirements, available routes, fares,
// time slots, discount policies and other system features.
void userGuide(void){
    printf("\n+============================================================================+\n");
    printf("|                    BUS TICKET RESERVATION - USER GUIDE                     |\n");
    printf("+============================================================================+\n");

    printf("\n+----------------------------------------------------------------------------+\n");
    printf("| [1] REQUIRED DETAILS FOR BOOKING:                                          |\n");
    printf("+----------------------------------------------------------------------------+\n");
    printf("| * Passenger Name : Min 3 letters; alphabets, spaces, (.), (-), (') only.   |\n");
    printf("| * Contact Number : 11-digit mobile starting with '03' (e.g. 03001234567).  |\n");
    printf("| * Age            : Must be between 1 and 120 years.                        |\n");
    printf("| * Gender         : Male (M), Female (F), or Other.                         |\n");
    printf("| * Travel Date    : Format dd/mm/yyyy (e.g., 25/11/2026).                   |\n");
    printf("|                    - Same-day or past dates are NOT allowed.               |\n");
    printf("|                    - Advance booking open up to 60 days (2 months) ahead.  |\n");
    printf("| * Time Slots     : 3 Time Slots available for our Service                  |\n");
    printf("|                    - 1. 09:00 AM                                           |\n");
    printf("|                    - 2. 02:00 PM                                           |\n");
    printf("|                    - 3. 08:00 PM                                           |\n");
    printf("| * Seat Number    : Choose from 1 to %-2d (booked seats cannot be picked).    |\n", MAX_SEATS);
    printf("+----------------------------------------------------------------------------+\n");

    printf("\n+----------------------------------------------------------------------------+\n");
    printf("| [2] AVAILABLE CITIES, FARES & TIMINGS (Two-Way):                           |\n");
    printf("+----------------------------------------------------------------------------+\n");
    printf("| Cities on the route (in order):                                            |\n");
    printf("| Karachi -> Hyderabad -> Sukkur -> Multan -> Lahore -> Islamabad            |\n");
    printf("| * You can travel between ANY two of these cities, in either direction.     |\n");
    printf("| * Fare     : Rs. %-2d per km.                                                |\n", PRICE_PER_KM);
    printf("| * Duration : Distance / %-2d km/h, rounded up to the next hour.              |\n", AVG_SPEED);
    printf("+----------------------------+----------+----------+-------------------------+\n");
    printf("| Route                      | Distance | Duration | Base Fare               |\n");
    printf("+----------------------------+----------+----------+-------------------------+\n");
    printf("| Karachi   <-> Hyderabad    |  165 km  |  3 hrs   | Rs. %-19d |\n", 165  * PRICE_PER_KM);
    printf("| Karachi   <-> Sukkur       |  470 km  |  7 hrs   | Rs. %-19d |\n", 470  * PRICE_PER_KM);
    printf("| Karachi   <-> Multan       |  880 km  | 12 hrs   | Rs. %-19d |\n", 880  * PRICE_PER_KM);
    printf("| Karachi   <-> Lahore       | 1218 km  | 17 hrs   | Rs. %-19d |\n", 1218 * PRICE_PER_KM);
    printf("| Karachi   <-> Islamabad    | 1593 km  | 22 hrs   | Rs. %-19d |\n", 1593 * PRICE_PER_KM);
    printf("| Lahore    <-> Islamabad    |  375 km  |  5 hrs   | Rs. %-19d |\n", 375  * PRICE_PER_KM);
    printf("+----------------------------+----------+----------+-------------------------+\n");
    printf("| * Other city pairs (e.g. Sukkur-Multan) are charged the same by distance.  |\n");
    printf("| * Discounts (see below) are applied on top of the base fare.               |\n");
    printf("+----------------------------------------------------------------------------+\n");

    printf("\n+----------------------------------------------------------------------------+\n");
    printf("| [3] DISCOUNT POLICY:                                                       |\n");
    printf("+----------------------------------------------------------------------------+\n");
    printf("| * Children (Age 12 or below)     : 50%% Discount                            |\n");
    printf("| * Senior Citizens (Age 60+)      : 30%% Discount                            |\n");
    printf("| * Regular Passengers (Age 13-59) : No Discount (Full Fare)                 |\n");
    printf("+----------------------------------------------------------------------------+\n");

    printf("\n+----------------------------------------------------------------------------+\n");
    printf("| [4] OTHER SYSTEM FEATURES & RULES:                                         |\n");
    printf("+----------------------------------------------------------------------------+\n");
    printf("| * Ticket ID    : Automatically generated (starts from 1001) upon booking.  |\n");
    printf("| * Cancel Ticket: Cancel anytime using Ticket ID; seat becomes free again.  |\n");
    printf("| * Search Ticket: Search existing bookings by Ticket ID or Passenger Name.  |\n");
    printf("| * Capacity     : Maximum %-3d total tickets can be stored in the system.    |\n", MAX_TICKETS);
    printf("| * Auto-Save    : Tickets Auto-save in 'tickets.csv'.                       |\n");
    printf("+============================================================================+\n");

    clearInput();
    printf("Press Enter to return to the Main Menu...");
    getchar();
}
