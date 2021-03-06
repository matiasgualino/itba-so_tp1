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
    printf(GREEN "\nAerolITBA | Bienvenido\n" RESET);
    Client currentUser = login();
    initClient();
    while(true){
        printf("----------------------------------------------------------------------------");
        printf("\nSeleccione una opcion del menu: \n");
        printf("\t1. Listar vuelos disponibles.\n");
        printf("\t2. Comprar un vuelo.\n");
        printf("\t3. Cancelar una compra.\n");
        printf("\t4. Salir.\n");
        printf("Opcion:");
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
    printf("\nListado de vuelos disponibles\n");
    int cantFlights = 0;
    Matrix flights;
    flights = get_flights_list();
    switch(flights.responseCode) {
        case 0:
            cantFlights = sizeof(flights.values) / sizeof(flights.values[0]);
            print_flights(flights.values, cantFlights);
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

void substringToIndex(char string[], int index, char sub[]) {
    int c = 0;
    while (c < index) {
        sub[c] = string[c];
        c++;
    }
    sub[c] = '\0';
}

void print_flights(FlightInfo flights[], int count) {
    int i = 0;

    char *flightNumber = malloc(FLIGHT_NUMBER_LENGTH * sizeof(char));
    char *origin = malloc(CITY_LENGTH * sizeof(char));
    char *destination = malloc(CITY_LENGTH * sizeof(char));
    char *date = malloc(DATE_LENGTH * sizeof(char));
    char *hour = malloc(TIME_LENGTH * sizeof(char));

    printf("Nro. Vuelo|\t       Origen       |\t      Destino       |\t   Fecha  \t|\tHora \t|\n");
    for (i = 0; i < count; i++) {
        strncpy(flightNumber, flights[i].flightNumber, FLIGHT_NUMBER_LENGTH);
        strncpy(origin, flights[i].origin, CITY_LENGTH);
        strncpy(destination, flights[i].destination, CITY_LENGTH);
        strncpy(date, flights[i].date, DATE_LENGTH);
        strncpy(hour, flights[i].hour, TIME_LENGTH);

        printf("  %s |\t%s|\t%s|\t%s\t|\t%s\t|\n", 
            flightNumber, origin, destination, date, hour);       
    }
    
    free(flightNumber);
    free(origin);
    free(destination);
    free(date);
    free(hour);

}

void cancel_order(Client c) {
    printf("\nCancelacion de una reserva\n");
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
    printf("\nCompra de un vuelo\n");
    int seat, aux = 1;
    char *flightNumber = malloc(FLIGHT_NUMBER_LENGTH * sizeof(char));
    char *date = malloc(DATE_LENGTH * sizeof(char));
    char *hour = malloc(TIME_LENGTH * sizeof(char));
    char buf[1024];
    Flight f;
    
    printf("\nNumero de vuelo: ");
    fflush(stdout);
    scanf("%7s", flightNumber);

    f = get_flight(flightNumber);    

    if (f.origin[0] == 0) {
        printf("Error al acceder a la base de datos.\n");
        return;
    }

    if (f.seatsLeft == 0) {
        printf("No tenemos mas lugar en este vuelo.\n");
        return;
    }

    strncpy(flightNumber, f.flightNumber, FLIGHT_NUMBER_LENGTH);
    strncpy(date, f.date, DATE_LENGTH);
    strncpy(hour, f.hour, TIME_LENGTH);
    
    printf("\n%s %s - %s\n", flightNumber, date, hour);
    print_seats(f.seats);

    printf("\nElija un asiento:"); 
    while(aux) {
        fgets(buf, sizeof(buf), stdin);

        if(scanf("%d", &seat) != 1){
            aux = INVALID_SEAT;
            printf(RED "El numero de asiento es invalido.\n\nAsiento:" RESET);
            continue;
        }
        aux = reserve_seat(c, flightNumber, seat);
        if (aux == SEAT_TAKEN) {
            printf(YELLOW "Este asiento ya pertenece a otro pasajero. Elija otro.\n\nAsiento:" RESET);
        } else if(aux == INVALID_SEAT) {
            printf(RED "El numero de asiento es invalido.\n\nAsiento:" RESET);
        }
    }
    printf(GREEN "\nFelicitaciones! El asiento %d del vuelo %s ya es tuyo.\n" RESET, seat, flightNumber);
    free(flightNumber);
    free(date);
    free(hour);
}

void print_seats(char seats[STD_SEAT_QTY][MAX_NAME_LENGTH]) {
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

    printf("Nombre de usuario:");
    scanf("%s", username);
    printf("Clave:");
    scanf("%s", password);
    
    Client c;
    strncpy(c.username, username, MAX_NAME_LENGTH);
    strncpy(c.password, password, MAX_NAME_LENGTH);
    return c;
}
