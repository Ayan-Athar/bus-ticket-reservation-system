#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#define MAX_TICKETS 100
#define MAX_SEATS 40
#define BOOKED 1
#define CANCELLED 0

typedef struct{
    int ticket_id;
    char name[50];
    char contact[15];
    int age;
    char gender[10];
    char date[20];
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

} BusTicket;
BusTicket tickets[MAX_TICKETS];
int count = 0;
void inputTicket(BusTicket tickets[], int index);
void displayTicket(BusTicket tickets[], int index);
void displayAllTickets(void);
void bookTicket(void);
void cancelTicket(void);
int findTicketByID(int ticket_id);
void searchTicket(void);
int calculateDiscount(int age);
float calculateTotalPrice(float price, int discount);
void setRouteDetails(BusTicket tickets[], int index);
int generateTicketID(void);
void menu(void);

/* Simple route table: distance (km) of each city from a starting point.
   Distance between two cities = |difference|. */
typedef struct { const char *city; int km; } City;
City cities[] = {
    {"Lahore", 0}, {"Faisalabad", 130}, {"Multan", 340},
    {"Islamabad", 375}, {"Peshawar", 480}, {"Karachi", 1210}
};
int cityCount = sizeof(cities) / sizeof(cities[0]);

/* ---------- helpers ---------- */

int findCity(const char *name){
    for(int i = 0; i < cityCount; i++)
        if(strcasecmp(cities[i].city, name) == 0) return i;
    return -1;
}

void clearInput(void){
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

int generateTicketID(void){
    static int nextID = 1001;
    return nextID++;
}

int calculateDiscount(int age){
    if(age < 12) return 50;     /* children */
    if(age >= 60) return 30;    /* senior citizens */
    return 0;
}

float calculateTotalPrice(float price, int discount){
    return price - (price * discount / 100.0f);
}

void setRouteDetails(BusTicket tickets[], int index){
    int s = findCity(tickets[index].source);
    int d = findCity(tickets[index].destination);
    int dist = (int)fabs((double)(cities[s].km - cities[d].km));

    tickets[index].routeDistance = dist;
    tickets[index].duration = (dist + 49) / 50;      /* hours at ~50 km/h */
    tickets[index].price = dist * 2.5f;              /* Rs 2.5 per km */
    snprintf(tickets[index].routeDisplay, sizeof(tickets[index].routeDisplay),
             "%s -> %s", cities[s].city, cities[d].city);
}

int isSeatTaken(int seat, const char *date, const char *src, const char *dst){
    for(int i = 0; i < count; i++){
        if(tickets[i].bookingStatus == BOOKED &&
           tickets[i].seatNumber == seat &&
           strcmp(tickets[i].date, date) == 0 &&
           strcasecmp(tickets[i].source, src) == 0 &&
           strcasecmp(tickets[i].destination, dst) == 0)
            return 1;
    }
    return 0;
}

/* ---------- main features ---------- */

void inputTicket(BusTicket tickets[], int index){
    int s, d;

    printf("Enter passenger name: ");
    scanf(" %49[^\n]", tickets[index].name);
    printf("Enter contact number: ");
    scanf(" %14s", tickets[index].contact);

    do{
        printf("Enter age: ");
        if(scanf("%d", &tickets[index].age) != 1){ clearInput(); tickets[index].age = -1; }
    } while(tickets[index].age <= 0 || tickets[index].age > 120);

    printf("Enter gender (Male/Female/Other): ");
    scanf(" %9s", tickets[index].gender);
    printf("Enter travel date (DD-MM-YYYY): ");
    scanf(" %19s", tickets[index].date);

    printf("\nAvailable cities: ");
    for(int i = 0; i < cityCount; i++) printf("%s%s", cities[i].city, i < cityCount - 1 ? ", " : "\n");

    do{
        printf("Enter source: ");
        scanf(" %49[^\n]", tickets[index].source);
        s = findCity(tickets[index].source);
        if(s < 0) printf("Invalid city. Try again.\n");
    } while(s < 0);

    do{
        printf("Enter destination: ");
        scanf(" %49[^\n]", tickets[index].destination);
        d = findCity(tickets[index].destination);
        if(d < 0) printf("Invalid city. Try again.\n");
        else if(d == s){ printf("Destination must differ from source.\n"); d = -1; }
    } while(d < 0);

    /* store proper capitalization */
    strcpy(tickets[index].source, cities[s].city);
    strcpy(tickets[index].destination, cities[d].city);

    int seat;
    do{
        printf("Enter seat number (1-%d): ", MAX_SEATS);
        if(scanf("%d", &seat) != 1){ clearInput(); seat = 0; }
        if(seat < 1 || seat > MAX_SEATS) printf("Invalid seat number.\n");
        else if(isSeatTaken(seat, tickets[index].date, tickets[index].source, tickets[index].destination)){
            printf("Seat %d is already booked for this trip.\n", seat);
            seat = 0;
        }
    } while(seat < 1 || seat > MAX_SEATS);
    tickets[index].seatNumber = seat;

    setRouteDetails(tickets, index);
    tickets[index].discount = calculateDiscount(tickets[index].age);
    tickets[index].total_price = calculateTotalPrice(tickets[index].price, tickets[index].discount);
    tickets[index].ticket_id = generateTicketID();
    tickets[index].bookingStatus = BOOKED;
}

void displayTicket(BusTicket tickets[], int index){
    printf("\n-------------------------------------\n");
    printf("Ticket ID   : %d\n", tickets[index].ticket_id);
    printf("Name        : %s\n", tickets[index].name);
    printf("Contact     : %s\n", tickets[index].contact);
    printf("Age/Gender  : %d / %s\n", tickets[index].age, tickets[index].gender);
    printf("Date        : %s\n", tickets[index].date);
    printf("Route       : %s\n", tickets[index].routeDisplay);
    printf("Distance    : %d km\n", tickets[index].routeDistance);
    printf("Duration    : %d hour(s)\n", tickets[index].duration);
    printf("Seat No.    : %d\n", tickets[index].seatNumber);
    printf("Price       : %.2f\n", tickets[index].price);
    printf("Discount    : %d%%\n", tickets[index].discount);
    printf("Total Price : %.2f\n", tickets[index].total_price);
    printf("Status      : %s\n", tickets[index].bookingStatus == BOOKED ? "Booked" : "Cancelled");
    printf("-------------------------------------\n");
}

void displayAllTickets(void){
    if(count == 0){
        printf("\nNo tickets booked yet.\n");
        return;
    }
    printf("\n===== ALL TICKETS (%d) =====", count);
    for(int i = 0; i < count; i++)
        displayTicket(tickets, i);
}

void bookTicket(void){
    if(count >= MAX_TICKETS){
        printf("\nBus is full. No more tickets can be booked.\n");
        return;
    }
    inputTicket(tickets, count);
    printf("\nTicket booked successfully!");
    displayTicket(tickets, count);
    count++;
}

int findTicketByID(int ticket_id){
    for(int i = 0; i < count; i++)
        if(tickets[i].ticket_id == ticket_id) return i;
    return -1;
}

void cancelTicket(void){
    int id;
    printf("Enter ticket ID to cancel: ");
    if(scanf("%d", &id) != 1){ clearInput(); printf("Invalid input.\n"); return; }

    int idx = findTicketByID(id);
    if(idx == -1){
        printf("Ticket not found.\n");
    } else if(tickets[idx].bookingStatus == CANCELLED){
        printf("This ticket is already cancelled.\n");
    } else {
        tickets[idx].bookingStatus = CANCELLED;
        printf("Ticket %d cancelled. Seat %d is now free.\n", id, tickets[idx].seatNumber);
    }
}

void searchTicket(void){
    int choice;
    printf("\nSearch by:\n1. Ticket ID\n2. Passenger Name\nChoice: ");
    if(scanf("%d", &choice) != 1){ clearInput(); printf("Invalid input.\n"); return; }

    if(choice == 1){
        int id;
        printf("Enter ticket ID: ");
        scanf("%d", &id);
        int idx = findTicketByID(id);
        if(idx == -1) printf("Ticket not found.\n");
        else displayTicket(tickets, idx);
    } else if(choice == 2){
        char name[50];
        int found = 0;
        printf("Enter passenger name: ");
        scanf(" %49[^\n]", name);
        for(int i = 0; i < count; i++){
            if(strcasecmp(tickets[i].name, name) == 0){
                displayTicket(tickets, i);
                found = 1;
            }
        }
        if(!found) printf("No ticket found for \"%s\".\n", name);
    } else {
        printf("Invalid choice.\n");
    }
}

void menu(void){
    printf("\n====== BUS RESERVATION SYSTEM ======\n");
    printf("1. Book Ticket\n");
    printf("2. Cancel Ticket\n");
    printf("3. Search Ticket\n");
    printf("4. Display All Tickets\n");
    printf("5. Exit\n");
    printf("Enter choice: ");
}

int main(){
    int choice;
    do{
        menu();
        if(scanf("%d", &choice) != 1){
            clearInput();
            choice = 0;
        }
        switch(choice){
            case 1: bookTicket(); break;
            case 2: cancelTicket(); break;
            case 3: searchTicket(); break;
            case 4: displayAllTickets(); break;
            case 5: printf("Goodbye!\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while(choice != 5);
    return 0;
}