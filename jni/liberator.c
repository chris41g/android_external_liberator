/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/sockets.h>
#include <cutils/logd.h>
#include <android/log.h>

#define CONFIG_ROOT "/system/etc/liberator/"

#define SYS_SCHED "/sys/class/block/mmcblk0/queue/scheduler"
#define SYS_CGOV_C0 "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define SYS_CMAX_C0 "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define SYS_CMIN_C0 "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"

#define SYS_ONLI_C1 "/sys/devices/system/cpu/cpu1/online"
#define SYS_CGOV_C1 "/sys/devices/system/cpu/cpu1/cpufreq/scaling_governor"
#define SYS_CMAX_C1 "/sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq"
#define SYS_CMIN_C1 "/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq"

#define SYS_WAKE "/sys/power/wait_for_fb_status"
#define SYS_CHARGE "/sys/class/power_supply/battery/status"
#define SYS_BATT "/sys/class/power_supply/battery/capacity"

#define APPNAME "Liberator"

typedef struct s_ocConfig
{
    char default_min_freq[30];
    char default_max_freq[30];
    char default_governor[30];
    char default_scheduler[30];

    char soff_min_freq[30];
    char soff_max_freq[30];
    char soff_governor[30];
    char soff_scheduler[30];

    char charge_min_freq[30];
    char charge_max_freq[30];
    char charge_governor[30];
    char charge_scheduler[30];

    char lowb_level[30];
    char lowb_min_freq[30];
    char lowb_max_freq[30];
    char lowb_governor[30];
    char lowb_scheduler[30];

} ocConfig;

void my_trim(char *str)
{
    int i;
    for (i = 0; str[i] != 0; i++)
        if ((str[i] == '\n' || str[i] == '\r'))
            str[i] = 0;
}

int write_to_file(char *path, char *value)
{
    FILE  *fd;
    int   res = 0;

    fd = fopen(path, "w");
    if (fd == NULL)
        return -1;
    if (fputs(value, fd) < 0)
        res = -1;
    fclose(fd);
    return res;
}

int read_from_file(char *path, int len, char *result)
{
    FILE *fd;
    int res = 0;
    fd = fopen(path, "r");
    if (fd == NULL)
    {
//	__android_log_print(ANDROID_LOG_ERROR, APPNAME, "Oopsie!");
        return -1;
    }
    if (fgets(result, len, fd) == NULL)
    {
//	__android_log_print(ANDROID_LOG_ERROR, APPNAME, "Oopsie 2!");
        res = -1;
    }
    fclose(fd);
//    __android_log_print(ANDROID_LOG_INFO, APPNAME, "result=%s, len=%i", result, len);
    return res;
}

int set_cpu_params(char *governor, char *scheduler, char *min_freq, char *max_freq)
{
    if (write_to_file(SYS_CGOV_C0, governor) != 0)
        return -1;
    if (write_to_file(SYS_SCHED, scheduler) != 0)
        return -1;
    if (write_to_file(SYS_CMAX_C0, max_freq) != 0)
        return -1;
    if (write_to_file(SYS_CMIN_C0, min_freq) != 0)
        return -1;

    write_to_file(SYS_CGOV_C1, governor);
    write_to_file(SYS_SCHED, scheduler);
    write_to_file(SYS_CMAX_C1, max_freq);
    write_to_file(SYS_CMIN_C1, min_freq);

    __android_log_print(ANDROID_LOG_INFO, APPNAME, "Setting Params: Governor=%s scheduler=%s min_freq=%s max_freq=%s", governor, scheduler, min_freq, max_freq);
    return 0;
}

int get_config_value(char *config_key, char *reference)
{
    char config_path[60];

    strcpy(config_path, CONFIG_ROOT);
    strcat(config_path, config_key);
//    __android_log_print(ANDROID_LOG_INFO, APPNAME, "config_path=%s", config_path);
    return read_from_file(config_path, 30, reference);
}

int  load_config(ocConfig *conf)
{
char config_path[60];
char lowbunf[30];

    if (conf == NULL)
    {
//	__android_log_print(ANDROID_LOG_ERROR, APPNAME, "Meltdown!");
        return -1;
    }
    if (get_config_value("default_min_freq", conf->default_min_freq) == -1)
    {
//	__android_log_print(ANDROID_LOG_ERROR, APPNAME, "Cant get default profile min_freq == %s", conf->default_min_freq);
        return -1;
    }
    if (get_config_value("default_max_freq", conf->default_max_freq) == -1)
    {
//	__android_log_print(ANDROID_LOG_ERROR, APPNAME, "Cant get default profile max_freq");
        return -1;
    }
    if (get_config_value("default_governor", conf->default_governor) == -1)
    {
//	__android_log_print(ANDROID_LOG_ERROR, APPNAME, "Cant get default profile governor");
        return -1;
    }
    if (get_config_value("default_scheduler", conf->default_scheduler) == -1)
    {
//	__android_log_print(ANDROID_LOG_ERROR, APPNAME, "Cant get default profile scheduler");
        return -1;
    }
    if (get_config_value("soff_min_freq", conf->soff_min_freq) == -1)
	get_config_value("default_min_freq", conf->soff_min_freq);
    if (get_config_value("soff_max_freq", conf->soff_max_freq) == -1)
	get_config_value("default_max_freq", conf->soff_max_freq);
    if (get_config_value("soff_governor", conf->soff_governor) == -1)
	get_config_value("default_governor", conf->soff_governor);
    if (get_config_value("soff_scheduler", conf->soff_scheduler) == -1)
	get_config_value("default_scheduler", conf->soff_scheduler);
    if (get_config_value("charge_min_freq", conf->charge_min_freq) == -1)
	get_config_value("default_min_freq", conf->charge_min_freq);
    if (get_config_value("charge_max_freq", conf->charge_max_freq) == -1)
	get_config_value("default_max_freq", conf->charge_max_freq);
    if (get_config_value("charge_governor", conf->charge_governor) == -1)
	get_config_value("default_governor", conf->charge_governor);
    if (get_config_value("charge_scheduler", conf->charge_scheduler) == -1)
	get_config_value("default_scheduler", conf->charge_scheduler);
    if (get_config_value("lowb_level", conf->lowb_level) == -1)
        __android_log_print(ANDROID_LOG_ERROR, APPNAME, "No battery level file");
    if (get_config_value("lowb_min_freq", conf->lowb_min_freq) == -1)
	get_config_value("default_min_freq", conf->lowb_min_freq);
    if (get_config_value("lowb_max_freq", conf->lowb_max_freq) == -1)
	get_config_value("default_max_freq", conf->lowb_max_freq);
    if (get_config_value("lowb_governor", conf->lowb_governor) == -1)
	get_config_value("default_governor", conf->lowb_governor);
    if (get_config_value("lowb_scheduler", conf->lowb_scheduler) == -1)
	get_config_value("default_scheduler", conf->lowb_scheduler);
    return 0;
}

int wait_for_cpu1_online()
{
    struct stat file_info;

    int i=0;
    while (0 != stat(SYS_CMAX_C1, &file_info) && i < 20)
    {
        usleep(50000);
        i++;
    }
    if (i == 20)
        return 1;

    return 0;
}

int set_cpu1_online(int online)
{
    if (online)
    {
        write_to_file(SYS_ONLI_C1, "1");

        if (0 != wait_for_cpu1_online())
            return 1;
        else
            return 0;
    }
    else
    {
        write_to_file(SYS_ONLI_C1, "0");
        return 0;
    }
}

int main (int argc, char **argv)
{
    ocConfig  conf;
    pid_t pid, sid;
    char awake_buffer[9];
    char charge_buffer[15];
    char batt_buffer[3];

    __android_log_write(ANDROID_LOG_INFO, APPNAME, "Starting 4Ace daemon.");

    if (load_config(&conf) == -1)
    {
        __android_log_write(ANDROID_LOG_ERROR, APPNAME, "Unable to load configuration. Stopping.");
        return 1;
    }

    pid = fork();
    if (pid < 0)
        exit(2);
    if (pid > 0)
        exit(0);
    umask(0);

    sid = setsid();
    if (sid < 0)
        exit(2);
    if ((chdir("/")) < 0)
        exit(2);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    while (1)
    {
        if (read_from_file(SYS_WAKE, 6, awake_buffer) == -1)
        {
        	__android_log_write(ANDROID_LOG_ERROR, APPNAME, "Unable to get data from file. Cannot continue.");
        	return 1;
        }
	if (read_from_file(SYS_CHARGE, 15, charge_buffer) == -1)
	{
		 __android_log_write(ANDROID_LOG_ERROR, APPNAME, "Unable to get data from file. Cannot continue.");
		return 1;
	}
	if (read_from_file(SYS_BATT, 30, batt_buffer) == -1)
        {
        	__android_log_write(ANDROID_LOG_ERROR, APPNAME, "Unable to get data from file. Cannot continue.");
        	return 1;
        }
//	__android_log_print(ANDROID_LOG_INFO, APPNAME, "awake_buffer==%s charge_buffer=%s  batt_buffer=%s", awake_buffer, charge_buffer, batt_buffer);
        if (strncmp(awake_buffer, "on",3) == 0)
        {

        int lowblvl = (int)conf.lowb_level;
	int battlvl = (int)batt_buffer;

		if (strncmp(charge_buffer, "Charging",8) == 0)
		{
			__android_log_write(ANDROID_LOG_INFO, APPNAME, "Setting Charging profile.");

			if (0 !=set_cpu1_online(1))
			__android_log_write(ANDROID_LOG_INFO, APPNAME, "Failed setting Charging profile for cpu1.");

			set_cpu_params(conf.charge_governor, conf.charge_scheduler, conf.charge_min_freq, conf.charge_max_freq);
		}
		else 
		{
			if (battlvl < lowblvl)
			{
				__android_log_write(ANDROID_LOG_INFO, APPNAME, "Setting Low Battery profile.");

				if (0 !=set_cpu1_online(1))
				__android_log_write(ANDROID_LOG_INFO, APPNAME, "Failed setting Low Battery profile for cpu1.");

				set_cpu_params(conf.lowb_governor, conf.lowb_scheduler, conf.lowb_min_freq, conf.lowb_max_freq);
			}
			else
			{
				__android_log_write(ANDROID_LOG_INFO, APPNAME, "Setting Normal profile.");

				if (0 !=set_cpu1_online(1))
				__android_log_write(ANDROID_LOG_INFO, APPNAME, "Failed setting Normal profile for cpu1.");

				set_cpu_params(conf.default_governor, conf.default_scheduler, conf.default_min_freq, conf.default_max_freq);
			}

		}
        }
	else
	{	
	    __android_log_write(ANDROID_LOG_INFO, APPNAME, "Setting sleep profile.");

            set_cpu1_online(0);
            set_cpu_params(conf.soff_governor, conf.soff_scheduler, conf.soff_min_freq, conf.soff_max_freq);
	}
	
	awake_buffer[0] = '\0';
	charge_buffer[0] = '\0';
	batt_buffer[0] = '\0';

    }

    return 0;
}
