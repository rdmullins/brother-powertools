#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "internet.h"
#include "transfer.h"
#include "mail.h"
#include "games.h"
#include "cards.h"
#include "catalog.h"
#include "bibliography.h"
#include "cards.h"
#include "notes.h"
#include "library_menu.h"
#include "wiktionary.h"
#include "gutenberg.h"
#include "sourcebooks.h"
#include "weather.h"
#include "npr.h"
#include "lan_gateway.h"

void splash_screen(void);
void main_menu(void);
void wait_for_enter(void);
void draw_weather_tile(int row, WeatherCondition condition, int temperature);
void weather_menu(void);
void lan_gateway(void);

int main (void)
{

    //sourcebooks_lookup();
    splash_screen();
    main_menu();
    return 0;
}
static void forecast_weekday(const char *date, char *weekday, size_t size)
{
    struct tm tm_date;

    memset(&tm_date, 0, sizeof(tm_date));

    if (sscanf(date,
               "%d-%d-%d",
               &tm_date.tm_year,
               &tm_date.tm_mon,
               &tm_date.tm_mday) != 3) {

        snprintf(weekday, size, "???");
        return;
    }

    tm_date.tm_year -= 1900;
    tm_date.tm_mon -= 1;

    /*
     * mktime() needs a reasonably complete struct tm.
     */
    tm_date.tm_hour = 12;

    if (mktime(&tm_date) == (time_t)-1) {
        snprintf(weekday, size, "???");
        return;
    }

    strftime(weekday, size, "%a", &tm_date);
}

static const char *forecast_condition_name(WeatherCondition condition)
{
    switch (condition) {
        case WEATHER_SUNNY:
            return "Sunny";

        case WEATHER_CLOUDY:
            return "Cloudy";

        case WEATHER_RAINY:
            return "Rain";

        case WEATHER_SNOWY:
            return "Snow";

        default:
            return "Unknown";
    }
}

    void splash_screen(void) {

        //int choice = 0;

            printf(
                "_______________________________________________________________________________\n"
                "|                     _              _    _                                   |\n"
                "|                    | |            | |  | |                                  |\n"
                "|                    | |_   __   _  | |_ | |__    __   __                     |\n"
                "|                    |   \\ | _\\ / \\ | _| | __  \\ /-_\\ | _\\                    |\n"
                "|                    | O | | | | O || |_ | | | | ||__ | |                     |\n"
                "|                    \\___/ |_|  \\_/ \\__| |_| |_| \\__/ |_|                     |\n"
                "|                                                                             |\n"
                "|                              PowerTools v0.1.0                              |\n"
                "|_____________________________________________________________________________|\n"
                "|/////////////////////////////////////////////////////////////////////////////|\n"
                "|                                                                             |\n"
                "|/////////////////////////////////////////////////////////////////////////////|\n"
                "|                                                                             |\n"
                "|/////////////////////////////////////////////////////////////////////////////|\n"
                "|/////////////////////////////////////////////////////////////////////////////|\n"
                "|                                                                             |\n"
                "|/////////////////////////////////////////////////////////////////////////////|\n"
                "|///////////////////////// Press [ENTER] to continue /////////////////////////|\n"
                "|/////////////////////////////////////////////////////////////////////////////|\n"
                "|_____________________________________________________________________________|"
        );
        
        wait_for_enter();
        
        return;
        //main_menu();

    } // End splash_screen

    void main_menu(void) {
    int choice;
    WeatherData weather;

    weather_get_current(&weather);

        while (1) {

            printf(
                "_______________________________________________________________________________\n"
                "|                     _              _    _                                   |\n"
                "|                    | |            | |  | |                                  |\n"
                "|                    | |_   __   _  | |_ | |__    __   __                     |\n"
                "|                    |   \\ | _\\ / \\ | _| | __  \\ /-_\\ | _\\                    |\n"
                "|                    | O | | | | O || |_ | | | | ||__ | |                     |\n"
                "|                    \\___/ |_|  \\_/ \\__| |_| |_| \\__/ |_|                     |\n"
                "|_______________________________PowerTools v1.0_______________________________|\n"
                "| +---------------+  +---------------+  +---------------+  +---------------+  |\n"
                "| |               |  |               |  |  <E>    *     |  | 10 PRINT \'HI\' |  |\n"
                "| | <-------      |  |   010011010   |  |    .          |  | 20 GOTO 10    |  |\n"
                "| |     ------->  |  |               |  |        *!*    |  |               |  |\n"
                "| |  1. Transfer  |  |  2. Internet  |  |   3. Games    |  |   4. BASIC    |  |\n"
                "| +---------------+  +---------------+  +---------------+  +---------------+  |\n"
                "| +---------------+  +---------------+  +---------------+  +---------------+  |\n"
                );

//            );            

//        printf(
//            "+---------------------------------------------+\n"
//            "|   |_   _   _  _|_ |_   _   _                |\n"
//            "|   | \\ | \\ / \\  |  | \\ / \\ | \\  PowerTools   |\n"
//            "|   | | |   | |  |  | | |_| |      v0.0.1     |\n"
//            "|   \\_/ |   \\_/  |  | | \\__ |                 |\n"       
//            "|             brother PowerTools              |\n"
//            "|                  Main Menu                  |\n"
//            "+---------------------------------------------+\n"
//            "|     1. File Transfer     4. BASIC           |\n"
//            "|     2. Internet          5. Email           |\n"
//            "|     3. Games             6. Library         |\n"
//            "|                                             |\n"
//            "|                 7. Exit                     |\n"
//            "+---------------------------------------------+\n"
draw_weather_tile(0, weather.condition, weather.temperature);
printf("  |     ______    |  | []  [][]      |  ");
printf("|  __________   |  |\n");

draw_weather_tile(1, weather.condition, weather.temperature);
printf("  |     |\\  /|    |  | [][][][][][]  |  ");
printf("| |===  npr  |  |  |\n");

draw_weather_tile(2, weather.condition, weather.temperature);
printf("  |     |_\\/_|    |  | [][][][][][]  |  ");
printf("| |=== o   o |  |  |\n");

draw_weather_tile(3, weather.condition, weather.temperature);
printf("  |    6. Email   |  |  7. Library   |  |  8. NPR News  |  ");
printf("|\n");
        //printf("|    6. Email   |  |  7. Library   |  |  8. NPR News  |                  |\n");
        printf("| +---------------+  +---------------+  +---------------+  +---------------+  |\n"
//            "|______________________________Enter 9 to Exit________________________________|\n"
"|__________________________0. LAN Gateway   9. Exit___________________________|\n"
"\n"
            "Enter your choice: "
        );
// +--------------+
// |  __________  |
// |  |===  npr | |
// |  |=== o   o| |
// | 8. NPR News  |
// +--------------+
    
        if (scanf("%d", &choice) != 1) {
            printf("scanf failed!\n");
            return;
        };


        
        switch (choice) {
            case 0:
    printf("Starting LAN Gateway...\n");
    lan_gateway();
    break;
            case 1:
                transfer_menu();
                break;
            case 2:
                printf("You selected Internet\n");
                internet_menu();
                break;
            case 3:
                printf("You selected Games\n");
                game_menu();
                break;
            case 4:
                printf("You selected BASIC\n");
                system("bwbasic");
                break;
            case 5:
                printf("You selected Weather\n");
                weather_menu();
                break;
            case 6:
                mail_menu();
                break;
            case 7:
                printf("You selected Library\n");
                library_menu();
                break;
            case 8:
                printf("You selected NPR News\n");
                npr_lookup();
                break; 
            case 9:
                printf("Exiting...\n");
                return;
            default:
                printf("Invalid option. Please try again.\n");
            };

        } // End while
  
        } // End main_menu


void weather_menu(void)
{
    int choice;
    WeatherData weather;

    weather_get_current(&weather);

    printf("DEBUG WEATHER: valid=%d city='%s' temp=%d condition=%d\n",
       weather.valid,
       weather.city,
       weather.temperature,
       weather.condition);

    while (1) {
        printf(
            "\f"
            "_______________________________________________________________________________\n"
            "|                                                                             |\n"
            "|                              WEATHER                                       |\n"
            "|                                                                             |\n"
            "|   1. Current Conditions                                                    |\n"
            "|   2. Extended Forecast                                                     |\n"
            "|   3. Change Location                                                       |\n"
            "|   4. Refresh                                                               |\n"
            "|   5. Return                                                                |\n"
            "|                                                                             |\n"
            "|_____________________________________________________________________________|\n"
            "\n"
            "Choice: "
        );

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n')
                ;
            continue;
        }

        while (getchar() != '\n')
            ;

        switch (choice) {
               
            case 1:
                printf("\nCurrent Conditions\n");
                printf("------------------\n");
                printf("%s, %s\n\n", weather.city, weather.state);

                printf("%s\n", weather.description);
                printf("%d F\n", weather.temperature);
                printf("Feels like %d F\n", weather.feels_like);

                printf("\nHumidity: %d%%\n", weather.humidity);
                printf("Pressure: %d hPa\n", weather.pressure);
                printf("Wind: %.0f mph\n", weather.wind_speed);

                wait_for_enter();
                break;

case 2:
{
    WeatherForecast forecast;

    printf("\nExtended Forecast\n");
    printf("------------------\n");

    if (weather_get_forecast(&forecast)) {

printf("%s, %s\n\n",
       forecast.city,
       forecast.state);

for (int i = 0; i < forecast.count; i++) {

    char weekday[4];
    const char *condition;

    forecast_weekday(forecast.days[i].day,
                     weekday,
                     sizeof(weekday));

    condition = forecast_condition_name(
                    forecast.days[i].condition);

    printf("%s  %-10s %2d / %2d   Rain %2d%%\n",
           weekday,
           condition,
           forecast.days[i].high,
           forecast.days[i].low,
           forecast.days[i].precipitation_chance);
}

    } else {
        printf("Unable to retrieve forecast.\n");
    }

    wait_for_enter();
    break;
}

case 3:
{
    char city[64];
    char state[32];
    char country[8];

    printf("\nChange Location\n");
    printf("---------------\n");

    printf("City: ");
    if (fgets(city, sizeof(city), stdin) == NULL)
        break;

    city[strcspn(city, "\n")] = '\0';

    printf("State: ");
    if (fgets(state, sizeof(state), stdin) == NULL)
        break;

    state[strcspn(state, "\n")] = '\0';

    printf("Country: ");
    if (fgets(country, sizeof(country), stdin) == NULL)
        break;

    country[strcspn(country, "\n")] = '\0';

if (weather_set_location(city, state, country)) {

    if (weather_get_current(&weather)) {
        printf("\nLocation changed to %s, %s, %s.\n",
               city, state, country);
    } else {
        printf("\nLocation changed, but weather could not be retrieved.\n");
    }

} else {
    printf("\nLocation not found. Location unchanged.\n");
}

wait_for_enter();
break;
}

            case 4:
    printf("\nRefreshing weather...\n");

    if (weather_get_current(&weather)) {
        printf("Weather updated.\n");
    } else {
        printf("Unable to retrieve weather.\n");
    }

    wait_for_enter();
    break;

            case 5:
                return;

            default:
                printf("\nInvalid choice.\n");
                wait_for_enter();
                break;
        }
    }
}

    void draw_weather_tile(int row, WeatherCondition condition, int temperature)
    {
        switch (condition) {

            case WEATHER_SUNNY:
                switch (row) {
                    case 0:
                        printf("| |  \\|/ Sunny    |");
                        break;
                    case 1:
                        printf("| | --*--  %3d    |", temperature);
                        break;
                    case 2:
                        printf("| |  /|\\          |");
                        break;
                    case 3:
                        printf("| |  5. Weather   |");
                        break;
                }
                break;

            case WEATHER_CLOUDY:
                switch (row) {
                    case 0:
                        printf("| | ((())) Cloudy |");
                        break;
                    case 1:
                        printf("| | ((()))   %3d  |", temperature);
                        break;
                    case 2:
                        printf("| |               |");
                        break;
                    case 3:
                        printf("| |  5. Weather   |");
                        break;
                }
                break;

            case WEATHER_RAINY:
                switch (row) {
                    case 0:
                        printf("| | ((())) Rainy  |");
                        break;
                    case 1:
                        printf("| |  ////   %3d   |", temperature);
                        break;
                    case 2:
                        printf("| | ////          |");
                        break;
                    case 3:
                        printf("| |  5. Weather   |");
                        break;
                }
                break;

            case WEATHER_SNOWY:
                switch (row) {
                    case 0:
                        printf("| | ((())) Snow   |");
                        break;
                    case 1:
                        printf("| |  * *    %3d   |", temperature);
                        break;
                    case 2:
                        printf("| | * *           |");
                        break;
                    case 3:
                        printf("| |  5. Weather   |");
                        break;
                }
                break;
        }
    }

    void wait_for_enter(void) {
        fd_set input;
        struct timeval timeout;
        int c;

        FD_ZERO(&input);
        FD_SET(STDIN_FILENO, &input);

        timeout.tv_sec=0;
        timeout.tv_usec=0;

        if (select(STDIN_FILENO + 1, &input, NULL, NULL, &timeout) > 0) {
            while ((c = getchar()) != '\n' && c != EOF) {
                // Consume the input until newline or EOF
            }                
        }
        getchar();
        }


 
//    printf("+---------------------------------------------+\n");
//    printf("|   |_   _   _  _|_ |_   _   _                |\n");
//    printf("|   | \\ | \\ / \\  |  | \\ / \\ | \\  PowerTools   |\n");
//    printf("|   | | |   | |  |  | | |_| |      v0.0.1     |\n");
//    printf("|   |_/ |   \\_/  |  | | \\__ |                 |\n");       
//    printf("+---------------------------------------------+\n");
//    printf("|               1. Send Text                  |\n");
//    printf("|               2. Receive Text               |\n");
//    printf("|               3. BASIC                      |\n");
//    printf("|               4. Settings                   |\n");
//    printf("|               5. Exit                       |\n");
//    printf("+---------------------------------------------+\n");
//    printf("Enter your choice: ");
//    printf(
//        "_______________________________________________________________________________\n"
//        "|                    _              _     _                                   |\n"
//        "|                   | |            | |   | |                                  |\n"
//        "|                   | |_   __   _  | |_  | |__    __   __                     |\n"
//        "|                   |   \\ | _\\ / \\ |  _| | __  \\ /-_\\ | _\\                    |\n"
//        "|                   | O | | | | O || |_  | | | | ||__ | |                     |\n"
//        "|                   \\___/ |_|  \\_/ \\___| |_| |_| \\__/ |_|                     |\n"
//        "|/////////////////////////////////////////////////////////////////////////////|\n//"
//        "|///////////////     /////   ///  //////////  //     //     //////////////////|\n"
//        "|///////////////  //  //  //  //  ///    ///  //  /////  //  /////////////////|\n"
//        "|///////////////     ///  //  //  //  //  //  //    ///     //////////////////|\n"
//        "|///////////////  //////  //  //  //  //  //  //  /////  //  /////////////////|\n"
//        "|///////////////  ///////    ////   /////    ///     //  //  /////////////////|\n"
//        "|/////////////////////////////////////////////////////////////////////////////|\n"
//        "|////////////////      /////   /////    ////  /////////    ///////////////////|\n"
//        "|//////////////////  /////  //  ///  //  ///  ///////    /////////////////////|\n"
//        "|//////////////////  /////  //  ///  //  ///  //////////   ///////////////////|\n"
//        "|//////////////////  /////  //  ///  //  ///  //////////   ///////////////////|\n"
//        "|//////////////////  //////    /////    ////      ///    ///////v0.0.1////////|\n"
//        "|/////////////////////////////////////////////////////////////////////////////|\n"
//        "|////////////////////////Press [ENTER} to continue.../////////////////////////|\n"
//        "|/////////////////////////////////////////////////////////////////////////////|\n"
//    );
