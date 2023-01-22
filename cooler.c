#include <stdio.h>
#include <bcm2835.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#define FAN_PIN RPI_BPLUS_GPIO_J8_12                     // Defined meinen Lüfter Pin
#define PWM_CHANNEL 0                                    // PWM-Channel
#define PWM_RANGE 1024                                   // PWM-Range

void daemonize()                                         // Erstellt einen Daemon ( Hintergrundprozess) vom Programm
{
    pid_t pid, sid;
    pid = fork();                                        // Ein neuer Prozess wird erstellt
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    umask(0);                                            // Standard-Datei-Erstellungsrechte werden deaktiviert, um sicherzustellen, dass der neue Prozess keine Verbidnung zum Terminal hat
    sid = setsid();                                      // neuer Prozess wird in eine neue Sitzung und in ein neues Prozessgruppen-ID verschoben; trennt den Prozess von seinem Elternprosess
    if (sid < 0)                                         // prüft ob obrige Zeile geklappt hat
    {
        exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0)                                // verschiebt den Prozess ins Stammverzeichnis
    {
        exit(EXIT_FAILURE);
    }
    // trennt den Prozess von seinen Datenkanälen und macht in unabhängig von I/O auf diesen Kanälen
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(void)
{
    float temp;
    int fanSpeed;
    daemonize();
    if (!bcm2835_init())
    {
        return 1;
    }
    bcm2835_gpio_fsel(FAN_PIN, BCM2835_GPIO_FSEL_ALT5);  // PWM Funktionalität an Pin freischalten
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16); // PWM Frequenz einstellen
    bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);             // PWM Modus einstellen und starten
    bcm2835_pwm_set_range(PWM_CHANNEL, PWM_RANGE);       // PWM Range einstellen

    while (1)
    {
        // Lesen der CPU-Temperatur
        FILE *temperatureFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
        fscanf(temperatureFile, "%f", &temp);
        fclose(temperatureFile);
        temp /= 1000;
        // Einstellen der Lüftergeschwindigkeit abhängig von der CPU-Temperatur
        if (temp < 30)
        {
            fanSpeed = 0;
        }
        else if (temp >= 30 && temp < 40)
        {
            fanSpeed = PWM_RANGE / 3;
        }
        else if (temp >= 40 && temp < 50)
        {
            fanSpeed = (2 * PWM_RANGE) / 3;
        }
        else
        {
            fanSpeed = PWM_RANGE;
        }
        bcm2835_pwm_set_data(FAN_PIN, fanSpeed);
        sleep(1);
    }
    bcm2835_pwm_set_data(0, 0);                          //Setzt PVM Daten auf 0
    bcm2835_pwm_set_mode(0, 0, 0);                       // PWM aus
	if(bcm2835_close()==0) return 1;					 // Library schließen
	return 0;	
}