#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include "clientfront.h"
#include "clientback.h"
#include "dbAccess.h"

int main() {
    int command = -1;
    printf(GREEN "\nBienvenido a AerolITBA\n" RESET);
    Client currentUser = login();
    initClient();
    while(true){
        printf("\nSeleccione una opcion del menu: \n\n");
        printf("1. Listar vuelos disponibles.\n");
        printf("2. Comprar un vuelo.\n");
        printf("3. Cancelar una compra.\n");
        printf("4. Salir.\n");
        printf("Numero de opcion:");
        scanf("%d", &command);
        execute_command(command, currentUser);
    }
}

void execute_command(int command, Client currentUser) {
    switch(command){
        case 1:  
                list_flights();
                break;
        case 2:
                buy_flight(currentUser);
                break;
        case 3: 
                cancel_order(currentUser);
                break;
        case 4: 
                closeClient();
                break;
        default: 
                printf("Comando invalido\n");
                return;
    }
}

void list_flights() {
    Matrix flights;
    flights = get_flights_list();
    switch(flights.responseCode) {
        case 0:
            print_flights(flights.values, FLIGHTS_QTY);
            break;
        case -1:
            printf(RED "No pudimos encontrar el listado de vuelos en nuestra base de datos.\n" RESET);
            break;
        case -2:
            printf(RED "Error al leer la base de datos.\n" RESET); 
            break;
        default:
            printf(RED "Error desconocido. Intente telefonicamente. Codigo de error: = %d\n" RESET, flights.responseCode);
            break;
    }
}

void print_flights(Flight flights[], int count) {
    int i = 0;
    printf("Nro. Vuelo|\t       Origen       |\t      Destino       |\t   Fecha  |\tHora \n");
    for (i = 0; i < count; i++) {
        printf("  %s |\t%s|\t%s|\t%s|\t%s\n", 
            flights[i].flightNumber, flights[i].origin, flights[i].destination, flights[i].date, flights[i].hour);
    }
}

void cancel_order(Client c) {
    char flightNumber[FLIGHT_NUMBER_LENGTH];
    int seat;
    printf("Numero de vuelo: ");
    fflush(stdout);
    scanf("%s", flightNumber);    
    
    printf("\nNumero de asiento: ");
    fflush(stdout);
    scanf("%d", &seat);    

    if (cancel_seat(c, flightNumber, seat) == 1) {
        printf(RED "Error: El asiento que quiere cancelar no pertenece a una reserva realizada por usted.\n" RESET); 
    }
    else {
        printf(GREEN "Se ha cancelado su reserva para el asiento %d del vuelo %s\n" RESET, seat, flightNumber);
    }
}

void buy_flight(Client c) {
    char flightNumber[FLIGHT_NUMBER_LENGTH];
    int seat, aux = 1;

    printf("Numero de vuelo: ");
    fflush(stdout);
    scanf("%s", flightNumber);

    Flight f;
    f.origin[0] = 0;
    f = get_flight(flightNumber);    

    if (f.origin[0] == 0) {
        printf("Error al acceder a la base de datos.\n");
        return;
    }

    if (f.seatsLeft == 0) {
        printf("No tenemos mas lugar en este vuelo.\n");
        return;
    }

    char buf[1024];
    printf("Elija un asiento:\n"); 
    printf("%s %s - %s\n", f.flightNumber, f.date, f.hour);
    print_seats(f.seats);
    while(aux) {
        fgets(buf, sizeof(buf), stdin);

        if(scanf("%d", &seat) != 1){
            aux = INVALID_SEAT;
            printf(RED "El numero de asiento es invalido.\n" RESET);
            continue;
        }
        aux = reserve_seat(c, f.flightNumber, seat);
        if (aux == SEAT_TAKEN) {
            printf(YELLOW "Este asiento ya pertenece a otro pasajero. Elija otro.\n" RESET);
        } else if(aux == INVALID_SEAT) {
            printf(RED "El numero de asiento es invalido.\n" RESET);
        }
    }
    printf(GREEN "Felicitaciones! El asiento %d del vuelo %s ya es tuyo.\n" RESET, seat, flightNumber);
}

void print_seats(char seats[STD_SEAT_QTY][MAX_LENGTH]) {
    int row, col, seatNumber;
    for (row = 0; row < 4; row++) {
        for (col = 0; col < 10; col++) {
            if (strcmp(seats[seatNumber = row*10 + col], "\0") == 0) {
                printf(GREEN "%4d" RESET, seatNumber+1);
            } else {
                printf(RED "%4s" RESET, "X");
            }
        }
        printf("\n");
        if (row == 1) {
            printf("\n");
        }
    } 
}

Client login() {
    char username[MAX_NAME_LENGTH],
         password[MAX_NAME_LENGTH];

    printf("Ingrese su nombre de usuario:");
    scanf("%s", username);
    printf("Ingrese su clave:");
    scanf("%s", password);
    
    Client c;
    strncpy(c.username, username, MAX_NAME_LENGTH);
    strncpy(c.password, password, MAX_NAME_LENGTH);
    return c;
}