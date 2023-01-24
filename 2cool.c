#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#define FAN_PIN 12 // Defined meinen Lüfter Pin

void daemonize() // Erstellt einen Daemon ( Hintergrundprozess) vom Programm
{
    pid_t pid, sid;
    pid = fork(); // Ein neuer Prozess wird erstellt
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    umask(0);       // Standard-Datei-Erstellungsrechte werden deaktiviert, um sicherzustellen, dass der neue Prozess keine Verbidnung zum Terminal hat
    sid = setsid(); // neuer Prozess wird in eine neue Sitzung und in ein neues Prozessgruppen-ID verschoben; trennt den Prozess von seinem Elternprosess
    if (sid < 0)    // prüft ob obrige Zeile geklappt hat
    {
        exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0) // verschiebt den Prozess ins Stammverzeichnis
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
    wiringPiSetup();
    pinMode(FAN_PIN, PWM_OUTPUT); // PWM Funktionalität an Pin freischalten
    pwmSetClock(2);               // PWM Frequenz einstellen
    pwmSetRange(1024);            // PWM Range einstellen

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
            fanSpeed = 256;
        }
        else if (temp >= 40 && temp < 50)
        {
            fanSpeed = 512;
        }
        else
        {
            fanSpeed = 1024;
        }
        pwmWrite(FAN_PIN, fanSpeed);
        delay(1000);
    }
    pwmWrite(FAN_PIN, 0);
    return 0;
}